// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B3a: the mixer strip of tape tracks 5 and 6, their glyphs on the tape screen, and the
// ReadInterface mute getter for six tracks (OS 1.7.0+0).
//
// What stock does: the four tracks' volumes (Q31) sit at 0x11b01eb0, their pans (Q31 in -0.5..0.5, positive is
// right) at 0x11b01ec0 and the mute gains (0x7fffffff or 0, computed from the flag bytes 0x11902e34 by
// 0x8d50485c) at 0x11b01ed0, all in L1 data. read_block (0x8f01ec34) turns them into each track's playback gain
// matrix, the L1 core's input monitoring (0x11a02b28) reads the selected track's three values through the
// index in P0, and the screens read them back through ReadInterface getters: 0x8d5168f4 builds a float vector
// of the eight values, 0x8d516884(ch) answers "is track ch unmuted" and asserts ch < 4 (ReadInterface.cpp:469,
// fatal). The tape screen shows the selected track's number through four svg nodes "track1".."track4"
// (pointers at this + 0x34..0x40, display byte at node + 0x5c), remembers the shown track at this + 0xd0, which
// its clip loop and record bar use as the selected track, and shift + track key toggles a mute and shows that
// track's number through 0x8fc809b8(track) ([0x848d98e4], reset to -1 when shift is released).
//
// This unit (profile group "build-b3a", with changes in build_b_record.s/.cpp and build_b_play.cpp):
//   * op1f_ext_params: volume, pan and mute gain of tracks 5-8 (5 and 6 in use) in stock's own layout, so the
//     L1 monitoring code reads them unchanged: the two register stubs in build_b_record.s turn a selected track
//     4/5 into the index k with 0x11b01eb0 + 4k = &op1f_ext_params[track - 4], and the VU index likewise into
//     op1f_ext_vu. The B1 playback pass applies the same values with stock's gain law (build_b_play.cpp).
//   * op1f_ext_mix_adjust: tape key held + blue knob = volume, + green knob = pan, of the selected track
//     (stock's own words for tracks 1-4, this unit's for 5-6); op1f_ext_mix_toggle_mute: tape key + shift +
//     track key 1/2 toggles the mute of track 5/6. Both are called from the TapeController filters in
//     build_b_record.cpp. The values of tracks 5-6 live in RAM only (reset at boot) until a later step saves them.
//   * op1f_track_unmuted replaces the ReadInterface getter 0x8d516884 for six tracks (callers: the tape screen
//     with the shown track, the signal-flow screen with the selected track, the mixer screen with 0-3).
//   * op1f_tape_glyph_slot / op1f_tape_glyph_hide serve the two CALL.a stubs in build_b_mix.s that replace the
//     tape screen's node-table indexing: tracks 0-3 use the screen's table, tracks 4-5 the svg nodes "track5"
//     and "track6" of the B3a artwork, looked up once per screen object with the optional flag; a dummy node
//     stands in when the artwork lacks them, so the Build A six-lane file still works, without a digit for 5/6.
//   * op1f_record_track_label: the measurement overlay's " T5 v80 pR20 M" text (replaces the B2 label; since B3b
//     on its own line, as the first line ran past the screen edge at the pan, see the B3a result).
// B3b: the encoder delivers +/-1024 per detent (every event of the B3a test carried exactly that), so the delta is
// turned into detents before the +/-8 clamp; the knob and mute log lines have separate budgets, the knob line at
// most one per second. B3d: op1f_mixer_page and op1f_ext_mix_set_defaults serve the mixer page and the tape JSON
// persistence of Build B's mixer unit; the label reads " MIX 5-8" while the page is held.
// B3c: move_clamped tests the value against the end minus the move, since the room to the
// end, high - value, overflowed a 32-bit word for the pan range at either extreme (the B3b finding A: one detent
// flipped the pan from one end to the other).
// B3e: op1f_ext_mix_adjust notifies the tape model after a change, which writes the tape's JSON as stock's own knob
// handler does on every detent.
// Freestanding like the other units. Logging only from the GUI and gfx threads (key handlers, screen update);
// the audio thread only reads op1f_ext_params and writes op1f_ext_vu.

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);
typedef void (*FlashTrackFn)(int track);
typedef void* (*ObjectFn)();
typedef void (*NotifyFn)(void* object);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)     // Screen::find_node(this, id, optional): 0 when optional and missing
#define STOCK_FLASH_TRACK ((FlashTrackFn)0x8fc809b8u)   // [0x848d98e4] = track: the tape screen shows that track's number while shift is held
#define STOCK_TAPE_MODEL ((ObjectFn)0x8c0c1dd0u)        // the tape model the TapeController observes: a notify writes the tape JSON (0x8c0c116c)
#define STOCK_NOTIFY ((NotifyFn)0x8fca1ed2u)

#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)
#define STOCK_MUTE_FLAGS ((volatile unsigned char*)0x11902e34u)   // tracks 0-3: 1 = muted
#define STOCK_VOLUMES ((volatile i32*)0x11b01eb0u)                // tracks 0-3, Q31; the pans follow at +0x10

// Build D: the ROM soft-float routines stock's MIDI mixer handler 0x8f00de30 calls, in its order, and the two
// 16-bit pairs {lo, hi} it scales a 0..127 value into (0x848c732c: the volume 0..32767; 0x848c6f20: the pan
// -16384..16384), truncated to an int and shifted left 16 into the Q31 word.
typedef u32 (*RomFloat1Fn)(u32);
typedef u32 (*RomFloat2Fn)(u32, u32);
#define ROM_HALF_TO_FLOAT ((RomFloat1Fn)0x04021bdcu)                // the converter stock applies to the pair's halves
#define ROM_VALUE_TO_FLOAT ((RomFloat1Fn)0x040218a0u)               // the converter stock applies to the value
#define ROM_FLOAT_SUB ((RomFloat2Fn)0x040218e0u)
#define ROM_FLOAT_ADD ((RomFloat2Fn)0x040218e2u)
#define ROM_FLOAT_DIV ((RomFloat2Fn)0x040219e4u)
#define ROM_FLOAT_MUL ((RomFloat2Fn)0x04021ac0u)
#define ROM_FLOAT_TO_INT ((RomFloat1Fn)0x04021310u)
#define MIDI_PAIRS ((const u32*)0x848c732cu)                         // [0] the volume pair, [1] the pan pair
#define FLOAT_127 0x42fe0000u

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                   // F2: tracks 5-8 (6 from B2 to F1b)
    EXT_TRACKS = 4,                     // room for tracks 5-8; 5 and 6 in use
    UNIT = 0x7fffffff,                  // 1.0 in Q31
    HALF = 0x40000000,                  // 0.5 in Q31: the pan range is -HALF..HALF
    VOL_STEP = 0x028f5c28,              // UNIT / 50: a full turn of about 50 detents
    PAN_STEP = 0x028f5c28,              // (2 x HALF) / 50
    MAX_DELTA = 8,                      // detents per event
    DETENT = 1024,                      // B3b: the encoder's delta per detent (log of the B3a test)
    DETENT_SHIFT = 10,
    SCREEN_GLYPHS = 0x34,               // TapeScreen: four node pointers
    SCREEN_SHOWN_TRACK = 0xd0,          // TapeScreen: the track whose number is shown
    NODE_DISPLAY = 0x5c,                // svg node: display byte
    LOG_BUDGET = 16,
    KNOB_LOG_BUDGET = 64,               // B3b: knob lines, at most one per second
    CYCLES_PER_SECOND = 400000000
};
static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_mix.cpp";

// Stock's layout, so that 0x11b01eb0 + 4k, + 4(k + 4) and 0x11b01ed0 + 4k with k = index(op1f_ext_params) + j
// are the volume, pan and mute gain of track 4 + j (build_b_record.s computes k).
extern "C" i32 op1f_ext_params[3 * EXT_TRACKS] asm("op1f_ext_params") __attribute__((aligned(16)));
i32 op1f_ext_params[3 * EXT_TRACKS] = {
    UNIT, UNIT, UNIT, UNIT,             // volumes
    0, 0, 0, 0,                         // pans, centre
    UNIT, UNIT, UNIT, UNIT              // mute gains, unmuted
};
extern "C" u16 op1f_ext_vu[EXT_TRACKS] asm("op1f_ext_vu") __attribute__((aligned(4)));
u16 op1f_ext_vu[EXT_TRACKS];

// B3d (Build B's mixer unit): set while the mixer key is held on the mixer's levels page; the screen's value
// sources, the knobs and shift + track key then address tracks 5-8. Cleared on the key's release and by the
// frame hook when the mode is no longer the mixer.
extern "C" volatile unsigned char op1f_mixer_page asm("op1f_mixer_page");
volatile unsigned char op1f_mixer_page;

extern "C" u32 op1f_ext_mix_changes asm("op1f_ext_mix_changes");   // knob and mute changes on tracks 5-6
u32 op1f_ext_mix_changes;

static unsigned char ext_muted[EXT_TRACKS];
static u32 knob_log_budget = KNOB_LOG_BUDGET;
static u32 knob_log_cycles;
static u32 mute_log_budget = LOG_BUDGET;
static u32 bad_track_log_budget = 1;

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

static u32 percent(i32 value, i32 full) {      // |value| / (full / 100), 0..100
    u32 magnitude = (u32)(value < 0 ? -value : value);
    return udiv(magnitude, (u32)full / 100u);
}

static u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}

static int ext_index(int track) {           // 0..3 for tracks 4..7 (F2; 4..5 until F1b), else -1
    u32 j = (u32)track - STOCK_TRACKS;
    return j < EXT_TRACKS ? (int)j : -1;
}

static i32 move_clamped(i32 value, int delta, i32 step, i32 low, i32 high) {
    if (delta > MAX_DELTA) delta = MAX_DELTA;
    if (delta < -MAX_DELTA) delta = -MAX_DELTA;
    if (delta >= 0) {                       // B3c: compare against the end minus the move; the room to the end
        i32 add = step * delta;             // (high - value) overflowed for the pan range at either extreme
        return value > high - add ? high : value + add;
    }
    i32 sub = step * -delta;
    return value < low + sub ? low : value - sub;
}

// Tape key held + blue knob (0) = volume, + green knob (1) = pan, of the selected track. Returns 1 when applied.
extern "C" int op1f_ext_mix_adjust(int track, int knob, int delta) asm("op1f_ext_mix_adjust");
extern "C" int op1f_ext_mix_adjust(int track, int knob, int delta) {
    volatile i32* volume;
    volatile i32* pan;
    if ((u32)track < STOCK_TRACKS) {
        volume = STOCK_VOLUMES + track;
        pan = STOCK_VOLUMES + STOCK_TRACKS + track;
    } else if ((u32)track < EIGHT_TRACKS) {
        volume = &op1f_ext_params[track - STOCK_TRACKS];
        pan = &op1f_ext_params[EXT_TRACKS + track - STOCK_TRACKS];
    } else {
        return 0;
    }
    int detents = delta >= 0 ? (delta + DETENT / 2) >> DETENT_SHIFT : -((-delta + DETENT / 2) >> DETENT_SHIFT);   // B3b
    if (knob == 0) *volume = move_clamped(*volume, detents, VOL_STEP, 0, UNIT);
    else if (knob == 1) *pan = move_clamped(*pan, detents, PAN_STEP, -HALF, HALF);
    else return 0;
    if (track >= STOCK_TRACKS) ++op1f_ext_mix_changes;
    STOCK_NOTIFY(STOCK_TAPE_MODEL());                                   // B3e: the tape JSON is written, as after stock's knobs
    u32 now = cycles();
    if (knob_log_budget && now - knob_log_cycles >= CYCLES_PER_SECOND) {
        --knob_log_budget;
        knob_log_cycles = now;
        STOCK_LOG(3, LOG_FILE, 1, "track %d %s %s%u/100 (knob %d, delta %d = %d detent(s))", track + 1, knob == 0 ? "volume" : "pan",
                  knob == 1 ? (*pan < 0 ? "L" : *pan > 0 ? "R" : "C") : "",
                  knob == 0 ? percent(*volume, UNIT) : percent(*pan, HALF), knob, delta, detents);
    }
    return 1;
}

// B3d: the defaults stock writes for its own words when a tape has no mixer values (1.0, centre); the mutes stay.
extern "C" void op1f_ext_mix_set_defaults() asm("op1f_ext_mix_set_defaults");
extern "C" void op1f_ext_mix_set_defaults() {
    for (int j = 0; j < EXT_TRACKS; ++j) {
        op1f_ext_params[j] = UNIT;
        op1f_ext_params[EXT_TRACKS + j] = 0;
    }
}

// The mute of track 5-8 toggled. Returns 1 when the track exists.
static int ext_mix_toggle(int track) {
    int j = ext_index(track);
    if (j < 0) return 0;
    ext_muted[j] = (unsigned char)!ext_muted[j];
    op1f_ext_params[2 * EXT_TRACKS + j] = ext_muted[j] ? 0 : UNIT;
    ++op1f_ext_mix_changes;
    if (mute_log_budget) {
        --mute_log_budget;
        STOCK_LOG(3, LOG_FILE, 2, "track %d %s", track + 1, ext_muted[j] ? "muted" : "unmuted");
    }
    return 1;
}

// Tape key held + shift + track key 1/2 toggles the mute of track 5/6. Returns 1 when the track exists.
extern "C" int op1f_ext_mix_toggle_mute(int track) asm("op1f_ext_mix_toggle_mute");
extern "C" int op1f_ext_mix_toggle_mute(int track) {
    if (!ext_mix_toggle(track)) return 0;
    STOCK_FLASH_TRACK(track);                                           // as stock's shift + track key: show the number while shift is held
    return 1;
}

// 8 tracks 12: a knob's click on the mixer's page of tracks 5-8 (build_r4b_mixer.cpp) toggles the mute of track 5-8,
// as stock's click on the levels page toggles tracks 1-4's. Stock's click shows no track number, so no flash here:
// without shift held nothing would clear it, and the tape screen would show that track's number.
extern "C" int op1f_ext_mix_click_mute(int track) asm("op1f_ext_mix_click_mute");
extern "C" int op1f_ext_mix_click_mute(int track) {
    return ext_mix_toggle(track);
}

// Build D: stock's MIDI mixer handler (0x8f00de30; the channel in the status byte's low nibble, type 7 volume,
// 10 pan, 9 mute, the value 0..127) returns at 0x8f00f96c for a channel of 4 or more; the trampoline
// op1f_midi_bound (build_d_clip.s) sends channels 4-5 here. Stock's law through the same ROM routines:
// lo + value / 127 x (hi - lo) from the pair, truncated and shifted left 16; the mute is value >= 64. No
// tape-model notify (a controller can stream values): the tape JSON takes the words at its next save, as it
// does for stock's channels.
extern "C" u32 op1f_midi_ext_changes asm("op1f_midi_ext_changes");
u32 op1f_midi_ext_changes;
static u32 midi_log_budget = LOG_BUDGET;
static u32 midi_log_cycles;

static i32 midi_law(u32 pair, u32 value) {
    u32 lo = ROM_HALF_TO_FLOAT((u32)(i32)(short)(pair & 0xffffu));
    u32 hi = ROM_HALF_TO_FLOAT((u32)((i32)pair >> 16));
    u32 range = ROM_FLOAT_SUB(hi, lo);
    u32 fraction = ROM_FLOAT_DIV(ROM_VALUE_TO_FLOAT(value), FLOAT_127);
    u32 scaled = ROM_FLOAT_ADD(ROM_FLOAT_MUL(fraction, range), lo);
    return (i32)(ROM_FLOAT_TO_INT(scaled) << 16);
}

extern "C" void op1f_midi_mixer(i32 channel, i32 type, i32 value) asm("op1f_midi_mixer");
extern "C" void op1f_midi_mixer(i32 channel, i32 type, i32 value) {
    int j = ext_index(channel);
    if (j < 0) return;
    if (value < 0) value = 0;
    if (value > 127) value = 127;
    const char* what;
    if (type == 7) {
        op1f_ext_params[j] = midi_law(MIDI_PAIRS[0], (u32)value);
        what = "volume";
    } else if (type == 10) {
        op1f_ext_params[EXT_TRACKS + j] = midi_law(MIDI_PAIRS[1], (u32)value);
        what = "pan";
    } else if (type == 9) {
        ext_muted[j] = (unsigned char)(value >= 64);
        op1f_ext_params[2 * EXT_TRACKS + j] = ext_muted[j] ? 0 : UNIT;
        what = "mute";
    } else {
        return;
    }
    ++op1f_ext_mix_changes;
    ++op1f_midi_ext_changes;
    u32 now = cycles();
    if (midi_log_budget && now - midi_log_cycles >= CYCLES_PER_SECOND) {
        --midi_log_budget;
        midi_log_cycles = now;
        STOCK_LOG(3, LOG_FILE, 4, "midi: track %d %s %d (type %d)", channel + 1, what, value, type);
    }
}

// Replaces ReadInterface's is-unmuted(ch) 0x8d516884: stock's flag byte for tracks 0-3, this unit's mute for 4-5,
// 0 (muted) for anything else where stock asserted.
extern "C" int op1f_track_unmuted(int ch) asm("op1f_track_unmuted");
extern "C" int op1f_track_unmuted(int ch) {
    if ((u32)ch < STOCK_TRACKS) return STOCK_MUTE_FLAGS[ch] == 0;
    int j = ext_index(ch);
    if (j >= 0) return ext_muted[j] == 0;
    if (bad_track_log_budget) {
        --bad_track_log_budget;
        STOCK_LOG(2, LOG_FILE, 3, "is-unmuted asked for track index %d: answered muted", ch);
    }
    return 0;
}

// The tape screen's glyph nodes for tracks 5 and 6, looked up once per screen object at the first glyph access
// (the screen's first update, whatever track is selected) and hidden on the spot: the OS's SVG parser has no
// `display` attribute, every node starts shown, and stock hides its own digits by code (the constructor, then
// the first update). A missing id gets the dummy node, whose display byte absorbs the writes. (B3f; B3a-B3e
// looked them up only when track 5/6 was first shown, so both digits stayed visible until then.)
static void* glyph_screen;
static void* glyph_nodes[EXT_TRACKS];
static void* glyph_fallback_slot;
static unsigned char dummy_node[0x60] __attribute__((aligned(4)));
static const char* const GLYPH_IDS[EXT_TRACKS] = { "track5", "track6", "track7", "track8" };   // F2: 7 and 8 too

static void ensure_glyphs(void* screen) {
    if (screen == glyph_screen) return;
    glyph_screen = screen;
    for (int j = 0; j < EXT_TRACKS; ++j) {
        const char* id = GLYPH_IDS[j];
        void* node = STOCK_FIND_NODE(screen, id, 1);
        glyph_nodes[j] = node ? node : dummy_node;
        if (node) {
            *((unsigned char*)node + NODE_DISPLAY) = 0;                     // hidden until stock shows it
            STOCK_LOG(3, LOG_FILE, 4, "tape screen glyph '%s' found and hidden", id);
        } else {
            STOCK_LOG(2, LOG_FILE, 4, "tape screen glyph '%s' missing: the artwork has no digit for track %d", id, STOCK_TRACKS + j + 1);
        }
    }
}

// For the stub at 0x8fc842a0: remember the shown track (this + 0xd0, read by the clip loop and the record bar)
// and return the address of the node pointer whose display byte stock sets next.
extern "C" void** op1f_tape_glyph_slot(void* screen, i32 track) asm("op1f_tape_glyph_slot");
extern "C" void** op1f_tape_glyph_slot(void* screen, i32 track) {
    *(i32*)((char*)screen + SCREEN_SHOWN_TRACK) = track;
    ensure_glyphs(screen);                                                  // B3f: before the stock-track return
    if ((u32)track < STOCK_TRACKS) return (void**)((char*)screen + SCREEN_GLYPHS) + track;
    int j = ext_index(track);
    if (j >= 0) return &glyph_nodes[j];
    glyph_fallback_slot = dummy_node;
    return &glyph_fallback_slot;
}

// For the stub at 0x8fc8428c: hide the number shown before (stock: B[node + 0x5c] = 0 through its four-entry table).
extern "C" void op1f_tape_glyph_hide(void* screen, i32 index) asm("op1f_tape_glyph_hide");
extern "C" void op1f_tape_glyph_hide(void* screen, i32 index) {
    void* node;
    ensure_glyphs(screen);                                                  // B3f: before the stock-track branch
    if ((u32)index < STOCK_TRACKS) {
        node = ((void**)((char*)screen + SCREEN_GLYPHS))[index];
    } else {
        int j = ext_index(index);
        node = j >= 0 ? glyph_nodes[j] : dummy_node;
    }
    *((unsigned char*)node + NODE_DISPLAY) = 0;
}

// For the measurement line (the first frame hook's measuring unit, weak): the selected extension track with its strip, or nothing.
static char label[32];

static char* put_text(char* p, const char* s) {
    while (*s) *p++ = *s++;
    return p;
}

static char* put_small(char* p, u32 v) {        // 0..999
    u32 hundreds = udiv(v, 100);
    u32 tens = udiv(v - hundreds * 100, 10);
    u32 ones = v - hundreds * 100 - tens * 10;
    if (hundreds) *p++ = (char)('0' + hundreds);
    if (hundreds || tens) *p++ = (char)('0' + tens);
    *p++ = (char)('0' + ones);
    return p;
}

extern "C" const char* op1f_record_track_label() asm("op1f_record_track_label");
extern "C" const char* op1f_record_track_label() {
    if (op1f_mixer_page) return " MIX 5-8";                              // B3d: the mixer key is held
    i32 track = SELECTED_TRACK;
    if (track < STOCK_TRACKS) return "";
    int j = ext_index(track);
    if (j < 0) return " T?";
    char* p = put_text(label, " T");
    *p++ = (char)('5' + j);
    p = put_text(p, " v");
    p = put_small(p, percent(op1f_ext_params[j], UNIT));
    p = put_text(p, " p");
    i32 pan = op1f_ext_params[EXT_TRACKS + j];
    if (pan == 0) {
        *p++ = 'C';
    } else {
        *p++ = pan < 0 ? 'L' : 'R';
        p = put_small(p, percent(pan, HALF));
    }
    if (ext_muted[j]) p = put_text(p, " M");
    *p = 0;
    return label;
}
