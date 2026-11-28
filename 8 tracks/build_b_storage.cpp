// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B0b: storage plumbing for tape tracks 5 and 6 (track indices 4 and 5).
//
// Replaces four stock functions of tape_snapshots.cpp / tape_chunks.cpp (OS 1.7.0+0) through a JUMP.a at each
// entry (profile group "build-b0b"), and supersedes Build A's loader's op1f_load_block_map (group
// "build-a") with a version that also fills the two extension maps:
//
//   chunks_service    0x8fc217a0  -> op1f_chunks_service    reload and write back six tracks per cache slot
//   snapshots_reset   0x8fc224fc  -> op1f_snapshots_reset   stock work, plus clearing the extension maps
//   save_block_map    0x8fc22d70  -> op1f_save_block_map    version 1 as stock; version 2 when tracks 5-6 hold chunks
//                                                            (until K1; since K1 always stock's version 1, the tables of
//                                                            tracks 5-8 in user/tapeNN_ext.bin: Build K's maps unit)
//   take_snapshot_ex  0x8fc23128  -> op1f_take_snapshot_ex  stock's take with the extension records carried along (Build C;
//                                                            a no-op from B0b to B3f, undo off)
//   diff_range        0x8fc22c84  -> op1f_diff_range        the changed range over six tracks (Build C), so undo_ex and
//                                                            redo_ex reload the cache for extension-track changes too
//   load_block_map    0x8fc22eb0  -> op1f_load_block_map    Build A's loader; tracks 5-6 of a version-2 file fill the extension maps
//
// Tracks 0-3 keep stock's four-track record, stock's tape_io_read/tape_io_write and stock's allocator, untouched.
// Tracks 4-5 live in op1f_ext_records (2 x 969 physical chunk numbers per snapshot record, 0xFFFF = unallocated;
// the record at CURRENT_SNAPSHOT is live and stock's undo and redo move that index), reach the partition
// through stock's raw partition read/write (whole 128 KiB chunks only, which is all chunks_service ever moves),
// and take their chunks from stock's alloc_chunk, so the reference bytes remain the single record of what is
// free. Freestanding like the other units: no library, no division,
// no memset/memcpy of our own (the ROM memset stock uses is called by address).

#include "build_k_maps.h"       // K1: the block map in two files, stock's own and the extension file
#include "build_k_sums.h"       // K5: the chunks of tracks 5 to 8 from the top of the tape, and a sum for each

typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;
typedef int i32;

typedef void* (*FopenFn)(const char* path, const char* mode);
typedef i32 (*FreadFn)(void* destination, i32 size, i32 count, void* file);
typedef i32 (*FwriteFn)(const void* source, i32 size, i32 count, void* file);
typedef void (*FcloseFn)(void* file);
typedef void (*RenameFn)(const char* old_path, const char* new_path);
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*MemsetFn)(void* destination, int value, u32 count);
typedef i32 (*IntFn)();
typedef i32 (*PartitionIoFn)(i32 handle, u32 byte_offset, u32 length, void* buffer);   // negative on error
typedef i32 (*TapeIoFn)(u32 track, u32 byte_offset, u32 length, void* buffer);         // bytes done, or -1
typedef u32 (*AllocChunkFn)();
typedef void (*VoidFn)();
typedef void (*SetSnapshotFn)(i32 index);
typedef i32 (*SnapshotStepFn)();
typedef void* (*MemmoveFn)(void* destination, const void* source, u32 count);

#define STOCK_FOPEN ((FopenFn)0x8fc22180u)                        // disk_io.h fopen(path, "rb"/"w+b"/...)
#define STOCK_FREAD ((FreadFn)0x8fc2214cu)                        // returns the item count
#define STOCK_FWRITE ((FwriteFn)0x8fc220b8u)                      // logs "write error" itself
#define STOCK_FCLOSE ((FcloseFn)0x8fc220a0u)
#define STOCK_RENAME ((RenameFn)0x8fc222f4u)
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MEMSET ((MemsetFn)0x04022ea8u)                      // ROM memset, as stock's tape_io_read uses
#define STOCK_TAPE_INDEX ((IntFn)0x8fc219a8u)                     // get_current_tape_index(), -1 = none
#define STOCK_TAPE_HANDLE ((IntFn)0x8fc21994u)                    // get_tape_file_handle(), -1 = none
#define STOCK_PARTITION_READ ((PartitionIoFn)0x8d500688u)         // as called at 0x8fc2018e
#define STOCK_PARTITION_WRITE ((PartitionIoFn)0x8d500b88u)        // as called at 0x8fc204d4
#define STOCK_TAPE_IO_READ ((TapeIoFn)0x8fc21bacu)                // tracks 0-3, unchanged stock
#define STOCK_TAPE_IO_WRITE ((TapeIoFn)0x8fc21d1cu)
#define STOCK_ALLOC_CHUNK ((AllocChunkFn)0x8fc22838u)             // next-fit over the reference bytes, sets the live bit; tracks 5 to 8 took their chunks here until K5 (now op1f_sums_alloc, from the top)
#define STOCK_MARK_MAP_DIRTY ((VoidFn)0x8fc22064u)
#define STOCK_SET_CURRENT_SNAPSHOT ((SetSnapshotFn)0x8fc22498u)
#define STOCK_TAKE_SNAPSHOT ((SnapshotStepFn)0x8fc22af0u)           // Build C: stock's take over its own records and all reference bytes
#define STOCK_DROP_OLDEST ((SnapshotStepFn)0x8fc22a14u)             // 0 when the live record is record 0
#define STOCK_COUNT_FREE ((SnapshotStepFn)0x8fc22804u)              // reference bytes at zero
#define STOCK_MEMMOVE ((MemmoveFn)0x04020218u)                       // ROM, as drop_oldest_snapshot shifts its records

#define CURRENT_SNAPSHOT (*(volatile i32*)0x848cff90u)
#define SNAPSHOT_COUNT (*(volatile i32*)0x848cff94u)
#define ALLOC_CURSOR (*(volatile i32*)0x848cff98u)
#define MAP_DIRTY (*(volatile unsigned char*)0x848cff9cu)
#define RECORDING (*(volatile unsigned char*)0x848ce538u)           // the L1 record function's "recording" byte
#define RECORDS ((unsigned char*)0x84c68d88u)                      // 8 records x 22,160 bytes
#define REFERENCES ((volatile unsigned char*)0x84c94208u)           // one byte per physical chunk
#define LRU_HEAD (*(Slot* volatile*)0x848ced58u)

struct Slot {                 // tape_chunks.cpp slot descriptor, 28 bytes
    unsigned char valid;
    unsigned char stale;
    unsigned char write_pending;
    unsigned char pad;
    u16 dirty_tracks;
    i16 logical_chunk;
    u16 index;
    u16 pad2;
    i32 frame_pos;
    unsigned char* data;
    Slot* next;
    Slot* prev;
};

enum {
    TAPES = 8,
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                    // Build F: tracks 4-7 live in the extension maps (6 from B0b to E5)
    EXT_TRACKS = EIGHT_TRACKS - STOCK_TRACKS,
    MAX_SNAPSHOTS = 8,
    CHUNKS_PER_TRACK = 969,
    CHUNK_FRAMES = 16384,
    CHUNK_BYTES = 0x20000,
    TRACK_MAP_BYTES = 1938,
    MAP_BYTES = STOCK_TRACKS * TRACK_MAP_BYTES,      // 7,752
    RECORD_BYTES = 22160,
    RECORD_TAG_OFFSET = 22156,           // int32 tag at the end of a record (take_snapshot_ex stores it)
    TAPE_FRAMES = 0x00f23fa0,            // 15,876,000: diff_range's initial minimum
    CHUNKS_PER_PARTITION = 8192,
    UNALLOCATED = 0xffff,
    MAGIC = 0x424d,
    MAX_FILE_TRACKS = 16,
    EIGHT_TRACK_XFADE_MASK = 0xff,       // B3b: the extension tracks' bits too (bits 4-7 since F2, build_b_xfade.cpp)
    LOG_BUDGET = 8,
    FREE_COUNT_EVERY = 64                // service calls between two counts of the free chunks (about a second)
};
static const char MAP_LOG_FILE[] = "../src/dsp/op1f_tape_map.cpp";       // the loader keeps Build A's tag
static const char STORE_LOG_FILE[] = "../src/dsp/op1f_tape_store.cpp";

// From 8 tracks/build_b_cache.cpp (step B0a): 6 with the heap-1 cache, 4 after its fallback. A slot then has no
// room for tracks 4-5, so the service loop must not touch them.
extern "C" u32 op1f_cache_tracks asm("op1f_cache_tracks");
extern "C" void op1f_xfade_mark_loaded_range(i32 start, i32 end, i32 loop_a, i32 loop_b, u32 track_mask) asm("op1f_xfade_mark_loaded_range");   // B3b (build_b_xfade.cpp): six-track version of 0x8fc2337c

// The extension maps, one per snapshot record as stock keeps its own (Build C; B0b to B3f had one live map and
// took no snapshots). The record at CURRENT_SNAPSHOT is live; stock's undo and redo move that index, so the
// live extension map follows without a hook. Read by nothing in stock.
extern "C" u16 op1f_ext_records[MAX_SNAPSHOTS][EXT_TRACKS][CHUNKS_PER_TRACK] asm("op1f_ext_records");
extern "C" u32 op1f_snapshots_taken asm("op1f_snapshots_taken");
extern "C" u32 op1f_snapshots_dropped asm("op1f_snapshots_dropped");
u16 op1f_ext_records[MAX_SNAPSHOTS][EXT_TRACKS][CHUNKS_PER_TRACK];
u32 op1f_snapshots_taken;
u32 op1f_snapshots_dropped;
#define EXT_MAP(t) (op1f_ext_records[CURRENT_SNAPSHOT][(t)])
static u32 take_log_budget = LOG_BUDGET;

static u32 ext_read_log_budget = LOG_BUDGET;
static u32 ext_write_log_budget = LOG_BUDGET;
static u32 ext_sum_log_budget = 2 * LOG_BUDGET;   // K5: a chunk that is not what was written
extern "C" void op1f_ext_unmap_chunk(u32 track, u32 logical) asm("op1f_ext_unmap_chunk");   // defined below; ext_read lets an entry go (K5)
static u32 quiet_saves;                       // version-2 saves made while recording, not logged yet (B2)
static u32 quiet_tape, quiet_allocated[EXT_TRACKS];   // what the last quiet save wrote

// Build F, image F1: the eMMC measurement and the free-chunk count, read by the first frame hook's measuring unit (weak references)
// and by the help screen's tape gauge (build_e_screens.cpp). Counted on the tape thread; the frame hook reads.
extern "C" u32 op1f_io_reloads asm("op1f_io_reloads");                 // slots reloaded (every track of the slot)
extern "C" u32 op1f_io_writebacks asm("op1f_io_writebacks");           // slots written back (their dirty tracks)
extern "C" u32 op1f_io_read_chunks asm("op1f_io_read_chunks");         // allocated chunks read from the eMMC
extern "C" u32 op1f_io_read_cycles asm("op1f_io_read_cycles");         // CYCLES spent in those reads (zero fills excluded)
extern "C" u32 op1f_io_reload_cycles asm("op1f_io_reload_cycles");     // CYCLES spent in whole slot reloads, zero fills included
extern "C" u32 op1f_tape_free_chunks asm("op1f_tape_free_chunks");     // the live tape's free chunks, refreshed about once a second
u32 op1f_io_reloads;
u32 op1f_io_writebacks;
u32 op1f_io_read_chunks;
u32 op1f_io_read_cycles;
u32 op1f_io_reload_cycles;
u32 op1f_tape_free_chunks;
static u32 service_calls;

// A stock track's entry in the live record's map, as map_lookup 0x8fc20acc computes it (0x8fc20afa..0x8fc20b22:
// 0x84c68d88 + current x 22,160 + track x 1,938 + logical x 2, a 16-bit read). Read here directly: the first F1
// image called 0x8fc20b08, an instruction inside that function, and froze at its first slot reload.
static inline u32 live_map_entry(u32 track, u32 logical) {
    const unsigned char* record = RECORDS + CURRENT_SNAPSHOT * RECORD_BYTES;
    return *(const u16*)(record + track * TRACK_MAP_BYTES + logical * 2);
}

#ifdef OP1F_HOST
static inline u32 cycles() { return 0; }                               // the unit on the PC (for tests this tree does not hold): no clock
#else
static inline u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");               // F2: ordered against the reads it times
    return c;
}
#endif

// F2: the zero fill of an unallocated chunk's region, a word-store loop (about 0.1-0.3 ms per 128 KiB) in place
// of the ROM memset (2.4 ms, a byte loop), which the F1b read-out found to be most of a slot reload's time.
// Stock's own tape_io_read keeps its memset for tracks 1-4 elsewhere;
// the service loop below does not call it for an unallocated chunk any more.
static void zero_chunk(unsigned char* destination) {
    u32* word = (u32*)destination;
    for (u32 i = 0; i < CHUNK_BYTES / 4; ++i) word[i] = 0;
}

static void ext_clear_record(u32 record) {
    for (u32 t = 0; t < EXT_TRACKS; ++t)
        for (u32 i = 0; i < CHUNKS_PER_TRACK; ++i) op1f_ext_records[record][t][i] = UNALLOCATED;
}

static void ext_clear() {                       // the live record's extension maps
    ext_clear_record((u32)CURRENT_SNAPSHOT);
}

static u32 ext_allocated(u32 t) {
    u32 n = 0;
    for (u32 i = 0; i < CHUNKS_PER_TRACK; ++i) if (EXT_MAP(t)[i] != UNALLOCATED) ++n;
    return n;
}

static void reference_track(const u16* map, u32 bit) {
    for (int i = 0; i < CHUNKS_PER_TRACK; ++i) {
        u32 physical = map[i];
        if (physical != UNALLOCATED && physical < CHUNKS_PER_PARTITION) REFERENCES[physical] |= bit;
    }
}

// ---------------------------------------------------------------------------------------------------------
// Raw chunk I/O for tracks 4-5: whole chunks, through the open tape partition.

static void ext_read(u32 track, i32 logical, unsigned char* destination) {
    i32 handle = STOCK_TAPE_HANDLE();
    if (handle == -1) return;                                       // no tape's file is open: nothing read (stock says so on its log)
    if (logical < 0 || logical >= CHUNKS_PER_TRACK) return;
    u32 physical = EXT_MAP(track - STOCK_TRACKS)[logical];
    if (physical == UNALLOCATED || physical >= CHUNKS_PER_PARTITION) {
        zero_chunk(destination);                                    // an unallocated chunk reads as silence, as in stock (F2: the fast fill)
        return;
    }
    u32 before = cycles();
    i32 result = STOCK_PARTITION_READ(handle, physical << 17, CHUNK_BYTES, destination);
    op1f_io_read_cycles += cycles() - before;
    ++op1f_io_read_chunks;
    if (ext_read_log_budget) {
        --ext_read_log_budget;
        STOCK_LOG(3, STORE_LOG_FILE, 1, "ext read: track %d logical %d <- chunk %d (%d)", track + 1, logical, physical, result);
    }
    // K5: a chunk another firmware recorded over is not our sound any more: silence, and the entry goes (the map is
    // marked for saving by the unmap). A read that failed is left to what it left.
    if (result >= 0 && !op1f_sums_read(physical, destination)) {
        zero_chunk(destination);
        op1f_ext_unmap_chunk(track, (u32)logical);
        if (ext_sum_log_budget) {
            --ext_sum_log_budget;
            STOCK_LOG(2, STORE_LOG_FILE, 16, "ext sums: track %d chunk %d (physical %d) is not what was written: it plays silence and its entry goes", track + 1, logical, physical);
        }
    }
}

static int chunk_is_shared(u32 physical, u32 live) {               // stock chunk_is_shared 0x8fc20ea0
    u32 bits = REFERENCES[physical];
    if (bits == live) return 0;
    return (bits & (bits - 1)) != 0;                                // two or more bits set
}

static void ext_write(u32 track, i32 logical, const unsigned char* source) {
    i32 handle = STOCK_TAPE_HANDLE();
    if (handle == -1) return;
    if (logical < 0 || logical >= CHUNKS_PER_TRACK) return;
    u16* entry = &EXT_MAP(track - STOCK_TRACKS)[logical];
    u32 physical = *entry;
    u32 live = 1u << CURRENT_SNAPSHOT;
    if (physical == UNALLOCATED) {
        physical = op1f_sums_alloc();                               // K5: from the top of the tape; sets the live bit, leaves stock's cursor
        if (physical == UNALLOCATED) {
            STOCK_LOG(1, STORE_LOG_FILE, 2, "ext write: no free chunk for track %d logical %d", track + 1, logical);
            return;
        }
        *entry = (u16)physical;
        STOCK_MARK_MAP_DIRTY();
    } else if (physical < CHUNKS_PER_PARTITION && chunk_is_shared(physical, live)) {
        u32 fresh = op1f_sums_alloc();                              // copy on write, as stock's tape_io_write does
        if (fresh == UNALLOCATED) {
            STOCK_LOG(1, STORE_LOG_FILE, 3, "ext write: COW failed for track %d logical %d", track + 1, logical);
            return;
        }
        REFERENCES[physical] &= (unsigned char)~live;
        *entry = (u16)fresh;
        physical = fresh;
        STOCK_MARK_MAP_DIRTY();
    }
    if (physical >= CHUNKS_PER_PARTITION) return;
    i32 result = STOCK_PARTITION_WRITE(handle, physical << 17, CHUNK_BYTES, (void*)source);
    // K5: the chunk's sum is what was just written (a write that failed leaves it unknown); the file of tracks 5 to 8
    // is to say so, so the map is marked for saving when the sum changed.
    if (result >= 0 ? op1f_sums_wrote(physical, source) : op1f_sums_part(physical)) STOCK_MARK_MAP_DIRTY();
    if (ext_write_log_budget) {
        --ext_write_log_budget;
        STOCK_LOG(3, STORE_LOG_FILE, 4, "ext write: track %d logical %d -> chunk %d (%d)", track + 1, logical, physical, result);
    }
}

// B3c (build_b_edit.cpp): the lift and the range clear of tracks 4-5 release whole chunks the way stock's
// unmap_chunk 0x8fc22750 releases its own: the live bit of the physical chunk is cleared, the map entry becomes
// unallocated and the map is marked dirty (the version-2 saver then writes it). op1f_ext_chunk_mapped tells the
// zeroing of a partial chunk whether there is anything to zero.
extern "C" int op1f_ext_chunk_mapped(u32 track, u32 logical) asm("op1f_ext_chunk_mapped");
extern "C" int op1f_ext_chunk_mapped(u32 track, u32 logical) {
    u32 j = track - STOCK_TRACKS;
    if (j >= EXT_TRACKS || logical >= CHUNKS_PER_TRACK) return 0;
    u32 physical = EXT_MAP(j)[logical];
    return physical != UNALLOCATED && physical < CHUNKS_PER_PARTITION;
}

extern "C" void op1f_ext_unmap_chunk(u32 track, u32 logical) asm("op1f_ext_unmap_chunk");
extern "C" void op1f_ext_unmap_chunk(u32 track, u32 logical) {
    u32 j = track - STOCK_TRACKS;
    if (j >= EXT_TRACKS || logical >= CHUNKS_PER_TRACK) return;
    u16* entry = &EXT_MAP(j)[logical];
    u32 physical = *entry;
    if (physical == UNALLOCATED) return;
    if (physical < CHUNKS_PER_PARTITION) REFERENCES[physical] &= (unsigned char)~(1u << CURRENT_SNAPSHOT);
    *entry = UNALLOCATED;
    STOCK_MARK_MAP_DIRTY();
}

// ---------------------------------------------------------------------------------------------------------
// tape_io_read / tape_io_write for tracks 4-5 (B2, after the first B2 image aborted in tape_io.cpp:89 at
// record stop: stock reads 512 frames at each edge of a new clip as its "padding" through tape_io_read with
// the record track). The stock functions 0x8fc21bac / 0x8fc21d1c keep serving tracks 0-3 through the entry
// trampolines in build_b_record.s; a track of 4 or 5 arrives here with the same arguments (byte offset within
// the track, length, buffer) and the same contract (bytes done, or -1). Semantics as stock, read from
// 0x8fc21bac..0x8fc2021b and 0x8fc21d1c..0x8fc2055f: a range may cross chunks and stops at logical chunk 969;
// an unallocated chunk reads as zeros; a write to an unallocated chunk allocates one and zero-fills the part
// the range does not cover; a chunk shared with a snapshot is copied first (live since Build C, when the
// snapshots came back); a failed allocation or partition call ends the call with -1. These maps are the extension
// maps, so stock's map_lookup / alloc_for_write (which assert track < 4) are never reached for tracks 4-5.

typedef void* (*MemcpyFn)(void* destination, const void* source, u32 count);
#define STOCK_MEMCPY ((MemcpyFn)0x040201c4u)                      // ROM memcpy, as stock's tape_io_write uses

#ifdef OP1F_HOST
alignas(32) static unsigned char ext_stage[CHUNK_BYTES];
#else
static unsigned char ext_stage[CHUNK_BYTES] __attribute__((aligned(32)));   // one chunk for partial writes
#endif
static u32 ext_io_log_budget = LOG_BUDGET;

static i32 ext_io_open(u32 track, const char* what) {           // the tape partition handle, or -1 (logged)
    if (track < STOCK_TRACKS || track >= EIGHT_TRACKS || op1f_cache_tracks < EIGHT_TRACKS) {
        STOCK_LOG(1, STORE_LOG_FILE, 8, "tape io %s: track %d is not available (%d-track cache)", what, track + 1, op1f_cache_tracks);
        return -1;
    }
    if (STOCK_TAPE_INDEX() < 0) return -1;
    i32 handle = STOCK_TAPE_HANDLE();
    if (handle == -1) {
        STOCK_LOG(1, STORE_LOG_FILE, 9, "tape io %s: the tape's file is not open", what);
        return -1;
    }
    return handle;
}

extern "C" i32 op1f_tape_io_read_ext(u32 track, u32 byte_offset, u32 length, unsigned char* buffer) asm("op1f_tape_io_read_ext");
extern "C" i32 op1f_tape_io_read_ext(u32 track, u32 byte_offset, u32 length, unsigned char* buffer) {
    i32 handle = ext_io_open(track, "read");
    if (handle < 0) return -1;
    if (ext_io_log_budget) {
        --ext_io_log_budget;
        STOCK_LOG(3, STORE_LOG_FILE, 10, "tape io read: track %d offset %u length %u", track + 1, byte_offset, length);
    }
    u32 done = 0;
    while (done < length) {
        u32 logical = byte_offset >> 17;
        if (logical >= CHUNKS_PER_TRACK) break;
        u32 in_chunk = byte_offset & (CHUNK_BYTES - 1);
        u32 n = CHUNK_BYTES - in_chunk;
        if (n > length - done) n = length - done;
        u32 physical = EXT_MAP(track - STOCK_TRACKS)[logical];
        if (physical == UNALLOCATED || physical >= CHUNKS_PER_PARTITION) {
            STOCK_MEMSET(buffer + done, 0, n);
        } else if (!op1f_sums_ours(physical)) {
            // K5: a chunk still held to its sum, or one that failed it. The whole chunk is looked at through the stage
            // before any of it is handed over (a lift, a fade's read, the disk mode's export), and what is not ours
            // reads as silence. The entry is left to the chunk cache's read, which lets it go.
            i32 result = STOCK_PARTITION_READ(handle, physical << 17, CHUNK_BYTES, ext_stage);
            if (result < 0) return result;
            if (op1f_sums_read(physical, ext_stage)) STOCK_MEMCPY(buffer + done, ext_stage + in_chunk, n);
            else STOCK_MEMSET(buffer + done, 0, n);
        } else {
            i32 result = STOCK_PARTITION_READ(handle, (physical << 17) + in_chunk, n, buffer + done);
            if (result < 0) return result;
        }
        done += n;
        byte_offset += n;
    }
    return (i32)done;
}

extern "C" i32 op1f_tape_io_write_ext(u32 track, u32 byte_offset, u32 length, const unsigned char* buffer) asm("op1f_tape_io_write_ext");
extern "C" i32 op1f_tape_io_write_ext(u32 track, u32 byte_offset, u32 length, const unsigned char* buffer) {
    i32 handle = ext_io_open(track, "write");
    if (handle < 0) return -1;
    if (ext_io_log_budget) {
        --ext_io_log_budget;
        STOCK_LOG(3, STORE_LOG_FILE, 11, "tape io write: track %d offset %u length %u", track + 1, byte_offset, length);
    }
    u32 done = 0;
    while (done < length) {
        u32 logical = byte_offset >> 17;
        if (logical >= CHUNKS_PER_TRACK) break;
        u32 in_chunk = byte_offset & (CHUNK_BYTES - 1);
        u32 n = CHUNK_BYTES - in_chunk;
        if (n > length - done) n = length - done;
        int whole = in_chunk == 0 && n == CHUNK_BYTES;
        u16* entry = &EXT_MAP(track - STOCK_TRACKS)[logical];
        u32 physical = *entry;
        u32 live = 1u << CURRENT_SNAPSHOT;
        u32 source = UNALLOCATED;                                   // the chunk a copy-on-write starts from
        int fresh = 0;
        if (physical == UNALLOCATED || physical >= CHUNKS_PER_PARTITION) {
            physical = op1f_sums_alloc();                           // K5: from the top of the tape
            if (physical == UNALLOCATED) {
                STOCK_LOG(1, STORE_LOG_FILE, 12, "tape io write: no free chunk for track %d logical %d", track + 1, logical);
                return -1;
            }
            *entry = (u16)physical;
            STOCK_MARK_MAP_DIRTY();
            fresh = 1;
        } else if (chunk_is_shared(physical, live)) {
            u32 copy = op1f_sums_alloc();
            if (copy == UNALLOCATED) {
                STOCK_LOG(1, STORE_LOG_FILE, 13, "tape io write: COW failed for track %d logical %d", track + 1, logical);
                return -1;
            }
            REFERENCES[physical] &= (unsigned char)~live;
            source = physical;
            physical = copy;
            *entry = (u16)copy;
            STOCK_MARK_MAP_DIRTY();
            fresh = 1;
        } else if (!whole && !op1f_sums_ours(physical)) {           // K5: a part into a chunk still held to its sum, or that failed it:
            source = physical;                                      // through the stage, where the whole chunk is looked at first
            fresh = 1;
        }
        i32 result;
        int changed;                                                // K5: what the file should say of the chunk's sum has changed
        if (fresh && !whole) {                                      // the rest of a new chunk: zeros, or the copied chunk
            if (source == UNALLOCATED) {
                STOCK_MEMSET(ext_stage, 0, CHUNK_BYTES);
            } else {
                result = STOCK_PARTITION_READ(handle, source << 17, CHUNK_BYTES, ext_stage);
                if (result < 0) return result;
                if (!op1f_sums_read(source, ext_stage)) STOCK_MEMSET(ext_stage, 0, CHUNK_BYTES);   // K5: not what was written: silence around the part
            }
            STOCK_MEMCPY(ext_stage + in_chunk, buffer + done, n);
            result = STOCK_PARTITION_WRITE(handle, physical << 17, CHUNK_BYTES, ext_stage);
            changed = result >= 0 ? op1f_sums_wrote(physical, ext_stage) : op1f_sums_part(physical);
        } else {
            result = STOCK_PARTITION_WRITE(handle, (physical << 17) + in_chunk, n, (void*)(buffer + done));
            changed = (whole && result >= 0) ? op1f_sums_wrote(physical, buffer + done) : op1f_sums_part(physical);   // a part in place: the sum is not known any more
        }
        if (changed) STOCK_MARK_MAP_DIRTY();
        if (result < 0) return result;
        done += n;
        byte_offset += n;
    }
    return (i32)done;
}

// ---------------------------------------------------------------------------------------------------------
// chunks_service(flush_only, loop_a, loop_b), stock 0x8fc217a0..0x8fc1fdf7, with the track loops widened.
//
// One unit of work per call, walking the LRU list from its head. A slot with write_pending: clear the flag and,
// if a tape is open, write every dirty track (clearing its bit first); return 1. Otherwise, unless flush_only,
// the first slot that holds a logical chunk and is not valid or is stale: if a tape is open read every track,
// then valid = (the logical chunk did not change meanwhile), stale = 0, tell the crossfade code which frames
// were loaded; return 1. Nothing to do: return 0 (the caller then saves the map if it is dirty).

extern "C" int op1f_chunks_service(int flush_only, int loop_a, int loop_b) asm("op1f_chunks_service");
static void log_v2_save(u32 tape, const u32* allocated, u32 quiet) {
    STOCK_LOG(3, STORE_LOG_FILE, 7, "block map and ext map for tape %d saved: tracks 5-8 hold %d/%d/%d/%d chunk(s), %d quiet save(s) while recording, %d snapshot(s) taken, %d dropped",
              tape + 1, allocated[0], allocated[1], allocated[2], allocated[3], quiet, op1f_snapshots_taken, op1f_snapshots_dropped);
}

// A stock track's chunk read through stock's tape_io_read, timed when the chunk is allocated (an unallocated
// chunk is a zero fill in stock, not an eMMC read), for the F1 measurement line.
static void stock_read(u32 track, i32 logical, unsigned char* data) {
    u32 allocated = logical >= 0 && logical < CHUNKS_PER_TRACK && live_map_entry(track, (u32)logical) != UNALLOCATED;
    if (!allocated) {                                               // F2: stock's read would memset the region; the fast fill instead
        zero_chunk(data);
        return;
    }
    u32 before = cycles();
    STOCK_TAPE_IO_READ(track, (u32)logical << 17, CHUNK_BYTES, data);
    op1f_io_read_cycles += cycles() - before;
    ++op1f_io_read_chunks;
}

extern "C" int op1f_chunks_service(int flush_only, int loop_a, int loop_b) {
    u32 tracks = op1f_cache_tracks ? op1f_cache_tracks : STOCK_TRACKS;
    if (quiet_saves && !RECORDING) {                                 // B2: one line for the saves a recording caused
        log_v2_save(quiet_tape, quiet_allocated, quiet_saves);
        quiet_saves = 0;
    }
    if ((++service_calls & (FREE_COUNT_EVERY - 1)) == 0 && STOCK_TAPE_INDEX() >= 0)
        op1f_tape_free_chunks = (u32)STOCK_COUNT_FREE();              // F1: for the help screen's gauge and the log line
    for (Slot* slot = LRU_HEAD; slot; slot = slot->next) {
        if (slot->write_pending) {
            slot->write_pending = 0;
            if (STOCK_TAPE_INDEX() < 0) return 1;
            for (u32 track = 0; track < tracks; ++track) {
                if (!(slot->dirty_tracks & (1u << track))) continue;
                slot->dirty_tracks = (u16)(slot->dirty_tracks & ~(1u << track));
                i32 logical = slot->logical_chunk;
                unsigned char* data = slot->data + track * CHUNK_BYTES;
                if (track < STOCK_TRACKS) STOCK_TAPE_IO_WRITE(track, (u32)logical << 17, CHUNK_BYTES, data);
                else ext_write(track, logical, data);
            }
            ++op1f_io_writebacks;
            return 1;
        }
        if (flush_only) continue;
        i32 logical = slot->logical_chunk;
        if (logical < 0) continue;
        if (slot->valid && !slot->stale) continue;
        if (STOCK_TAPE_INDEX() >= 0) {
            u32 before = cycles();
            for (u32 track = 0; track < tracks; ++track) {
                unsigned char* data = slot->data + track * CHUNK_BYTES;
                if (track < STOCK_TRACKS) stock_read(track, logical, data);
                else ext_read(track, logical, data);
            }
            op1f_io_reload_cycles += cycles() - before;
            ++op1f_io_reloads;
        }
        slot->valid = (slot->logical_chunk == logical) ? 1 : 0;
        slot->stale = 0;
        i32 position = slot->frame_pos;
        op1f_xfade_mark_loaded_range(position, position + CHUNK_FRAMES, loop_a, loop_b, EIGHT_TRACK_XFADE_MASK);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------
// snapshots_reset(), stock 0x8fc224fc..0x8fc20a6b: clear the reference bytes, set all eight records' maps to
// unallocated, make record 0 live, cursor 0, count 1. Called by the tape engine init, by tape_open before the
// map is loaded, and by the erase-tape path before an empty map is saved. Plus: the extension maps are cleared.

extern "C" void op1f_snapshots_reset() asm("op1f_snapshots_reset");
extern "C" void op1f_snapshots_reset() {
    STOCK_MEMSET((void*)REFERENCES, 0, CHUNKS_PER_PARTITION);
    for (u32 record = 0; record < MAX_SNAPSHOTS; ++record) {
        u32* map = (u32*)(RECORDS + record * RECORD_BYTES);
        for (u32 i = 0; i < MAP_BYTES / 4; ++i) map[i] = 0xffffffffu;
    }
    STOCK_SET_CURRENT_SNAPSHOT(0);
    ALLOC_CURSOR = 0;
    SNAPSHOT_COUNT = 1;
    for (u32 record = 0; record < MAX_SNAPSHOTS; ++record) ext_clear_record(record);
    op1f_sums_reset();                                               // K5: no chunk of tracks 5 to 8 is known any more
}

// ---------------------------------------------------------------------------------------------------------
// take_snapshot_ex(min_free, tag), stock 0x8fc23128..0x8fc216b3, with the extension records carried along
// (Build C). Stock's take_snapshot 0x8fc22af0 drops the oldest record while the array is full, clears every
// reference bit at or above the new index (the redo tail dies there), copies the live 22,160-byte record into
// the next slot, gives every chunk the live record references the new record's bit as well, and makes the
// new record live; the bit operations run over the whole reference array, so they cover the chunks of the
// extension tracks too (their bits are set by the same alloc_chunk). This function runs stock's drop loop
// itself, each drop shifting the extension records as drop_oldest_snapshot shifts stock's (records 1..7 down
// by one, bit 0 cleared, every byte shifted right), calls stock's take_snapshot with the array not full,
// copies the extension maps of the record that was live into the new live record, stores the tag, drops the
// oldest records until min_free chunks are free (stock's message when it cannot), and marks the map dirty.
// B0b to B3f took no snapshot here, so undo did nothing on any tape.

static int drop_oldest_six() {                  // stock drop_oldest_snapshot 0x8fc22a14, plus the extension records
    if (!STOCK_DROP_OLDEST()) return 0;         // refused at record 0: nothing changed
    STOCK_MEMMOVE(op1f_ext_records[0], op1f_ext_records[1], (MAX_SNAPSHOTS - 1) * sizeof(op1f_ext_records[0]));
    ++op1f_snapshots_dropped;
    return 1;
}

extern "C" void op1f_take_snapshot_ex(int min_free, int tag) asm("op1f_take_snapshot_ex");
extern "C" void op1f_take_snapshot_ex(int min_free, int tag) {
    while (CURRENT_SNAPSHOT + 1 >= MAX_SNAPSHOTS) {
        if (!drop_oldest_six()) break;          // cannot happen: the live record is record 7 here
    }
    i32 previous = CURRENT_SNAPSHOT;
    STOCK_TAKE_SNAPSHOT();                      // the array is not full, so stock drops nothing inside
    i32 current = CURRENT_SNAPSHOT;
    if (current != previous && current > 0 && current < MAX_SNAPSHOTS && previous >= 0 && previous < MAX_SNAPSHOTS)
        STOCK_MEMCPY(op1f_ext_records[current], op1f_ext_records[previous], sizeof(op1f_ext_records[0]));
    if (current >= 0 && current < MAX_SNAPSHOTS)
        *(i32*)(RECORDS + current * RECORD_BYTES + RECORD_TAG_OFFSET) = tag;
    ++op1f_snapshots_taken;
    if (min_free > 0) {
        while (STOCK_COUNT_FREE() < min_free) {
            if (!drop_oldest_six()) {
                STOCK_LOG(2, STORE_LOG_FILE, 14, "snapshots: no older snapshot is left to give up for %d free chunks", min_free);
                break;
            }
        }
    }
    STOCK_MARK_MAP_DIRTY();
    if (take_log_budget) {
        --take_log_budget;
        STOCK_LOG(3, STORE_LOG_FILE, 15, "snapshot %d of %d taken (tag %d, %d chunks wanted), %d free chunks",
                  CURRENT_SNAPSHOT + 1, SNAPSHOT_COUNT, tag, min_free, STOCK_COUNT_FREE());
    }
}

// ---------------------------------------------------------------------------------------------------------
// diff_range(other, min, max), stock 0x8fc22c84..0x8fc2127f, over six tracks (Build C): the frame range in
// which record `other` and the live record differ, from stock's four maps and the two extension maps, so an
// undo or redo of a change on track 5 or 6 reloads the cache for that range (undo_ex 0x8fc216b4 and redo_ex
// 0x8fc2175c hand the result to chunks_mark_range when no tag was stored). As stock: *min starts at the tape's
// length, *max at 0, a difference at logical chunk l widens the range to l x 16384 .. (l + 1) x 16384, and the
// result is 1 when anything differed. Stock asserts the index and the pointers; here a bad call answers 0.

extern "C" int op1f_diff_range(i32 other, i32* minimum, i32* maximum) asm("op1f_diff_range");
extern "C" int op1f_diff_range(i32 other, i32* minimum, i32* maximum) {
    if (other < 0 || other >= MAX_SNAPSHOTS || minimum == 0 || maximum == 0) return 0;
    i32 current = CURRENT_SNAPSHOT;
    if (current < 0 || current >= MAX_SNAPSHOTS) return 0;
    *minimum = TAPE_FRAMES;
    *maximum = 0;
    int differs = 0;
    for (u32 track = 0; track < EIGHT_TRACKS; ++track) {
        const u16* a;
        const u16* b;
        if (track < STOCK_TRACKS) {
            a = (const u16*)(RECORDS + other * RECORD_BYTES + track * TRACK_MAP_BYTES);
            b = (const u16*)(RECORDS + current * RECORD_BYTES + track * TRACK_MAP_BYTES);
        } else {
            a = op1f_ext_records[other][track - STOCK_TRACKS];
            b = op1f_ext_records[current][track - STOCK_TRACKS];
        }
        for (u32 logical = 0; logical < CHUNKS_PER_TRACK; ++logical) {
            if (a[logical] == b[logical]) continue;
            i32 frame = (i32)(logical * CHUNK_FRAMES);
            if (frame < *minimum) *minimum = frame;
            if (frame + (i32)CHUNK_FRAMES > *maximum) *maximum = frame + (i32)CHUNK_FRAMES;
            differs = 1;
        }
    }
    return differs;
}

// ---------------------------------------------------------------------------------------------------------
// save_block_map(tape), stock 0x8fc22d70..0x8fc213bf: write magic, version and the live record's four track
// maps to user/tapeNN_map.tmp, close, rename to user/tapeNN_map.bin, clear the dirty byte. Since K1 that file is
// always stock's own, version 1, and the tables of tracks 5 to 8 go into user/tapeNN_ext.bin beside it
// (Build K's maps unit: the extension file first, then stock's), so that stock firmware reads every tape. From B0b
// to J4 a tape with a chunk on a track beyond four was saved as one version-2 file, which stock rejects.
// The log line for a tape with extension chunks is held back while a recording runs (every allocated chunk
// marks the map dirty and the idle flush saves it) and printed by the service loop once recording has stopped,
// with the count.

extern "C" void op1f_save_block_map(int tape) asm("op1f_save_block_map");
extern "C" void op1f_save_block_map(int tape) {
    if (tape < 0 || tape >= TAPES) {                                 // stock asserts here
        STOCK_LOG(1, STORE_LOG_FILE, 5, "save block map: tape index %d out of range", tape);
        return;
    }
    const unsigned char* record = RECORDS + CURRENT_SNAPSHOT * RECORD_BYTES;
    u32 allocated[EXT_TRACKS];
    int extended = 0;
    for (u32 t = 0; t < EXT_TRACKS; ++t) {
        allocated[t] = ext_allocated(t);
        if (allocated[t]) extended = 1;
    }
    // The record's four track maps lie one after the other (4 x 1,938 bytes), and so do the four extension maps.
    // K5: the sums of the chunks of tracks 5 to 8 go with them, when the sums in memory are this tape's.
    // The dirty byte is cleared before the tables and the sums are collected, where stock clears it after the file
    // is written: a mark another thread makes while the save runs (the command thread writes a part into a chunk,
    // whose sum is then not known any more) is kept that way, and the next idle pass saves again. Cleared after,
    // such a mark was lost and the file kept a sum that was behind its chunk.
    MAP_DIRTY = 0;
    op1f_xmap_save_sums = op1f_sums_collect(tape, EXT_MAP(0), op1f_xmap_save_entry_sums, op1f_xmap_save_entry_flags);
    if (!op1f_xmap_save(tape, (const u16*)record, EXT_MAP(0))) {
        MAP_DIRTY = 1;                                               // not written: the dirty byte stays set, as in stock
        return;
    }
    if (!extended) return;
    if (RECORDING) {                                                 // B2: the idle flush saves once per allocated chunk while
        ++quiet_saves;                                               // recording; the service loop prints one line afterwards
        quiet_tape = (u32)tape;
        for (u32 t = 0; t < EXT_TRACKS; ++t) quiet_allocated[t] = allocated[t];
        return;
    }
    log_v2_save((u32)tape, allocated, quiet_saves);
    quiet_saves = 0;
}

// ---------------------------------------------------------------------------------------------------------
// load_block_map(tape): Build A's loader (ran on hardware 2026-11-04), which since
// B0b also fills the extension maps. Contract as stock: 1 when the live record's map was loaded, 0 when tape_open
// must fall back to the identity map (no file, bad magic, unsupported version, short read); the extension maps
// are then empty. Since K1 the files are read by Build K's maps unit: stock's version-1 file for tracks 1 to 4 and
// the extension file beside it for tracks 5 to 8, or one of this project's old version-2 files. Once a start,
// before the first tape is read, every tape's file that is still version 2 is rewritten as the two files, so
// that a unit which has run this pack holds only tapes stock firmware reads.

// K4: what the last load found of tracks 5 to 8, for the clips' loader (build_k_clips.cpp), which runs right after
// a tape's mount: the tape's index and the XEXT_ state of its extension file, -1 while no map was loaded for it.
extern "C" int op1f_ext_load_tape asm("op1f_ext_load_tape");
extern "C" int op1f_ext_load_state asm("op1f_ext_load_state");
int op1f_ext_load_tape = -1;
int op1f_ext_load_state = -1;

extern "C" int op1f_load_block_map(int tape) asm("op1f_load_block_map");
extern "C" int op1f_load_block_map(int tape) {
    static const char* const EXT_STATE[7] = { "no ext map", "ext map read", "ext map read; stock's file was changed by another firmware",
                                              "ext map unusable", "tracks 5-8 from a version-2 file", "ext map of a newer pack, left alone",
                                              "ext map of another tape, left out" };
    static int swept;
    op1f_ext_load_tape = tape;
    op1f_ext_load_state = -1;
    if (tape < 0 || tape >= TAPES) {
        STOCK_LOG(1, MAP_LOG_FILE, 1, "block map: tape index %d out of range", tape);
        return 0;
    }
    ext_clear();
    if (!swept) {                                                    // K1: no version-2 file left on the unit
        swept = 1;
        for (int t = 0; t < TAPES; ++t) op1f_xmap_convert(t);
    }
    i32 current = CURRENT_SNAPSHOT;
    unsigned char* record = RECORDS + current * RECORD_BYTES;
    XmapLoad load;
    if (!op1f_xmap_load(tape, (u16*)record, EXT_MAP(0), &load)) {
        ext_clear();
        op1f_sums_loaded(tape, 0, 0, 0, 0);                          // K5: nothing of tracks 5 to 8
        return 0;
    }
    op1f_ext_load_state = load.ext;
    // K5: the sums the file of tracks 5 to 8 carries, by physical chunk; held to when another firmware wrote the tape
    op1f_sums_loaded(tape, EXT_MAP(0), load.sums ? op1f_xmap_entry_sums : 0, op1f_xmap_entry_flags, load.foreign);
    u32 bit = 1u << current;
    for (int i = 0; i < CHUNKS_PER_PARTITION; ++i) REFERENCES[i] = 0;
    for (u32 track = 0; track < STOCK_TRACKS; ++track) reference_track((const u16*)(record + track * TRACK_MAP_BYTES), bit);
    for (u32 t = 0; t < EXT_TRACKS; ++t) reference_track(EXT_MAP(t), bit);
    // Dirty when what was read is not what a save would write: an old file, an extension file that had to be
    // filtered or could not be used. The idle flush then writes the two files as they now stand.
    MAP_DIRTY = (load.status == XMAP_LEGACY || load.ext == XEXT_CHANGED || load.ext == XEXT_BAD || load.ext == XEXT_STALE || load.dropped ||
                 load.rewrite) ? 1 : 0;                      // K2: a file of another tape is emptied, an older file written anew
    op1f_tape_free_chunks = (u32)STOCK_COUNT_FREE();
    STOCK_LOG(3, MAP_LOG_FILE, 7, "block map v%d for tape %d: %d track(s) in file, %d free chunks",
              load.version, tape + 1, load.tracks_in_file, op1f_tape_free_chunks);
    if (load.ext != XEXT_NONE)
        STOCK_LOG(load.ext == XEXT_SAME ? 3 : 2, MAP_LOG_FILE, 8, "ext map for tape %d: tracks 5-8 hold %d/%d/%d/%d chunk(s), %d dropped; %s; v%d, id %08x, json %08x, bound %d",
                  tape + 1, load.allocated[0], load.allocated[1], load.allocated[2], load.allocated[3], load.dropped, EXT_STATE[load.ext],
                  load.ext_version, load.id, load.json_id, load.bound);
    return 1;
}

// ---------------------------------------------------------------------------------------------------------
// The tracks 5 to 8 for another pack (8 tracks 14, for speedlock: a pack that rewrites a tape in place and wants
// tracks 5 to 8 to go with tracks 1 to 4). Additive: nothing of 8 tracks calls these four names, and with no pack
// that asks for them 8 tracks does what 8 tracks 11 did. The recipe offers them to every pack ("exports").
//
// The contract, for a pack that rewrites a chunk of tracks 5 to 8 the way stock's own edits rewrite tracks 1 to 4:
//
//   1. On the main loop (where the tape service and stock's edits run) and only while op1f_ext_quiet() answers 1,
//      read the live record's maps with op1f_ext_map(record, track).
//   2. Take free chunks on the reference bytes (the live record's bit, as stock's allocator does; this pack takes
//      its own from the top of the tape, so a pack that takes from the bottom meets it only on a full tape) and
//      write the new audio there with the tape partition's own write. No chunk an undo record holds is written.
//   3. In one pass of the main loop: stock's take_snapshot_ex (this pack's hook copies the maps of tracks 5 to 8
//      into the new record, so stock's undo takes them back), then op1f_ext_put for every entry that changes, then
//      stock's map flush: this pack's save writes the file of tracks 5 to 8 first, then stock's.
//
// A chunk put here has no sum until its next whole read gives it one (K5's rule for a chunk with none).

extern "C" const u32 op1f_ext_api asm("op1f_ext_api");
const u32 op1f_ext_api = 1;                     // what these names mean; a new meaning takes a new number

// The map of track 5 + track (track 0..3) in undo record `record` (0..7): 969 physical chunk numbers, 0xFFFF for
// none. The live record is stock's 0x848cff90. The pointer is good until the next snapshot or tape load moves the
// records; read it in the same pass of the main loop. 0 for a record or track out of range.
extern "C" const u16* op1f_ext_map(i32 record, i32 track) asm("op1f_ext_map");
extern "C" const u16* op1f_ext_map(i32 record, i32 track) {
    if (record < 0 || record >= MAX_SNAPSHOTS || track < 0 || track >= EXT_TRACKS) return 0;
    return op1f_ext_records[record][track];
}

// 1 when a change of the maps of tracks 5 to 8 crosses nothing of this pack's: a tape is open, no take is being
// recorded (the command thread writes ranges then, and allocates), and the cache holds tracks 5 to 8.
extern "C" int op1f_ext_quiet() asm("op1f_ext_quiet");
extern "C" int op1f_ext_quiet() {
    return STOCK_TAPE_HANDLE() != -1 && !RECORDING && op1f_cache_tracks >= EIGHT_TRACKS ? 1 : 0;
}

// The live record's entry of track 5 + track at chunk `logical` made `physical` (0xFFFF: none), as stock's map
// setter 0x8fc22650 does for tracks 1 to 4: the chunk that was there loses the live record's bit (unless it is
// none or the same chunk), the new one gains it (unless none), the new one's sum is not known (K5), and the map is
// marked for saving. Answers 0, or -1 (nothing changed) for an argument out of range or when not quiet.
extern "C" int op1f_ext_put(i32 track, i32 logical, u32 physical) asm("op1f_ext_put");
extern "C" int op1f_ext_put(i32 track, i32 logical, u32 physical) {
    if (track < 0 || track >= EXT_TRACKS || logical < 0 || logical >= CHUNKS_PER_TRACK) return -1;
    if (physical != UNALLOCATED && physical >= CHUNKS_PER_PARTITION) return -1;
    if (!op1f_ext_quiet()) return -1;
    u16* entry = &EXT_MAP(track)[logical];
    u32 old = *entry;
    u32 live = 1u << CURRENT_SNAPSHOT;
    if (old != physical && old != UNALLOCATED && old < CHUNKS_PER_PARTITION) REFERENCES[old] &= (unsigned char)~live;
    if (physical != UNALLOCATED) {
        REFERENCES[physical] |= (unsigned char)live;
        if (old != physical) op1f_sums_part(physical);
    }
    *entry = (u16)physical;
    STOCK_MARK_MAP_DIRTY();
    return 0;
}
