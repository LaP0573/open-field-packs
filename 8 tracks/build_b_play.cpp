// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B1: playback of tape tracks 5 and 6 (track indices 4 and 5).
// Build F, image F1: four extension tracks (indices 4-7), the filter loop in assembly, the silence skip per track.
//
// Stock plays a tape block in the L1 driver 0x11a07fe4 (OS 1.7.0+0) in three steps:
//
//   read_block(speed, state, rolling)  0x8f01ec34  fills four 600-frame window buffers (0x84c61e88 + t * 0x12c0)
//                                                   with source frames [W0, W0 + wanted) from the chunk cache,
//                                                   with the per-track volume/pan gains and the loop crossfades
//                                                   applied; returns W0, the window's first frame
//   resample(rel64, speed)             0x11a07748  for 64 output samples: a 16-tap, 64-phase polyphase FIR per
//                                                   track over the window, times the tape level; per-track
//                                                   outputs into the output area and the stereo sums into
//                                                   0x119027a0 (left) and 0x119028a0 (right)
//   mixer(speed, state)                0x8f01f4e0  a speed-dependent low-pass and the speed-squared level ramp
//                                                   on the sums (in place), then the smoothing of the per-track
//                                                   outputs; the DSP core carries the sums on as the main mix
//
// This unit hooks the driver's two absolute calls (profile group "build-b1"): the read_block call becomes
// op1f_read_block_ext, which calls stock and then fills the extension window buffers for tracks 4-7 from the
// same cache slots (regions 4-7 of each slot, which the cache unit lays out and the storage unit fills); the
// mixer call becomes op1f_mixer_ext, which first runs the same resampler arithmetic as stock over those buffers
// and adds the result into the stereo sums, then calls the stock mixer. Tracks 1-4 are untouched: their window
// buffers, gains, resampling and outputs are stock's own. Tracks 5-8 have no per-track output (so no USB
// channel for them).
//
// B3a: each extension track's resampled stereo pair goes through stock's gain law before it is added, with
// the values of build_b_mix.cpp (op1f_ext_params: volumes, pans, mute gains in stock's layout). read_block
// 0x8f01d3d4..0x8f01d4d4 derives four gains per track from g = volume x mute and the pan p in [-0.5, 0.5]
// (A = sat(1 - 2p), B = sat(1 + 2p)): left out = sat(B/2 + 0.5) g A x L + g (0.5 - B/2) x R, right out =
// sat(A/2 + 0.5) g B x R + g (0.5 - A/2) x L, a balance that folds the far channel into the near one towards
// the extremes; stock ramps each gain by (target - current) >> shift per frame, this unit by (target -
// current) >> 6 per output sample, so a change settles within one block. The per-track peak of the result
// goes to op1f_ext_vu (the words the L1 monitoring stub also targets), cleared by the frame hook.
//
// B1b: the fill keeps the OR of every sample it copies, per track since F1; a track whose window is all zero
// is left out of the filter (its FIR would be zero) and a block in which every extension window is silent,
// which is every block on a tape whose tracks 5-8 hold nothing at the position, skips the resampling and only
// counts the block. F2: the fill asks the extension map before copying and writes a track's window only where
// the track recorded something, so an empty track costs no stores at all (the F1b read-out measured the four
// windows' fill at about 4.5 points of the core whatever they held). The 16-tap loop itself is op1f_fir_ext in Build B's playback stub (F1): stock's own loop shape with
// the loads paired to the multiplies, four instructions per tap where the compiled C spent twelve; this file
// prepares a job per output sample (the active windows, the frame offset, the coefficient column) and applies
// the gains, the level, the peaks and the sums to what comes back. Since packs L4 the routine of that name is
// build_l4_fir.s, written from what it computes, with the same job and the same two sums bit for bit; the one in
// Build B's playback stub is its elder, which every pack before L4 has.
//
// B3b: the loop crossfade. While stock's loop flag B[0x8400129d] is set (read_block sets it within 297 frames
// of a loop point and clears it after) the window start is wrapped into [loop_a, loop_b) as stock wraps its
// own, and every frame from loop_b - 297 on (frames past loop_b continuing at loop_a) or within 297 frames
// after loop_a comes from the track's loop buffer in build_b_xfade.cpp when that buffer is valid, else from
// the cache slot of the same source frame; op1f_ext_xfade_compute fills the buffers, with stock's junction
// test and fades, when they are not valid. A loop shorter than 594 frames keeps the plain read. The crossfade
// unit keeps buffers for tracks 5-6 (XFADE_TRACKS); tracks 7-8 join with image F2.
//
// Fixed point as stock: samples are 32-bit fractions; `(T)` multiplies and `(S)` adds are the Blackfin
// instructions themselves, through inline assembly, so the arithmetic matches stock's bit for bit.
// Freestanding like the other units.

typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;
typedef int i32;

typedef int (*ReadBlockFn)(int speed, void* state, int rolling);
typedef void (*MixerFn)(int speed, void* state);

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
typedef Slot* (*SlotForLogicalFn)(int logical);

#define STOCK_READ_BLOCK ((ReadBlockFn)0x8f01ec34u)
#define STOCK_MIXER ((MixerFn)0x8f01f4e0u)
#define STOCK_SLOT_FOR_LOGICAL ((SlotForLogicalFn)0x8fc21280u)   // slot_by_logical[l], or 0
#define POSITION64 (*(volatile i32*)0x11905a18u)                    // tape position in 1/64 frames
#define LEVEL_PAIR (*(volatile u32*)0x848ce530u)                    // low half: current level (Q15), high half: target
#define SUM_LEFT ((volatile i32*)0x119027a0u)                       // 64 samples each, the stereo mix of the tape
#define SUM_RIGHT ((volatile i32*)0x119028a0u)
#define COEFFICIENTS_END 0x11904af4u                                // phase 0 of the 16 x 64 polyphase table

enum {
    EXT_TRACKS = 4,                   // F1: tracks 5-8 (indices 4-7)
    XFADE_TRACKS = 4,                 // the crossfade unit's loop buffers: tracks 5-8 since F2 (5-6 on F1b)
    STOCK_TRACKS = 4,
    ALL_TRACKS = STOCK_TRACKS + EXT_TRACKS,
    WINDOW_FRAMES = 600,
    BLOCK_SAMPLES = 64,
    TAPS = 16,
    HISTORY = 8,                      // the FIR starts 8 frames before the current frame
    WINDOW_LEAD = 19,                 // stock reads |frames per block| + 19 frames
    CHUNK_FRAMES = 16384,
    CHUNK_BYTES = 0x20000,
    CHUNKS_PER_TRACK = 969,
    XFADE_FRAMES = 297,               // B3b: stock's window on each side of a loop point
    LOOP_BUFFER_FRAMES = 594,         // the frames before loop_b, then the frames from loop_a
    FRAME_BYTES = 8
};

extern "C" u32 op1f_cache_tracks asm("op1f_cache_tracks");           // 8 with the two-region cache, 4 after the fallback
extern "C" i32 op1f_ext_params[12] asm("op1f_ext_params");           // B3a (build_b_mix.cpp): volumes [0..3], pans [4..7], mute gains [8..11] of tracks 5-8
extern "C" u16 op1f_ext_vu[4] asm("op1f_ext_vu");                   // B3a: per-track peaks of tracks 5-8, as stock's W[0x11905a34 + 2t]
extern "C" i32 op1f_ext_loop_buffer[XFADE_TRACKS][LOOP_BUFFER_FRAMES * 2] asm("op1f_ext_loop_buffer");   // B3b (build_b_xfade.cpp)
extern "C" u32 op1f_ext_xfade_valid asm("op1f_ext_xfade_valid");               // B3b: bit j = the buffer of track 4 + j is computed
extern "C" void op1f_ext_xfade_compute(i32 loop_a, i32 loop_b) asm("op1f_ext_xfade_compute");
#define XFADE_FLAG (*(volatile unsigned char*)0x8400129du)                     // B3b: read_block's "near a loop point" flag (0x8fc21864 / 0x8fc21878)
#define LOOP_A (*(volatile i32*)0x84c62220u)                                   // the loop points, in frames
#define LOOP_B (*(volatile i32*)0x84c62224u)
#define XFADE_MASK ((1u << XFADE_TRACKS) - 1u)

// The filter job for Build B's playback stub (F1) and build_l4_fir.s (L4): the layout is the assembly's contract, see there.
struct FirJob {
    const i32* bases[EXT_TRACKS];
    u32 count;
    u32 offset_bytes;
    const i32* coefficient;
    i32 out[2 * EXT_TRACKS];
};
extern "C" void op1f_fir_ext(FirJob* job) asm("op1f_fir_ext");

// Counters for the measurement line (read by the first frame hook's measuring unit through weak references).
extern "C" u32 op1f_ext_play_blocks asm("op1f_ext_play_blocks");     // blocks in which the extension tracks were considered
extern "C" u32 op1f_ext_play_active asm("op1f_ext_play_active");     // of those, blocks that added a non-zero sample
extern "C" i32 op1f_ext_play_w0 asm("op1f_ext_play_w0");             // the last window start frame
extern "C" u32 op1f_ext_play_skipped asm("op1f_ext_play_skipped");   // B1b: blocks whose windows were all zero, resampling skipped
extern "C" u32 op1f_ext_play_tracks asm("op1f_ext_play_tracks");     // F1: track-blocks filtered (a block with two active tracks counts two)
u32 op1f_ext_play_blocks;
u32 op1f_ext_play_active;
i32 op1f_ext_play_w0;
u32 op1f_ext_play_skipped;
u32 op1f_ext_play_tracks;
extern "C" u32 op1f_ext_loop_blocks asm("op1f_ext_loop_blocks");       // B3b: blocks that took frames from the loop buffers
u32 op1f_ext_loop_blocks;

static i32 ext_window[EXT_TRACKS][WINDOW_FRAMES * 2] __attribute__((aligned(8)));   // interleaved left, right per frame
static i32 ext_gain[EXT_TRACKS][4];                                 // B3a: the ramped gain matrix per track: L->L, R->L, R->R, L->R
static u32 ext_count;                                               // frames the current window holds
static u32 ext_any[EXT_TRACKS];                                     // OR of every sample copied into a track's window (B1b, per track since F1)
static u32 ext_filled[EXT_TRACKS];                                  // F2: frames [0, ext_filled[t]) of a track's window hold this block's data
static FirJob fir_job __attribute__((aligned(4)));
extern "C" int op1f_ext_chunk_mapped(u32 track, u32 logical) asm("op1f_ext_chunk_mapped");   // build_b_storage.cpp: the track recorded that chunk
// Build G (G1, Build G's duck unit): the duck. Weak, so images without that unit link unchanged.
extern "C" void __attribute__((weak)) op1f_duck_block_gains() asm("op1f_duck_block_gains");          // this block's gains into the hold-to-solo words (1-4) and op1f_duck_gain (5-8)
extern "C" void __attribute__((weak)) op1f_duck_detect(int speed, int w0) asm("op1f_duck_detect");  // the detector over this block's frames
extern "C" i32 __attribute__((weak)) op1f_duck_gain[EXT_TRACKS] asm("op1f_duck_gain");             // Q31 per extension track, 0x7fffffff when idle
// Step R6g (build_r6g_solo.cpp): the hold-to-solo of tracks 5-8, LaP0573/open-field#6. Weak, so a link without that unit is as before.
extern "C" i32 __attribute__((weak)) op1f_ext_solo_gain(u32 t) asm("op1f_ext_solo_gain");          // 0 for a track soloed out in this block, else 0x7fffffff
extern "C" int __attribute__((weak)) op1f_ext_solo_heard(u32 t) asm("op1f_ext_solo_heard");        // not 0 while the track is held: its mute is passed over

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

// B3a: the four gain targets of extension track t from the module's strip, stock's law (see the header).
// R6g: times the solo's factor, a factor of its own (0 for a track soloed out); a held track passes over its mute
// while held, as stock's press clears a held track 1-4's mute byte and its release puts it back.
static void ext_gain_targets(u32 t, i32* target) {
    const i32 half = 0x40000000;
    i32 mute = op1f_ext_params[8 + t];
    if (&op1f_ext_solo_heard != 0 && op1f_ext_solo_heard(t)) mute = 0x7fffffff;
    i32 g = mult_t(op1f_ext_params[t], mute);                         // volume x mute gain
    if (&op1f_duck_gain != 0) g = mult_t(g, op1f_duck_gain[t]);   // Build G: the duck's gain of this track
    if (&op1f_ext_solo_gain != 0 && op1f_ext_solo_gain(t) == 0) g = 0;   // R6g: soloed out (no multiply by 0x7fffffff otherwise, so the gain stays version 10's bit for bit)
    i32 p = op1f_ext_params[4 + t];                                   // pan
    i32 r1 = add_s(p, half);                                          // p + 0.5
    i32 a = shl1_s(sub_s(0x7fffffff, r1));                            // sat(1 - 2p)
    i32 b = shl1_s(r1);                                               // sat(1 + 2p)
    target[0] = mult_t(add_s(b >> 1, half), mult_t(g, a));            // L -> L
    target[1] = mult_t(g, sub_s(half, b >> 1));                       // R -> L
    target[2] = mult_t(add_s(a >> 1, half), mult_t(g, b));            // R -> R
    target[3] = mult_t(g, sub_s(half, a >> 1));                       // L -> R
}

static inline i16 sat16(i32 v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (i16)v;
}

static void zero_track(u32 t, u32 first, u32 count) {
    i32* d = ext_window[t] + 2 * first;
    for (u32 i = 0; i < 2 * count; ++i) d[i] = 0;
}

// F2: a track's window is written only where the track has something: before a piece lands at index j, the
// frames before it that no piece covered become zeros, and after the fill the tail of an active track's window
// is zeroed likewise. A track that gets no piece at all is never written and stays out of the filter (ext_any
// 0), which is the common case for an empty track and the reason the fill of silent tracks costs nothing.
static void begin_piece(u32 t, u32 j) {
    if (ext_filled[t] < j) zero_track(t, ext_filled[t], j - ext_filled[t]);
}

// B3b: the loop buffer index of a frame while the loop flag is set (build_b_xfade.cpp holds the buffers):
// frames from loop_b - 297 on, the ones past loop_b included, continue into the second half, the loop start;
// frames from loop_a on map into the second half directly; -1 for any other frame.
static int loop_index(i32 frame, i32 loop_a, i32 loop_b) {
    i32 d = frame - (loop_b - XFADE_FRAMES);
    if (d >= 0 && d < LOOP_BUFFER_FRAMES) return d;
    d = frame - loop_a;
    if (d >= 0 && d < XFADE_FRAMES) return XFADE_FRAMES + d;
    return -1;
}

// Copy `run` frames of every extension track that recorded the chunk holding `frame` from its cache slot into
// the track's window at index j (the run stays inside one chunk). A frame before the tape start, beyond the
// tape, in a slot that is missing or still loading, or in a chunk the track never recorded, is silence: nothing
// is written for it (F2; the zeros come from begin_piece and the fill's tail when the track has data elsewhere
// in the window), as a missing slot reads as silence for tracks 1-4.
static void copy_run(u32 j, i32 frame, u32 run) {
    if (frame < 0) return;
    u32 logical = (u32)frame >> 14;
    if (logical >= CHUNKS_PER_TRACK) return;
    Slot* slot = STOCK_SLOT_FOR_LOGICAL((int)logical);
    if (!slot || !slot->valid) return;
    u32 offset = (u32)frame & (CHUNK_FRAMES - 1);
    for (u32 t = 0; t < EXT_TRACKS; ++t) {
        if (!op1f_ext_chunk_mapped(STOCK_TRACKS + t, logical)) continue;
        begin_piece(t, j);
        const i32* src = (const i32*)(slot->data + (STOCK_TRACKS + t) * CHUNK_BYTES) + 2 * offset;
        i32* dst = ext_window[t] + 2 * j;
        u32 any = ext_any[t];
        for (u32 i = 0; i < 2 * run; ++i) {
            i32 v = src[i];
            dst[i] = v;
            any |= (u32)v;
        }
        ext_any[t] = any;
        ext_filled[t] = j + run;
    }
}

// Fill the extension windows with frames [w0, w0 + wanted) of tracks 4-7. With the loop flag set (B3b) a
// frame the loop buffers cover comes from the track's buffer when it is valid, else from the slot of the same
// source frame (frames past loop_b continue at loop_a); every other frame comes from the slots, in runs that
// stop at a chunk end and at the next covered frame. Returns 1 when a buffer frame was used.
static int fill_window(i32 w0, u32 wanted, int loop_flag, u32 loop_valid, i32 loop_a, i32 loop_b) {
    u32 j = 0;
    int from_buffer = 0;
    while (j < wanted) {
        i32 frame = w0 + (i32)j;
        if (loop_flag) {
            int index = loop_index(frame, loop_a, loop_b);
            if (index >= 0) {
                copy_run(j, frame >= loop_b ? loop_a + (frame - loop_b) : frame, 1);
                for (u32 t = 0; t < XFADE_TRACKS; ++t) {
                    if (!(loop_valid & (1u << t))) continue;
                    const i32* src = op1f_ext_loop_buffer[t] + 2 * index;
                    begin_piece(t, j);
                    i32* dst = ext_window[t] + 2 * j;
                    dst[0] = src[0];
                    dst[1] = src[1];
                    ext_any[t] |= (u32)src[0] | (u32)src[1];
                    ext_filled[t] = j + 1;
                    from_buffer = 1;
                }
                ++j;
                continue;
            }
        }
        if (frame < 0) {                                              // the pre-roll: silence, nothing written (F2)
            u32 pre = (u32)(-frame);
            if (pre > wanted - j) pre = wanted - j;
            j += pre;
            continue;
        }
        u32 run = CHUNK_FRAMES - ((u32)frame & (CHUNK_FRAMES - 1));
        if (run > wanted - j) run = wanted - j;
        if (loop_flag) {                                              // stop the run at the next covered frame
            i32 before_b = loop_b - XFADE_FRAMES;
            if (frame < before_b && (u32)(before_b - frame) < run) run = (u32)(before_b - frame);
            if (frame < loop_a && (u32)(loop_a - frame) < run) run = (u32)(loop_a - frame);
        }
        copy_run(j, frame, run);
        j += run;
    }
    for (u32 t = 0; t < EXT_TRACKS; ++t)                              // F2: an active track's window ends in zeros where nothing landed
        if (ext_any[t] && ext_filled[t] < wanted) zero_track(t, ext_filled[t], wanted - ext_filled[t]);
    return from_buffer;
}

// Replaces the driver's call of 0x8f01ec34 at 0x11a0825e.
extern "C" int op1f_read_block_ext(int speed, void* state, int rolling) asm("op1f_read_block_ext");
extern "C" int op1f_read_block_ext(int speed, void* state, int rolling) {
    if (&op1f_duck_block_gains != 0) op1f_duck_block_gains();       // Build G: this block's duck gains, before stock reads the solo words
    int w0 = STOCK_READ_BLOCK(speed, state, rolling);
    if (&op1f_duck_detect != 0) op1f_duck_detect(speed, w0);        // Build G: the detector over this block's frames
    ext_count = 0;
    for (u32 t = 0; t < EXT_TRACKS; ++t) { ext_any[t] = 0; ext_filled[t] = 0; }
    if (op1f_cache_tracks >= ALL_TRACKS) {
        int frames_per_block = speed >> 21;                          // as stock: EXTRACT of bits 21.. of the speed
        if (frames_per_block < 0) frames_per_block = -frames_per_block;
        u32 wanted = (u32)frames_per_block + WINDOW_LEAD;
        if (wanted > WINDOW_FRAMES) wanted = WINDOW_FRAMES;
        i32 loop_a = LOOP_A;
        i32 loop_b = LOOP_B;
        i32 base = w0;
        int loop_flag = XFADE_FLAG != 0 && loop_b - loop_a >= LOOP_BUFFER_FRAMES;   // B3b
        u32 loop_valid = 0;
        if (loop_flag) {
            i32 length = loop_b - loop_a;                            // stock wraps its window start into the loop
            if (base < loop_a) base += length;
            else if (base >= loop_b) base -= length;
            if ((op1f_ext_xfade_valid & XFADE_MASK) != XFADE_MASK) op1f_ext_xfade_compute(loop_a, loop_b);
            loop_valid = op1f_ext_xfade_valid & XFADE_MASK;
        }
        if (fill_window(base, wanted, loop_flag, loop_valid, loop_a, loop_b)) ++op1f_ext_loop_blocks;
        ext_count = wanted;
        op1f_ext_play_w0 = w0;                                       // the resampler's reference stays stock's unwrapped W0
    }
    return w0;
}

// Replaces the driver's call of 0x8f01f4e0 at 0x11a0829a: the stock resampler has just written the 64 stereo
// sums of tracks 1-4; add the active extension tracks with the same arithmetic, then let the stock mixer
// filter the result.
extern "C" void op1f_mixer_ext(int speed, void* state) asm("op1f_mixer_ext");
extern "C" void op1f_mixer_ext(int speed, void* state) {
    if (ext_count) {
        u32 track_of[EXT_TRACKS];                                      // the active tracks, in job order
        u32 count = 0;
        for (u32 t = 0; t < EXT_TRACKS; ++t) {
            if (!ext_any[t]) continue;                                 // B1b: a silent window filters to zero; leave it out
            fir_job.bases[count] = ext_window[t];
            track_of[count] = t;
            ++count;
        }
        i32 gain_step[EXT_TRACKS][4];                                  // B3a: per-sample gain increments, every track
        for (u32 t = 0; t < EXT_TRACKS; ++t) {
            i32 target[4];
            ext_gain_targets(t, target);
            for (u32 k = 0; k < 4; ++k) gain_step[t][k] = (target[k] - ext_gain[t][k]) >> 6;
        }
        if (count == 0) {                                              // B1b: silent extension tracks cost nothing
            for (u32 i = 0; i < BLOCK_SAMPLES; ++i)                    // but the ramps keep moving, as on the window's edges
                for (u32 t = 0; t < EXT_TRACKS; ++t)
                    for (u32 k = 0; k < 4; ++k) ext_gain[t][k] += gain_step[t][k];
            ++op1f_ext_play_blocks;
            ++op1f_ext_play_skipped;
        } else {
            i32 rel64 = POSITION64 - (op1f_ext_play_w0 << 6);          // position relative to the window, 1/64 frames
            u32 phase = (u32)rel64 << 16;                              // frames in 10.22 fixed point
            i32 increment = (speed + (speed < 0 ? 31 : 0)) >> 5;        // frames per output sample, 10.22
            u32 pair = LEVEL_PAIR;
            i16 level = (i16)(pair & 0xffff);
            i16 delta = (i16)(sat16((i32)(i16)(pair >> 16) - (i32)level) >> 6);
            u32 peak[EXT_TRACKS];
            for (u32 t = 0; t < EXT_TRACKS; ++t) peak[t] = 0;
            fir_job.count = count;
            int active = 0;
            for (u32 i = 0; i < BLOCK_SAMPLES; ++i) {
                phase += (u32)increment;
                u32 n = phase >> 22;
                u32 fraction = (phase >> 16) & 63;
                i32 gain = (i32)level << 16;
                i32 sum_left = 0, sum_right = 0;
                if (n >= HISTORY && n + HISTORY <= ext_count) {
                    fir_job.offset_bytes = (n - HISTORY) * FRAME_BYTES;
                    fir_job.coefficient = (const i32*)(COEFFICIENTS_END - 4 * fraction);
                    op1f_fir_ext(&fir_job);                            // F1: the 16 taps of every active track, in assembly
                    for (u32 k = 0; k < count; ++k) {
                        u32 t = track_of[k];
                        i32 left = fir_job.out[2 * k];
                        i32 right = fir_job.out[2 * k + 1];
                        i32* gm = ext_gain[t];                         // B3a: the strip, then the tape level as before
                        i32 out_left = add_s(mult_t(gm[0], left), mult_t(gm[1], right));
                        i32 out_right = add_s(mult_t(gm[2], right), mult_t(gm[3], left));
                        u32 magnitude = (u32)(out_left < 0 ? -out_left : out_left);
                        u32 magnitude_right = (u32)(out_right < 0 ? -out_right : out_right);
                        if (magnitude_right > magnitude) magnitude = magnitude_right;
                        if (magnitude > peak[t]) peak[t] = magnitude;
                        sum_left = add_s(sum_left, mult_t(gain, out_left));
                        sum_right = add_s(sum_right, mult_t(gain, out_right));
                    }
                }
                for (u32 t = 0; t < EXT_TRACKS; ++t) {                  // the ramps move every sample, active or not
                    i32* gm = ext_gain[t];
                    gm[0] += gain_step[t][0];
                    gm[1] += gain_step[t][1];
                    gm[2] += gain_step[t][2];
                    gm[3] += gain_step[t][3];
                }
                if (sum_left | sum_right) {
                    active = 1;
                    SUM_LEFT[i] = add_s(SUM_LEFT[i], sum_left);
                    SUM_RIGHT[i] = add_s(SUM_RIGHT[i], sum_right);
                }
                level = sat16((i32)level + delta);
            }
            for (u32 t = 0; t < EXT_TRACKS; ++t) {                      // B3a: the block's peak, as stock's VU words
                u32 vu = peak[t] >> 16;
                if (vu > op1f_ext_vu[t]) op1f_ext_vu[t] = (u16)vu;
            }
            ++op1f_ext_play_blocks;
            op1f_ext_play_tracks += count;
            if (active) ++op1f_ext_play_active;
        }
    }
    STOCK_MIXER(speed, state);
}
