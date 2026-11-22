// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B3c: lift, drop and the range clear on tape tracks 5 and 6 (track indices 4 and 5), and the
// guard that keeps a lifted track 5/6 out of the sampler import (OS 1.7.0+0).
//
// What stock does (tape_c.cpp and audio_clipboard.cpp in record 307): a lift posts command 3 with the track
// (0x8f00c00c, which asserts track < 4 at audio_clipboard.cpp:814); the tape thread's handler 0x8f009548 and
// its worker 0x8f00a62c move the clip records through the Clips store (track-agnostic) and call the helper
// 0x8f0209f0(track, start, end): it copies the range into the track's own clipboard partition
// "clipboard/track_<n+1>.raw" (names at 0x84020e2c, 24 bytes apart; the builder 0x8f01ecec asserts track < 4),
// unmaps every whole chunk of the range from the block map (unmap_chunk 0x8fc22750) and writes zeros over the
// partial ones, marks the cache slots covering the range stale (0x8fc218f8) and syncs the partition
// (0x8d500f94). When no clip covers the 512 frames before the lift the worker first clears them with
// 0x8f01f73c(track, start - 512, start) (zeros only). A drop posts command 0 (copy) or 2 (mix, shift + drop)
// with the selected track; the handler 0x8f0098b0 and its worker 0x8f00a870 add the clip record and call
// 0x8f021038(src, dst, pos, frames) (the source partition read in pieces, written to the track) or
// 0x8f020d90(params) (the destination read, every source partition added, written back), both asserting their
// tracks < 4, then 0x8fc218f8(pos, pos + 2 x frames). The sampler import 0x8f004204 renders a tape clipboard
// into a sample and asserts the lifted track < 4 on both of its paths (Sampler.cpp:74, the mixdown's names).
//
// This unit (profile group "build-b3c", with the trampolines in build_b_edit.s and two chunk helpers in
// build_b_storage.cpp):
//   * The clipboard of track 4 + j is stock's partition "clipboard/track_<j+1>.raw". Stock keeps the lift of
//     track j there, and the two never coexist: a lift replaces the clipboard's track vector, and lift-all stays
//     four-track. The content survives tape switches and restarts like stock's; user/clipboard.json carries the
//     channel 4/5 as an int, which its loader stores back without touching a partition.
//   * op1f_lift_ext: stock's helper for tracks 4-5. The copy goes through op1f_tape_io_read_ext and the
//     partition write in 4,096-frame pieces; whole chunks are released through op1f_ext_unmap_chunk (the live
//     bit cleared, the map entry unallocated, the map dirty, as stock's unmap_chunk), partial chunks are zeroed
//     through op1f_tape_io_write_ext (skipped for an unallocated chunk, which already reads as zeros); then the
//     same cache invalidation and sync as stock.
//   * op1f_clear_range_ext: the release alone (stock's clearer writes zeros and makes no cache call).
//   * op1f_drop_copy_ext / op1f_drop_mix_ext: stock's two writers for any source or destination of 4-5, with the
//     destination read and written through the hooked tape_io entries (tracks 0-3 stock, 4-5 the module since
//     B2b) and each source read from its partition, stock's or the extension's.
//   * op1f_clip_to_sample_refused: the sampler import of a clipboard holding a track 4/5 logs and returns.
//   Every check stock asserts on is a refusal with a level-2 line here. Counters for the measurement line:
//   op1f_ext_lifts, op1f_ext_drops, op1f_ext_clears, op1f_ext_edit_refusals.
// Build D (step D1) adds: the rule for where a lifted track's audio
//   lives (stock's partition for a clipboard of one track; the files user/op1f_clip_5.raw and _6.raw for
//   tracks 5-6 of a clipboard of several, since a lift-all can hold j and 4 + j together), the clipboard's
//   effective volume, pan and clip record of tracks 5-6 (written by build_d_clip.cpp's lift-all, read by the
//   mix drop and the sampler render), and op1f_clip_to_sample_ext in place of the refusal: stock's two
//   renders of a clipboard into a sample, for any vector that holds a track of 4 or more.
// Freestanding like the other units: no library, no division, no memset/memcpy of our own (the ROM memset by
// address). Everything here runs on the tape thread (the command handlers), so logging is allowed; the lines
// are budgeted.

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef i32 (*OpenPartitionFn)(const char* name);                       // handle, or -1 (stock logs the failure)
typedef i32 (*PartitionIoFn)(i32 handle, u32 byte_offset, u32 length, void* buffer);   // negative on error
typedef void (*ClosePartitionFn)(i32 handle);
typedef void (*VoidFn)();
typedef i32 (*TapeIoFn)(u32 track, u32 byte_offset, u32 length, void* buffer);        // bytes done, or -1
typedef void (*InvalidateFn)(i32 start, i32 end);
typedef void* (*MemsetFn)(void* destination, int value, u32 count);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_OPEN_PARTITION ((OpenPartitionFn)0x8d501008u)             // as the lift helper calls it at 0x8f01ef6c
#define STOCK_PARTITION_READ ((PartitionIoFn)0x8d500688u)
#define STOCK_PARTITION_WRITE ((PartitionIoFn)0x8d500b88u)
#define STOCK_CLOSE_PARTITION ((ClosePartitionFn)0x8d500438u)
#define STOCK_PARTITION_SYNC ((VoidFn)0x8d500f94u)                     // after the lift's writes, as at 0x8f01f1de
#define STOCK_TAPE_IO_READ ((TapeIoFn)0x8fc21bacu)                      // the hooked entries: 0-3 stock, 4-5 the module
#define STOCK_TAPE_IO_WRITE ((TapeIoFn)0x8fc21d1cu)
#define STOCK_INVALIDATE_RANGE ((InvalidateFn)0x8fc218f8u)              // cache slots overlapping [start, end] become stale
#define STOCK_MEMSET ((MemsetFn)0x04022ea8u)
#define STOCK_CLIPBOARD_NAMES ((const char*)0x84020e2cu)                // "clipboard/track_1.raw" .. "_4.raw"
#define CLIPBOARD_VECTOR_BEGIN (*(i32* volatile*)0x84bdd948u)           // the state's vector of lifted tracks
#define CLIPBOARD_VECTOR_END (*(i32* volatile*)0x84bdd94cu)

// Build D: stock's file API (disk_io.h, as build_b_storage.cpp uses it for the map files) and the sample
// object the sampler import renders into.
typedef i32 (*FopenFn)(const char* path, const char* mode);             // a descriptor: 0 or more, negative when the open fails (K3: it had
typedef i32 (*FreadFn)(void* destination, i32 size, i32 count, i32 file);   // been taken for a pointer and tested against zero since D1, as the
typedef i32 (*FwriteFn)(const void* source, i32 size, i32 count, i32 file); // map code had been until K1b: a descriptor 0 read as a failure, a
typedef void (*FcloseFn)(i32 file);                                     // failed open as a file)
typedef void (*SampleFinalizeFn)(void* sample);
#define STOCK_FOPEN ((FopenFn)0x8fc22180u)
#define STOCK_FREAD ((FreadFn)0x8fc2214cu)                              // the item count
#define STOCK_FWRITE ((FwriteFn)0x8fc220b8u)
#define STOCK_FCLOSE ((FcloseFn)0x8fc220a0u)
#define STOCK_SAMPLE_FINALIZE ((SampleFinalizeFn)0x8f0037beu)           // the overview the sampler draws, as both stock renders end
#define STATE_LENGTH (*(volatile i32*)0x84bdd9e4u)                      // the clipboard state: the length in frames
#define STATE_VOLUMES ((const volatile i32*)0x84bdd954u)                // its four effective volumes (Q31) and pans
#define STATE_PANS ((const volatile i32*)0x84bdd964u)
#define SAMPLE_CAPACITY_OFFSET 0xd768                                   // the sample object: +0 the 16-bit data, the capacity in
#define SAMPLE_STEREO_OFFSET 0xd770                                     // frames, the stereo byte

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                   // F2: tracks 5-8 (6 from B2 to F1b)
    FRAME_BYTES = 8,
    CHUNK_FRAMES = 16384,
    CHUNK_SHIFT = 14,
    MAX_FRAME = 0x00f23fa0,             // kMaxTapePos, as the posters bound the ranges
    PIECE_FRAMES = 4096,                // 32 KiB per stage
    NAME_STRIDE = 24,
    HALF = 0x40000000,                  // 0.5 in Q31: the pan range
    LOG_BUDGET = 32
};
static const char LOG_FILE[] = "../src/dsp/op1f_tape_edit.cpp";

extern "C" i32 op1f_tape_io_read_ext(u32 track, u32 byte_offset, u32 length, unsigned char* buffer) asm("op1f_tape_io_read_ext");
extern "C" i32 op1f_tape_io_write_ext(u32 track, u32 byte_offset, u32 length, const unsigned char* buffer) asm("op1f_tape_io_write_ext");
extern "C" int op1f_ext_chunk_mapped(u32 track, u32 logical) asm("op1f_ext_chunk_mapped");     // build_b_storage.cpp (B3c)
extern "C" void op1f_ext_unmap_chunk(u32 track, u32 logical) asm("op1f_ext_unmap_chunk");

extern "C" u32 op1f_ext_lifts asm("op1f_ext_lifts");
extern "C" u32 op1f_ext_drops asm("op1f_ext_drops");
extern "C" u32 op1f_ext_clears asm("op1f_ext_clears");
extern "C" u32 op1f_ext_edit_refusals asm("op1f_ext_edit_refusals");
u32 op1f_ext_lifts;
u32 op1f_ext_drops;
u32 op1f_ext_clears;
u32 op1f_ext_edit_refusals;

static i32 stage_a[PIECE_FRAMES * 2] __attribute__((aligned(32)));   // the destination, or the lifted frames
static i32 stage_b[PIECE_FRAMES * 2] __attribute__((aligned(32)));   // a source, or zeros
static u32 edit_log_budget = LOG_BUDGET;
static u32 refusal_log_budget = LOG_BUDGET;

static int extension_track(u32 track) {
    return track >= STOCK_TRACKS && track < EIGHT_TRACKS;
}

static const char* clipboard_name(u32 track) {          // tracks 0-3 their own partitions; 4-5 the first two
    u32 slot = track < STOCK_TRACKS ? track : track - STOCK_TRACKS;
    return STOCK_CLIPBOARD_NAMES + slot * NAME_STRIDE;
}

// Build D: where a lifted track's audio lives. Stock keeps track j in clipboard/track_<j+1>.raw. A clipboard
// of one extension track reuses stock track j's partition (B3c: a lift replaces the vector, so the two never
// meet). A clipboard of several tracks (a lift-all) can hold j and 4 + j together, so a track 5-6 of such a
// clipboard lives in a file on the user file system, user/op1f_clip_5.raw or _6.raw, through stock's
// fopen/fread/fwrite/fclose. The rule reads the state's vector, which every handler sets before the workers
// run. Readers and writers here go in pieces, in order, from offset 0, which is all a file needs; a source
// table holds one open store per track.
enum { EXT_TRACKS = EIGHT_TRACKS - STOCK_TRACKS };                   // F2: four extension tracks
extern "C" i32 op1f_ext_clip_vol[EXT_TRACKS] asm("op1f_ext_clip_vol");       // the effective volume (volume x mute gain) and pan
extern "C" i32 op1f_ext_clip_pan[EXT_TRACKS] asm("op1f_ext_clip_pan");       // of tracks 5-8 as the last lift-all found them
extern "C" i32 op1f_ext_clip_records[EXT_TRACKS][7] asm("op1f_ext_clip_records");   // their 28-byte clip records from that lift-all
extern "C" u32 op1f_ext_renders asm("op1f_ext_renders");
i32 op1f_ext_clip_vol[EXT_TRACKS] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
i32 op1f_ext_clip_pan[EXT_TRACKS];
// K4: the four records start as stock's own four do. The clipboard state's constructor 0x8f008690 runs the record's
// default constructor 0x8d4eccc0 on each of its slots: track 0, the four paddings -1, start and stop 0. Until K4
// these were zeroed memory, and a drop copies the record's paddings into the new clip (0x8f00a870), so the clips
// of tracks 5 to 8 dropped by a drop of several tracks after a restart carried the paddings 0, 0, 0, 0: a number
// the crossfade store is then asked for (0x8f00cda4; below 0 is "none"). Found in the K3 read-out.
i32 op1f_ext_clip_records[EXT_TRACKS][7] = { { 0, -1, -1, -1, -1, 0, 0 }, { 0, -1, -1, -1, -1, 0, 0 },
                                             { 0, -1, -1, -1, -1, 0, 0 }, { 0, -1, -1, -1, -1, 0, 0 } };
u32 op1f_ext_renders;
static const char* const CLIP_FILE_NAMES[EXT_TRACKS] = { "user/op1f_clip_5.raw", "user/op1f_clip_6.raw", "user/op1f_clip_7.raw", "user/op1f_clip_8.raw" };

struct ClipSource { u32 kind; i32 handle; i32 file; u32 position; };     // kind: 0 closed, 1 a partition, 2 a file (its descriptor)
static ClipSource sources[EIGHT_TRACKS];
static u32 store_log_budget = LOG_BUDGET;

static u32 clipboard_count() {
    const i32* begin = CLIPBOARD_VECTOR_BEGIN;
    const i32* end = CLIPBOARD_VECTOR_END;
    return (begin && end >= begin) ? (u32)(end - begin) : 0;
}

static const char* clip_file_name(u32 track) {           // tracks 4-7: user/op1f_clip_5.raw .. _8.raw
    u32 j = track - STOCK_TRACKS;
    return CLIP_FILE_NAMES[j < EXT_TRACKS ? j : 0];
}

static int source_is_open(u32 track) {
    return track < EIGHT_TRACKS && sources[track].kind != 0;
}

static int source_open(u32 track, int write) {
    if (track >= EIGHT_TRACKS) return 0;
    ClipSource* s = &sources[track];
    s->kind = 0;
    s->position = 0;
    if (extension_track(track) && clipboard_count() > 1) {
        s->file = STOCK_FOPEN(clip_file_name(track), write ? "w+b" : "rb");
        if (s->file < 0) {                                              // stock has logged the open that failed
            if (store_log_budget) {
                --store_log_budget;
                STOCK_LOG(2, LOG_FILE, 4, "clipboard file %s could not be opened for %s", clip_file_name(track), write ? "writing" : "reading");
            }
            return 0;
        }
        s->kind = 2;
        return 1;
    }
    s->handle = STOCK_OPEN_PARTITION(clipboard_name(track));
    if (s->handle < 0) return 0;                                       // stock logs the failure
    s->kind = 1;
    return 1;
}

static i32 source_read(u32 track, u32 offset, u32 length, void* buffer) {
    ClipSource* s = &sources[track];
    if (s->kind == 1) return STOCK_PARTITION_READ(s->handle, offset, length, buffer);
    if (s->kind != 2 || offset != s->position) return -1;
    if (STOCK_FREAD(buffer, 1, (i32)length, s->file) != (i32)length) return -1;
    s->position += length;
    return (i32)length;
}

static i32 source_write(u32 track, u32 offset, u32 length, const void* buffer) {
    ClipSource* s = &sources[track];
    if (s->kind == 1) return STOCK_PARTITION_WRITE(s->handle, offset, length, (void*)buffer);
    if (s->kind != 2 || offset != s->position) return -1;
    if (STOCK_FWRITE(buffer, 1, (i32)length, s->file) != (i32)length) return -1;
    s->position += length;
    return (i32)length;
}

static void source_close(u32 track) {
    if (track >= EIGHT_TRACKS) return;
    ClipSource* s = &sources[track];
    if (s->kind == 1) STOCK_CLOSE_PARTITION(s->handle);
    else if (s->kind == 2) STOCK_FCLOSE(s->file);
    s->kind = 0;
}

static void refuse(const char* what, u32 track, i32 a, i32 b) {
    ++op1f_ext_edit_refusals;
    if (refusal_log_budget) {
        --refusal_log_budget;
        STOCK_LOG(2, LOG_FILE, 1, "%s refused on track %d (frames %d..%d)", what, track + 1, a, b);
    }
}

static void note(const char* what, u32 track, i32 a, i32 b, u32 source) {
    if (edit_log_budget) {
        --edit_log_budget;
        STOCK_LOG(3, LOG_FILE, 2, "%s: track %d frames %d..%d (clipboard of track %d)", what, track + 1, a, b, source + 1);
    }
}

// The instructions stock's mix loop uses (the same helpers as build_b_play.cpp and build_b_xfade.cpp; the
// module links no libgcc, so no 64-bit arithmetic).
static inline i32 mult_t(i32 a, i32 b) {                            // R = Ra * Rb (T): fraction multiply, truncated
    i32 r;
    asm("%0 = %1 * %2 (T);" : "=d"(r) : "d"(a), "d"(b));
    return r;
}

static inline i32 add_s(i32 a, i32 b) {                             // R = Ra + Rb (S): saturating add
    i32 r;
    asm("%0 = %1 + %2 (S);" : "=d"(r) : "d"(a), "d"(b));
    return r;
}

static inline i32 sub_s(i32 a, i32 b) {                             // R = Ra - Rb (S): saturating subtract
    i32 r;
    asm("%0 = %1 - %2 (S);" : "=d"(r) : "d"(a), "d"(b));
    return r;
}

static inline i32 shl1_s(i32 a) {                                   // R = Ra << 1 (S): saturating doubling
    i32 r;
    asm("%0 = %1 << 1 (S);" : "=d"(r) : "d"(a));
    return r;
}

// Zeros over [from, to) of an extension track through the extension write (allocating and copying as a write does).
static int zero_frames(u32 track, i32 from, i32 to) {
    STOCK_MEMSET(stage_b, 0, sizeof stage_b);
    while (from < to) {
        u32 n = (u32)(to - from);
        if (n > PIECE_FRAMES) n = PIECE_FRAMES;
        if (op1f_tape_io_write_ext(track, (u32)from * FRAME_BYTES, n * FRAME_BYTES, (const unsigned char*)stage_b) < 0) return 0;
        from += (i32)n;
    }
    return 1;
}

// Removes [start, end) from an extension track as stock's lift helper removes it from its own: a chunk the range
// covers entirely is unmapped, a chunk it covers in part is zeroed over that part (nothing to do when the chunk
// is not allocated, as it already reads as zeros).
static void release_range(u32 track, i32 start, i32 end) {
    i32 frame = start;
    while (frame < end) {
        u32 logical = (u32)frame >> CHUNK_SHIFT;
        i32 chunk_start = (i32)(logical << CHUNK_SHIFT);
        i32 chunk_end = chunk_start + CHUNK_FRAMES;
        i32 to = end < chunk_end ? end : chunk_end;
        if (frame == chunk_start && to == chunk_end) op1f_ext_unmap_chunk(track, logical);
        else if (op1f_ext_chunk_mapped(track, logical) && !zero_frames(track, frame, to)) return;
        frame = to;
    }
}

// Stock 0x8f0209f0(track, start, end) for tracks 4-5, reached through the entry trampoline op1f_lift.
extern "C" void op1f_lift_ext(u32 track, i32 start, i32 end) asm("op1f_lift_ext");
extern "C" void op1f_lift_ext(u32 track, i32 start, i32 end) {
    if (!extension_track(track) || start < 0 || !(start < end) || end > MAX_FRAME) {
        refuse("lift", track, start, end);
        return;
    }
    if (!source_open(track, 1)) {
        refuse("lift (no clipboard store)", track, start, end);
        return;
    }
    u32 frames = (u32)(end - start);
    u32 done = 0;
    int copied = 1;
    while (done < frames) {
        u32 n = frames - done;
        if (n > PIECE_FRAMES) n = PIECE_FRAMES;
        if (op1f_tape_io_read_ext(track, (u32)start * FRAME_BYTES + done * FRAME_BYTES, n * FRAME_BYTES, (unsigned char*)stage_a) < 0 ||
            source_write(track, done * FRAME_BYTES, n * FRAME_BYTES, stage_a) < 0) {
            copied = 0;
            break;
        }
        done += n;
    }
    source_close(track);
    if (copied) release_range(track, start, end);                    // a failed copy leaves the audio on the tape
    else refuse("lift (copy failed, audio kept)", track, start, end);
    STOCK_INVALIDATE_RANGE(start, end);
    STOCK_PARTITION_SYNC();
    ++op1f_ext_lifts;
    note("lift", track, start, end, track);
}

// Stock 0x8f01f73c(track, start, end) for tracks 4-5, reached through op1f_clear_range.
extern "C" void op1f_clear_range_ext(u32 track, i32 start, i32 end) asm("op1f_clear_range_ext");
extern "C" void op1f_clear_range_ext(u32 track, i32 start, i32 end) {
    if (!extension_track(track) || start < 0 || !(start < end) || end > MAX_FRAME) {
        refuse("clear", track, start, end);
        return;
    }
    release_range(track, start, end);
    ++op1f_ext_clears;
    note("clear", track, start, end, track);
}

// Stock 0x8f021038(src, dst, pos, frames) when src or dst is 4-5, reached through op1f_drop_copy.
extern "C" void op1f_drop_copy_ext(u32 src, u32 dst, i32 pos, i32 frames) asm("op1f_drop_copy_ext");
extern "C" void op1f_drop_copy_ext(u32 src, u32 dst, i32 pos, i32 frames) {
    if (src >= EIGHT_TRACKS || dst >= EIGHT_TRACKS || pos < 0 || frames <= 0 || pos >= MAX_FRAME) {
        refuse("drop", dst, pos, pos + frames);
        return;
    }
    if (frames > MAX_FRAME - pos) frames = MAX_FRAME - pos;
    if (!source_open(src, 0)) {
        refuse("drop (no clipboard store)", dst, pos, pos + frames);
        return;
    }
    u32 total = (u32)frames;
    u32 done = 0;
    while (done < total) {
        u32 n = total - done;
        if (n > PIECE_FRAMES) n = PIECE_FRAMES;
        if (source_read(src, done * FRAME_BYTES, n * FRAME_BYTES, stage_a) < 0) break;
        if (STOCK_TAPE_IO_WRITE(dst, (u32)pos * FRAME_BYTES + done * FRAME_BYTES, n * FRAME_BYTES, stage_a) < 0) break;
        done += n;
    }
    source_close(src);
    STOCK_INVALIDATE_RANGE(pos, pos + 2 * frames);                    // twice the length, as stock
    ++op1f_ext_drops;
    note("drop", dst, pos, pos + frames, src);
}

// Stock 0x8f020d90(params) when the destination or a source is 4-5, reached through op1f_drop_mix. params:
// +0 the clipboard state (its vector at +4/+8), +4 dst, +8 pos, +0xc frames, +0x10 the volumes, +0x14 the pans.
extern "C" void op1f_drop_mix_ext(const i32* params) asm("op1f_drop_mix_ext");
extern "C" void op1f_drop_mix_ext(const i32* params) {
    const i32* state = (const i32*)params[0];
    u32 dst = (u32)params[1];
    i32 pos = params[2];
    i32 frames = params[3];
    const i32* vols = (const i32*)params[4];
    const i32* pans = (const i32*)params[5];
    const i32* begin = state ? (const i32*)state[1] : 0;
    const i32* end = state ? (const i32*)state[2] : 0;
    if (dst >= EIGHT_TRACKS || pos < 0 || frames <= 0 || pos >= MAX_FRAME || !vols || !pans || !begin || end < begin) {
        refuse("drop mix", dst, pos, pos + frames);
        return;
    }
    if (frames > MAX_FRAME - pos) frames = MAX_FRAME - pos;
    u32 count = (u32)(end - begin);
    u32 total = (u32)frames;
    u32 done = 0;
    u32 first_source = count ? (u32)begin[0] : dst;
    for (const i32* p = begin; p != end; ++p)                          // Build D: every source opened once, read in order
        if ((u32)*p < EIGHT_TRACKS) source_open((u32)*p, 0);
    while (done < total) {
        u32 n = total - done;
        if (n > PIECE_FRAMES) n = PIECE_FRAMES;
        u32 dst_offset = (u32)pos * FRAME_BYTES + done * FRAME_BYTES;
        if (STOCK_TAPE_IO_READ(dst, dst_offset, n * FRAME_BYTES, stage_a) < 0) break;
        for (const i32* p = begin; p != end; ++p) {
            u32 src = (u32)*p;
            if (!source_is_open(src)) continue;
            if (source_read(src, done * FRAME_BYTES, n * FRAME_BYTES, stage_b) < 0) {
                source_close(src);                                     // a source that fails contributes nothing further
                continue;
            }
            if (count <= 1) {                                          // stock: a single source is added as it is
                for (u32 i = 0; i < n * 2; ++i) stage_a[i] = add_s(stage_a[i], stage_b[i]);
            } else {                                                   // stock's gains: volume x pan law, per source
                i32 vol = src < STOCK_TRACKS ? vols[src] : op1f_ext_clip_vol[src - STOCK_TRACKS];
                i32 pan = src < STOCK_TRACKS ? pans[src] : op1f_ext_clip_pan[src - STOCK_TRACKS];
                i32 left = mult_t(vol, shl1_s(sub_s(HALF, pan)));
                i32 right = mult_t(vol, shl1_s(add_s(pan, HALF)));
                for (u32 i = 0; i < n; ++i) {
                    stage_a[2 * i] = add_s(stage_a[2 * i], mult_t(stage_b[2 * i], left));
                    stage_a[2 * i + 1] = add_s(stage_a[2 * i + 1], mult_t(stage_b[2 * i + 1], right));
                }
            }
        }
        if (STOCK_TAPE_IO_WRITE(dst, dst_offset, n * FRAME_BYTES, stage_a) < 0) break;
        done += n;
    }
    for (const i32* p = begin; p != end; ++p) source_close((u32)*p);
    STOCK_INVALIDATE_RANGE(pos, pos + 2 * frames);
    ++op1f_ext_drops;
    note("drop mix", dst, pos, pos + frames, first_source);
}

// Build D: the sampler import 0x8f004204 with a clipboard that holds a track of 4 or more (op1f_clip_to_sample
// found it in the state's vector), stock's two renders in one. The sample object: +0 the 16-bit data, +0xd768
// the capacity in frames, B[+0xd770] stereo. One lifted track is copied as stock's 0x8f002abc copies it (the
// high halves of the 32-bit frames; mono = (L >> 1 + R >> 1) saturated, then the high half); several are
// mixed as stock's 0x8f002ccc mixes them: the destination zeroed over the frames rendered, each source scaled
// by the clipboard's effective volume and pan law (left = vol x sat((1/2 - pan) << 1), right = vol x
// sat((pan + 1/2) << 1); stock's state arrays for 0-3, op1f_ext_clip_vol/pan for 4-5), the high half of each
// product added into the 16-bit sum with saturation; mono halves both products first. Then stock's finalize.
// The callers ignore the return value (1 rendered, 0 refused).
static short to16(i32 x) {                                          // W[P] = R.H
    return (short)(x >> 16);
}

static short acc16(short acc, i32 x) {                              // R0.L = Rx.H + R0.L (S)
    i32 s = (i32)acc + (x >> 16);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    return (short)s;
}

extern "C" i32 op1f_clip_to_sample_ext(void* sample) asm("op1f_clip_to_sample_ext");
extern "C" i32 op1f_clip_to_sample_ext(void* sample) {
    const i32* begin = CLIPBOARD_VECTOR_BEGIN;
    const i32* end = CLIPBOARD_VECTOR_END;
    u32 count = clipboard_count();
    short* data = sample ? *(short**)sample : 0;
    if (!data || count == 0) {
        ++op1f_ext_edit_refusals;
        if (refusal_log_budget) {
            --refusal_log_budget;
            STOCK_LOG(2, LOG_FILE, 3, "clipboard to sample refused: %s", data ? "an empty clipboard" : "no sample data");
        }
        return 0;
    }
    i32 capacity = *(const i32*)((const unsigned char*)sample + SAMPLE_CAPACITY_OFFSET);
    int stereo = *((const unsigned char*)sample + SAMPLE_STEREO_OFFSET) != 0;
    u32 channels = stereo ? 2 : 1;
    i32 length = STATE_LENGTH;
    if (length > capacity) length = capacity;
    if (length < 0) length = 0;
    STOCK_MEMSET(data, 0, (u32)length * 2 * channels);
    u32 opened = 0;
    for (const i32* p = begin; p != end; ++p)
        if ((u32)*p < EIGHT_TRACKS && source_open((u32)*p, 0)) ++opened;
    u32 done = 0;
    while (opened && done < (u32)length) {
        u32 n = (u32)length - done;
        if (n > PIECE_FRAMES) n = PIECE_FRAMES;
        short* out = data + done * channels;
        for (const i32* p = begin; p != end; ++p) {
            u32 src = (u32)*p;
            if (!source_is_open(src)) continue;
            if (source_read(src, done * FRAME_BYTES, n * FRAME_BYTES, stage_b) < 0) {
                source_close(src);
                continue;
            }
            if (count <= 1) {                                          // stock's single-track render
                if (stereo) {
                    for (u32 i = 0; i < n; ++i) {
                        out[2 * i] = to16(stage_b[2 * i]);
                        out[2 * i + 1] = to16(stage_b[2 * i + 1]);
                    }
                } else {
                    for (u32 i = 0; i < n; ++i) out[i] = to16(add_s(stage_b[2 * i] >> 1, stage_b[2 * i + 1] >> 1));
                }
            } else {                                                   // stock's mixdown
                i32 vol = src < STOCK_TRACKS ? STATE_VOLUMES[src] : op1f_ext_clip_vol[src - STOCK_TRACKS];
                i32 pan = src < STOCK_TRACKS ? STATE_PANS[src] : op1f_ext_clip_pan[src - STOCK_TRACKS];
                i32 left = mult_t(vol, shl1_s(sub_s(HALF, pan)));
                i32 right = mult_t(vol, shl1_s(add_s(pan, HALF)));
                if (stereo) {
                    for (u32 i = 0; i < n; ++i) {
                        out[2 * i] = acc16(out[2 * i], mult_t(stage_b[2 * i], left));
                        out[2 * i + 1] = acc16(out[2 * i + 1], mult_t(stage_b[2 * i + 1], right));
                    }
                } else {
                    for (u32 i = 0; i < n; ++i)
                        out[i] = acc16(out[i], add_s(mult_t(stage_b[2 * i], left) >> 1, mult_t(stage_b[2 * i + 1], right) >> 1));
                }
            }
        }
        done += n;
    }
    for (const i32* p = begin; p != end; ++p) source_close((u32)*p);
    STOCK_SAMPLE_FINALIZE(sample);
    ++op1f_ext_renders;
    if (edit_log_budget) {
        --edit_log_budget;
        STOCK_LOG(3, LOG_FILE, 5, "clipboard to sample: %d of %d frames from %u track(s), %s, %u store(s) opened (first track %d)",
                  length, STATE_LENGTH, count, stereo ? "stereo" : "mono", opened, (int)begin[0] + 1);
    }
    return 1;
}
