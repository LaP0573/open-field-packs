// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4c (base 27), the pack 8 tracks (version 10): 8 tracks 9's values unit (which stays as it was) with
// one change and nothing else: a value the base answers OP1F_NOT_NOW for is tried again at the next frame, as before,
// and is no longer said on the log. From base 27 on the base answers so for a tape's value while a tape load waits to be
// told to the packs (the load came between the base's frame work and this pack's frame); at the next frame the load is
// told and taken first. That is no refusal. Any other answer is said as before.
//
// What follows is the elder's description, which holds.
//
// Step R4b, the pack 8 tracks (version 9): tracks 5 to 8's strip and the true selected track kept with the tape, through
// level 7 of the base (module/build_r4b_card.h, OP1F_ASK_KEEPS). In the place of the persistence of module/
// Build B's mixer unit (B3d, B3e), which stays as it was for the packs before; 8 tracks/build_r4b_mixer.cpp is that unit with
// its persistence taken out and everything else as it was.
//
// What changed. Until base 26 this pack held the two places of the tape's file (the save's last set of a member, the
// load's entry) and wrote its members there itself, and the sidechain's and the cross-map's beside them. Base 26 holds
// those two places, and this pack keeps its members through its card: its ask of kind 6 lists `op1f_mixer_ext` (the
// strip: an array of eight whole numbers, the Q31 volumes of tracks 5 to 8 and then their pans) and `op1f_track` (the
// selected track, 0 to 7, which the firmware's own `track` holds only up to 3), both with the tape. They are the same
// members in the same form, so a tape this pack writes holds the same bytes as one B3d's unit wrote for the same
// strip and track, and every tape that unit wrote is read here as there. The sidechain keeps its own member through
// its own card now (sidechain 9's values unit); the cross-map's id, `op1f_tape_id`, stays what it has been since
// K2: set into the tape's document by the cross-map's frame work through the tape's setter (8 tracks/build_l3_maps.cpp),
// a call and no place, and kept there by the tape's own document. Its second set at every write of the tape, which
// rode on this pack's place, is gone with the place.
//
//   A tape switched to: the base takes the members from the tape's document and tells this pack at its next frame
//   (`changed`, OP1F_KEPT_LOADED). The frame reads them (op1f_value_get), as B3d's load took them after the firmware's:
//   the strip, each volume held to 0..1.0 and each pan to -0.5..0.5, and the selected track put back when it names track
//   5 to 8 and the six-track cache is up; a tape whose file has no strip leaves the strip as it was, as the firmware
//   leaves its own four tracks'.
//   A tape the unit has just made, new or copied (OP1F_KEPT_MADE, the firmware's own load with its flag set): the
//   strip back to its defaults (1.0 and the centre), as the firmware sets its own four tracks' and B3d set these four,
//   whatever a copy's file holds, and the track not put back. The base holds a copy's values as its file had them, and
//   the strip set here is written over them at the copy's next write, as B3d wrote it.
//   A tape written: the base writes the values it holds. So once a tape has been loaded, this pack sets each member
//   whenever what B3d's save would have written differs from what the base holds: the strip as op1f_ext_params holds
//   it, the selected track as the firmware's word holds it. Nothing is set before the first tape is told: the base
//   never writes over a tape's own values with values that were not read from it.
//
// The threads. `changed` comes on the display thread outside the pack's frame function: it notes a bit and does
// nothing else. op1f_tape_keeps_frame runs from the pack's frame function (8 tracks/build_r4b_8_tracks_frame.cpp). The
// strip's words are written by the mixer page on the key thread and read here whole, one word at a time, as B3d's
// save read them.
//
// Compiled for the PC with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for tests this
// tree does not hold). Nothing is divided by a number that is not a power of two but through udiv (the module links
// no runtime).

#include "build_r4b_card.h"
#ifdef OP1F_HOST
#include "mock_features_r4b.h"                                   // the PC's stand-ins: the selected track's word
#endif

typedef unsigned int u32;
typedef int i32;

#ifndef OP1F_HOST
#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)             // the GUI's selected track: the record track
#endif

// The pack's own card (tools/pack_build.py writes it from the pack's recipe).
extern "C" const Op1fCard op1f_card_8_tracks OP1F_NAMED("op1f_card_8_tracks");

// The pack's other units (8 tracks/build_b_mix.cpp, build_b_cache.cpp, build_r4b_mixer.cpp).
extern "C" i32 op1f_ext_params[12] OP1F_NAMED("op1f_ext_params");             // volumes 5-8, pans 5-8, mute gains
extern "C" void op1f_ext_mix_set_defaults() OP1F_NAMED("op1f_ext_mix_set_defaults");
extern "C" unsigned op1f_cache_tracks OP1F_NAMED("op1f_cache_tracks");        // 8 with the heap-1 cache
extern "C" unsigned op1f_ext_params_saves OP1F_NAMED("op1f_ext_params_saves");
extern "C" unsigned op1f_ext_params_loads OP1F_NAMED("op1f_ext_params_loads");
extern "C" unsigned op1f_ext_track_restores OP1F_NAMED("op1f_ext_track_restores");

static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_mixer.cpp";   // B3d's lines, under B3d's tag
static const char EXT_KEY[] = "op1f_mixer_ext";
static const char TRACK_KEY[] = "op1f_track";

enum {
    STOCK_TRACKS = 4, EIGHT_TRACKS = 8, EXT_TRACKS = 4, EXT_VALUES = 2 * EXT_TRACKS,
    UNIT = 0x7fffffff, HALF = 0x40000000,
    SAVE_LOG_BUDGET = 64, LOG_BUDGET = 16, SAVE_LINE_MS = 1000, REFUSED_LINES = 8, TEXT_ROOM = 256
};
// What the base holds for a member, as this unit last read or set it.
enum { HELD_UNREAD = 0, HELD_NONE = 1, HELD_VALUES = 2, HELD_OTHER = 3 };

struct Held { u32 how; i32 values[EXT_VALUES]; };
static Held held_ext, held_track;
static volatile u32 told;                                        // 1 << OP1F_KEPT_..., from `changed`, taken by the frame
static volatile u32 last_load;                                   // the later of the tape's loads told: OP1F_KEPT_LOADED or OP1F_KEPT_MADE
static u32 tape_known;                                           // a tape was told: its values were read
static u32 save_log_budget = SAVE_LOG_BUDGET, save_log_ms, save_logged;
static u32 load_log_budget = LOG_BUDGET, refused_lines;
static char text[TEXT_ROOM];

static u32 udiv(u32 n, u32 d) {                                  // restoring shift-subtract division; d == 0 yields 0
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static int percent(i32 value, i32 full) {                        // signed percent of the full scale
    u32 magnitude = (u32)(value < 0 ? -value : value);
    int p = (int)udiv(magnitude, (u32)full / 100u);
    return value < 0 ? -p : p;
}

// ---------------------------------------------------------------- the values' text

static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };

static u32 put_number(char* out, u32 at, i32 v) {
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

static const char* number_at(const char* p, i32* out, int* ok) {
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
    *out = negative ? (i32)(0u - m) : (i32)m;
    return p;
}

// The strip's text: an array of whole numbers. B3d read elements 0 to 7 of the array whatever the array held, a
// missing element as 0; so here: the first EXT_VALUES numbers are taken, the rest are 0. Answers how many numbers the
// array holds, -1 when the text is not an array of whole numbers.
static int read_strip(const char* t, i32* values) {
    for (int i = 0; i < EXT_VALUES; ++i) values[i] = 0;
    const char* p = skip(t);
    if (*p != '[') return -1;
    p = skip(p + 1);
    if (*p == ']') return *skip(p + 1) == 0 ? 0 : -1;
    for (int i = 0;; ++i) {
        int ok = 1;
        i32 v = 0;
        p = number_at(p, &v, &ok);
        if (!ok) return -1;
        if (i < EXT_VALUES) values[i] = v;
        p = skip(p);
        if (*p == ',') { p = skip(p + 1); continue; }
        return *p == ']' && *skip(p + 1) == 0 ? i + 1 : -1;
    }
}

static int read_track(const char* t, i32* value) {
    int ok = 1;
    const char* p = number_at(skip(t), value, &ok);
    return ok && *skip(p) == 0;
}

// ---------------------------------------------------------------- the base's word (the display thread, outside the frame function)

extern "C" void op1f_tape_keeps_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_tape_keeps_changed");
extern "C" void op1f_tape_keeps_changed(unsigned where, unsigned which, unsigned how) {
    if (where != (unsigned)OP1F_KEEP_TAPE || which != 0 || how > 31u) return;
    told |= 1u << how;
    if (how == (unsigned)OP1F_KEPT_LOADED || how == (unsigned)OP1F_KEPT_MADE) last_load = how;   // of two loads in one frame, the later counts
}

// ---------------------------------------------------------------- once a frame (the pack's frame function)

// The member as the base holds it now: none, its values read into `h` (the strip: exactly eight numbers, as this pack
// writes it), or a text that is not this pack's form. Answers 1 when the text is one B3d's load took values from (the
// strip: an array of whole numbers of any length).
static int read_member(const char* key, Held* h, int strip) {
    int got = op1f_value_get(&op1f_card_8_tracks, OP1F_KEEP_TAPE, 0, key, text, sizeof text);
    if (got == OP1F_NO_FILE) { h->how = HELD_NONE; return 0; }
    if (got < 0) { h->how = HELD_OTHER; return 0; }
    if (strip) {
        int n = read_strip(text, h->values);
        h->how = n == EXT_VALUES ? (u32)HELD_VALUES : (u32)HELD_OTHER;
        return n >= 0;
    }
    int ok = read_track(text, &h->values[0]);
    h->how = ok ? (u32)HELD_VALUES : (u32)HELD_OTHER;
    return ok;
}

// A tape was loaded: its strip and its track, as B3d's load after the firmware's took them. `made`: a tape the unit has
// just made, new or copied: the strip's defaults, as B3d's load with the firmware's flag set.
static void loaded(int made) {
    read_member(TRACK_KEY, &held_track, 0);
    int strip = read_member(EXT_KEY, &held_ext, 1);
    tape_known = 1;
    if (made) {
        op1f_ext_mix_set_defaults();
        return;
    }
    if (held_track.how == (u32)HELD_VALUES) {                    // B3e: the selected extension track comes back
        i32 track = held_track.values[0];
        if (track >= STOCK_TRACKS && track < EIGHT_TRACKS && op1f_cache_tracks >= (unsigned)EIGHT_TRACKS) {
            SELECTED_TRACK = track;
            ++op1f_ext_track_restores;
            if (load_log_budget) {
                --load_log_budget;
                op1f_log(3, LOG_FILE, 5, "track %d restored from the tape json", track + 1);
            }
        }
    }
    if (!strip) {                                                // none of its own: the strip stays as it was
        if (load_log_budget) {
            --load_log_budget;
            op1f_log(3, LOG_FILE, 2, "tape json without %s: tracks 5-8 keep their strip", EXT_KEY);
        }
        return;
    }
    i32 values[EXT_VALUES];
    for (int i = 0; i < EXT_VALUES; ++i) {
        i32 v = held_ext.values[i];
        if (i < EXT_TRACKS) {                                    // a volume: 0..1.0
            if (v < 0) v = 0;
            if (v > UNIT) v = UNIT;
        } else {                                                 // a pan: -0.5..0.5
            if (v < -HALF) v = -HALF;
            if (v > HALF) v = HALF;
        }
        values[i] = v;
    }
    for (int i = 0; i < EXT_VALUES; ++i) op1f_ext_params[i] = values[i];
    ++op1f_ext_params_loads;
    if (load_log_budget) {
        --load_log_budget;
        op1f_log(3, LOG_FILE, 3, "mixer params of tracks 5-8 loaded: volumes %d %d %d %d pans %d %d %d %d",
                 percent(values[0], UNIT), percent(values[1], UNIT), percent(values[2], UNIT), percent(values[3], UNIT),
                 percent(values[4], HALF), percent(values[5], HALF), percent(values[6], HALF), percent(values[7], HALF));
    }
}

// The base wrote the tape: B3d's line of a save, at most once a second.
static void saved() {
    ++op1f_ext_params_saves;
    u32 now = op1f_ms();
    if (save_log_budget && (!save_logged || now - save_log_ms >= (u32)SAVE_LINE_MS)) {
        --save_log_budget;
        save_logged = 1;
        save_log_ms = now;
        op1f_log(3, LOG_FILE, 1, "mixer params of tracks 5-8 saved: volumes %d %d %d %d pans %d %d %d %d, track %d",
                 percent(op1f_ext_params[0], UNIT), percent(op1f_ext_params[1], UNIT),
                 percent(op1f_ext_params[2], UNIT), percent(op1f_ext_params[3], UNIT),
                 percent(op1f_ext_params[4], HALF), percent(op1f_ext_params[5], HALF),
                 percent(op1f_ext_params[6], HALF), percent(op1f_ext_params[7], HALF), (i32)SELECTED_TRACK + 1);
    }
}

// One member made what B3d's save would have written, when the base holds something else.
static void keep(const char* key, Held* h, const i32* now, int count) {
    if (h->how == (u32)HELD_VALUES) {
        int same = 1;
        for (int i = 0; i < count; ++i) if (h->values[i] != now[i]) same = 0;
        if (same) return;
    }
    u32 at = 0;
    if (count > 1) text[at++] = '[';
    for (int i = 0; i < count; ++i) {
        if (i) text[at++] = ',';
        at = put_number(text, at, now[i]);
    }
    if (count > 1) text[at++] = ']';
    text[at] = 0;
    int answer = op1f_value_set(&op1f_card_8_tracks, OP1F_KEEP_TAPE, 0, key, text);
    if (answer == 0) {
        h->how = HELD_VALUES;
        for (int i = 0; i < count; ++i) h->values[i] = now[i];
        return;
    }
    h->how = HELD_UNREAD;                                        // tried again at the next frame
    if (answer == OP1F_NOT_NOW) return;                          // R4c: not now (a tape load not told yet): no refusal, nothing said
    if (refused_lines < (u32)REFUSED_LINES) {
        ++refused_lines;
        op1f_log(2, LOG_FILE, 7, "tape json: the base did not take %s (answer %d)", key, answer);
    }
}

extern "C" void op1f_tape_keeps_frame() OP1F_NAMED("op1f_tape_keeps_frame");
extern "C" void op1f_tape_keeps_frame() {
    u32 bits = told;
    told = 0;
    if (bits & (1u << OP1F_KEPT_LOADED | 1u << OP1F_KEPT_MADE)) loaded(last_load == (u32)OP1F_KEPT_MADE);   // made: B3d's load with the flag set
    if (!tape_known) return;                                     // no tape told yet: nothing is set over a tape's own values
    i32 strip[EXT_VALUES];
    for (int i = 0; i < EXT_VALUES; ++i) strip[i] = op1f_ext_params[i];
    i32 track = SELECTED_TRACK;                                  // the true selected track (the firmware's "track" holds min(track, 3))
    keep(EXT_KEY, &held_ext, strip, EXT_VALUES);
    keep(TRACK_KEY, &held_track, &track, 1);
    if (bits & (1u << OP1F_KEPT_SAVED)) saved();
}
