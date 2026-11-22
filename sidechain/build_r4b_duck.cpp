// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4b, the pack sidechain (version 9): Build G's duck unit with its tape JSON part replaced, and nothing else
// changed. Until base 26 the tape's save and load called op1f_duck_json_write and op1f_duck_json_read, from the places
// of the tape's file that 8 tracks (or, where it was away, this pack's J5g's unit for the sidechain without 8 tracks) held; base 26
// holds those places, and this pack keeps the ducks with the tape through level 7 (sidechain 9's values unit).
// In their place stand op1f_duck_values, the ducks as the same whole numbers op1f_duck_json_write put into the tape's
// member, and op1f_duck_take, the ducks taken from those numbers as op1f_duck_json_read took them from the member (the
// tag, G2's array renumbered, each value held to its range, a track never its own source, the defaults when the tape
// has none). The engine, the faces, the knobs, the page and the drawing are the elder's, line for line; what follows is
// the elder's description, which holds but for its paragraph on the tape JSON.
//
// Build G: the duck, a tape-level sidechain. Each track may name the track that ducks it: a hit on the source dips the
// target's gain by the amount, holds
// it, and lets it back over the release. The page lives under shift + mixer as a second face of the signal-flow
// screen (the mode-13 screen): shift + mixer again shows the stock flow screen, and again the duck page.
//
// Image G1: the engine, the faces, the knobs, the page. Image G2 (from the notes
// on G1c): the release shape cycled by a press of the orange knob (out, in, linear, S), bypass on a press of the
// blue knob, the target by the track keys on the page (the mixer key held + 1-4 for tracks 5-8, a tap of the
// mixer key leaving to the levels page), the beat source (the tape's tempo, a hit per division), the animation
// (the splash, the ripples, the bubbles from the beak, the idle bob and blink, the swim back; the stone for the
// beat source, whose hits are known ahead), the dive path starting ahead of the hit point by `pre`, and the
// settings saved with the tape as "op1f_duck". Image G2b (from G2's try on the unit): the detector
// heard right (two one-pole stages on the signed waveform at tone's cutoff before rectifying, where G1 to G2
// smoothed the rectified magnitude and no tone could tell a kick from a snare; an onset rule against a slow
// envelope in place of the fixed 6 dB re-arm, so a second kick fires), a hit during the hold restarting the hold
// instead of being swallowed, the detector's trace under the source digit with the sens gate through it and the
// hits above it, and the divisions in order (1/2, 1/4, 1/8, 1/16).
//
// What stock does, as read (sections 29.1, 30.1 and 31.1):
//   * The key dispatchers call the current controller's vtable: key-down 0x8c0b1ae8 -> [vtable + 0xc](this, event*)
//     (event[0] = the key; the knob presses are the keys 54-57, blue to orange), shifted key-down 0x8c0b1b58 ->
//     [+0x10](this, event*), key-up 0x8c0b1b8c -> [+0x14](this, key), the encoders 0x8c0b13f8 -> [+0x18](this,
//     index + 4 while shift is held (0x8fc24ca4), delta); a non-zero return consumes the key, zero sends it to the
//     fallback. The signal-flow controller's vtable at 0x8401004c holds the base class's return-0 stubs and an empty
//     encoder handler (0x8c0c1018); the profile's four "word" edits (group build-g1) point those slots at the
//     handlers below. The fallback's mixer key switches with 0x8c0bce20(3) and writes 3 to the main-mode word
//     0x84012458; its track keys switch back to the main mode and re-send the key there.
//   * The tape block driver writes 0x7fffffff into the four hold-to-solo words at 0x11905aa8 every block (0 for the
//     tracks a held track key solos out) and read_block's gain section multiplies them into tracks 1-4's ramped gains;
//     the module's read_block wrapper (build_b_play.cpp) calls op1f_duck_block_gains() before stock's read_block and
//     op1f_duck_detect() after it, and ext_gain_targets multiplies op1f_duck_gain[] into tracks 5-8.
//   * The tape's samples are 32-bit fixed point, 1.31 (the gains multiply them as Q31 fractions): 0 dB is 0x7fffffff.
//   * The tape's tempo is the float at 0x8490c8fc (the getter 0x8fc9ca18 the JSON writer uses for
//     "tempobpmdecimal", bpm x 10 rounded; the loader's default 1200); the tape position in 1/64 frames at
//     0x11905a18. A track change goes through the selected-track word 0x11905a08 and a tape-model notify
//     0x8fca1ed2(0x8c0c1dd0()), as build_b_record.cpp does for tracks 5-8, with stock's two conditions (the
//     transport mode at 0x11905a04 not 1, the clips idle 0x8d4ed6e4()).
//   * An svg node's matrix is nine 22.10 words at node + 4 (x' = m0 x + m3 y + m6, y' = m1 x + m4 y + m7, the third
//     column 0, 0, 1024; confirmed on the device by G1b's diagnostic lines); 0x8c03dbbe(node, matrix) stores them;
//     a node's display byte is at +0x5c; the needle transform comes from 0x8fc2facc(anchor x, anchor y - 1024, value).
//   * The tape JSON: the writer's retarget op1f_tape_json_set_mixer (Build B's mixer unit) and the loader's continuation
//     op1f_tape_json_loaded call op1f_duck_json_write / op1f_duck_json_read through weak references; the json API
//     (12-byte values; from-int 0x8c083b1c, a null 0x8c085ba8(&v, 0), an array after the first push, push 0x8c085d54, destroy 0x8c0844e0,
//     the db set 0x8d4f24dc(2, key, &v); has 0x8c085068, get 0x8c084f74, at 0x8c084ee0, int 0x8c084840) as B3d uses it.
//
// The artwork: the served signalflow.svg carries the stock drawing in the group `flow` and the
// page in `duck`; the module shows one and hides the other. The page's nodes carry ids with the prefix dk (listed in
// the tool); unit lines (0,0)-(1,0), unit circles and origin dots take their place through matrices built here.
//
// J5g: the duck stands by itself. The three names it took from the
// tape's units are optional, the source knob walks the tracks the tape has here (four on the original firmware's
// tape), a stored source the tape has no track for shows as off and is kept, and the hooks that call this unit
// when the tape's units are away are in J5g's unit for the sidechain without 8 tracks. With the tape's units there nothing changes.
//
// Audio thread: op1f_duck_block_gains, op1f_duck_detect (no logging, no floats, integer division by shift-subtract).
// GUI thread: the handlers, op1f_duck_screen (per update of the signal-flow screen), op1f_duck_frame (per frame),
// the JSON functions.

#include "build_g_duck_tables.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;
typedef short i16;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);   // 0x8fc25cc0: 0 when optional and missing
typedef void (*ApplyMatrixFn)(void* node, const u32* matrix);               // 0x8c03dbbe
typedef void (*BoundsFn)(void* node, i32* box);                             // a node's vtable slot 3: min x, max x, min y, max y (22.10)
typedef int (*ShiftHeldFn)();                                               // 0x8fc24ca4: non-zero while shift is held
typedef u32 (*FloatAddFn)(u32 a, u32 b);                                    // ROM 0x040218e2
typedef u32 (*FloatMulFn)(u32 a, u32 b);                                    // ROM 0x04021ac0
typedef i32 (*FloatToIntFn)(u32 a);                                         // ROM 0x04020fd8
typedef u32 (*DbToAmpFn)(u32 db);                                           // 0x8f0061a8: 10 ^ (db / 20)
typedef i32 (*ToFixedFn)(u32 amp);                                          // 0x8fc356a0: ((amp x 1.4) - 1) x 1024
typedef void (*ModeSwitchFn)(int mode);                                     // 0x8c0bce20
typedef int (*IntFn)();
typedef void* (*ObjectFn)();
typedef void (*NotifyFn)(void* object);
typedef void* (*JsonRootFn)();                                              // 0x8d4f2b80: the tape db's root object
typedef int (*JsonHasFn)(void* json, const char* key);                      // 0x8c085068
typedef void* (*JsonGetFn)(void* json, const char* key);                    // 0x8c084f74: the member (a reference)
typedef void* (*JsonAtFn)(void* json, int index);                           // 0x8c084ee0: the array element (a reference)
typedef int (*JsonIntFn)(void* json);                                       // 0x8c084840
typedef void (*JsonCtorFn)(void* json, int kind);                           // 0x8c085ba8(json, kind): 0 a null, an array after the first push (kind 6 is an array; H1g)
typedef void (*JsonFromIntFn)(void* json, int value);                       // 0x8c083b1c
typedef void (*JsonPushFn)(void* array, void* value);                       // 0x8c085d54
typedef void (*JsonDtorFn)(void* json);                                     // 0x8c0844e0
typedef void (*DbSetFn)(int db, const char* key, void* value);              // 0x8d4f24dc

struct Slot {                 // tape_chunks.cpp slot descriptor (build_b_play.cpp)
    u8 valid;
    u8 stale;
    u8 write_pending;
    u8 pad;
    u16 dirty_tracks;
    i16 logical_chunk;
    u16 index;
    u16 pad2;
    i32 frame_pos;
    u8* data;
};
typedef Slot* (*SlotForLogicalFn)(int logical);                             // 0x8fc21280

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_APPLY_MATRIX ((ApplyMatrixFn)0x8c03dbbeu)
#define STOCK_SHIFT_HELD ((ShiftHeldFn)0x8fc24ca4u)
#define STOCK_FLOAT_ADD ((FloatAddFn)0x040218e2u)
#define STOCK_FLOAT_MUL ((FloatMulFn)0x04021ac0u)
#define STOCK_FLOAT_TO_INT ((FloatToIntFn)0x04020fd8u)
#define STOCK_DB_TO_AMP ((DbToAmpFn)0x8f0061a8u)
#define STOCK_METER_FIXED ((ToFixedFn)0x8fc356a0u)
#define STOCK_SLOT_FOR_LOGICAL ((SlotForLogicalFn)0x8fc21280u)
#define STOCK_MODE_SWITCH ((ModeSwitchFn)0x8c0bce20u)
#define STOCK_CLIPS_IDLE ((IntFn)0x8d4ed6e4u)
#define STOCK_TAPE_MODEL ((ObjectFn)0x8c0c1dd0u)
#define STOCK_NOTIFY ((NotifyFn)0x8fca1ed2u)
#define STOCK_JSON_ROOT ((JsonRootFn)0x8d4f2b80u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_DB_SET ((DbSetFn)0x8d4f24dcu)
#define METER_SMOOTH_GAIN (*(volatile i32*)0x84c9752cu)
#define MODE_WORD (*(volatile i32*)0x8400f978u)
#define MAIN_MODE_WORD (*(volatile i32*)0x84012458u)
#define SOLO_WORDS ((volatile i32*)0x11905aa8u)
#define POSITION64 (*(volatile i32*)0x11905a18u)
#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)
#define TRANSPORT_MODE (*(volatile i32*)0x11905a04u)
#define TEMPO_FLOAT (*(volatile u32*)0x8490c8fcu)

// The three names the duck takes from the tape's units. Since J5g each is
// optional: where the units that add tracks 5 to 8 are away (the pack `sidechain` without the pack `8 tracks`), the
// duck works on the original firmware's four tracks with answers of its own: four tracks, stock's selected-track
// word, and a copy of the two-instruction stub (J5g's stubs for the sidechain without 8 tracks). With those units there, their own three
// are used, as since G1.
extern "C" void __attribute__((weak)) op1f_matrix_call(i32 x, i32 y, i32 value, u32* matrix) asm("op1f_matrix_call");   // build_e_screens.s
extern "C" int __attribute__((weak)) op1f_gui_selected_track() asm("op1f_gui_selected_track");                           // build_b_record.cpp: 0-7
extern "C" u32 __attribute__((weak)) op1f_cache_tracks asm("op1f_cache_tracks");                                         // build_b_cache.cpp: 8, or 4 after the fallback
extern "C" void __attribute__((weak)) op1f_duck_matrix_call(i32 x, i32 y, i32 value, u32* matrix) asm("op1f_duck_matrix_call");   // J5g's stubs for the sidechain without 8 tracks

enum {
    TRACKS = 8,
    STOCK_TRACKS = 4,
    PARAMS = 10,                        // source, amount, hold, release, sens, attack, tone, pre, shape, div
    KNOB_PARAMS = 8,                    // the knob indices 0-7
    CARD_SHAPE = 8,                     // the card's name index for the shape (the orange press)
    CARD_BYPASS = 9,                    // the blue press
    CARD_DIV = 10,                      // shift + blue with the beat source
    CARDS = 11,
    SOURCE_BEAT = 9,                    // the source knob's position after the eight tracks
    SHAPES = 4,                         // out, in, linear, S
    DIVS = 4,                           // 1/2, 1/4, 1/8, 1/16 (G2b; G2 had 1/4, 1/8, 1/16, 1/2)
    DIV_DEFAULT = 1,                    // 1/4
    TONE_DEFAULT = 20,                  // about 93 Hz: the kick's band, the snare's body above it
    MODE_SIGNALFLOW = 13,
    MODE_MIXER = 3,
    KEY_MIXER = 5,
    KEY_TRACK_1 = 6,
    KEY_TRACK_4 = 9,
    KEY_KNOB_1 = 54,                    // the knob presses: 54 blue .. 57 orange
    KEY_KNOB_4 = 57,
    DETENT = 1024,                      // the encoder's delta per detent (B3b)
    CHUNK_FRAMES = 16384,
    CHUNK_BYTES = 0x20000,
    CHUNKS_PER_TRACK = 969,
    NODE_DISPLAY = 0x5c,
    VTABLE_BOUNDS = 3,
    MATRIX_WORDS = 9,
    FIXED_ONE = 1024,                   // 22.10
    GAIN_ONE = 32768,                   // the envelope's gain, Q15
    LEVEL_MAX = 32767,                  // the detector's 16-bit magnitude scale: 0 dB
    ENV_RELEASE_Q15 = 2294,             // the fast envelope's fall per block: a 20 ms time constant
    SLOW_Q15 = 316,                     // the slow envelope's step per block: 150 ms both ways
    SLOW_SHIFT = 4,                     // the slow envelope kept x16 for precision
    SMOOTH_KEEP = 921,
    FLOAT_6 = 0x40c00000,               // 6.0f
    FLOAT_3_3 = 0x40533333,             // 3.3f
    FLOAT_10 = 0x41200000,              // 10.0f
    FLOAT_HALF = 0x3f000000,            // 0.5f
    VECTOR_BEGIN_OFFSET = 4,            // the VU vector's first float is at [vector + 4]
    CARD_FRAMES = 70,                   // the parameter card stays about 1.2 s after the last knob move
    SPLASH_FRAMES = 9,
    RIPPLE_FRAMES = 54,
    ARROW_FRAMES = 6,
    SWIM_FRAMES = 24,
    BUBBLES = 6,
    BUBBLE_EVERY = 7,
    STONE_FLIGHT_FRAMES = 13230,        // 0.3 s of tape at 44.1 kHz: the stone is in the air that long before a beat
    JSON_NOTIFY_FRAMES = 30,            // a change reaches the tape's JSON at most twice a second
    JSON_VALUES = 11,                   // per track: the ten parameters and the bypass
    JSON_TAG_G2 = TRACKS * JSON_VALUES,         // 88: G2's array (the divisions 1/4, 1/8, 1/16, 1/2; tone a smoothing)
    JSON_TAG_G2B = TRACKS * JSON_VALUES + 1,    // 89: G2b's array (the divisions in order; tone a filter)
    TRACE_COLUMNS = 60,                 // the detector's trace: one column per frame, about a second
    HIT_DOTS = 8,
    FACE_DUCK = 0,
    FACE_FLOW = 1,
    PHASE_REST = 0,
    PHASE_ATTACK = 1,
    PHASE_HOLD = 2,
    PHASE_RELEASE = 3
};

// the page's geometry in pixels, as the artwork tool draws it
enum {
    AXIS = 124,                         // the gain line
    POND_X = 140,
    POND_W = 200,
    HIT = 174,                          // where a hit lands: the splash, the ripples, the stone
    PATH_END = POND_X + POND_W - 12,    // the rise ends here at the latest
    DEPTH_MAX = 34,                     // the dive at amount 99
    PRE_MAX_PX = 20,                    // the dive starts up to this far left of the hit point at pre 99
    METER_CENTRE = 240,
    METER_STEP = 28,
    TRACE_X = 34,                       // the detector's trace in the source box: the first column
    TRACE_BOTTOM = 164,                 // -60 dB
    TRACE_H = 16,                       // 0 dB this far above the bottom
    HIT_DOT_Y = 146                     // the hit dots above the columns
};
static const i32 BOB[16] = { 0, 274, 507, 662, 717, 662, 507, 274, 0, -274, -507, -662, -717, -662, -507, -274 };   // 0.7 px x sin, 22.10
static const char LOG_FILE[] = "../src/gui/op1f_duck.cpp";
static const char JSON_KEY[] = "op1f_duck";

// ---- the parameters ----------------------------------------------------------------------------------------

struct Params { u8 source, amount, hold, release, sens, attack, tone, pre, shape, div; };   // 0..99; source 0 = off, 1..8 a track, 9 the beat
static const Params DEFAULTS = { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT };
static Params params[TRACKS] = {
    { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT }, { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT },
    { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT }, { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT },
    { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT }, { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT },
    { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT }, { 0, 50, 25, 40, 50, 0, TONE_DEFAULT, 0, 0, DIV_DEFAULT },
};
static u8 bypass[TRACKS];

static u8 param_of(const Params& p, u32 index) {
    switch (index) {
    case 0: return p.source;
    case 1: return p.amount;
    case 2: return p.hold;
    case 3: return p.release;
    case 4: return p.sens;
    case 5: return p.attack;
    case 6: return p.tone;
    case 7: return p.pre;
    case 8: return p.shape;
    default: return p.div;
    }
}

static void copy_params(Params& d, const Params& s);

static void set_param(Params& p, u32 index, u8 value) {
    switch (index) {
    case 0: p.source = value; break;
    case 1: p.amount = value; break;
    case 2: p.hold = value; break;
    case 3: p.release = value; break;
    case 4: p.sens = value; break;
    case 5: p.attack = value; break;
    case 6: p.tone = value; break;
    case 7: p.pre = value; break;
    case 8: p.shape = value; break;
    default: p.div = value; break;
    }
}

static void copy_params(Params& d, const Params& s) {   // field by field: a struct assignment would call memcpy, which the module has not
    for (u32 k = 0; k < PARAMS; ++k) set_param(d, k, param_of(s, k));
}

// duck-tracks-begin
// J5g: the duck on four tracks or on eight. These two functions are all that knows the difference; with eight
// tracks both answer what the code did before them (a test runs them on the PC against the walk of G2).

// The source knob's value after `steps` clicks: off (0), the tracks the tape has here (1 to `tracks`, never the
// track the duck is on), the beat (SOURCE_BEAT). With four tracks the values 5 to 8 are stepped over, and a value
// a tape brought from a unit with eight tracks leaves for the next one below it, or for the beat.
static int source_step(int value, int steps, u32 track, u32 tracks) {
    for (int k = steps; k > 0; --k) {
        ++value;
        if (value == (int)track + 1) ++value;
        if (value > (int)tracks && value < SOURCE_BEAT) value = SOURCE_BEAT;
        if (value > SOURCE_BEAT) value = SOURCE_BEAT;
    }
    for (int k = steps; k < 0; ++k) {
        --value;
        if (value > (int)tracks && value < SOURCE_BEAT) value = (int)tracks;
        if (value == (int)track + 1) --value;
        if (value < 0) value = 0;
    }
    return value;
}

// A stored source as the page shows it: a track the tape has here, the beat, or off. The stored value is kept: a
// tape's duck that listens to track 6 shows off on four tracks, is not heard (the detector reads the tracks there
// are), and is as it was when the tape is loaded with eight tracks again.
static u32 source_live(u32 source, u32 tracks) {
    return (source <= tracks || source == SOURCE_BEAT) ? source : 0;
}
// duck-tracks-end

static u32 udiv(u32 n, u32 d) {              // restoring shift-subtract division (no libgcc routine); d == 0 yields 0
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static i32 floor_of(const Params& p) {       // the gain at the bottom of the dip, Q15: amount 99 ducks to silence
    return GAIN_ONE - (i32)udiv((u32)GAIN_ONE * p.amount, 99u);
}

// The release's progress f(u) in Q15 for u in Q15: out 1 - (1 - u)^2, in u^2, linear u, S 3u^2 - 2u^3.
static i32 shape_curve(i32 u, u32 shape) {
    if (u < 0) u = 0;
    if (u > GAIN_ONE) u = GAIN_ONE;
    i32 u2 = (u * u) >> 15;
    switch (shape) {
    case 1: return u2;
    case 2: return u;
    case 3: { i32 u3 = (u2 * u) >> 15; return 3 * u2 - 2 * u3; }
    default: { i32 r = GAIN_ONE - u; return GAIN_ONE - ((r * r) >> 15); }
    }
}

// The level's place on the sens scale, 0..99 (-60 dB .. 0 dB, linear in dB): the threshold table read backwards.
static u32 level_position(i32 level) {
    if (level <= DUCK_THRESHOLD[0]) return 0;
    if (level >= DUCK_THRESHOLD[99]) return 99;
    u32 lo = 0, hi = 99;                                                    // the largest index whose threshold is <= level
    while (hi - lo > 1) {
        u32 mid = (lo + hi) >> 1;
        if (DUCK_THRESHOLD[mid] <= level) lo = mid; else hi = mid;
    }
    return lo;
}

// ---- the engine (audio thread) -----------------------------------------------------------------------------

struct Env { u8 phase; u16 pos; u16 len; i32 gain; i32 start; i32 floor_; };
static Env env[TRACKS] = {
    { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE },
    { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE }, { 0, 0, 1, GAIN_ONE, GAIN_ONE, GAIN_ONE },
};
// The detector of one source track: the two filter stages on the signed waveform (1.31, halved twice for headroom),
// the fast envelope (the block's peak, a 20 ms fall), the slow envelope (150 ms), the armed flag, and the peak the
// page has not read yet.
struct Det { i32 y1, y2; i32 env; i32 slow16; u8 armed; volatile i32 gui_peak; };
static Det det[TRACKS];                      // per source track
static volatile u32 hit_serial[TRACKS];      // counts the hits of each target, for the page's animation
static volatile u32 beat_period[DIVS];       // frames per hit of each division, from the tape's tempo (the frame hook writes them)
static u32 beat_index[DIVS];                 // the last hit index seen per division
static i32 beat_last_frame = -1;

extern "C" i32 op1f_duck_gain[TRACKS - STOCK_TRACKS] asm("op1f_duck_gain");   // Q31 (1.0 = 0x7fffffff), read by build_b_play.cpp for tracks 5-8
i32 op1f_duck_gain[TRACKS - STOCK_TRACKS] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
extern "C" u32 op1f_duck_blocks asm("op1f_duck_blocks");     // blocks with at least one duck active
extern "C" u32 op1f_duck_hits asm("op1f_duck_hits");         // detector and beat hits that started or restarted a dive
extern "C" u32 op1f_duck_missed asm("op1f_duck_missed");     // frames the detector wanted from a slot that was missing or loading
extern "C" u32 op1f_duck_guarded asm("op1f_duck_guarded");   // hits that fell into an attack and were ignored
u32 op1f_duck_blocks, op1f_duck_hits, op1f_duck_missed, op1f_duck_guarded;
extern "C" u32 op1f_duck_stops asm("op1f_duck_stops");       // G2e: envelopes put to rest by the frame hook because the tape stopped inside a dive
u32 op1f_duck_stops;
static volatile u32 block_serial;            // G2e: counts every block of the engine, duck active or not; the frame hook watches it
static i32 gain_min_q15 = GAIN_ONE;          // since the last log line

static inline i32 mult_t(i32 a, i32 b) {     // R = Ra * Rb (T): fraction multiply, truncated
    i32 r;
    asm("%0 = %1 * %2 (T);" : "=d"(r) : "d"(a), "d"(b));
    return r;
}

// J5g: how many tracks the tape has here: eight when the tape's units are there and their cache has its eight
// regions; else four (their cache's fallback, or no such units: the original firmware's tape).
static u32 tracks_here() {
    return (&op1f_cache_tracks != 0 && op1f_cache_tracks >= TRACKS) ? (u32)TRACKS : (u32)STOCK_TRACKS;
}

static i32 q31_of(i32 gain_q15) {
    return gain_q15 >= GAIN_ONE ? 0x7fffffff : gain_q15 << 16;
}

// One block of the envelope of target t: the fire-once law, attack from the gain at the hit down to the floor,
// the hold, the release back to 1 along the track's shape (the drawn path).
static void advance(Env& e, const Params& p) {
    switch (e.phase) {
    case PHASE_ATTACK:
        ++e.pos;
        if (e.pos >= e.len) {
            e.phase = PHASE_HOLD;
            e.pos = 0;
            e.len = DUCK_HOLD_BLOCKS[p.hold];
            e.gain = e.floor_;
        } else {
            e.gain = e.start - (i32)udiv((u32)(e.start - e.floor_) * e.pos, e.len);
        }
        break;
    case PHASE_HOLD:
        ++e.pos;
        e.gain = e.floor_;
        if (e.pos >= e.len) {
            e.phase = PHASE_RELEASE;
            e.pos = 0;
            e.len = DUCK_RELEASE_BLOCKS[p.release];
        }
        break;
    case PHASE_RELEASE:
        ++e.pos;
        if (e.pos >= e.len) {
            e.phase = PHASE_REST;
            e.pos = 0;
            e.gain = GAIN_ONE;
        } else {
            i32 u = (i32)udiv((u32)e.pos << 15, e.len);
            e.gain = e.floor_ + (((GAIN_ONE - e.floor_) * shape_curve(u, p.shape)) >> 15);
        }
        break;
    default:
        e.gain = GAIN_ONE;
        break;
    }
}

// A hit for target t: during its attack ignored (counted); during its hold the hold starts over (a second kick
// keeps the dip down); otherwise a dive from the current gain.
static void dive(u32 t) {
    Env& e = env[t];
    const Params& p = params[t];
    if (e.phase == PHASE_ATTACK) { ++op1f_duck_guarded; return; }
    if (e.phase == PHASE_HOLD) {
        e.pos = 0;
        ++hit_serial[t];
        ++op1f_duck_hits;
        return;
    }
    e.start = e.gain;
    e.floor_ = floor_of(p);
    e.phase = PHASE_ATTACK;
    e.pos = 0;
    e.len = DUCK_ATTACK_BLOCKS[p.attack];
    ++hit_serial[t];
    ++op1f_duck_hits;
}

static void hit(u32 source) {                // the source track (0-7) fired: every target listening to it dives
    for (u32 t = 0; t < TRACKS; ++t)
        if (params[t].source == source + 1 && !bypass[t]) dive(t);
}

static void beat_hit(u32 div) {              // the beat of one division fell: every target on it dives
    for (u32 t = 0; t < TRACKS; ++t)
        if (params[t].source == SOURCE_BEAT && params[t].div == div && !bypass[t]) dive(t);
}

// The beat source: the tape position in frames against the division's period; a hit when the hit index changes
// while the tape moves (a stopped tape fires nothing, a wind fires once per jump).
static void beats() {
    i32 frame = POSITION64 >> 6;
    if (frame == beat_last_frame) return;
    int moved = beat_last_frame >= 0;
    beat_last_frame = frame;
    u32 used = 0;
    for (u32 t = 0; t < TRACKS; ++t)
        if (params[t].source == SOURCE_BEAT && !bypass[t]) used |= 1u << params[t].div;
    for (u32 d = 0; d < DIVS; ++d) {
        u32 period = beat_period[d];
        if (!period) continue;
        u32 index = udiv((u32)(frame < 0 ? 0 : frame), period);
        if ((used & (1u << d)) && moved && index != beat_index[d]) beat_hit(d);
        beat_index[d] = index;
    }
}

// Before stock's read_block: this block's gains, from the envelopes advanced by one block, into the hold-to-solo
// words of tracks 1-4 (stock has just rewritten them: 0x7fffffff, or 0 for a soloed-out track) and the module's
// gain words of tracks 5-8.
extern "C" void op1f_duck_block_gains() asm("op1f_duck_block_gains");
extern "C" void op1f_duck_block_gains() {
    ++block_serial;                                                         // G2e: the frame hook sees the engine running
    beats();
    int active = 0;
    for (u32 t = 0; t < TRACKS; ++t) {
        Env& e = env[t];
        const Params& p = params[t];
        if (p.source == 0 || bypass[t]) {
            e.phase = PHASE_REST;
            e.gain = GAIN_ONE;
        } else {
            advance(e, p);
            active = 1;
        }
        i32 q31 = q31_of(e.gain);
        if (t < STOCK_TRACKS) {
            if (e.gain < GAIN_ONE) SOLO_WORDS[t] = mult_t(SOLO_WORDS[t], q31);
        } else {
            op1f_duck_gain[t - STOCK_TRACKS] = q31;
        }
        if (e.gain < gain_min_q15) gain_min_q15 = e.gain;
    }
    if (active) ++op1f_duck_blocks;
}

// After stock's read_block: the detector over this block's frames [w0 + 9, w0 + 9 + n) of every source track in
// use, shifted ahead by the lookahead, read from the chunk cache (a missing or loading slot is silence, counted), at
// most 64 frames examined per source whatever the speed. Each frame's signed mono sample passes two one-pole
// low-pass stages at tone's cutoff (so a snare's body and crack are taken down before anything is measured), is
// rectified, and the block's peak feeds the fast envelope; a hit is an onset: the fast envelope above the sens
// threshold and 3.5 dB above the slow envelope; the detector re-arms when the fast envelope falls back to within
// 2 dB of the slow one or under half the threshold.
extern "C" void op1f_duck_detect(int speed, int w0) asm("op1f_duck_detect");
extern "C" void op1f_duck_detect(int speed, int w0) {
    int n = speed >> 21;
    if (n < 0) n = -n;
    if (n == 0) return;
    u32 regions = tracks_here();
    for (u32 s = 0; s < TRACKS; ++s) {
        const Params* owner = 0;                                            // the first target listening to source s
        for (u32 t = 0; t < TRACKS; ++t) {
            if (params[t].source == s + 1 && !bypass[t]) { owner = &params[t]; break; }
        }
        if (!owner || s >= regions) continue;
        Det& d = det[s];
        i32 first = w0 + 9 + (speed < 0 ? -(i32)owner->pre * 9 : (i32)owner->pre * 9);   // the lookahead, up to 20 ms
        u32 stride = ((u32)n + 63u) >> 6;
        i32 k = DUCK_TONE_K31[owner->tone];
        if (stride > 1) {                                                   // a wind: the coefficient per examined frame
            u32 lim = udiv(0x7fffffffu, stride);
            k = (u32)k > lim ? 0x7fffffff : k * (i32)stride;
        }
        i32 peak = 0;
        i32 logical_seen = -1;
        const u8* data = 0;
        for (u32 i = 0; i < (u32)n; i += stride) {
            i32 f = first + (i32)i;
            i32 x = 0;
            if (f >= 0) {
                i32 logical = f >> 14;
                if (logical < CHUNKS_PER_TRACK) {
                    if (logical != logical_seen) {
                        logical_seen = logical;
                        Slot* slot = STOCK_SLOT_FOR_LOGICAL(logical);
                        data = (slot && slot->valid) ? slot->data + s * CHUNK_BYTES : 0;
                    }
                    if (data) {
                        const i32* fr = (const i32*)data + 2 * (f & (CHUNK_FRAMES - 1));
                        x = (fr[0] >> 2) + (fr[1] >> 2);                    // mono, half scale: headroom for the differences
                    } else {
                        ++op1f_duck_missed;
                    }
                }
            }
            d.y1 += mult_t(x - d.y1, k);
            d.y2 += mult_t(d.y1 - d.y2, k);
            i32 a = (d.y2 < 0 ? -d.y2 : d.y2) >> 14;                        // back to the 16-bit scale
            if (a > peak) peak = a;
        }
        if (peak > LEVEL_MAX) peak = LEVEL_MAX;
        if (peak >= d.env) d.env = peak;                                    // the fast envelope: instant up, 20 ms down
        else d.env -= (d.env * ENV_RELEASE_Q15) >> 15;
        d.slow16 += (((d.env << SLOW_SHIFT) - d.slow16) * SLOW_Q15) >> 15;   // the slow envelope, 150 ms, kept x16
        i32 slow = d.slow16 >> SLOW_SHIFT;
        if (d.env > d.gui_peak) d.gui_peak = d.env;
        i32 threshold = DUCK_THRESHOLD[owner->sens];
        if (!d.armed) {
            if (d.env > threshold && d.env > slow + (slow >> 1)) { d.armed = 1; hit(s); }
        } else if (d.env < (threshold >> 1) || d.env < slow + (slow >> 2)) {
            d.armed = 0;
        }
    }
}

// ---- the faces and the keys (GUI thread) --------------------------------------------------------------------

static volatile u8 face = FACE_DUCK;
static u32 card_frames;
static u32 card_index;                       // the card's name: 0-7 the knobs, 8 shape, 9 bypass, 10 div
static u8 card_value;
static i32 detent_acc;
static u32 detent_index = 99;
static u8 mixer_held, mixer_used;
static u8 json_dirty;
static u32 json_notify_frame;
static u32 frame_counter;
static u32 seen_block_serial;                // G2e: the engine's block count at the last frame
static u8 idle_frames;                       // G2e: frames in a row without a block of the engine
extern "C" u32 op1f_duck_knob_moves asm("op1f_duck_knob_moves");
extern "C" u32 op1f_duck_face_toggles asm("op1f_duck_face_toggles");
extern "C" u32 op1f_duck_key_selects asm("op1f_duck_key_selects");     // track selections from the page
extern "C" u32 op1f_duck_json_saves asm("op1f_duck_json_saves");
extern "C" u32 op1f_duck_json_loads asm("op1f_duck_json_loads");
u32 op1f_duck_knob_moves, op1f_duck_face_toggles, op1f_duck_key_selects, op1f_duck_json_saves, op1f_duck_json_loads;

static void show_card(u32 index, u8 value) {
    card_index = index;
    card_value = value;
    card_frames = CARD_FRAMES;
    json_dirty = 1;
}

static u32 selected_track() {
    u32 t = &op1f_gui_selected_track != 0 ? (u32)op1f_gui_selected_track() : (u32)SELECTED_TRACK;   // J5g: the tape units' getter reads this same word
    return t < TRACKS ? t : 0;
}

// A track change from the page, as build_b_record.cpp selects tracks 5-8: stock's two conditions, the word, the
// tape model's notify (the screens follow, the JSON is written).
static void select_track(u32 track) {
    if (track >= TRACKS) return;
    if (track >= STOCK_TRACKS && tracks_here() < TRACKS) return;
    if (TRANSPORT_MODE == 1 || !STOCK_CLIPS_IDLE()) return;
    SELECTED_TRACK = (i32)track;
    STOCK_NOTIFY(STOCK_TAPE_MODEL());
    ++op1f_duck_key_selects;
}

// The four vtable slots of the signal-flow controller (profile group build-g1). On the flow face every key goes to
// the fallback as stock had it; shift + mixer toggles the face on both; the duck face takes the knobs and their
// presses, the track keys and the mixer key (held for tracks 5-8, tapped to leave to the levels page).
extern "C" int op1f_duck_key_down(void* self, const int* event) asm("op1f_duck_key_down");
extern "C" int op1f_duck_key_down(void*, const int* event) {
    if (face != FACE_DUCK || !event) return 0;
    int key = event[0];
    if (key == KEY_MIXER) { mixer_held = 1; mixer_used = 0; return 1; }
    if (key >= KEY_TRACK_1 && key <= KEY_TRACK_4) {
        u32 track = (u32)(key - KEY_TRACK_1);
        if (mixer_held) { track += STOCK_TRACKS; mixer_used = 1; }
        select_track(track);
        return 1;
    }
    if (key >= KEY_KNOB_1 && key <= KEY_KNOB_4) {
        u32 t = selected_track();
        if (key == KEY_KNOB_1) {                                            // blue: bypass and back
            bypass[t] = (u8)(bypass[t] ? 0 : 1);
            show_card(CARD_BYPASS, bypass[t]);
        } else if (key == KEY_KNOB_4) {                                     // orange: the release's shape
            params[t].shape = (u8)((params[t].shape + 1) % SHAPES);
            show_card(CARD_SHAPE, params[t].shape);
        }
        return 1;
    }
    return 0;
}

extern "C" int op1f_duck_key_shift(void* self, const int* event) asm("op1f_duck_key_shift");
extern "C" int op1f_duck_key_shift(void*, const int* event) {
    if (event && event[0] == KEY_MIXER) {
        face = face == FACE_DUCK ? FACE_FLOW : FACE_DUCK;
        ++op1f_duck_face_toggles;
        return 1;
    }
    return 0;
}

extern "C" int op1f_duck_key_up(void* self, int key) asm("op1f_duck_key_up");
extern "C" int op1f_duck_key_up(void*, int key) {
    if (face != FACE_DUCK) return 0;
    if (key == KEY_MIXER && mixer_held) {
        mixer_held = 0;
        if (!mixer_used) {                                                  // a tap: to the mixer, as the fallback's key-down does
            STOCK_MODE_SWITCH(MODE_MIXER);
            MAIN_MODE_WORD = MODE_MIXER;
        }
        return 1;
    }
    return 0;
}

extern "C" int op1f_duck_encoder(void* self, int index, int delta) asm("op1f_duck_encoder");
extern "C" int op1f_duck_encoder(void*, int index, int delta) {
    if (face != FACE_DUCK || (u32)index >= KNOB_PARAMS) return 0;
    if ((u32)index != detent_index) { detent_index = (u32)index; detent_acc = 0; }
    detent_acc += delta;
    int steps = 0;
    while (detent_acc >= DETENT) { detent_acc -= DETENT; ++steps; }
    while (detent_acc <= -DETENT) { detent_acc += DETENT; --steps; }
    u32 t = selected_track();
    Params& p = params[t];
    u32 which = (u32)index;
    if (which == 4 && p.source == SOURCE_BEAT) which = 9;                   // shift + blue with the beat: the division
    int value = param_of(p, which);
    if (which == 0) {                                                       // the source: off, the tracks there are (never the track itself), the beat
        value = source_step(value, steps, t, tracks_here());
    } else if (which == 9) {
        value += steps;
        if (value < 0) value = 0;
        if (value >= DIVS) value = DIVS - 1;
    } else {
        value += steps;
        if (value < 0) value = 0;
        if (value > 99) value = 99;
    }
    set_param(p, which, (u8)value);
    show_card(which == 9 ? CARD_DIV : (u32)index, (u8)value);
    if (steps) ++op1f_duck_knob_moves;
    return 1;
}

// Every frame (from the first frame hook's measuring unit's frame hook): away from the signal-flow screen the next entry lands on the
// duck face and the mixer key's hold is forgotten; the card's timer runs here; the beat periods come from the
// tape's tempo (the float at 0x8490c8fc, as the JSON writer rounds it to tenths); a change reaches the tape's JSON
// through the tape model's notify, at most twice a second.
extern "C" void op1f_duck_frame() asm("op1f_duck_frame");
extern "C" void op1f_duck_frame() {
    ++frame_counter;
    if (MODE_WORD != MODE_SIGNALFLOW) { face = FACE_DUCK; mixer_held = 0; }
    if (card_frames) --card_frames;
    // G2e: the tape stopped inside a dive. The L1 tape driver skips read_block at speed 0 (its test at 0x11a081e4),
    // so the engine's block function does not run and an envelope keeps its phase, position and gain: the duck
    // stayed under water on a pause and the next play went on with the old dive. With no block for two frames
    // (a playing tape gives about twelve per frame) the audio thread is idle, and every envelope not at rest
    // is put to rest from here, its gain with it; the page sees the phase change and swims the duck home.
    if (block_serial == seen_block_serial) {
        if (idle_frames < 255) ++idle_frames;
        if (idle_frames >= 2) {
            for (u32 t = 0; t < TRACKS; ++t) {
                Env& e = env[t];
                if (e.phase == PHASE_REST) continue;
                e.gain = GAIN_ONE;
                e.pos = 0;
                e.phase = PHASE_REST;
                if (t >= STOCK_TRACKS) op1f_duck_gain[t - STOCK_TRACKS] = 0x7fffffff;
                ++op1f_duck_stops;
            }
        }
    } else {
        seen_block_serial = block_serial;
        idle_frames = 0;
    }
    i32 bpm10 = STOCK_FLOAT_TO_INT(STOCK_FLOAT_ADD(STOCK_FLOAT_MUL(TEMPO_FLOAT, FLOAT_10), FLOAT_HALF));
    if (bpm10 < 200) bpm10 = 200;                                           // 20 bpm at the least: a sane period
    if (bpm10 > 9990) bpm10 = 9990;
    u32 quarter = udiv(26460000u, (u32)bpm10);                              // frames per beat: 44100 x 60 x 10 / (bpm x 10)
    beat_period[0] = quarter << 1;                                          // 1/2
    beat_period[1] = quarter;                                               // 1/4
    beat_period[2] = quarter >> 1;                                          // 1/8
    beat_period[3] = quarter >> 2;                                          // 1/16
    if (json_dirty && frame_counter - json_notify_frame >= JSON_NOTIFY_FRAMES) {
        json_dirty = 0;
        json_notify_frame = frame_counter;
        STOCK_NOTIFY(STOCK_TAPE_MODEL());
    }
}

// ---- the drawing (GUI thread, per update of the signal-flow screen) -----------------------------------------

static void* duck_screen;
static u8* node_flow;
static u8* node_duck;
static u8* node_vu[TRACKS];
static i32 vu_anchor[TRACKS][2];
static i32 vu_value[TRACKS];
static u8* node_num[TRACKS][3];              // blue, lit, dim
static u8* node_mini[TRACKS][2];             // lit, dim
static u8* node_src[TRACKS];
static u8* node_srcoff;
static u8* node_srcbeat;
static u8* node_tgt[TRACKS];
static u8* node_arrowlit;
static u8* node_hitmark;
static u8* node_drop;
static u8* node_hold;
static u8* node_rise[6];
static u8* node_dot[3];
static u8* node_sprite[4];                   // up, down, asleep, blink
static u8* node_splash[3];
static u8* node_ripple[2];
static u8* node_bubble[BUBBLES];
static u8* node_stone;
static u8* node_trace[TRACE_COLUMNS];
static u8* node_gate;
static u8* node_hitdot[HIT_DOTS];
static u8* node_labels;
static u8* node_labelsdim;
static u8* node_pills;
static u8* node_card;
static u8* node_cardbg[4];
static u8* node_knob[4];
static u8* node_name[CARDS];
static u8* node_digit[2][10];
static u8* node_word[11];                    // out, in, lin, s, on, off, 1/2, 1/4, 1/8, 1/16, beat
static const char* const WORD_IDS[11] = { "dkvalout", "dkvalin", "dkvallin", "dkvals", "dkvalon", "dkvaloff", "dkval12", "dkval14", "dkval18", "dkval116", "dkvalbeat" };
static u32 nodes_found;
static int shown_face = -1;
static u32 path_key = 0xffffffff;
static u32 seen_track = 99;
static u32 seen_serial;
static u8 seen_phase;
static u32 splash_frames, ripple_frames, arrow_frames, swim_frames;
static i32 swim_from;
static i32 last_duck_x;                      // G2e: where the duck was drawn last, the swim-back's start
struct Bubble { i32 x, y; u8 live; u8 wobble; };
static Bubble bubble[BUBBLES];
static u32 bubble_clock;
static i32 last_position;
static u8 tape_moving;
static u8 trace_level[TRACE_COLUMNS];        // the detector's level per frame on the sens scale, 0..99; a ring
static u8 trace_hit[TRACE_COLUMNS];
static u32 trace_head;                       // the oldest column
static u32 trace_source = 99;
static i32 det_log_max;                      // the detector's peak since the last log line
extern "C" u32 op1f_duck_frames asm("op1f_duck_frames");   // updates drawn on the duck face
u32 op1f_duck_frames;

static u8* node(void* screen, const char* id, u32 a = 100, u32 b = 100, const char* tail = 0) {
    char buffer[24];
    u32 i = 0;
    while (id[i] && i < 16) { buffer[i] = id[i]; ++i; }
    if (a < 100) { u32 q = udiv(a, 10u); if (a >= 10) buffer[i++] = (char)('0' + q); buffer[i++] = (char)('0' + (a - q * 10u)); }
    if (b < 100) buffer[i++] = (char)('0' + b);
    if (tail) { u32 j = 0; while (tail[j] && i < 22) buffer[i++] = tail[j++]; }
    buffer[i] = 0;
    u8* found = (u8*)STOCK_FIND_NODE(screen, buffer, 1);
    if (found) ++nodes_found;
    return found;
}

static void show(u8* n, int on) {
    if (n) n[NODE_DISPLAY] = (u8)(on ? 1 : 0);
}

static void translate_to(u8* n, i32 tx, i32 ty) {                           // 22.10
    if (!n) return;
    u32 m[MATRIX_WORDS] = { FIXED_ONE, 0, 0, 0, FIXED_ONE, 0, (u32)tx, (u32)ty, FIXED_ONE };
    STOCK_APPLY_MATRIX(n, m);
}

static void segment(u8* n, i32 x1, i32 y1, i32 x2, i32 y2) {                // the unit line onto (x1,y1)-(x2,y2), 22.10; for lines that lie flat or lean
    if (!n) return;
    u32 m[MATRIX_WORDS] = { (u32)(x2 - x1), (u32)(y2 - y1), 0, 0, FIXED_ONE, 0, (u32)x1, (u32)y1, FIXED_ONE };
    STOCK_APPLY_MATRIX(n, m);
}

static void upright(u8* n, i32 x, i32 y1, i32 y2) {                         // the unit line stood upright from (x,y1) to (x,y2): a quarter turn, so the matrix stays regular
    if (!n) return;
    // x' = m3 * py + x (the stroke's width across), y' = m1 * px + y1 (the length down the column). The third
    // column must stay 0, 0, 1: the renderer multiplies the node's full 3 x 3 into its parent's (0x8c03da6c, child
    // x parent), so a word there drags the root transform's translation into the line's direction. G2b had the
    // 1024 at index 2 and every upright line ran from its foot to the screen's left edge (G2c).
    u32 m[MATRIX_WORDS] = { 0, (u32)(y2 - y1), 0, (u32)FIXED_ONE, 0, 0, (u32)x, (u32)y1, FIXED_ONE };
    STOCK_APPLY_MATRIX(n, m);
}

static void ellipse(u8* n, i32 cx, i32 cy, i32 rx, i32 ry) {                 // the unit circle onto an ellipse, 22.10
    if (!n) return;
    u32 m[MATRIX_WORDS] = { (u32)rx, 0, 0, 0, (u32)ry, 0, (u32)cx, (u32)cy, FIXED_ONE };
    STOCK_APPLY_MATRIX(n, m);
}

static int float_below(u32 a, u32 b) {      // a < b, by sign and magnitude as stock compares the level with 3.3
    int na = (a >> 31) != 0, nb = (b >> 31) != 0;
    if (na != nb) return na;
    return na ? (a & 0x7fffffffu) > (b & 0x7fffffffu) : (a & 0x7fffffffu) < (b & 0x7fffffffu);
}

static void setup(void* screen) {
    duck_screen = screen;
    nodes_found = 0;
    node_flow = node(screen, "flow");
    node_duck = node(screen, "duck");
    for (u32 k = 0; k < TRACKS; ++k) {
        node_vu[k] = node(screen, "dkvu", k + 1);
        u8* dot = node(screen, "dkbase", k + 1);
        vu_value[k] = 0;
        vu_anchor[k][0] = vu_anchor[k][1] = 0;
        if (node_vu[k] && dot) {
            i32 box[4] = { 0x7fffffff, (i32)0x80000000, 0x7fffffff, (i32)0x80000000 };
            BoundsFn bounds = (*(BoundsFn**)dot)[VTABLE_BOUNDS];
            bounds(dot, box);
            vu_anchor[k][0] = box[1];
            vu_anchor[k][1] = box[3];
        } else {
            node_vu[k] = 0;
        }
        node_num[k][0] = node(screen, "dknum", k + 1, 100, "b");
        node_num[k][1] = node(screen, "dknum", k + 1, 100, "l");
        node_num[k][2] = node(screen, "dknum", k + 1, 100, "d");
        node_mini[k][0] = node(screen, "dkmini", k + 1, 100, "l");
        node_mini[k][1] = node(screen, "dkmini", k + 1, 100, "d");
        node_src[k] = node(screen, "dksrc", k + 1);
        node_tgt[k] = node(screen, "dktgt", k + 1);
    }
    for (u32 k = 0; k < CARDS; ++k) node_name[k] = node(screen, "dkname", k + 1);
    node_srcoff = node(screen, "dksrcoff");
    node_srcbeat = node(screen, "dksrcbeat");
    node_arrowlit = node(screen, "dkarrowlit");
    node_hitmark = node(screen, "dkhitmark");
    node_drop = node(screen, "dkdrop");
    node_hold = node(screen, "dkhold");
    for (u32 k = 0; k < 6; ++k) node_rise[k] = node(screen, "dkrise", k + 1);
    for (u32 k = 0; k < 3; ++k) node_dot[k] = node(screen, "dkdot", k + 1);
    node_sprite[0] = node(screen, "dkduckup");
    node_sprite[1] = node(screen, "dkduckdn");
    node_sprite[2] = node(screen, "dkduckzz");
    node_sprite[3] = node(screen, "dkduckblink");
    for (u32 k = 0; k < 3; ++k) node_splash[k] = node(screen, "dksplash", k + 1);
    node_ripple[0] = node(screen, "dkripple", 1);
    node_ripple[1] = node(screen, "dkripple", 2);
    for (u32 k = 0; k < BUBBLES; ++k) node_bubble[k] = node(screen, "dkbubble", k + 1);
    node_stone = node(screen, "dkstone");
    for (u32 k = 0; k < TRACE_COLUMNS; ++k) node_trace[k] = node(screen, "dktrace", k + 1);
    node_gate = node(screen, "dkgate");
    for (u32 k = 0; k < HIT_DOTS; ++k) node_hitdot[k] = node(screen, "dkhit", k + 1);
    node_labels = node(screen, "dklabels");
    node_labelsdim = node(screen, "dklabelsdim");
    node_pills = node(screen, "dkpills");
    node_card = node(screen, "dkcard");
    for (u32 k = 0; k < 4; ++k) {
        node_cardbg[k] = node(screen, "dkcardbg", k + 1);
        node_knob[k] = node(screen, "dkknob", k + 1);
    }
    for (u32 p = 0; p < 2; ++p)
        for (u32 d = 0; d < 10; ++d) node_digit[p][d] = node(screen, "dkdig", p, d);
    for (u32 k = 0; k < 11; ++k) node_word[k] = node(screen, WORD_IDS[k]);
    shown_face = -1;
    path_key = 0xffffffff;
    seen_track = 99;
    trace_source = 99;
    STOCK_LOG(3, LOG_FILE, 1, "duck page: %u nodes found, flow %08x duck %08x", nodes_found, (u32)node_flow, (u32)node_duck);
}

// The dive path of the selected track in 22.10: the drop where the dive starts (ahead of the hit point by pre),
// the hold along the bottom, the rise along the track's shape in six segments; also the duck's position.
struct Path { i32 start, depth, x_bottom, x_rise, x_end, bottom; };

static Path path_of(const Params& p) {
    Path g;
    g.start = HIT * FIXED_ONE - (i32)udiv((u32)(PRE_MAX_PX * FIXED_ONE) * p.pre, 99u);
    g.depth = (i32)udiv((u32)(DEPTH_MAX * FIXED_ONE) * p.amount, 99u);
    i32 drop_w = 3 * FIXED_ONE + (i32)udiv((u32)(20 * FIXED_ONE) * p.attack, 99u);
    i32 hold_w = 10 * FIXED_ONE + (i32)udiv((u32)(90 * FIXED_ONE) * p.hold, 99u);
    i32 rel_w = 15 * FIXED_ONE + (i32)udiv((u32)(95 * FIXED_ONE) * p.release, 99u);
    g.x_bottom = g.start + drop_w;
    g.x_rise = g.x_bottom + hold_w;
    g.x_end = g.x_rise + rel_w;
    if (g.x_end > PATH_END * FIXED_ONE) g.x_end = PATH_END * FIXED_ONE;
    g.bottom = AXIS * FIXED_ONE + g.depth;
    return g;
}

static i32 rise_y(const Path& g, u32 k, u32 of, u32 shape) {               // y of the rise at u = k / of: the bottom at 0, the line at `of`
    i32 u = (i32)udiv(k << 15, of);                                        // Q15
    i32 f = shape_curve(u, shape);                                         // Q15, 0 at the bottom .. 1 at the line
    return g.bottom - ((g.depth * f) >> 15);
}

static void draw_path(const Path& g, u32 shape) {
    segment(node_drop, g.start, AXIS * FIXED_ONE, g.x_bottom, g.bottom);
    segment(node_hold, g.x_bottom, g.bottom, g.x_rise, g.bottom);
    for (u32 k = 0; k < 6; ++k) {
        i32 xa = g.x_rise + (i32)udiv((u32)(g.x_end - g.x_rise) * k, 6u);
        i32 xb = g.x_rise + (i32)udiv((u32)(g.x_end - g.x_rise) * (k + 1), 6u);
        segment(node_rise[k], xa, rise_y(g, k, 6, shape), xb, rise_y(g, k + 1, 6, shape));
    }
    translate_to(node_dot[0], g.x_bottom, g.bottom);
    translate_to(node_dot[1], g.x_rise, g.bottom);
    translate_to(node_dot[2], g.x_end, AXIS * FIXED_ONE);
    upright(node_hitmark, HIT * FIXED_ONE, (AXIS - 4) * FIXED_ONE, (AXIS + 4) * FIXED_ONE);
}

static void duck_position(const Path& g, const Env& e, i32* x, i32* y) {
    u32 len = e.len ? e.len : 1;
    u32 pos = e.pos < len ? e.pos : len;
    i32 span = GAIN_ONE - e.floor_;
    i32 frac = span > 0 ? (i32)udiv((u32)(GAIN_ONE - e.gain) << 10, (u32)span) : 0;   // how far down, 22.10
    if (frac < 0) frac = 0;
    if (frac > FIXED_ONE) frac = FIXED_ONE;
    switch (e.phase) {
    case PHASE_ATTACK:
        *x = g.start + (i32)udiv((u32)(g.x_bottom - g.start) * pos, len);
        *y = AXIS * FIXED_ONE + ((g.depth * frac) >> 10);
        break;
    case PHASE_HOLD:
        *x = g.x_bottom + (i32)udiv((u32)(g.x_rise - g.x_bottom) * pos, len);
        *y = g.bottom;
        break;
    case PHASE_RELEASE:
        *x = g.x_rise + (i32)udiv((u32)(g.x_end - g.x_rise) * pos, len);
        *y = AXIS * FIXED_ONE + ((g.depth * frac) >> 10);
        break;
    default:
        *x = g.start;
        *y = AXIS * FIXED_ONE;
        break;
    }
}

static void hide_effects() {
    for (u32 k = 0; k < 3; ++k) show(node_splash[k], 0);
    show(node_ripple[0], 0);
    show(node_ripple[1], 0);
    for (u32 k = 0; k < BUBBLES; ++k) { show(node_bubble[k], 0); bubble[k].live = 0; }
    show(node_stone, 0);
    show(node_arrowlit, 0);
}

static void hide_trace() {
    for (u32 k = 0; k < TRACE_COLUMNS; ++k) show(node_trace[k], 0);
    show(node_gate, 0);
    for (u32 k = 0; k < HIT_DOTS; ++k) show(node_hitdot[k], 0);
}

// The detector's trace under the source digit: the newest column at the right, the sens gate across, the hits
// above. The level is the peak the audio thread saw since the last frame, placed on the sens scale (0..99 =
// -60..0 dB) so the gate's line and the columns share one rule.
static void draw_trace(u32 source, u32 sens, int hit_now) {
    if (source != trace_source) {                                           // another source: the trace starts clean
        trace_source = source;
        for (u32 k = 0; k < TRACE_COLUMNS; ++k) { trace_level[k] = 0; trace_hit[k] = 0; }
        trace_head = 0;
    }
    Det& d = det[source];
    i32 level = d.gui_peak;
    d.gui_peak = 0;
    if (level > det_log_max) det_log_max = level;
    trace_level[trace_head] = (u8)level_position(level);
    trace_hit[trace_head] = (u8)(hit_now ? 1 : 0);
    if (++trace_head == TRACE_COLUMNS) trace_head = 0;
    u32 dots = 0;
    for (u32 i = 0; i < TRACE_COLUMNS; ++i) {
        u32 idx = trace_head + i;
        if (idx >= TRACE_COLUMNS) idx -= TRACE_COLUMNS;
        i32 x = (TRACE_X + (i32)i) * FIXED_ONE + (FIXED_ONE >> 1);
        i32 h = (i32)udiv((u32)trace_level[idx] * TRACE_H * FIXED_ONE, 99u);
        if (h < (FIXED_ONE >> 2)) h = FIXED_ONE >> 2;                       // a quarter pixel keeps the column's foot visible
        upright(node_trace[i], x, TRACE_BOTTOM * FIXED_ONE, TRACE_BOTTOM * FIXED_ONE - h);
        show(node_trace[i], 1);
        if (trace_hit[idx] && dots < HIT_DOTS) {
            translate_to(node_hitdot[dots], x, HIT_DOT_Y * FIXED_ONE);
            show(node_hitdot[dots], 1);
            ++dots;
        }
    }
    for (u32 k = dots; k < HIT_DOTS; ++k) show(node_hitdot[k], 0);
    i32 gy = TRACE_BOTTOM * FIXED_ONE - (i32)udiv(sens * TRACE_H * FIXED_ONE, 99u);
    segment(node_gate, (TRACE_X - 1) * FIXED_ONE, gy, (TRACE_X + TRACE_COLUMNS + 1) * FIXED_ONE, gy);
    show(node_gate, 1);
}

// Called at the end of build_e_screens.cpp's op1f_signalflow_vu_ext on every update of the signal-flow screen:
// `vector` is stock's VU vector (the dB floats of tracks 1-4 at [vector + 4]), `ext_db` the dB floats of tracks 5-8
// that E4's loop computed this update.
extern "C" void op1f_duck_screen(u8* screen, const u8* vector, const u32* ext_db) asm("op1f_duck_screen");
extern "C" void op1f_duck_screen(u8* screen, const u8* vector, const u32* ext_db) {
    if (duck_screen != (void*)screen) setup(screen);
    if (!node_flow || !node_duck) return;                                   // the device's own artwork: nothing to show
    int f = face;
    if (f != shown_face) {
        show(node_flow, f == FACE_FLOW);
        show(node_duck, f == FACE_DUCK);
        shown_face = f;
        path_key = 0xffffffff;
        hide_effects();
        hide_trace();
    }
    if (f != FACE_DUCK) return;
    ++op1f_duck_frames;
    u32 t = selected_track();
    const Params& p = params[t];
    u32 here = tracks_here();
    u32 src = source_live(p.source, here);                                  // J5g: a source the tape has no track for here shows as off
    int on = src != 0 && !bypass[t];
    int shift = STOCK_SHIFT_HELD() != 0;
    i32 position = POSITION64;
    tape_moving = (u8)(position != last_position);
    last_position = position;
    if (t != seen_track) {                                                  // another track: its animation starts clean
        seen_track = t;
        seen_serial = hit_serial[t];
        seen_phase = env[t].phase;
        splash_frames = ripple_frames = arrow_frames = swim_frames = 0;
        hide_effects();
    }
    // the meters: stock's law on the eight copies
    const u8* begin = vector ? *(const u8* const*)(vector + VECTOR_BEGIN_OFFSET) : 0;
    for (u32 k = 0; k < TRACKS; ++k) {
        if (!node_vu[k]) continue;
        u32 db = k < STOCK_TRACKS ? (begin ? *(const u32*)(begin + 4 * k) : 0xff800000u) : (ext_db ? ext_db[k - STOCK_TRACKS] : 0xff800000u);
        u32 tt = STOCK_FLOAT_ADD(FLOAT_6, db);
        u32 level = (float_below(tt, FLOAT_3_3) && (tt & 0x7fffffffu) < 0x7f800001u) ? tt : (u32)FLOAT_3_3;
        i32 fixed = STOCK_METER_FIXED(STOCK_DB_TO_AMP(level));
        i32 value = ((METER_SMOOTH_GAIN - SMOOTH_KEEP) * fixed + vu_value[k] * SMOOTH_KEEP) >> 10;
        vu_value[k] = value;
        u32 m[MATRIX_WORDS];
        if (&op1f_matrix_call != 0) op1f_matrix_call(vu_anchor[k][0], vu_anchor[k][1] - FIXED_ONE, value, m);
        else if (&op1f_duck_matrix_call != 0) op1f_duck_matrix_call(vu_anchor[k][0], vu_anchor[k][1] - FIXED_ONE, value, m);   // J5g: the tape's units are away
        else continue;
        STOCK_APPLY_MATRIX(node_vu[k], m);
    }
    // the digits and the small ducks under the meters, the source and target digits
    int beat = src == SOURCE_BEAT;
    for (u32 k = 0; k < TRACKS; ++k) {
        int is_source = on && !beat && src == k + 1;
        show(node_num[k][0], is_source);
        show(node_num[k][1], !is_source && k == t);
        show(node_num[k][2], !is_source && k != t);
        int has = source_live(params[k].source, here) != 0;
        show(node_mini[k][0], has && k == t);
        show(node_mini[k][1], has && k != t);
        show(node_src[k], src == k + 1);
        show(node_tgt[k], k == t);
    }
    show(node_srcoff, src == 0);
    show(node_srcbeat, beat);
    show(node_pills, shift);
    show(node_labels, !shift && on);
    show(node_labelsdim, !shift && !on);
    // the dive path
    Path g = path_of(p);
    u32 key = (t << 28) | ((u32)p.amount << 21) | ((u32)p.hold << 14) | ((u32)p.release << 7) | (u32)p.attack;
    u32 key2 = ((u32)p.pre << 8) | ((u32)p.shape << 4) | 1u;
    if (on && (key ^ key2) != path_key) { draw_path(g, p.shape); path_key = key ^ key2; }
    show(node_drop, on);
    show(node_hold, on);
    for (u32 k = 0; k < 6; ++k) show(node_rise[k], on);
    for (u32 k = 0; k < 3; ++k) show(node_dot[k], on);
    show(node_hitmark, on && p.pre != 0);
    // the hit's animation: a new dive of the selected track starts the splash, the ripples and the lit arrow
    Env e = env[t];                                                         // a copy: the audio thread moves on
    u32 serial = hit_serial[t];
    int hit_now = 0;
    if (on && serial != seen_serial) {
        seen_serial = serial;
        hit_now = 1;
        splash_frames = SPLASH_FRAMES;
        ripple_frames = RIPPLE_FRAMES;
        arrow_frames = ARROW_FRAMES;
        swim_frames = 0;
    }
    if (on && seen_phase != PHASE_REST && e.phase == PHASE_REST) {           // the dive ended, or the tape stopped inside it (G2e): the duck swims back
        swim_frames = SWIM_FRAMES;
        swim_from = last_duck_x;
    }
    seen_phase = e.phase;
    // the detector's trace, for a track source
    if (on && !beat) draw_trace(src - 1, p.sens, hit_now);
    else if (trace_source != 99) { trace_source = 99; hide_trace(); }
    // the duck
    i32 x = g.start, y = AXIS * FIXED_ONE;
    if (on) duck_position(g, e, &x, &y);
    int at_rest = !on || e.phase == PHASE_REST;
    if (at_rest && swim_frames) {
        --swim_frames;
        x = g.start + (i32)udiv((u32)(swim_from - g.start) * swim_frames, SWIM_FRAMES);
    }
    last_duck_x = x;
    if (at_rest) y += BOB[(frame_counter >> 2) & 15];                       // the idle bob, a slow 0.7-pixel sway
    int under = y > (AXIS + 5) * FIXED_ONE;
    u32 variant = !on ? 2u : (under ? 1u : ((at_rest && (frame_counter % 140u) < 6u) ? 3u : 0u));
    for (u32 k = 0; k < 4; ++k) {
        translate_to(node_sprite[k], x - HIT * FIXED_ONE, y - AXIS * FIXED_ONE);
        show(node_sprite[k], k == variant);
    }
    // the splash and the ripples at the hit point
    if (splash_frames) {
        --splash_frames;
        i32 u = (i32)udiv((u32)(SPLASH_FRAMES - splash_frames) << 10, SPLASH_FRAMES);   // 0..1 in 22.10
        i32 h = 4 * FIXED_ONE + 8 * u;
        static const i32 DX[3] = { -5, 0, 5 };
        static const i32 DY[3] = { -1024, -1434, -1024 };
        for (u32 k = 0; k < 3; ++k) {
            i32 x1 = HIT * FIXED_ONE + DX[k] * 614;                            // 0.6 of the spread, 22.10
            i32 x2 = HIT * FIXED_ONE + DX[k] * (FIXED_ONE + u);
            i32 y2 = AXIS * FIXED_ONE + ((DY[k] * (h >> 5)) >> 5);
            if (k == 1) upright(node_splash[k], x1, (AXIS - 1) * FIXED_ONE, y2);
            else segment(node_splash[k], x1, (AXIS - 1) * FIXED_ONE, x2, y2);
            show(node_splash[k], 1);
        }
    } else {
        for (u32 k = 0; k < 3; ++k) show(node_splash[k], 0);
    }
    if (ripple_frames) {
        --ripple_frames;
        i32 u = (i32)udiv((u32)(RIPPLE_FRAMES - ripple_frames) << 10, RIPPLE_FRAMES);
        i32 rx1 = 4 * FIXED_ONE + 14 * u, rx2 = 9 * FIXED_ONE + 14 * u;
        ellipse(node_ripple[0], HIT * FIXED_ONE, AXIS * FIXED_ONE, rx1, (rx1 * 307) >> 10);   // the ellipse 0.3 as tall as wide
        ellipse(node_ripple[1], HIT * FIXED_ONE, AXIS * FIXED_ONE, rx2, (rx2 * 184) >> 10);   // 0.18
        show(node_ripple[0], 1);
        show(node_ripple[1], 1);
    } else {
        show(node_ripple[0], 0);
        show(node_ripple[1], 0);
    }
    // the bubbles from the beak while the duck is under water
    ++bubble_clock;
    if (on && under && (bubble_clock % BUBBLE_EVERY) == 0) {
        for (u32 k = 0; k < BUBBLES; ++k) {
            if (bubble[k].live) continue;
            bubble[k].live = 1;
            bubble[k].x = x - 19 * FIXED_ONE + (i32)(k & 3) * 512;
            bubble[k].y = y - 15 * FIXED_ONE;
            bubble[k].wobble = (u8)(k * 5);
            break;
        }
    }
    for (u32 k = 0; k < BUBBLES; ++k) {
        if (!bubble[k].live) { show(node_bubble[k], 0); continue; }
        bubble[k].y -= 512;                                                 // half a pixel a frame
        bubble[k].wobble = (u8)(bubble[k].wobble + 3);
        if (bubble[k].y <= (AXIS * FIXED_ONE) + 2560) { bubble[k].live = 0; show(node_bubble[k], 0); continue; }
        translate_to(node_bubble[k], bubble[k].x + (BOB[bubble[k].wobble & 15] >> 1), bubble[k].y);
        show(node_bubble[k], 1);
    }
    // the stone, for the beat source only: its hits are known ahead, so the stone flies in for 0.3 s before one
    int stone = 0;
    if (on && beat && tape_moving) {
        u32 period = beat_period[p.div];
        if (period) {
            u32 frames = (u32)(position < 0 ? 0 : position >> 6);
            u32 remaining = period - (frames - udiv(frames, period) * period);
            u32 flight = period < STONE_FLIGHT_FRAMES ? period : STONE_FLIGHT_FRAMES;   // a short division: the stone flies the whole beat
            if (remaining < flight) {
                i32 u = (i32)udiv((flight - remaining) << 10, flight);                   // 0..1 in 22.10
                i32 v = FIXED_ONE - u;
                i32 x0 = POND_X * FIXED_ONE, y0 = AXIS * FIXED_ONE;
                i32 x1 = (POND_X + 14) * FIXED_ONE, y1 = (AXIS - 34) * FIXED_ONE;
                i32 x2 = (HIT - 12) * FIXED_ONE, y2 = (AXIS - 2) * FIXED_ONE;
                i32 sx = (i32)((((v * v) >> 10) * (x0 >> 5) + ((2 * v * u) >> 10) * (x1 >> 5) + ((u * u) >> 10) * (x2 >> 5)) >> 5);
                i32 sy = (i32)((((v * v) >> 10) * (y0 >> 5) + ((2 * v * u) >> 10) * (y1 >> 5) + ((u * u) >> 10) * (y2 >> 5)) >> 5);
                translate_to(node_stone, sx, sy);
                stone = 1;
            }
        }
    }
    show(node_stone, stone);
    if (arrow_frames) --arrow_frames;
    show(node_arrowlit, on && (arrow_frames != 0 || stone));
    // the parameter card while a knob turns or after a press
    if (card_frames) {
        show(node_card, 1);
        u32 knob = card_index < KNOB_PARAMS ? (card_index & 3) : (card_index == CARD_SHAPE ? 3u : 0u);
        for (u32 k = 0; k < 4; ++k) {
            show(node_cardbg[k], k == knob);
            show(node_knob[k], k == knob);
        }
        for (u32 k = 0; k < CARDS; ++k) show(node_name[k], k == card_index);
        int word = -1;                                                      // the value block: digits, or one of the words
        if (card_index == CARD_SHAPE) word = card_value < SHAPES ? (int)card_value : 0;
        else if (card_index == CARD_BYPASS) word = card_value ? 4 : 5;
        else if (card_index == CARD_DIV) word = 6 + (card_value < DIVS ? (int)card_value : 0);
        else if (card_index == 0 && card_value == SOURCE_BEAT) word = 10;
        for (u32 k = 0; k < 11; ++k) show(node_word[k], (int)k == word);
        u32 tens = word < 0 ? udiv(card_value, 10u) : 10, ones = word < 0 ? card_value - tens * 10u : 10;
        for (u32 d = 0; d < 10; ++d) {
            show(node_digit[0][d], d == tens);
            show(node_digit[1][d], d == ones);
        }
    } else {
        show(node_card, 0);
    }
}

// ---- the tape's member, through the pack's card (GUI thread, from sidechain 9's values unit) ---------------

// "op1f_duck": [the tag, then per track source, amount, hold, release, sens, attack, tone, pre, shape, div, bypass].
// The tag is 89 since G2b (88 was G2's array, whose divisions ran 1/4, 1/8, 1/16, 1/2 and whose tone was a smoothing).
// The ducks as those numbers, into `out` (room for 89): what op1f_duck_json_write put into the tape's member. Answers
// how many.
extern "C" int op1f_duck_values(int* out) asm("op1f_duck_values");
extern "C" int op1f_duck_values(int* out) {
    int i = 0;
    out[i++] = JSON_TAG_G2B;
    for (u32 t = 0; t < TRACKS; ++t) {
        for (u32 k = 0; k < PARAMS; ++k) out[i++] = param_of(params[t], k);
        out[i++] = bypass[t];
    }
    return i;
}

static u8 clamp_param(u32 k, int v) {
    int top = k == 0 ? SOURCE_BEAT : (k == 8 ? SHAPES - 1 : (k == 9 ? DIVS - 1 : 99));
    if (v < 0) v = 0;
    if (v > top) v = top;
    return (u8)v;
}

// The member's element `i`, as the library answered op1f_duck_json_read for an element past the array's end: 0.
static int element(const int* values, int count, int i) {
    return i < count ? values[i] : 0;
}

// The tape's ducks from the member's numbers (`count` of them; 0 when the tape has no member, or one that is not a list
// of whole numbers), or the defaults when the tape has none (a tape switch never carries the previous tape's ducks
// over). A G2 array keeps its meaning: its divisions are renumbered into G2b's order and its tone, a smoothing that
// filtered nothing, is replaced by the default. What op1f_duck_json_read did with the member, number for number.
extern "C" void op1f_duck_take(const int* values, int count) asm("op1f_duck_take");
extern "C" void op1f_duck_take(const int* values, int count) {
    int loaded = 0;
    if (count > 0) {
        int tag = element(values, count, 0);
        if (tag == JSON_TAG_G2 || tag == JSON_TAG_G2B) {
            int i = 1;
            for (u32 t = 0; t < TRACKS; ++t) {
                Params p;
                for (u32 k = 0; k < PARAMS; ++k) set_param(p, k, clamp_param(k, element(values, count, i++)));
                if (p.source == t + 1) p.source = 0;                           // never the track itself
                if (tag == JSON_TAG_G2) {
                    p.div = (u8)((p.div + 1) & 3);                             // 1/4 1/8 1/16 1/2 -> 1/2 1/4 1/8 1/16
                    p.tone = TONE_DEFAULT;
                }
                u8 by = (u8)(element(values, count, i++) ? 1 : 0);
                copy_params(params[t], p);
                bypass[t] = by;
            }
            loaded = tag;
        }
    }
    if (!loaded) {
        for (u32 t = 0; t < TRACKS; ++t) { copy_params(params[t], DEFAULTS); bypass[t] = 0; }
    }
    path_key = 0xffffffff;
    ++op1f_duck_json_loads;
    STOCK_LOG(3, LOG_FILE, 4, "duck settings %s the tape json (tag %d, track 1 <- %u)", loaded ? "loaded from" : "absent from, defaults;", loaded, params[0].source);
}

// ---- the log line (GUI thread, from the first frame hook's measuring unit) -------------------------------------------------------

static char label[160];

static char* put(char* q, const char* s) {
    while (*s) *q++ = *s++;
    return q;
}

static char* put_u32(char* q, u32 v) {
    char tmp[12];
    int n = 0;
    do { u32 d = udiv(v, 10u); tmp[n++] = (char)('0' + (v - d * 10u)); v = d; } while (v);
    while (n) *q++ = tmp[--n];
    return q;
}

// "track 3 <- 1 amt 50 hold 25 rel 40 sens 50 att 0 tone 20 pre 0 shp 0 div 1, gain min 0.42, det max 1234" for
// the selected track; the minimum and the detector's maximum are reset for the next line.
extern "C" const char* op1f_duck_label() asm("op1f_duck_label");
extern "C" const char* op1f_duck_label() {
    u32 t = selected_track();
    const Params& p = params[t];
    char* q = label;
    q = put(q, "track ");
    q = put_u32(q, t + 1);
    q = put(q, " <- ");
    if (p.source == SOURCE_BEAT) q = put(q, "beat"); else if (p.source) q = put_u32(q, p.source); else q = put(q, "off");
    if (bypass[t]) q = put(q, " (bypassed)");
    q = put(q, " amt "); q = put_u32(q, p.amount);
    q = put(q, " hold "); q = put_u32(q, p.hold);
    q = put(q, " rel "); q = put_u32(q, p.release);
    q = put(q, " sens "); q = put_u32(q, p.sens);
    q = put(q, " att "); q = put_u32(q, p.attack);
    q = put(q, " tone "); q = put_u32(q, p.tone);
    q = put(q, " pre "); q = put_u32(q, p.pre);
    q = put(q, " shp "); q = put_u32(q, p.shape);
    q = put(q, " div "); q = put_u32(q, p.div);
    q = put(q, ", gain min 0.");
    u32 permille = (u32)((gain_min_q15 * 1000) >> 15);
    if (permille > 999) permille = 999;
    if (permille < 100) *q++ = '0';
    if (permille < 10) *q++ = '0';
    q = put_u32(q, permille);
    q = put(q, ", det max ");
    q = put_u32(q, (u32)(det_log_max < 0 ? 0 : det_log_max));
    *q = 0;
    gain_min_q15 = GAIN_ONE;
    det_log_max = 0;
    return label;
}
