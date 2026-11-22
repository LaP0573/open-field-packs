// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K, pack K5: the chunks of tracks 5 to 8 lie at the top of
// the tape, and each has a sum, so that what another firmware recorded over is heard as silence and not as a
// piece of something else.
//
// Why. Stock firmware knows four tracks. Its allocator (0x8fc22838) walks the reference bytes upwards from a
// cursor that every tape load puts back to 0 and takes the first chunk none of its own four tables holds, so
// under stock firmware the chunks of tracks 5 to 8 are free, and the lowest of them are the first it takes. Until
// K5 the module took the chunks of tracks 5 to 8 from that same allocator: they lay low on the tape, among the
// chunks of tracks 1 to 4. K1's load drops an entry whose chunk stock's file holds. A chunk stock took, recorded
// on and gave back (an undo, a lift) is free again in stock's file and no longer our sound: nothing told it apart.
//
// Two things, both here:
//
//   * op1f_sums_alloc: a chunk for a track of 5 to 8 is the highest free one on the tape (chunk 8191 downwards, on
//     the same reference bytes, with the live record's bit set as stock's own set_ref_bit 0x8fc20934 sets it).
//     Stock's cursor is not touched. Stock and the module then meet only on a tape that is nearly full. Chunks
//     recorded before K5 stay where they are.
//
//   * a sum for every chunk of tracks 5 to 8, kept by physical chunk (so that the chunks older undo records hold
//     keep theirs), taken at every whole write, looked at when a whole chunk is read, and saved with the tape's
//     file of tracks 5 to 8 as a third section (Build K's maps unit).
//
// When is a chunk "not what was written"? A sum on the disk can be behind the chunk it stands for without anybody
// else having written the tape: the unit went off between a chunk's write and the next save of the file. So a
// difference alone proves nothing, and the rule is biased the safe way: sound is given up only when another
// firmware is known to have written the tape since this pack last saved it.
//
//   * That is known from the files' times: this pack writes the file of tracks 5 to 8 so that it is never older
//     than stock's map file (Build K's maps unit writes it once more when the clock has moved between the two), and
//     any save of the map by another firmware makes the map file the newer one, also when the map's content comes
//     out the same (a take and its undo). K1's own sign, stock's file changed, counts too.
//   * After such a load every chunk that has a sum is HELD to it: read whole, summed, and when the sum differs the
//     chunk is not ours any more (op1f_sums_read answers 0: the caller plays silence, lets the entry go, and the
//     chunk is never read as sound or written into in place again). A held chunk that has not been read yet is
//     saved as held, so that it is still held after the next load, when the files' times no longer say anything.
//   * Without such a load a difference means our own write outran the save: the chunk is kept and its sum taken
//     anew ("changed and kept" on the log; in ordinary use the count stays 0, and a count that does not is a
//     write path this unit was not told of).
//   * A chunk with no sum (a file of before K5, a part of the chunk written in place) gets its sum at its next
//     whole read.
//
// So sound can be given up wrongly only when a sum was left behind by a unit that went off inside the save's
// window, the chunk was never read again under this pack, and another firmware then wrote the tape: two faults.
// And a chunk another firmware recorded over is missed (and played) when it had no sum, or when that firmware
// never saved the map after taking it.
//
// The sum is 32 bits over the chunk's 32,768 words, a rotate and an add a word: enough to tell one recording from
// another; it is no defence against anybody.
//
// Threads. The service loop reads and writes whole chunks (build_b_storage.cpp: ext_read, ext_write); the tape's
// command thread writes ranges (op1f_tape_io_write_ext); a load and the frame hook run on the main and the GUI
// thread. A state is one byte and a sum one word; a save that catches a chunk between its two stores writes a
// sum that the next save, which the same change asks for, puts right.
// Nothing here runs on the audio thread. Freestanding: no library, no division.

#include "build_k_sums.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*VoidFn)();
typedef int (*IntFn)();

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MARK_MAP_DIRTY ((VoidFn)0x8fc22064u)                // sets the map's dirty byte: the idle flush saves the map
#define STOCK_MOUNTED_TAPE ((IntFn)0x8fc219a8u)                   // the mounted tape's index, -1 for none
#define STOCK_REFERENCES ((volatile u8*)0x84c94208u)              // one byte a physical chunk: bit n set, record n holds it
#define STOCK_CURRENT_SNAPSHOT (*(volatile i32*)0x848cff90u)      // the live record's index

enum {
    CHUNKS = 8192,                               // a tape partition
    CHUNK_WORDS = 0x20000 / 4,
    ENTRIES = 4 * 969,                           // the tables of tracks 5 to 8
    UNALLOCATED = 0xffff,
    PENDING_FRAMES = 600,                        // some ten seconds between a sum learnt by reading and its save
    LOAD_LINE_FRAMES = 60,
    ACTIVITY_FRAMES = 240,
    LOG_BUDGET = 40,
};

enum {                                           // a physical chunk, as far as tracks 5 to 8 know it
    S_NONE = 0,                                  // no sum
    S_SOFT = 1,                                  // a sum from the file; a difference at the next read is our own write
    S_HELD = 2,                                  // a sum from the file, and another firmware wrote the tape since
    S_SURE = 3,                                  // the sum is what this session wrote or read
    S_BAD = 4,                                   // held, read, and not what was written
};

enum {                                           // op1f_sums_counts
    C_AS_WRITTEN = 0, C_GIVEN, C_CHANGED_KEPT, C_NOT_OURS, C_WRITTEN, C_PART, C_FROM_TOP, C_NONE_FREE, C_BAD_AGAIN, C_FORWARDS, C_LOADS, C_COLLECTS, C_COUNTS
};

static const char LOG_FILE[] = "../src/dsp/op1f_tape_sums.cpp";

extern "C" u32 op1f_sums_counts[C_COUNTS] asm("op1f_sums_counts");
// read as written, given a sum, changed and kept, not ours, whole writes, parts written, chunks from the top, no
// chunk free, reads of a chunk known as not ours, saves asked for sums learnt by reading, loads, collects
u32 op1f_sums_counts[C_COUNTS];

static u32 sum_of[CHUNKS];
static u8 state_of[CHUNKS];
static int loaded_tape = -1;
static u32 pending, pending_frames;              // sums learnt by reading, not yet on their way to the file
static u32 lowest = CHUNKS;                      // the lowest chunk taken from the top since the load
static u32 load_with, load_held, load_none, load_foreign;
static u32 load_frames, load_line_due;
static u32 activity_seen, activity_frames;
static u32 log_budget = LOG_BUDGET;

static u32 chunk_sum(const u8* chunk) {
    const u32* w = (const u32*)chunk;
    u32 h = 0x4f503146u;
    for (u32 i = 0; i < CHUNK_WORDS; ++i) h = ((h << 1) | (h >> 31)) + w[i];
    return h;
}

static int aligned(const u8* chunk) { return ((u32)chunk & 3) == 0; }

u32 op1f_sums_alloc() {
    volatile u8* references = STOCK_REFERENCES;
    for (i32 chunk = CHUNKS - 1; chunk >= 0; --chunk) {
        if (references[chunk]) continue;
        references[chunk] = (u8)(1u << STOCK_CURRENT_SNAPSHOT);
        state_of[chunk] = S_NONE;
        ++op1f_sums_counts[C_FROM_TOP];
        if ((u32)chunk < lowest) lowest = (u32)chunk;
        return (u32)chunk;
    }
    ++op1f_sums_counts[C_NONE_FREE];
    return UNALLOCATED;
}

int op1f_sums_wrote(u32 physical, const u8* chunk) {
    if (physical >= CHUNKS) return 0;
    ++op1f_sums_counts[C_WRITTEN];
    if (!aligned(chunk)) {                       // not to be summed where it stands: the next whole read gives the sum
        int known = state_of[physical] != S_NONE;
        state_of[physical] = S_NONE;
        return known;
    }
    u32 sum = chunk_sum(chunk);
    int changed = state_of[physical] != S_SURE || sum_of[physical] != sum;
    sum_of[physical] = sum;
    state_of[physical] = S_SURE;
    return changed;
}

int op1f_sums_part(u32 physical) {
    if (physical >= CHUNKS) return 0;
    ++op1f_sums_counts[C_PART];
    if (state_of[physical] == S_NONE) return 0;
    state_of[physical] = S_NONE;
    return 1;
}

int op1f_sums_ours(u32 physical) {
    if (physical >= CHUNKS) return 1;
    u8 state = state_of[physical];
    return state != S_HELD && state != S_BAD;
}

int op1f_sums_read(u32 physical, const u8* chunk) {
    if (physical >= CHUNKS) return 1;
    u8 state = state_of[physical];
    if (state == S_SURE) return 1;
    if (state == S_BAD) {
        ++op1f_sums_counts[C_BAD_AGAIN];
        return 0;
    }
    if (!aligned(chunk)) return 1;               // not to be summed where it stands: left as it is
    u32 sum = chunk_sum(chunk);
    if (state == S_NONE) {
        sum_of[physical] = sum;
        state_of[physical] = S_SURE;
        ++op1f_sums_counts[C_GIVEN];
        pending = 1;
        return 1;
    }
    if (sum == sum_of[physical]) {
        state_of[physical] = S_SURE;
        ++op1f_sums_counts[C_AS_WRITTEN];
        if (state == S_HELD) pending = 1;        // no longer held: the file should say so
        return 1;
    }
    if (state == S_SOFT) {                       // nobody else wrote the tape: our own write outran the save
        sum_of[physical] = sum;
        state_of[physical] = S_SURE;
        ++op1f_sums_counts[C_CHANGED_KEPT];
        pending = 1;
        return 1;
    }
    state_of[physical] = S_BAD;                  // held, and not what was written
    ++op1f_sums_counts[C_NOT_OURS];
    return 0;
}

void op1f_sums_reset() {
    for (u32 chunk = 0; chunk < CHUNKS; ++chunk) state_of[chunk] = S_NONE;
    pending = 0;
    pending_frames = 0;
    lowest = CHUNKS;
}

void op1f_sums_loaded(int tape, const u16* ext, const u32* sums, const u8* flags, int foreign) {
    op1f_sums_reset();
    loaded_tape = tape;
    load_with = 0;
    load_held = 0;
    load_none = 0;
    load_foreign = foreign ? 1 : 0;
    ++op1f_sums_counts[C_LOADS];
    if (ext) {
        for (u32 i = 0; i < ENTRIES; ++i) {
            u32 chunk = ext[i];
            if (chunk == UNALLOCATED || chunk >= CHUNKS) continue;
            u8 flag = (sums && flags) ? flags[i] : (u8)XSUM_NONE;
            if (flag == XSUM_SURE || flag == XSUM_HELD) {
                sum_of[chunk] = sums[i];
                if (foreign || flag == XSUM_HELD) {
                    state_of[chunk] = S_HELD;
                    ++load_held;
                } else {
                    state_of[chunk] = S_SOFT;
                }
                ++load_with;
            } else {
                ++load_none;
            }
        }
    }
    load_frames = 0;
    load_line_due = 1;
}

int op1f_sums_collect(int tape, const u16* ext, u32* sums, u8* flags) {
    if (tape < 0 || tape != loaded_tape) return 0;
    ++op1f_sums_counts[C_COLLECTS];
    for (u32 i = 0; i < ENTRIES; ++i) {
        u32 chunk = ext[i];
        u32 sum = 0;
        u8 flag = XSUM_NONE;
        if (chunk != UNALLOCATED && chunk < CHUNKS) {
            u8 state = state_of[chunk];
            if (state == S_SURE || state == S_SOFT) {
                flag = XSUM_SURE;
                sum = sum_of[chunk];
            } else if (state == S_HELD) {
                flag = XSUM_HELD;
                sum = sum_of[chunk];
            }
        }
        sums[i] = sum;
        flags[i] = flag;
    }
    return 1;
}

void op1f_sums_frame() {
    if (pending) {
        if (++pending_frames >= PENDING_FRAMES) {
            pending_frames = 0;
            pending = 0;
            if (loaded_tape >= 0 && STOCK_MOUNTED_TAPE() == loaded_tape) {
                STOCK_MARK_MAP_DIRTY();
                ++op1f_sums_counts[C_FORWARDS];
            }
        }
    } else {
        pending_frames = 0;
    }
    if (load_line_due && ++load_frames >= LOAD_LINE_FRAMES) {
        load_line_due = 0;
        if (log_budget && (load_with || load_none || load_foreign)) {
            --log_budget;
            STOCK_LOG(load_foreign && load_held ? 2 : 3, LOG_FILE, 1, "ext sums: tape %d: %d chunk(s) of tracks 5-8 with a sum, %d of them held to it, %d with none; %s",
                      loaded_tape + 1, load_with, load_held, load_none,
                      load_foreign ? "the map was saved after the sums were (another firmware, or an older pack)" : "nothing else wrote the tape");
        }
    }
    u32 activity = op1f_sums_counts[C_AS_WRITTEN] + op1f_sums_counts[C_GIVEN] + op1f_sums_counts[C_CHANGED_KEPT] + op1f_sums_counts[C_NOT_OURS] +
                   op1f_sums_counts[C_WRITTEN] + op1f_sums_counts[C_PART] + op1f_sums_counts[C_FROM_TOP] + op1f_sums_counts[C_NONE_FREE];
    if (++activity_frames >= ACTIVITY_FRAMES && activity != activity_seen) {
        activity_frames = 0;
        activity_seen = activity;
        if (log_budget) {
            --log_budget;
            STOCK_LOG(op1f_sums_counts[C_NOT_OURS] || op1f_sums_counts[C_CHANGED_KEPT] ? 2 : 3, LOG_FILE, 2,
                      "ext sums: %d read as written, %d given a sum, %d changed and kept, %d not ours (silence); %d written, %d part-written; %d taken from the top, the lowest %d",
                      op1f_sums_counts[C_AS_WRITTEN], op1f_sums_counts[C_GIVEN], op1f_sums_counts[C_CHANGED_KEPT], op1f_sums_counts[C_NOT_OURS],
                      op1f_sums_counts[C_WRITTEN], op1f_sums_counts[C_PART], op1f_sums_counts[C_FROM_TOP], lowest < CHUNKS ? (int)lowest : -1);
        }
    }
}
