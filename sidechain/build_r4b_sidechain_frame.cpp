// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4b, the pack sidechain (version 9): sidechain 8's frame unit with one call more, and nothing else
// changed: at the head of each frame, op1f_duck_keeps_frame (sidechain 9's values unit), which reads the ducks
// from a tape the base has just told of and keeps the base's value of the tape what the ducks are. It is called inside
// the pack's frame function, where op1f_value_get and op1f_value_set are answered. What follows is the elder's
// description, which holds.
//
// Step R2: the pack `sidechain` on a card of level 1. Its frame function does, through the pack's own card, what the
// base's frame hook did for this pack until base 19 (M1's measuring unit): the duck's frame call, and the
// duck's two lines on the log.
//
// Once a frame: op1f_duck_frame (Build G's duck unit: the duck page's face and its card timer). Then, once every
// sixteen seconds of the base's clock, a period of the gate (module/build_q_quiet.h): the two lines are printed for
// the first three periods of a start, for a period in which a count they state or the selected track's settings
// changed and the two after it, when the power popup comes up, and otherwise once in 75 periods (twenty minutes).
// Their words are the base's, byte for byte, under the same file and the same tags (`op1f_cfw[11]: duck: ...`). The
// duck's label is asked once a period, printed or not, as the base asked it once a period: it starts its least gain
// and its detector's most anew at each asking, so the line's two measures are the period's, as before.
//
// What it needs: op1f_level_1 (the card), the duck's own counters and label, and the firmware's logger at the
// address the pack's other units call it at. The pack's choice among its pictures is op1f_art_pick_sidechain
// (J5g's unit for the sidechain without 8 tracks), as it is.
//
// Runs on the display thread. Nothing is divided (the module links no runtime). Compiled for the PC with OP1F_HOST
// as it is.

#include "build_r1_card.h"
#include "build_q_quiet.h"

typedef unsigned int u32;
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);

#ifdef OP1F_HOST
extern "C" void mock_log(int level, const char* file, int line, const char* format, ...);
#define STOCK_LOG mock_log
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)             // log(level, file, line, fmt, ...), level 3 = I
#endif

// The duck's unit (Build G's duck unit): its frame call, its label and what it counts.
extern "C" void op1f_duck_frame() OP1F_NAMED("op1f_duck_frame");
extern "C" void op1f_duck_keeps_frame() OP1F_NAMED("op1f_duck_keeps_frame");   // R4b: the tape's value through level 7
extern "C" const char* op1f_duck_label() OP1F_NAMED("op1f_duck_label");
extern "C" u32 op1f_duck_blocks OP1F_NAMED("op1f_duck_blocks");
extern "C" u32 op1f_duck_hits OP1F_NAMED("op1f_duck_hits");
extern "C" u32 op1f_duck_missed OP1F_NAMED("op1f_duck_missed");
extern "C" u32 op1f_duck_frames OP1F_NAMED("op1f_duck_frames");
extern "C" u32 op1f_duck_knob_moves OP1F_NAMED("op1f_duck_knob_moves");
extern "C" u32 op1f_duck_face_toggles OP1F_NAMED("op1f_duck_face_toggles");
extern "C" u32 op1f_duck_guarded OP1F_NAMED("op1f_duck_guarded");
extern "C" u32 op1f_duck_key_selects OP1F_NAMED("op1f_duck_key_selects");
extern "C" u32 op1f_duck_json_saves OP1F_NAMED("op1f_duck_json_saves");
extern "C" u32 op1f_duck_json_loads OP1F_NAMED("op1f_duck_json_loads");
extern "C" u32 op1f_duck_stops OP1F_NAMED("op1f_duck_stops");

static const char LOG_FILE[] = "../src/main/op1f_cfw.cpp";     // the logger shows `op1f_cfw` and the tag, as before
static const char MEASURES[] = ", gain min ";                   // where the label's settings end and its two measures begin
enum { FRAME_LEVEL_1_BYTES = 28, PERIOD_MS = 16000 };

static struct {
    u32 started;
    u32 period_from;                  // the clock at the start of this period
    QuietGate quiet;
} state;

// What the lines state, as one word: the eleven counts and the selected track's settings as the label says them,
// never the label's least gain and its detector's most, which are measures.
static QUIET_OUT_OF_LINE u32 signature(const char* label) {
    u32 s = quiet_start();
    s = quiet_mix(s, op1f_duck_blocks); s = quiet_mix(s, op1f_duck_hits); s = quiet_mix(s, op1f_duck_guarded);
    s = quiet_mix(s, op1f_duck_missed); s = quiet_mix(s, op1f_duck_frames); s = quiet_mix(s, op1f_duck_knob_moves);
    s = quiet_mix(s, op1f_duck_face_toggles); s = quiet_mix(s, op1f_duck_key_selects); s = quiet_mix(s, op1f_duck_json_saves);
    s = quiet_mix(s, op1f_duck_json_loads); s = quiet_mix(s, op1f_duck_stops);
    for (const char* c = label; *c; ++c) {
        const char* m = MEASURES;
        const char* d = c;
        while (*m && *d == *m) { ++m; ++d; }
        if (*m == 0) break;                                           // the measures begin here
        s = quiet_mix(s, (u32)(unsigned char)*c);
    }
    return s;
}

// The gate's reason as a word: its floor is 75 periods, twenty minutes at sixteen seconds a period.
static const char* why_word(int why) {
    return why == QUIET_WHY_FLOOR ? "twenty minutes" : quiet_why(why);
}

// The two lines, in the base's words and order (two, since the logger cuts a line at 260 characters), and the gate's
// line when lines were held before these.
static QUIET_OUT_OF_LINE void say(int why, const char* label) {
    STOCK_LOG(3, LOG_FILE, 11, "duck: %s", label);
    STOCK_LOG(3, LOG_FILE, 12, "duck counts: %u blocks, %u hits, %u guarded, %u missed slot frames, %u page frames, %u knob moves, %u face toggles, %u key selects, %u saves, %u loads, %u tape stops",
              op1f_duck_blocks, op1f_duck_hits, op1f_duck_guarded, op1f_duck_missed,
              op1f_duck_frames, op1f_duck_knob_moves, op1f_duck_face_toggles, op1f_duck_key_selects,
              op1f_duck_json_saves, op1f_duck_json_loads, op1f_duck_stops);
    if (state.quiet.silent_before)
        STOCK_LOG(3, LOG_FILE, 15, "duck quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  state.quiet.silent_before, why_word(why), state.quiet.spoken, state.quiet.skipped);
}

extern "C" void sidechain_frame(const Op1fFrame* now) OP1F_NAMED("sidechain_frame");
extern "C" void sidechain_frame(const Op1fFrame* now) {
    op1f_duck_keeps_frame();                                          // R4b: a tape told of, and the base's value of the tape
    op1f_duck_frame();                                                // the duck page's face and card timer
    if (now == 0 || now->bytes < (unsigned)FRAME_LEVEL_1_BYTES) return;   // a frame of an older base: no clock to go by
    if (!state.started) {
        state.started = 1;
        state.period_from = now->ms;
    }
    int why = quiet_popup(&state.quiet, now->popup != 0);             // the lines once more before the unit is off
    const char* label = 0;
    if (now->ms - state.period_from >= (u32)PERIOD_MS) {              // sixteen seconds: a period of the gate
        state.period_from = now->ms;
        label = op1f_duck_label();
        why |= quiet_period(&state.quiet, signature(label), 0);
    }
    if (!why) return;
    if (label == 0) label = op1f_duck_label();                        // the power popup between two periods
    say(why, label);
}
