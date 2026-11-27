// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4b, the pack 8 tracks (version 9): Build B's mixer unit with its persistence taken out, and nothing else
// changed. Base 26 holds the two places of the tape's file that unit's persistence stood at (the save's set of
// "mixer_params" at 0x8c0c1db8, the load's entry at 0x8c0c348c), and this pack keeps tracks 5 to 8's strip and the
// selected track with the tape through level 7 instead (8 tracks 9's values unit). The mixer page, its filters,
// its value sources and its counters are the elder's, line for line; what follows is the elder's description, which
// holds but for its first point.
//
// Build B, step B3d: the persistence of tracks 5-8's volume and pan in the tape's JSON, and the mixer screen's
// page for tracks 5-8 while the mixer key is held (OS 1.7.0+0).
//
// What stock does:
//   * The TapeController writes user/tapeNN.json through the tape db (0x8c0c116c): loop, loop_in, loop_out, pos,
//     track, tape_type, the tempo fields and "mixer_params", a JSON array of eight floats built from the Q31
//     words 0x11b01eb0 (four volumes, four pans) with the ROM int-to-float 0x04021854 and an exponent shift;
//     each field is set with 0x8d4f24dc(2, key, &value) / 0x8d4f1428. Its loader (0x8c0c348c(this, flag)) reads
//     the fields back by name (0x8c085068 has, 0x8c084f74 get, 0x8c084ee0 at, 0x8c084840 as int, 0x8c084ac0 as
//     float) and converts "mixer_params" with the ROM float-to-Q31 0x040212cc into the eight words; with the flag
//     set it writes the defaults instead (1.0 and centre) and never reads them. Unknown keys are ignored. The
//     mutes are not saved by stock.
//   * The MixerController (vtable 0x8400f100, mode 3): in mixer mode the mixer key and track key 1 select the
//     levels page (at 0x8c0bb61a the page word 0x849a856c is cleared), track keys 2-4 the EQ, drive and master pages; shift + track
//     key on the levels page toggles the track's mute byte 0x11902e34 + k (0x8c0bb04e); the knobs (on_encoder
//     0x8c0bb294, index 0-3, 4-7 with shift) adjust the Q31 word 0x11b01eb0 + 4 x index through 0x8d510b64 and
//     notify the screen (0x8fca1ed2(this + 8)). The mixer key's release reaches nothing (the MixerController's
//     key-up handles 19 and 20 only; the fallback's entry for key 5 is the default).
//   * The MixerScreen (singleton 0x84c96c20, update 0x8fc2e6a4) draws the four pan needles from the float vector
//     of ReadInterface 0x8d5168f4 (eight floats: volumes then pans), the four level numbers straight from the
//     words at 0x11b01eb0 (an 8-byte P4 load at 0x8fc30340), the mute colours from 0x8d516884(0..3) and the VU
//     circles from the vector of 0x8d516c90 (four floats: 20 log10(peak / 32768) of W[0x11905a34 + 2k], the
//     words cleared after reading).
//
// This unit (profile group "build-b3d", with the trampolines and stubs in Build B's mixer stubs):
//   * op1f_tape_json_set_mixer replaces the writer's CALL.a of 0x8d4f24dc for "mixer_params": it makes that
//     call, then adds "op1f_mixer_ext", an array of eight ints, the Q31 volumes and pans of tracks 5-8 from
//     op1f_ext_params (build_b_mix.cpp). op1f_tape_json_loaded runs after the loader (the entry trampoline
//     returns through it): with the loader's flag set it restores the defaults, as stock does for its own words;
//     otherwise it reads "op1f_mixer_ext" back when present and leaves the values alone when absent, as stock
//     leaves its own. The mutes stay in RAM like stock's.
//   * The mixer page: the mixer key's press and release on the levels page set and clear op1f_mixer_page
//     (defined in build_b_mix.cpp; the frame hook clears it when the mode is no longer the mixer). While it is
//     set, the screen's three value sources answer for tracks 5-8 (op1f_mixer_values, op1f_mixer_vu and the P4
//     stub for the level numbers; op1f_mixer_track_unmuted adds four to the track), the knobs move tracks 5-8's
//     volume and pan through op1f_ext_mix_adjust (tracks 7-8 are refused with a log line) and shift + track key
//     1/2 toggles the mute of track 5/6 through op1f_ext_mix_toggle_mute. Stock's own handlers run for everything
//     else, so a tap of the mixer key still selects the levels page.
//   * B3e: the true selected track is written beside the strip as "op1f_track" (stock's "track" is clamped to 3 by
//     B2's load replacement, so that stock firmware never records on a track it does not have) and put back into
//     [0x11905a08] after the loader when it names track 5 or 6 and the six-track cache is up; a volume or pan
//     change of the strip notifies the tape model (build_b_mix.cpp), which writes the JSON as stock's knobs do;
//     the saved line prints at most once per second. On the page the module toggles nodes of the B3e mixer
//     artwork (Build B's picture loader): the faint column numbers 1 and 2 give way to 5 and 6,
//     the groups col3 and col4 (everything of the columns of tracks 7-8, their level numbers included) are hidden
//     while the key is held. Without those ids (another artwork) the toggles are no-ops with one line.
//   * 8 tracks 12, edited in place: while the page is held, a knob's click (keys 54-57) on the levels page toggles
//     the mute of track 5-8 through op1f_ext_mix_click_mute, where stock's click toggles tracks 1-4's. Stock's
//     key-down handler picks by the mixer's page word 0x849a89b4: on the levels page (0) it toggles the track's
//     mute byte 0x11902e34 + k and its gain, and answers 0; on the EQ page (1) the click is the EQ's. The filter
//     takes the click only on the levels page and answers 1, so stock's toggle of track 1-4 does not run; on any
//     other page the click goes to stock as before. Until 12 the filter took no click, so a click on the page of
//     tracks 5-8 muted track 1-4, out of sight.
// Freestanding like the other units: no library, no float types (the ROM and stock conversions are called by
// address with their bit patterns). Everything here runs on the GUI thread.

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*JsonRootFn)();                                    // 0x8d4f2b80: the tape db's root object
typedef int (*JsonHasFn)(void* json, const char* key);            // 0x8c085068
typedef void* (*JsonGetFn)(void* json, const char* key);          // 0x8c084f74: the member (a reference)
typedef void* (*JsonAtFn)(void* json, int index);                 // 0x8c084ee0: the array element (a reference)
typedef int (*JsonIntFn)(void* json);                             // 0x8c084840
typedef void (*JsonCtorFn)(void* json, int kind);                 // 0x8c085ba8(json, kind): 0 a null, an array after the first push (kind 6 is an array; H1g)
typedef void (*JsonFromIntFn)(void* json, int value);             // 0x8c083b1c
typedef void (*JsonPushFn)(void* array, void* value);             // 0x8c085d54
typedef void (*JsonDtorFn)(void* json);                           // 0x8c0844e0
typedef void (*DbSetFn)(int db, const char* key, void* value);    // 0x8d4f24dc
typedef void* (*VectorFn)();                                      // 0x8d5168f4 / 0x8d516c90: &std::vector<float>
typedef u32 (*FloatFromIntFn)(i32 value);                         // ROM 0x04021854: the float of an int
typedef u32 (*FloatDivFn)(u32 a, u32 b);                          // ROM 0x040219e4: a / b
typedef u32 (*FloatCurveFn)(u32 x);                               // 0x8f0061d4: 20 log10(x), stock's VU law
typedef void (*NotifyFn)(void* observable);                       // 0x8fca1ed2: the observers redraw
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);   // 0x8fc25cc0: 0 when optional and missing

#define STOCK_LOG ((LogFn)0x8c053a20u)
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
#define STOCK_MIXER_VALUES ((VectorFn)0x8d5168f4u)
#define STOCK_VU_VALUES ((VectorFn)0x8d516c90u)
#define STOCK_FLOAT_FROM_INT ((FloatFromIntFn)0x04021854u)
#define STOCK_FLOAT_DIV ((FloatDivFn)0x040219e4u)
#define STOCK_VU_CURVE ((FloatCurveFn)0x8f0061d4u)
#define STOCK_NOTIFY ((NotifyFn)0x8fca1ed2u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)
#define MIXER_PAGE (*(volatile i32*)0x849a89b4u)   // 12: the mixer's page, 0 the levels page (the mixer key and track key 1 put it there)

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                   // F2: tracks 5-8 (6 from B2 to F1b)
    EXT_TRACKS = 4,                     // the strip holds tracks 5-8; 5 and 6 exist
    EXT_VALUES = 2 * EXT_TRACKS,        // volumes then pans
    KEY_MIXER = 5,
    KEY_TRACK_1 = 6,
    KEY_TRACK_4 = 9,
    KEY_KNOB_1 = 54,                    // 12: the knobs' clicks, 54 (the first knob, blue) to 57 (the fourth)
    KEY_KNOB_4 = 57,
    LEVELS_PAGE = 0,
    TAPE_DB = 2,
    JSON_BYTES = 16,                    // a stock json value is 12 bytes; 16 for alignment
    UNIT = 0x7fffffff,
    HALF = 0x40000000,
    FLOAT_2_31 = 0x4f000000,            // 2147483648.0f
    FLOAT_32768 = 0x47000000,           // 32768.0f
    OBSERVABLE_OFFSET = 8,              // the MixerController's observable, this + 8
    NODE_DISPLAY = 0x5c,                // B3e: a node's display byte, the one the tape screen toggles for its digits
    CYCLES_PER_SECOND = 400000000,
    SAVE_LOG_BUDGET = 64,               // B3e: saved lines, at most one per second
    LOG_BUDGET = 16
};
static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_mixer.cpp";
extern "C" i32 op1f_ext_params[3 * EXT_TRACKS] asm("op1f_ext_params");       // build_b_mix.cpp: volumes, pans, mute gains
extern "C" u16 op1f_ext_vu[EXT_TRACKS] asm("op1f_ext_vu");
extern "C" volatile unsigned char op1f_mixer_page asm("op1f_mixer_page");    // build_b_mix.cpp: the mixer key is held
extern "C" int op1f_ext_mix_adjust(int track, int knob, int delta) asm("op1f_ext_mix_adjust");
extern "C" int op1f_ext_mix_toggle_mute(int track) asm("op1f_ext_mix_toggle_mute");
extern "C" int op1f_ext_mix_click_mute(int track) asm("op1f_ext_mix_click_mute");      // 12: the same, with no track number shown
extern "C" void op1f_ext_mix_set_defaults() asm("op1f_ext_mix_set_defaults");
extern "C" int op1f_track_unmuted(int track) asm("op1f_track_unmuted");
extern "C" u32 op1f_cache_tracks asm("op1f_cache_tracks");                  // build_b_cache.cpp: 6 with the heap-1 cache

// Counters for the measurement line (weak references in the first frame hook's measuring unit).
extern "C" u32 op1f_ext_page_holds asm("op1f_ext_page_holds");
extern "C" u32 op1f_ext_page_knob_moves asm("op1f_ext_page_knob_moves");
extern "C" u32 op1f_ext_params_saves asm("op1f_ext_params_saves");
extern "C" u32 op1f_ext_params_loads asm("op1f_ext_params_loads");
u32 op1f_ext_page_holds;
u32 op1f_ext_page_knob_moves;
u32 op1f_ext_params_saves;
u32 op1f_ext_params_loads;
extern "C" u32 op1f_ext_track_restores asm("op1f_ext_track_restores");       // B3e: the selected extension track put back after a load
u32 op1f_ext_track_restores;

static u32 refusal_log_budget = LOG_BUDGET;

struct Json {                            // a stock json value, opaque
    u32 words[JSON_BYTES / 4];
} __attribute__((aligned(4)));

struct FloatVector {                     // the shape the MixerScreen reads: begin at +4, end at +8
    u32 header;
    u32* begin;
    u32* end;
    u32* capacity;
};
static u32 ext_values[EXT_VALUES];        // float bit patterns: volumes 5-8, then pans 5-8
static u32 ext_vu[EXT_TRACKS];
static FloatVector ext_values_vector = { 0, ext_values, ext_values + EXT_VALUES, ext_values + EXT_VALUES };
static FloatVector ext_vu_vector = { 0, ext_vu, ext_vu + EXT_TRACKS, ext_vu + EXT_TRACKS };

static u32 q31_to_float(i32 value) {     // as stock's getter: (float)value / 2^31
    return STOCK_FLOAT_DIV(STOCK_FLOAT_FROM_INT(value), FLOAT_2_31);
}

// ---------------------------------------------------------------------------------------------------------
// The mixer page. The filters are called by the entry trampolines at the MixerController's handlers; 1 means
// the module took the event, 0 lets stock continue.

static void refuse_track(const char* what, int track) {   // unreachable since F2 (eight tracks); kept for the fallback cache
    if (refusal_log_budget) {
        --refusal_log_budget;
        STOCK_LOG(2, LOG_FILE, 4, "mixer page: %s on track %d refused (no such track)", what, track + 1);
    }
}

extern "C" int op1f_mixer_key_filter(void* self, const int* event) asm("op1f_mixer_key_filter");
extern "C" int op1f_mixer_key_filter(void*, const int* event) {
    int key = event[0];
    if (key == KEY_MIXER && !op1f_mixer_page) {
        op1f_mixer_page = 1;                                    // stock selects the levels page as before
        ++op1f_ext_page_holds;
    }
    if (op1f_mixer_page && key >= KEY_KNOB_1 && key <= KEY_KNOB_4 && MIXER_PAGE == LEVELS_PAGE) {
        op1f_ext_mix_click_mute(STOCK_TRACKS + key - KEY_KNOB_1);   // 12: track 5-8, as stock's click mutes 1-4
        return 1;                                               // stock's click would toggle track 1-4
    }
    return 0;
}

extern "C" int op1f_mixer_key_up_filter(void* self, int key) asm("op1f_mixer_key_up_filter");
extern "C" int op1f_mixer_key_up_filter(void* self, int key) {
    if (key == KEY_MIXER && op1f_mixer_page) {
        op1f_mixer_page = 0;
        STOCK_NOTIFY((char*)self + OBSERVABLE_OFFSET);          // redraw with tracks 1-4
    }
    return 0;
}

extern "C" int op1f_mixer_shift_key_filter(void* self, const int* event) asm("op1f_mixer_shift_key_filter");
extern "C" int op1f_mixer_shift_key_filter(void* self, const int* event) {
    if (!op1f_mixer_page) return 0;
    int key = event[0];
    if (key < KEY_TRACK_1 || key > KEY_TRACK_4) return 0;
    int track = STOCK_TRACKS + key - KEY_TRACK_1;
    if (track < EIGHT_TRACKS) op1f_ext_mix_toggle_mute(track);
    else refuse_track("mute", track);
    STOCK_NOTIFY((char*)self + OBSERVABLE_OFFSET);
    return 1;
}

extern "C" int op1f_mixer_encoder_filter(void* self, int index, int delta) asm("op1f_mixer_encoder_filter");
extern "C" int op1f_mixer_encoder_filter(void* self, int index, int delta) {
    if (!op1f_mixer_page || (u32)index >= EXT_VALUES) return 0;
    int track = STOCK_TRACKS + (index & 3);
    if (track < EIGHT_TRACKS) {
        op1f_ext_mix_adjust(track, index >> 2, delta);         // 0-3 volumes, 4-7 pans, as stock's index
        ++op1f_ext_page_knob_moves;
    } else {
        refuse_track("knob", track);
    }
    STOCK_NOTIFY((char*)self + OBSERVABLE_OFFSET);
    return 1;
}

// B3e: the nodes of the mixer artwork the page toggles, looked up once per screen object with the optional flag
// (the B3e mixer.svg has them; another artwork leaves them 0 and the toggles do nothing). B3f drops lev3 and
// lev4 from the list: after the screen's init has made them its text labels they are no longer found by id
// (the B3e test logged `Can't find SVG object 'lev3'`), and hiding col3/col4 hides their numbers anyway.
// F2: tracks 7 and 8 exist, so columns 3 and 4 stay shown on the page and their numbers switch to the hidden
// digits trackno7 and trackno8 of the F2 mixer artwork, as columns 1-2 switch to trackno5 and trackno6.
enum { NODE_TRACKNO1, NODE_TRACKNO2, NODE_TRACKNO3, NODE_TRACKNO4, NODE_TRACKNO5, NODE_TRACKNO6, NODE_TRACKNO7, NODE_TRACKNO8, NODE_COUNT };
static const char* const PAGE_NODE_IDS[NODE_COUNT] = { "trackno1", "trackno2", "trackno3", "trackno4", "trackno5", "trackno6", "trackno7", "trackno8" };
static void* page_screen;
static void* page_nodes[NODE_COUNT];
static int page_applied = -1;             // the page state the nodes show: -1 unknown, 0 tracks 1-4, 1 tracks 5-8
static u32 page_art_log_budget = 1;

static void ensure_page_nodes(void* screen) {
    if (screen == page_screen) return;
    page_screen = screen;
    page_applied = -1;
    int found = 0;
    for (int i = 0; i < NODE_COUNT; ++i) {
        page_nodes[i] = STOCK_FIND_NODE(screen, PAGE_NODE_IDS[i], 1);
        if (page_nodes[i]) ++found;
    }
    if (page_art_log_budget) {
        --page_art_log_budget;
        STOCK_LOG(found == NODE_COUNT ? 3 : 2, LOG_FILE, 6, "mixer page artwork: %d of %d nodes found%s", found, NODE_COUNT,
                  found == NODE_COUNT ? "" : " (the columns stay as the artwork draws them)");
    }
}

static void set_shown(int index, int shown) {
    unsigned char* node = (unsigned char*)page_nodes[index];
    if (node) node[NODE_DISPLAY] = (unsigned char)shown;
}

// At the start of the screen's update, on the gfx thread: the column numbers and columns 3-4 follow the page flag.
static void apply_page_nodes(void* screen) {
    ensure_page_nodes(screen);
    int held = op1f_mixer_page ? 1 : 0;
    if (held == page_applied) return;
    page_applied = held;
    for (int i = NODE_TRACKNO1; i <= NODE_TRACKNO4; ++i) set_shown(i, !held);
    for (int i = NODE_TRACKNO5; i <= NODE_TRACKNO8; ++i) set_shown(i, held);
}

// Replaces the MixerScreen's call of 0x8d5168f4 (0x8fc3023a): the eight floats of tracks 5-8 while the page is held.
// R0 still holds the screen object at the site (what precedes it copies R0 and keeps it, and nothing writes R0).
extern "C" void* op1f_mixer_values(void* screen) asm("op1f_mixer_values");
extern "C" void* op1f_mixer_values(void* screen) {
    apply_page_nodes(screen);                               // B3e
    if (!op1f_mixer_page) return STOCK_MIXER_VALUES();
    for (int i = 0; i < EXT_VALUES; ++i) ext_values[i] = q31_to_float(op1f_ext_params[i]);
    return &ext_values_vector;
}

// Replaces the call of 0x8d516c90 (0x8fc304c8): stock's VU law on the peaks of tracks 5-8, which are cleared as
// stock clears its own words.
extern "C" void* op1f_mixer_vu() asm("op1f_mixer_vu");
extern "C" void* op1f_mixer_vu() {
    if (!op1f_mixer_page) return STOCK_VU_VALUES();
    for (int j = 0; j < EXT_TRACKS; ++j) {
        i32 peak = op1f_ext_vu[j];
        op1f_ext_vu[j] = 0;
        ext_vu[j] = STOCK_VU_CURVE(STOCK_FLOAT_DIV(STOCK_FLOAT_FROM_INT(peak), FLOAT_32768));
    }
    return &ext_vu_vector;
}

// Replaces the four calls of 0x8d516884(0..3) of the update: the mute colour of track 4 + k while the page is held
// (tracks 7-8 show as unmuted).
extern "C" int op1f_mixer_track_unmuted(int track) asm("op1f_mixer_track_unmuted");
extern "C" int op1f_mixer_track_unmuted(int track) {
    if (!op1f_mixer_page) return op1f_track_unmuted(track);
    int ext = track + STOCK_TRACKS;
    if (ext >= EIGHT_TRACKS) return 1;
    return op1f_track_unmuted(ext);
}
