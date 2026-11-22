// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B3b: the loop crossfade of tape tracks 5 and 6, the crossfade masks for six tracks, and the
// clip-edge fade writers for tracks 5 and 6 (OS 1.7.0+0).
//
// What stock does (tape_xfade.cpp in record 309, tape_c.cpp in record 307):
//   * Four loop buffers of 594 frames (4,752 bytes each, from the L2 allocator; pointers at 0x0802fe60) hold,
//     per track, the 297 frames before loop_b followed by the 297 frames from loop_a. The compute
//     0x8fc21a38(loop_a, loop_b), called from read_block while the loop flag B[0x8400129d] is set and the valid
//     mask [0x848d07c0] is not 0xF, needs the cache slots of the chunks at loop_a, loop_a + 297, loop_b - 1 and
//     loop_b - 298 (else it logs and keeps the dirty mask); it copies the two halves for every track in the
//     dirty mask [0x848d07c4] (then cleared; the source frame is clamped at loop_b, so a loop shorter than 297
//     frames repeats that frame), tests every track's junction (the error of predicting each side from the
//     other against the signal's own steps plus the jump itself), fades the last 32 frames of the first half
//     out and the first 32 of the second half in for the dirty tracks whose junction fails the test, and sets
//     the valid bits. read_block then serves the frames near the loop points from the buffer while the flag is
//     set: index = frame - (loop_b - 297) from loop_b - 297 on, so frames past loop_b continue into the second
//     half, i.e. into the loop start; 297 + (frame - loop_a) from loop_a on. With the flag set it also wraps
//     the window start into [loop_a, loop_b) before reading.
//   * xfade_mark_loaded_range(start, end, loop_a, loop_b, mask) 0x8fc2337c: when a loop point, or a loop point
//     +-297, lies in [start, end], or the range lies within 297 frames after loop_a or before loop_b,
//     dirty |= mask; then valid &= ~dirty. Called by chunks_service after a reload (mask 0xF) and by the L1
//     record function with 1 << track for the recorded range. xfade_mark_dirty(mask) 0x8fc23458: dirty |= mask,
//     valid &= ~dirty, called with 0xF from the loop-point and punch logic (0x8f01e67a, 0x8f01e6f0, 0x8f01e842,
//     0x8f01e968, 0x8f01eaf8).
//   * The clip-edge helpers 0x8f00a4cc / 0x8f00a584 (record stop; B2b let them return for tracks 4-5) blend a
//     new clip's edges with the "outside" paddings of types 2 and 3 through 0x8f01dec4, which is track-agnostic
//     (malloc, tape_io_read, a linear ramp, tape_io_write, free). When a padding is missing, which is the case
//     for every clip recorded so far (paddings [id, id, -1, -1]), they fall back to 0x8f009130(track, clip,
//     mode): mode 0 fades the clip's first 128 frames in (0x8f01f8e0(track, start, start + 128)), mode 1 fades
//     its last 256 frames out (0x8f01f7d0(track, stop - 256, stop)), in place on the tape. Both writers assert
//     track < 4 (0x8f01dd00, 0x8f01de0e; fatal) and are otherwise track-agnostic: malloc, read through the
//     tape_c wrapper 0x8f01dc24, gain x sample with (T) multiplies and a gain step of 0x7fffffff / frames
//     applied with (S) adds, write through 0x8f01dc38, free.
//
// This unit (profile group "build-b3b", with the trampolines in build_b_xfade.s and changes in
// build_b_play.cpp, build_b_storage.cpp and build_b_record.s):
//   * op1f_xfade_mark_loaded_range and op1f_xfade_mark_dirty replace the two stock functions (JUMP.a at their
//     entries) with the same tests for every caller: bits 0-3 of the mask go to stock's two words exactly as
//     before, bits 4-5 to this unit's masks for tracks 5-6 (op1f_chunks_service now passes 0x3f). Stock's words
//     never see bits 4-5: stock's compute would copy a dirty bit 4 into its valid mask, where "== 0xF" would
//     fail from then on and the compute would run every block.
//   * op1f_ext_xfade_compute(loop_a, loop_b): stock's compute for the two extension tracks with this unit's
//     buffers and masks, called from op1f_read_block_ext after stock's read_block when the flag is set and not
//     both buffers are valid. op1f_ext_loop_buffer and op1f_ext_xfade_valid are read by the window fill in
//     build_b_play.cpp.
//   * op1f_fade_out_ext / op1f_fade_in_ext: the two fade writers for tracks 4-5, reached through the entry
//     trampolines in build_b_xfade.s (tracks 0-3 continue in stock), reading and writing through the
//     extension-map tape I/O of build_b_storage.cpp in pieces of 512 frames with the same gain arithmetic; a
//     range stock would assert on is refused with a log line.
// Freestanding like the other units: no library, no division operator, no memset/memcpy. The compute runs on
// the audio thread (as stock's), the fade writers on the tape thread (the record-stop handler); the log lines
// of the compute are budgeted.

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;

struct Slot {                 // tape_chunks.cpp slot descriptor, 28 bytes
    unsigned char valid;
    unsigned char stale;
    unsigned char write_pending;
    unsigned char pad;
    u16 dirty_tracks;
    short logical_chunk;
    u16 index;
    u16 pad2;
    i32 frame_pos;
    unsigned char* data;
    Slot* next;
    Slot* prev;
};
typedef Slot* (*SlotForLogicalFn)(int logical);
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);

#define STOCK_SLOT_FOR_LOGICAL ((SlotForLogicalFn)0x8fc21280u)   // slot_by_logical[l], or 0
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_XFADE_DIRTY (*(volatile u32*)0x848d07c4u)         // tracks 0-3: the loop buffer needs a recompute (init 0xF)
#define STOCK_XFADE_VALID (*(volatile u32*)0x848d07c0u)         // tracks 0-3: the loop buffer is computed

extern "C" i32 op1f_tape_io_read_ext(u32 track, u32 byte_offset, u32 length, unsigned char* buffer) asm("op1f_tape_io_read_ext");
extern "C" i32 op1f_tape_io_write_ext(u32 track, u32 byte_offset, u32 length, const unsigned char* buffer) asm("op1f_tape_io_write_ext");

enum {
    EXT_TRACKS = 4,                        // F2: tracks 5-8 (2 until F1b)
    STOCK_TRACKS = 4,
    STOCK_MASK = 0xf,
    EXT_MASK = 0xf,
    XFADE_FRAMES = 297,                    // stock's window on each side of a loop point
    LOOP_BUFFER_FRAMES = 2 * XFADE_FRAMES, // 594 frames: the frames before loop_b, then the frames from loop_a
    FADE_FRAMES = 32,
    FADE_STEP = 0x04000000,                // 1/32 in Q31: the fade-out's decrement
    FADE_RISE = 0x03ffffff,                // stock's increment on the way back up
    CHUNK_FRAMES = 16384,
    CHUNK_BYTES = 0x20000,
    FRAME_BYTES = 8,
    STAGE_FRAMES = 512,                    // the fade writers work in pieces of this size
    UNIT = 0x7fffffff,
    WAIT_LOG_BUDGET = 4,
    EDGE_LOG_BUDGET = 8
};
static const char LOG_FILE[] = "../src/dsp/op1f_tape_xfade.cpp";

extern "C" i32 op1f_ext_loop_buffer[EXT_TRACKS][LOOP_BUFFER_FRAMES * 2] asm("op1f_ext_loop_buffer");
i32 op1f_ext_loop_buffer[EXT_TRACKS][LOOP_BUFFER_FRAMES * 2];      // interleaved left, right per frame
extern "C" u32 op1f_ext_xfade_valid asm("op1f_ext_xfade_valid");   // bit j: the buffer of track 4 + j is computed
extern "C" u32 op1f_ext_xfade_dirty asm("op1f_ext_xfade_dirty");   // bit j: it needs a recompute
u32 op1f_ext_xfade_valid;
u32 op1f_ext_xfade_dirty = EXT_MASK;                                // as stock's initial 0xF

// Counters for the measurement line (the first frame hook's measuring unit, weak references).
extern "C" u32 op1f_ext_xfade_computes asm("op1f_ext_xfade_computes");   // computes that completed
extern "C" u32 op1f_ext_xfade_fades asm("op1f_ext_xfade_fades");         // junction fades applied, per track
extern "C" u32 op1f_ext_xfade_waits asm("op1f_ext_xfade_waits");         // computes that found a chunk missing
extern "C" u32 op1f_ext_edge_fades asm("op1f_ext_edge_fades");           // clip-edge fade writes on tracks 5-6
u32 op1f_ext_xfade_computes;
u32 op1f_ext_xfade_fades;
u32 op1f_ext_xfade_waits;
u32 op1f_ext_edge_fades;

static u32 wait_log_budget = WAIT_LOG_BUDGET;
static u32 edge_log_budget = EDGE_LOG_BUDGET;
static unsigned char stage[STAGE_FRAMES * FRAME_BYTES] __attribute__((aligned(32)));

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

static inline i32 abs_s(i32 a) {                                    // R = ABS Ra: saturating absolute value
    i32 r;
    asm("%0 = ABS %1;" : "=d"(r) : "d"(a));
    return r;
}

static inline i32 min_i(i32 a, i32 b) {
    return a < b ? a : b;
}

// Restoring shift-subtract division (no libgcc routine); d == 0 yields 0.
static u32 udiv(u32 n, u32 d) {
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

// ---------------------------------------------------------------------------------------------------------
// The two mask functions, stock's logic (0x8fc2337c..0x8fc21965, 0x8fc23458..0x8fc2199f) for six tracks.

static int range_touches_loop(i32 start, i32 end, i32 loop_a, i32 loop_b) {   // stock's six tests, in order
    i32 after_a = loop_a + XFADE_FRAMES;
    i32 before_b = loop_b - XFADE_FRAMES;
    if (start < loop_a && loop_a <= end) return 1;
    if (start < loop_b && loop_b <= end) return 1;
    if (start <= after_a && after_a < end) return 1;
    if (start < before_b && before_b <= end) return 1;
    if (loop_a <= start && end <= after_a) return 1;
    if (before_b <= start && end < loop_b) return 1;
    return 0;
}

extern "C" void op1f_xfade_mark_loaded_range(i32 start, i32 end, i32 loop_a, i32 loop_b, u32 mask) asm("op1f_xfade_mark_loaded_range");
extern "C" void op1f_xfade_mark_loaded_range(i32 start, i32 end, i32 loop_a, i32 loop_b, u32 mask) {
    if (range_touches_loop(start, end, loop_a, loop_b)) {
        STOCK_XFADE_DIRTY |= mask & STOCK_MASK;
        op1f_ext_xfade_dirty |= (mask >> STOCK_TRACKS) & EXT_MASK;
    }
    STOCK_XFADE_VALID &= ~STOCK_XFADE_DIRTY;
    op1f_ext_xfade_valid &= ~op1f_ext_xfade_dirty;
}

extern "C" void op1f_xfade_mark_dirty(u32 mask) asm("op1f_xfade_mark_dirty");
extern "C" void op1f_xfade_mark_dirty(u32 mask) {
    u32 ext = (mask >> STOCK_TRACKS) & EXT_MASK;
    if ((mask & STOCK_MASK) == STOCK_MASK) ext = EXT_MASK;         // "every track" covers the extension tracks too
    STOCK_XFADE_DIRTY |= mask & STOCK_MASK;
    op1f_ext_xfade_dirty |= ext;
    STOCK_XFADE_VALID &= ~STOCK_XFADE_DIRTY;
    op1f_ext_xfade_valid &= ~op1f_ext_xfade_dirty;
}

// ---------------------------------------------------------------------------------------------------------
// The compute for tracks 5-6, stock's 0x8fc21a38..0x8fc22060 on this unit's buffers and masks.

static Slot* loaded_slot_at(i32 frame) {      // the cache slot of the chunk holding a frame (clamped at 0), when loaded
    if (frame < 0) frame = 0;
    Slot* slot = STOCK_SLOT_FOR_LOGICAL(frame >> 14);
    return (slot && slot->valid) ? slot : 0;
}

// Copy `count` frames of extension track j into `out`, from frame `from` on, the source frame clamped at
// `limit` as stock clamps at loop_b; a chunk that is not in the cache reads as silence.
static void copy_frames(u32 j, i32 from, i32 limit, u32 count, i32* out) {
    i32 frame = from;
    Slot* slot = 0;
    i32 slot_first = 0;
    for (u32 i = 0; i < count; ++i) {
        if (!slot || frame < slot_first || frame >= slot_first + CHUNK_FRAMES) {
            slot = loaded_slot_at(frame);
            slot_first = slot ? slot->frame_pos : 0;
        }
        if (slot) {
            const i32* src = (const i32*)(slot->data + (STOCK_TRACKS + j) * CHUNK_BYTES) + 2 * (frame - slot_first);
            out[2 * i] = src[0];
            out[2 * i + 1] = src[1];
        } else {
            out[2 * i] = 0;
            out[2 * i + 1] = 0;
        }
        frame = min_i(frame + 1, limit);
    }
}

// Stock's junction test (0x8fc21c78..0x8fc21d54) on the first half's last two frames (295, 296) and the
// second half's first two (0, 1), left and right added: the error of predicting each side from the other,
// E = |f296 + (f296 - f295) - f0| + |f0 + (f0 - f1) - f296|, against S = |f295 - f296| + |f296 - f0| +
// |f0 - f1|. A junction whose E exceeds S has a jump and gets the fades.
static int junction_jumps(const i32* buffer) {
    const i32* f295 = buffer + 2 * (XFADE_FRAMES - 2);
    const i32* f296 = buffer + 2 * (XFADE_FRAMES - 1);
    const i32* f0 = buffer + 2 * XFADE_FRAMES;
    const i32* f1 = buffer + 2 * (XFADE_FRAMES + 1);
    i32 pl = add_s(f296[0], sub_s(f296[0], f295[0]));
    i32 pr = add_s(f296[1], sub_s(f296[1], f295[1]));
    i32 e1 = add_s(abs_s(sub_s(pl, f0[0])), abs_s(sub_s(pr, f0[1])));
    i32 ql = add_s(f0[0], sub_s(f0[0], f1[0]));
    i32 qr = add_s(f0[1], sub_s(f0[1], f1[1]));
    i32 e2 = add_s(abs_s(sub_s(ql, f296[0])), abs_s(sub_s(qr, f296[1])));
    i32 error = add_s(e1, e2);
    i32 s1 = add_s(abs_s(sub_s(f295[0], f296[0])), abs_s(sub_s(f295[1], f296[1])));
    i32 jump = add_s(abs_s(sub_s(f296[0], f0[0])), abs_s(sub_s(f296[1], f0[1])));
    i32 s2 = add_s(abs_s(sub_s(f0[0], f1[0])), abs_s(sub_s(f0[1], f1[1])));
    i32 steps = add_s(add_s(s1, jump), s2);
    return error > steps;                                           // stock: error <= steps is continuous
}

// Stock's fades (0x8fc21dac..0x8fc21e02): the last 32 frames of the first half by 1, 31/32, ... down to 1/32,
// then the first 32 frames of the second half from just under 1/32 up to just under 1.
static void fade_junction(i32* buffer) {
    i32 gain = UNIT;
    i32* p = buffer + 2 * (XFADE_FRAMES - FADE_FRAMES);
    for (u32 i = 0; i < FADE_FRAMES; ++i) {
        p[0] = mult_t(gain, p[0]);
        p[1] = mult_t(gain, p[1]);
        gain = add_s(gain, -FADE_STEP);
        p += 2;
    }
    gain = add_s(gain, FADE_RISE);
    p = buffer + 2 * XFADE_FRAMES;
    p[0] = mult_t(gain, p[0]);
    p[1] = mult_t(gain, p[1]);
    p += 2;
    for (u32 i = 1; i < FADE_FRAMES; ++i) {
        gain = add_s(gain, FADE_RISE);
        p[0] = mult_t(gain, p[0]);
        p[1] = mult_t(gain, p[1]);
        p += 2;
    }
}

extern "C" void op1f_ext_xfade_compute(i32 loop_a, i32 loop_b) asm("op1f_ext_xfade_compute");
extern "C" void op1f_ext_xfade_compute(i32 loop_a, i32 loop_b) {
    if (loop_b <= loop_a) return;
    if (!loaded_slot_at(loop_a) || !loaded_slot_at(loop_a + XFADE_FRAMES) ||
        !loaded_slot_at(loop_b - 1) || !loaded_slot_at(loop_b - XFADE_FRAMES - 1)) {
        ++op1f_ext_xfade_waits;                                     // stock: a level-1 line, the dirty mask kept
        if (wait_log_budget) {
            --wait_log_budget;
            STOCK_LOG(2, LOG_FILE, 1, "loop crossfade of tracks 5/6 waits: a chunk of loop %d..%d is not in the cache", loop_a, loop_b);
        }
        return;
    }
    u32 mask = op1f_ext_xfade_dirty & EXT_MASK;
    op1f_ext_xfade_dirty = 0;
    i32 first = loop_b - min_i(XFADE_FRAMES, loop_b - loop_a);
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        if (!(mask & (1u << j))) continue;
        copy_frames(j, first, loop_b, XFADE_FRAMES, op1f_ext_loop_buffer[j]);
        copy_frames(j, loop_a, loop_b, XFADE_FRAMES, op1f_ext_loop_buffer[j] + 2 * XFADE_FRAMES);
    }
    u32 fade = mask;
    for (u32 j = 0; j < EXT_TRACKS; ++j) {                          // stock tests every track, dirty or not
        if (!junction_jumps(op1f_ext_loop_buffer[j])) {
            op1f_ext_xfade_valid |= 1u << j;
            fade &= ~(1u << j);
        }
    }
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        if (!(fade & (1u << j))) continue;
        fade_junction(op1f_ext_loop_buffer[j]);
        ++op1f_ext_xfade_fades;
    }
    op1f_ext_xfade_valid |= fade & ~op1f_ext_xfade_dirty;
    ++op1f_ext_xfade_computes;
}

// ---------------------------------------------------------------------------------------------------------
// The clip-edge fade writers for tracks 4-5: stock's 0x8f01f7d0 (fade out) and 0x8f01f8e0 (fade in), in
// pieces of STAGE_FRAMES through the extension-map tape I/O. The gain starts at 1.0 (out) or 0 (in) and moves
// by 0x7fffffff / frames per frame after each frame, as stock's loops do.

static void fade_range(u32 track, i32 from, i32 to, int fade_in) {
    if (from < 0 || to < from) {
        STOCK_LOG(2, LOG_FILE, 2, "clip edge fade %s refused: track %d frames %d..%d", fade_in ? "in" : "out", track + 1, from, to);
        return;
    }
    u32 count = (u32)(to - from);
    if (count == 0) return;
    ++op1f_ext_edge_fades;
    if (edge_log_budget) {
        --edge_log_budget;
        STOCK_LOG(3, LOG_FILE, 3, "clip edge: track %d frames %d..%d fade %s", track + 1, from, to, fade_in ? "in" : "out");
    }
    i32 step = (i32)udiv((u32)UNIT, count);
    i32 gain = fade_in ? 0 : UNIT;
    u32 done = 0;
    while (done < count) {
        u32 n = count - done;
        if (n > STAGE_FRAMES) n = STAGE_FRAMES;
        u32 byte_offset = ((u32)from + done) * FRAME_BYTES;
        if (op1f_tape_io_read_ext(track, byte_offset, n * FRAME_BYTES, stage) < 0) return;
        i32* p = (i32*)stage;
        for (u32 i = 0; i < n; ++i) {
            p[2 * i] = mult_t(gain, p[2 * i]);
            p[2 * i + 1] = mult_t(gain, p[2 * i + 1]);
            gain = fade_in ? add_s(gain, step) : sub_s(gain, step);
        }
        if (op1f_tape_io_write_ext(track, byte_offset, n * FRAME_BYTES, stage) < 0) return;
        done += n;
    }
}

extern "C" void op1f_fade_out_ext(u32 track, i32 from, i32 to) asm("op1f_fade_out_ext");
extern "C" void op1f_fade_out_ext(u32 track, i32 from, i32 to) {
    fade_range(track, from, to, 0);
}

extern "C" void op1f_fade_in_ext(u32 track, i32 from, i32 to) asm("op1f_fade_in_ext");
extern "C" void op1f_fade_in_ext(u32 track, i32 from, i32 to) {
    fade_range(track, from, to, 1);
}
