// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R6g, the pack 8 tracks (version 11): tracks 5 to 8 in the tape's hold-to-solo (LaP0573/open-field#6, reported
// by hasis on GitHub). Holding tape + track key 1-4, which selects track 5-8, solos that track as holding a track key
// solos tracks 1-4; several held keys solo their tracks together, across 1-4 and 5-8; a solo of tracks 1-4 silences
// tracks 5-8 as well.
//
// Where it meets the firmware (OS 1.7.3):
//   * Stock keeps a held byte for each of track keys 1-4, B[0x84c62218 + t], set while that key is held in the tape. A
//     held track key also lets its track be heard while the track is muted, until the key is let go.
//   * Once a block, before the tape is read, the tape driver calls stock's solo function 0x8f01e850 (the call at
//     0x11a0820c, made on a stopped tape too). It leaves tracks 1-4's four solo words at 0x11905aa8 at 0x7fffffff, or at
//     0 for each track that held keys leave out once a key has been held for 55 blocks; the tape's gains of tracks 1-4
//     follow those words. 55 blocks of 64 samples are 80 ms: a press that only selects a track cuts nothing.
//   * The tape controller's key-up handler 0x8c0c2158 is called through a word of the controller's table, 0x84010250,
//     with (this, key); its answer goes back to the caller.
//
// This unit:
//   * op1f_ext_solo_press(j), from op1f_tape_key_filter (build_b_record.cpp) where tape + track key j+1 selects
//     track 5+j: the hold of track 5+j begins, before the select's two conditions, as stock's press sets its byte.
//   * op1f_tape_key_up, in the vtable's slot: a track key's release ends the hold its press began, whether the tape key
//     is still down or not; then stock's key-up runs as always (for a press of ours stock's release finds no saved mute
//     and a held byte already clear, so it changes nothing there).
//   * op1f_solo_block, in place of the driver's call of the writer: stock's writer first, then stock's rule over the
//     eight tracks with a counter of its own, which runs in step with stock's (put at 55 while none of the eight is
//     held, counted down one a block while one is; it runs in the same call, so on a stopped tape too). From the block
//     it is found at 0: each track 5-8 not held is soloed out, and with a track 5-8 held the solo word of each track 1-4
//     whose held byte is clear becomes 0 (where stock made it 0 already nothing changes). With only tracks 1-4 held the
//     words are stock's own. The duck (pack sidechain) multiplies its gains into the words later in the block.
//   * op1f_ext_solo_gain(t), for the gain targets of track 5+t (build_b_play.cpp): 0 for a track soloed out in
//     this block, else 0x7fffffff, a factor of its own beside the volume, the mute and the duck: nothing is written
//     into them. op1f_ext_solo_heard(t): the track is held in this block, so its gain passes over its mute while held,
//     as stock clears a held track 1-4's mute byte and puts it back at the release; the mute itself is never written.
//   * op1f_solo_frame(mode), from the pack's frame (build_r4b_8_tracks_frame.cpp): a hold cannot outlive the
//     tape. When the mode is not the tape's a release went to another controller, and every hold of 5-8 ends (the
//     mixer page's guard in the same frame is the model).
//
// Threads: the key handlers run on the GUI thread, the frame on the display thread, the writer on the audio thread.
// A hold is one byte, written whole; the audio thread reads the four once a block. Nothing is logged on the audio
// thread: its counts are read by the frame's lines. Compiled for the PC with OP1F_HOST (for tests this tree does not hold).

typedef unsigned int u32;
typedef int i32;
typedef unsigned char u8;

#ifndef OP1F_NAMED
#ifdef OP1F_HOST
#define OP1F_NAMED(x)
#else
#define OP1F_NAMED(x) asm(x)
#endif
#endif

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);

#ifdef OP1F_HOST
extern "C" volatile u8 mock_held_bytes[4];             // the PC run's stand-ins for stock's places and calls
extern "C" volatile i32 mock_solo_words[4];
extern "C" void mock_solo_writer();
extern "C" int mock_tape_key_up(void* self, int key);
extern "C" void mock_log(int level, const char* file, int line, const char* format, ...);
#define STOCK_HELD mock_held_bytes
#define SOLO_WORDS mock_solo_words
#define STOCK_SOLO_WRITER mock_solo_writer
#define STOCK_TAPE_KEY_UP mock_tape_key_up
#define STOCK_LOG mock_log
#else
typedef void (*WriterFn)();
typedef int (*KeyUpFn)(void* self, int key);
#define STOCK_HELD ((volatile u8*)0x84c62218u)         // the held bytes of track keys 1-4
#define SOLO_WORDS ((volatile i32*)0x11905aa8u)        // tracks 1-4's solo words, Q31
#define STOCK_SOLO_WRITER ((WriterFn)0x8f01e850u)      // the solo writer, no arguments
#define STOCK_TAPE_KEY_UP ((KeyUpFn)0x8c0c2158u)       // the TapeController's key-up (this, key)
#define STOCK_LOG ((LogFn)0x8c053a20u)                 // log(level, file, line, fmt, ...), level 3 = I
#endif

enum {
    TRACKS_HELD = 4,          // the holds of tracks 5 to 8, and stock's of tracks 1 to 4
    KEY_TRACK_1 = 6,
    KEY_TRACK_4 = 9,
    SOLO_WAIT = 55,           // stock's wait, in blocks, before held keys' solo takes
    MODE_TAPE = 2,            // the frame's mode: 0 synth, 1 drum, 2 tape, 3 mixer
    HELD_IS_HEARD = 1,        // a held track 5-8 passes over its mute while held, as stock's held 1-4 do
    LOG_BUDGET = 8
};
static const i32 GAIN_ONE = 0x7fffffff;
static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_solo.cpp";

extern "C" volatile u8 op1f_ext_solo_held[TRACKS_HELD] OP1F_NAMED("op1f_ext_solo_held");   // 1 while tape + track key j+1's hold of track 5+j lasts
volatile u8 op1f_ext_solo_held[TRACKS_HELD];

// Counts for the frame's line (8 tracks/build_r4b_8_tracks_frame.cpp).
extern "C" u32 op1f_ext_solo_holds OP1F_NAMED("op1f_ext_solo_holds");               // holds of 5-8 begun (GUI thread)
extern "C" u32 op1f_ext_solo_ends OP1F_NAMED("op1f_ext_solo_ends");                 // ended by their key's release (GUI thread)
extern "C" u32 op1f_ext_solo_drops OP1F_NAMED("op1f_ext_solo_drops");               // ended by the frame's guard (display thread)
extern "C" u32 op1f_ext_solo_blocks OP1F_NAMED("op1f_ext_solo_blocks");             // blocks soloed with a track 5-8 held (audio thread)
extern "C" u32 op1f_ext_solo_stock_blocks OP1F_NAMED("op1f_ext_solo_stock_blocks"); // blocks soloed with only tracks 1-4 held, 5-8 out (audio thread)
u32 op1f_ext_solo_holds;
u32 op1f_ext_solo_ends;
u32 op1f_ext_solo_drops;
u32 op1f_ext_solo_blocks;
u32 op1f_ext_solo_stock_blocks;

static u32 solo_wait = SOLO_WAIT;          // the audio thread's own
static volatile u32 solo_out;              // bit t: track 5+t soloed out in this block
static volatile u32 solo_held;             // bit t: track 5+t held in this block
static u32 log_budget = LOG_BUDGET;

// Tape + track key j+1 went down and selects track 5+j (GUI thread).
extern "C" void op1f_ext_solo_press(u32 j) OP1F_NAMED("op1f_ext_solo_press");
extern "C" void op1f_ext_solo_press(u32 j) {
    if (j >= TRACKS_HELD) return;
    if (!op1f_ext_solo_held[j]) ++op1f_ext_solo_holds;
    op1f_ext_solo_held[j] = 1;
    if (log_budget) {
        --log_budget;
        STOCK_LOG(3, LOG_FILE, 1, "track %u held (tape key + track key %u): solo after %u blocks", j + 5, j + 1, (u32)SOLO_WAIT);
    }
}

// In the TapeController vtable's key-up slot (GUI thread).
extern "C" int op1f_tape_key_up(void* self, int key) OP1F_NAMED("op1f_tape_key_up");
extern "C" int op1f_tape_key_up(void* self, int key) {
    if (key >= KEY_TRACK_1 && key <= KEY_TRACK_4) {
        u32 j = (u32)(key - KEY_TRACK_1);
        if (op1f_ext_solo_held[j]) {
            op1f_ext_solo_held[j] = 0;
            ++op1f_ext_solo_ends;
            if (log_budget) {
                --log_budget;
                STOCK_LOG(3, LOG_FILE, 2, "track %u's hold ended (track key %u up)", j + 5, j + 1);
            }
        }
    }
    return STOCK_TAPE_KEY_UP(self, key);
}

// In place of the tape driver's call of stock's solo writer, every block (audio thread).
extern "C" void op1f_solo_block() OP1F_NAMED("op1f_solo_block");
extern "C" void op1f_solo_block() {
    STOCK_SOLO_WRITER();
    u32 stock = 0, ext = 0;
    for (u32 t = 0; t < TRACKS_HELD; ++t) {
        if (STOCK_HELD[t]) stock |= 1u << t;
        if (op1f_ext_solo_held[t]) ext |= 1u << t;
    }
    solo_held = ext;
    if (!(stock | ext)) {                                  // nothing held: stock's rule puts the counter at 55
        solo_wait = SOLO_WAIT;
        solo_out = 0;
        return;
    }
    if (solo_wait) {                                       // held, the wait not over: counted down, nothing soloed yet
        --solo_wait;
        solo_out = 0;
        return;
    }
    if (ext) {
        for (u32 t = 0; t < TRACKS_HELD; ++t)
            if (!(stock & (1u << t))) SOLO_WORDS[t] = 0;   // tracks 1-4 not held go out as stock's do
        ++op1f_ext_solo_blocks;
    } else {
        ++op1f_ext_solo_stock_blocks;
    }
    solo_out = ~ext & ((1u << TRACKS_HELD) - 1u);
}

// The solo's factor in the gain targets of track 5+t (audio thread, after op1f_solo_block in the same block).
extern "C" i32 op1f_ext_solo_gain(u32 t) OP1F_NAMED("op1f_ext_solo_gain");
extern "C" i32 op1f_ext_solo_gain(u32 t) {
    return ((solo_out >> t) & 1u) ? 0 : GAIN_ONE;
}

// Not 0 while track 5+t is held: its gain passes over its mute (audio thread).
extern "C" int op1f_ext_solo_heard(u32 t) OP1F_NAMED("op1f_ext_solo_heard");
extern "C" int op1f_ext_solo_heard(u32 t) {
    return HELD_IS_HEARD && ((solo_held >> t) & 1u);
}

// The frame's guard (display thread): out of the tape, no hold of 5-8 lasts.
extern "C" void op1f_solo_frame(u32 mode) OP1F_NAMED("op1f_solo_frame");
extern "C" void op1f_solo_frame(u32 mode) {
    if (mode == (u32)MODE_TAPE) return;
    u32 dropped = 0;
    for (u32 j = 0; j < TRACKS_HELD; ++j) {
        if (!op1f_ext_solo_held[j]) continue;
        op1f_ext_solo_held[j] = 0;
        ++op1f_ext_solo_drops;
        dropped |= 1u << j;
    }
    if (dropped && log_budget) {
        --log_budget;
        STOCK_LOG(3, LOG_FILE, 3, "holds of tracks 5-8 let go (mask 0x%x): the mode %u is not the tape's", dropped, mode);
    }
}
