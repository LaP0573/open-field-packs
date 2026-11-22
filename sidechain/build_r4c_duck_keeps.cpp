// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4c (base 27), the pack sidechain (version 10): sidechain 9's values unit (which stays as it was) with
// one change and nothing else: a value the base answers OP1F_NOT_NOW for is tried again at the next frame, as before,
// and is no longer said on the log. From base 27 on the base answers so for a tape's value while a tape load waits to be
// told to the packs (the load came between the base's frame work and this pack's frame); at the next frame the load is
// told and taken first. That is no refusal. Any other answer is said as before.
//
// What follows is the elder's description, which holds.
//
// Step R4b, the pack sidechain (version 9): the ducks kept with the tape through level 7 of the base (module/
// build_r4b_card.h, OP1F_ASK_KEEPS). In the place of the tape's JSON part of Build G's duck unit (G2, G2b), which
// stays as it was for the packs before; sidechain/build_r4b_duck.cpp is that unit with the part replaced by two functions
// of whole numbers, op1f_duck_values and op1f_duck_take.
//
// What changed. Until base 26 the ducks were written into the tape's file and read from it at the tape file's two
// places, which the pack 8 tracks held (it called the duck's functions there) or, where it was away, this pack itself
// (J5g's unit for the sidechain without 8 tracks). Base 26 holds those places, and this pack keeps its member through its card: its
// ask of kind 6 lists `op1f_duck`, with the tape, an array of whole numbers (the tag 89, then for each of the eight
// tracks its ten settings and its bypass). The same member in the same form, with 8 tracks or without it: so a tape this
// pack writes holds the same bytes as one G2b's unit wrote for the same ducks, and every tape it wrote is read here as
// there.
//
//   A tape switched to: the base takes the member from the tape's document and tells this pack at its next frame
//   (`changed`, OP1F_KEPT_LOADED). The frame reads it (op1f_value_get) and hands its numbers to op1f_duck_take, which
//   takes the ducks as G2b's load did, or the defaults when the tape has none or the member is not a list of whole
//   numbers.
//   A tape the unit has just made, new or copied (OP1F_KEPT_MADE, the firmware's load with its flag set): the ducks'
//   defaults, as G2b's load gave them with the flag set, whatever a copy's file holds; the base holds the copy's value
//   as its file had it, and the defaults set here are written over it at the copy's next write, as G2b wrote them.
//   A tape written: the base writes the value it holds. So once a tape has been loaded, this pack sets the value
//   whenever the ducks' numbers (op1f_duck_values) differ from what the base holds; never before: the base never writes
//   over a tape's own ducks with ducks that were not read from it. The duck still tells the tape's model of a change
//   (sidechain/build_r4b_duck.cpp, at most twice a second), as G2 did, so that the firmware writes the tape.
//
// The threads. `changed` comes on the display thread outside the pack's frame function: it notes a bit and does nothing
// else. op1f_duck_keeps_frame runs from the pack's frame function (sidechain/build_r4b_sidechain_frame.cpp). The ducks'
// settings are written by the page's handlers on the key thread and read here one byte at a time, as G2's save read
// them.
//
// Compiled for the PC with OP1F_HOST defined (for tests this tree does not hold).
// Nothing is divided (the module links no runtime).

#include "build_r4b_card.h"

typedef unsigned int u32;

// The pack's own card (tools/pack_build.py writes it from the pack's recipe).
extern "C" const Op1fCard op1f_card_sidechain OP1F_NAMED("op1f_card_sidechain");

// The duck (sidechain/build_r4b_duck.cpp).
extern "C" int op1f_duck_values(int* out) OP1F_NAMED("op1f_duck_values");
extern "C" void op1f_duck_take(const int* values, int count) OP1F_NAMED("op1f_duck_take");
extern "C" unsigned op1f_duck_json_saves OP1F_NAMED("op1f_duck_json_saves");

static const char LOG_FILE[] = "../src/gui/op1f_duck.cpp";      // G2's lines, under G2's tag
static const char KEY[] = "op1f_duck";

enum { VALUES = 1 + 8 * 11, TEXT_ROOM = OP1F_VALUE_MOST + 1, REFUSED_LINES = 8 };
enum { HELD_UNREAD = 0, HELD_NONE = 1, HELD_VALUES = 2, HELD_OTHER = 3 };

static volatile u32 told;                                        // 1 << OP1F_KEPT_..., from `changed`, taken by the frame
static volatile u32 last_load;                                   // the later of the tape's loads told: OP1F_KEPT_LOADED or OP1F_KEPT_MADE
static u32 tape_known;                                           // a tape was told: its ducks were read
static u32 held_how;                                             // what the base holds for the tape, as last read or set
static int held_count;
static int held[VALUES];
static int now_values[VALUES], read_values[VALUES];
static u32 refused_lines;
static char text[TEXT_ROOM];

static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };

static u32 put_number(char* out, u32 at, int v) {
    u32 m = v < 0 ? 0u - (u32)v : (u32)v;
    if (v < 0) out[at++] = '-';
    int started = 0;
    for (int k = 0; k < 10; ++k) {
        u32 d = 0;
        while (m >= TENS[k]) { m -= TENS[k]; ++d; }
        if (d || started || k == 9) {
            out[at++] = (char)('0' + d);
            started = 1;
        }
    }
    return at;
}

static const char* skip(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
    return p;
}

static const char* number_at(const char* p, int* out, int* ok) {
    int negative = 0;
    if (*p == '-') { negative = 1; ++p; }
    if (*p < '0' || *p > '9') { *ok = 0; return p; }
    u32 m = 0;
    while (*p >= '0' && *p <= '9') {
        u32 d = (u32)(*p - '0');
        if (m > 214748364u || (m == 214748364u && d > (negative ? 8u : 7u))) { *ok = 0; return p; }
        m = m * 10u + d;
        ++p;
    }
    if (*p == '.' || *p == 'e' || *p == 'E') { *ok = 0; return p; }
    *out = negative ? (int)(0u - m) : (int)m;
    return p;
}

// The member's text as numbers: a list of whole numbers, of which the first VALUES are copied (G2b's load read no
// further). Answers how many the list holds; -1 for anything else.
static int read_list(const char* t, int* out) {
    const char* p = skip(t);
    if (*p != '[') return -1;
    p = skip(p + 1);
    if (*p == ']') return *skip(p + 1) == 0 ? 0 : -1;
    int n = 0;
    for (;;) {
        int ok = 1, v = 0;
        p = number_at(p, &v, &ok);
        if (!ok) return -1;
        if (n < VALUES) out[n] = v;
        ++n;
        p = skip(p);
        if (*p == ',') { p = skip(p + 1); continue; }
        if (*p == ']' && *skip(p + 1) == 0) return n;
        return -1;
    }
}

extern "C" void op1f_duck_keeps_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_duck_keeps_changed");
extern "C" void op1f_duck_keeps_changed(unsigned where, unsigned which, unsigned how) {
    if (where != (unsigned)OP1F_KEEP_TAPE || which != 0 || how > 31u) return;
    told |= 1u << how;
    if (how == (unsigned)OP1F_KEPT_LOADED || how == (unsigned)OP1F_KEPT_MADE) last_load = how;   // of two loads in one frame, the later counts
}

// A tape was loaded: its ducks, or the defaults (`made`: a tape the unit has just made, the defaults whatever it holds).
static void loaded(int made) {
    tape_known = 1;
    int got = op1f_value_get(&op1f_card_sidechain, OP1F_KEEP_TAPE, 0, KEY, text, sizeof text);
    int n = got >= 0 ? read_list(text, read_values) : -1;
    held_how = got == OP1F_NO_FILE ? (u32)HELD_NONE : n >= 0 && n <= VALUES ? (u32)HELD_VALUES : (u32)HELD_OTHER;
    held_count = n > 0 ? (n < VALUES ? n : VALUES) : 0;
    for (int i = 0; i < held_count; ++i) held[i] = read_values[i];
    op1f_duck_take(read_values, made ? 0 : held_count);
}

extern "C" void op1f_duck_keeps_frame() OP1F_NAMED("op1f_duck_keeps_frame");
extern "C" void op1f_duck_keeps_frame() {
    u32 bits = told;
    told = 0;
    if (bits & (1u << OP1F_KEPT_LOADED | 1u << OP1F_KEPT_MADE)) loaded(last_load == (u32)OP1F_KEPT_MADE);   // made: G2b's load with the flag set
    if (bits & (1u << OP1F_KEPT_SAVED)) ++op1f_duck_json_saves;
    if (!tape_known) return;                                     // no tape told yet: nothing is set over a tape's own ducks
    int n = op1f_duck_values(now_values);
    if (n < 0 || n > VALUES) return;
    if (held_how == (u32)HELD_VALUES && held_count == n) {
        int same = 1;
        for (int i = 0; i < n; ++i) if (held[i] != now_values[i]) same = 0;
        if (same) return;
    }
    u32 at = 0;
    text[at++] = '[';
    for (int i = 0; i < n; ++i) {
        if (i) text[at++] = ',';
        at = put_number(text, at, now_values[i]);
    }
    text[at++] = ']';
    text[at] = 0;
    int answer = op1f_value_set(&op1f_card_sidechain, OP1F_KEEP_TAPE, 0, KEY, text);
    if (answer == 0) {
        held_how = HELD_VALUES;
        held_count = n;
        for (int i = 0; i < n; ++i) held[i] = now_values[i];
        return;
    }
    held_how = HELD_UNREAD;                                      // tried again at the next frame
    if (answer == OP1F_NOT_NOW) return;                          // R4c: not now (a tape load not told yet): no refusal, nothing said
    if (refused_lines < (u32)REFUSED_LINES) {
        ++refused_lines;
        op1f_log(2, LOG_FILE, 7, "duck: the base did not take the ducks' %d numbers (answer %d)", n, answer);
    }
}
