// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K, pack K4: the copy of a tape takes tracks 5 to 8 with it.
//
// Stock's copy of a tape is shift + T3 in the tape list: 0x8c0d7824 finds a free slot, ejects the mounted tape,
// names the copy and calls the clone 0x8d4fb908(source, destination, progress), then loads the copy. The clone
// (tape_db.cpp) copies user/tapeNN.json as a file, reads the source's map file (magic and version 1 required),
// marks every chunk its four tables name, copies each marked chunk from the source's partition to the same chunk
// of the destination's (tapeNN.raw, opened by name through 0x8d501008; 128 KiB pieces through 0x8d500688 and
// 0x8d500b88 by way of a heap buffer from 0x8d4e4324(0x200, 0x20000)), calls the progress function with a
// fraction every sixteen chunks, copies the map file and says 1.0. It knows nothing of tracks 5 to 8: until K4
// the copy had the original's clips of tracks 5 to 8 in its json and nothing under them (the K2 test).
//
// One hook, the profile's group build-k4: the tape list's CALL.a of the clone at 0x8c0d84ae, retargeted to
// op1f_tape_clone. It calls stock's clone with stock's arguments and then, when the source has a file of tracks
// 5 to 8 that is its own:
//
//   1. holds the destination's map file against the source's (the same CRC-32: stock's copy landed);
//   2. copies every chunk the source's tracks 5 to 8 name to the same chunk of the destination's partition, as
//      stock copies its own. The numbers are free in the destination: its tracks 1 to 4 are the source's, and a
//      load keeps no chunk of tracks 5 to 8 that tracks 1 to 4 hold. The progress function is called with 1.0
//      every sixteen chunks, as stock calls it, so that whatever it serves is served;
//   3. writes the copy's own file of tracks 5 to 8 last, with no identity (Build K's maps unit, op1f_xmap_write_plain):
//      the load that follows takes it by stock's sum, and the save after that gives the copy an id of its own.
//      A unit that goes off before step 3 has a copy with no file of tracks 5 to 8; its clips of those tracks are
//      then left out at its load (build_k_clips.cpp).
//
// Both partitions are closed again before the function returns, on every path: the copy is loaded next, which
// opens its partition, and 0x8d501008 asserts on a partition that is open.
//
// The clips need nothing here: the copy's json is the original's file, with both lists.
// It runs on the main thread, from the tape list's key handler, with no tape mounted (the handler has ejected it).
// Freestanding: no library, no division.

#include "build_k_maps.h"
#include "build_k_sums.h"       // K5: the flags of an entry's sum

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*ProgressFn)(u32 fraction_float_bits);                   // the tape list's 0x8c0d5eb4: the bar, then a tick's sleep
typedef void (*CloneFn)(int source, int destination, ProgressFn progress, u32 fourth);   // 0x8d4fb908; the fourth word is whatever stock had at [SP + 0xc]
typedef i32 (*OpenPartitionFn)(const char* name);                      // 0x8d501008: a handle, or -1 (stock logs the failure)
typedef i32 (*PartitionIoFn)(i32 handle, u32 byte_offset, u32 length, void* buffer);   // negative on error
typedef void (*ClosePartitionFn)(i32 handle);
typedef void* (*AllocAlignedFn)(u32 alignment, u32 bytes);             // 0x8d4e4324: 0 when there is no room
typedef void (*FreeAlignedFn)(void* block);                            // 0x8d4e43a4

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_CLONE ((CloneFn)0x8d4fb908u)
#define STOCK_OPEN_PARTITION ((OpenPartitionFn)0x8d501008u)            // as the clone calls it at 0x8d4faa04 and 0x8d4faa3c
#define STOCK_PARTITION_READ ((PartitionIoFn)0x8d500688u)              // at 0x8d4fac08
#define STOCK_PARTITION_WRITE ((PartitionIoFn)0x8d500b88u)             // at 0x8d4fac24
#define STOCK_CLOSE_PARTITION ((ClosePartitionFn)0x8d500438u)          // at 0x8d4fab32 and 0x8d4fab38
#define STOCK_ALLOC_ALIGNED ((AllocAlignedFn)0x8d4e4324u)              // at 0x8d4faa58
#define STOCK_FREE_ALIGNED ((FreeAlignedFn)0x8d4e43a4u)                // at 0x8d4fab2c

enum {
    TAPES = 8,
    TABLE_ENTRIES = 4 * 969,
    CHUNKS_PER_PARTITION = 8192,
    UNALLOCATED = 0xffff,
    CHUNK_BYTES = 0x20000,
    CHUNK_SHIFT = 17,
    BUFFER_ALIGNMENT = 0x200,                    // as the clone asks for its buffer
    PROGRESS_EVERY = 16,                         // chunks, as the clone
    ONE_FLOAT = 0x3f800000,
    LOG_BUDGET = 16
};

static const char LOG_FILE[] = "../src/db2/op1f_tape_copy.cpp";

extern "C" void op1f_tape_clone(int source, int destination, ProgressFn progress, u32 fourth) asm("op1f_tape_clone");
extern "C" u32 op1f_copy_counts[6] asm("op1f_copy_counts");
u32 op1f_copy_counts[6];                         // copies, with tracks 5 to 8, chunks copied, stock's copy not landed or no map, failures, nothing to copy

static u32 log_budget = LOG_BUDGET;

static void partition_name(char* name, int tape) {       // "tapeNN.raw", NN = tape + 1, as 0x8d4fa4f8 makes it
    const char* prefix = "tape";
    while (*prefix) *name++ = *prefix++;
    *name++ = '0';
    *name++ = (char)('1' + tape);
    const char* suffix = ".raw";
    while (*suffix) *name++ = *suffix++;
    *name = 0;
}

static void say(int level, int line, const char* what, int source, int destination, u32 chunks) {
    if (!log_budget) return;
    --log_budget;
    STOCK_LOG(level, LOG_FILE, line, "tape copy: tracks 5-8 of tape %d to tape %d: %s (%d chunk(s))", source + 1, destination + 1, what, chunks);
}

// In place of the tape list's call of stock's clone.
void op1f_tape_clone(int source, int destination, ProgressFn progress, u32 fourth) {
    STOCK_CLONE(source, destination, progress, fourth);
    ++op1f_copy_counts[0];
    if (source < 0 || source >= TAPES || destination < 0 || destination >= TAPES || source == destination) return;

    XmapLoad load;
    const u16* ext = op1f_xmap_source(source, &load);
    if (!ext) {                                      // no version-1 map file: stock's clone has refused it too
        ++op1f_copy_counts[3];
        say(2, 7, "nothing done, the original's map file could not be read", source, destination, 0);
        return;
    }
    u32 chunks = 0;
    for (u32 i = 0; i < TABLE_ENTRIES; ++i)
        if (ext[i] != UNALLOCATED && ext[i] < CHUNKS_PER_PARTITION) ++chunks;
    u32 copy_crc = 0;
    int landed = op1f_xmap_file_crc(destination, &copy_crc) && copy_crc == load.stock_crc;   // the copy's map file is the original's
    if (!chunks) {                                   // four tracks, or a file of tracks 5 to 8 that is not the original's
        ++op1f_copy_counts[5];
        // a file an earlier tape left in the copy's slot is not the copy's: an empty one in its place (the note's 5.10)
        int emptied = landed && op1f_xmap_write_plain(destination, ext, load.stock_crc, 1, 0);
        say(3, 1, emptied ? "the original has none; the file an earlier tape left in the copy's slot is emptied" : "the original has none", source, destination, 0);
        return;
    }
    if (!landed) {
        ++op1f_copy_counts[3];
        say(2, 2, "NOT copied, the copy's map file is not the original's (stock's copy did not land)", source, destination, chunks);
        return;
    }
    u8* buffer = (u8*)STOCK_ALLOC_ALIGNED(BUFFER_ALIGNMENT, CHUNK_BYTES);
    if (!buffer) {
        ++op1f_copy_counts[4];
        say(1, 3, "NOT copied, no room for the chunk buffer", source, destination, chunks);
        return;
    }
    char source_name[16], copy_name[16];
    partition_name(source_name, source);
    partition_name(copy_name, destination);
    i32 from = STOCK_OPEN_PARTITION(source_name);
    i32 to = from < 0 ? -1 : STOCK_OPEN_PARTITION(copy_name);
    u32 copied = 0;
    int failed = from < 0 || to < 0;
    for (u32 i = 0; !failed && i < TABLE_ENTRIES; ++i) {
        u32 chunk = ext[i];
        if (chunk == UNALLOCATED || chunk >= CHUNKS_PER_PARTITION) continue;
        if (STOCK_PARTITION_READ(from, chunk << CHUNK_SHIFT, CHUNK_BYTES, buffer) < 0 ||
            STOCK_PARTITION_WRITE(to, chunk << CHUNK_SHIFT, CHUNK_BYTES, buffer) < 0) {
            failed = 1;
            break;
        }
        ++copied;
        if (!(copied & (PROGRESS_EVERY - 1)) && progress) progress(ONE_FLOAT);
    }
    if (to >= 0) STOCK_CLOSE_PARTITION(to);          // both closed before the copy is loaded
    if (from >= 0) STOCK_CLOSE_PARTITION(from);
    STOCK_FREE_ALIGNED(buffer);
    op1f_copy_counts[2] += copied;
    if (failed) {
        ++op1f_copy_counts[4];
        say(1, 4, "NOT copied, a partition could not be opened, read or written; the copy has tracks 1 to 4", source, destination, copied);
        return;
    }
    // K5: the original's sums go with the copy (its chunks are the original's bytes). When another firmware wrote the
    // original's tape since this pack did, and no load has put that into its file yet, the copy's chunks are to be
    // held to their sums too: the copy's own files will not say so.
    if (load.sums && load.foreign)
        for (u32 i = 0; i < TABLE_ENTRIES; ++i)
            if (op1f_xmap_entry_flags[i] == XSUM_SURE) op1f_xmap_entry_flags[i] = XSUM_HELD;
    if (!op1f_xmap_write_plain(destination, ext, load.stock_crc, 0, load.sums)) {
        ++op1f_copy_counts[4];
        say(1, 5, "the chunks are copied, but the copy's file of tracks 5 to 8 could not be written", source, destination, copied);
        return;
    }
    ++op1f_copy_counts[1];
    say(3, 6, "copied, and the copy has a file of its own", source, destination, copied);
}
