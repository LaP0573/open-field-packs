// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build E: the two cosmetic screens for tracks 5 and 6 (OS 1.7.0+0).
//
// The help screen (HelpScreen.cpp, help.svg): its update asks the selected-track getter 0x8d514348, then colours
// the four legend boxes t1tape..t4tape (node pointers at this + 0x2c, one 4-byte colour each) in a hardware loop:
// the highlight word at this + 0xa4 for the box whose index equals the track, the normal word at this + 0xa8 for
// the others. With track 4 or 5 selected no box matched. The CALL.a at 0x8fc2e5bc now lands in op1f_help_track
// (build_e_screens.s), which passes the screen here. Since E3 the module serves a help.svg whose legend row has
// six boxes: the stock four moved closer together and, for tracks 5 and 6, two boxes drawn
// twice each, a dim copy (t5tapeoff, t6tapeoff, in stock's normal colour #5B6670) and a lit copy (t5tapeon,
// t6tapeon, in stock's highlight #CDD0C2, both measured on the device with the first image). The OS parser has no
// display attribute, so every copy is visible after parsing; on every update this function shows the copy that
// matches the selected track and hides the other, through the nodes' display byte, and returns the true track to
// stock's loop, which lights one of its four boxes for 0-3 and none for 4-5. The first image (Build E) lit stock's
// box 1/2 for 5/6 by returning track - 4 and swapping the colour word; E2 fixed that colour's byte order; E3
// replaces both with the six-box row.
//
// The signal-flow screen (SignalFlowScreen.cpp, signalflow.svg, shift + mixer): its update asks the selected
// track, whether it is unmuted (0x8d516884) and, when it is, the track's level from the eight-float vector of
// 0x8d5168f4 (the volumes of tracks 1-4, then their pans; the index is the track) for the warning triangle beside
// the big track digit, shown below 0.2. With track 4 or 5 the index read the pan of track 1 or 2. The CALL.a at
// 0x8fc3574e now lands in op1f_signalflow_unmuted: tracks 0-3 go on to the six-track getter; for 4-5 the module
// answers the mute from its strip and rewrites the index slot so that stock's load lands on the module's float of
// that track's volume, converted as stock converts its own values (the ROM int-to-float, divided by 2^31).
//
// Build E4 adds two tape meters to the signal-flow screen. Stock draws one meter per track at the top of the chain
// drawing: a line node (tapevuN) that it reshapes on every update with a transform built from the track's level,
// anchored at the box of the base dot (tapevubaseN). Its loop over the four: the level in dB from the VU vector
// (20 log10 of the block peak / 32768, the words cleared as read), plus 6, capped at 3.3 (anything not below 3.3,
// infinities included, reads as 3.3), 10^(x / 20) through 0x8f0061a8, then 0x8fc356a0 (× 1.4, − 1, × 1024, to an
// int), a one-pole smoothing with the gain word at 0x84c9752c (new = ((gain − 921) × fixed + old × 921) >> 10, the
// old value kept per meter), 0x8fc2facc(anchor max x, anchor max y − 1024, new) building a 9-word transform, and
// 0x8c03dbbe(line, transform) applying it. The CALL.a that takes the VU vector (0x8fc35738) now lands in
// op1f_signalflow_vu, which returns stock's vector and runs the same steps for tracks 5 and 6 on the lines
// tapevu5 and tapevu6 of the module's signalflow.svg, from the module's own peaks.
//
// Build F, image F1: the tape gauge on the help screen. The served help.svg carries,
// between the speech bubble and the legend row, two reels in the style of the stock reel-to-reel tape screen and
// the free time of the tape as MM:SS in Teenage Engineering's own digit glyphs. The left reel's roll shows the
// free part of the tape partition, the right reel's roll the used part (audio and undo copies), each as one of 13
// sizes (gaugel00..gaugel12, gauger00..gauger12, the roll's area following the amount); each digit position has
// ten glyph nodes (gaugedPN) of which one shows. On every help-screen update in tape mode this unit sets the
// display bytes of those 66 nodes from op1f_tape_free_chunks, which the storage unit refreshes about once a
// second (build_b_storage.cpp). The parser shows every node after parsing, so the first update hides the rest.
//
// Nothing else changes: the help screen's other loop (the synth slots) and stock's four meters stay stock's. No
// assert is on any of these paths; a track of 6 or more behaves as stock (no box lit, the getter's answer).

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef u32 (*FloatFromIntFn)(i32 value);                          // ROM 0x04021854: the float of an int
typedef u32 (*FloatDivFn)(u32 a, u32 b);                           // ROM 0x040219e4: a / b
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);   // 0x8fc25cc0: 0 when optional and missing

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FLOAT_FROM_INT ((FloatFromIntFn)0x04021854u)
#define STOCK_FLOAT_DIV ((FloatDivFn)0x040219e4u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)               // the GUI's selected track, what 0x8d514348 returns

enum {
    STOCK_TRACKS = 4,
    EXT_TRACKS = 4,                     // F2: tracks 5-8 (2 until F1b)
    STRIP_SLOTS = 4,                    // build_b_mix.cpp keeps a strip of four (tracks 5-8): volumes, pans, mute gains
    NODE_DISPLAY = 0x5c,                // an svg node's display byte, the one the tape screen toggles for its digits
    LEGEND_NODES = 2 * EXT_TRACKS,      // t5tapeoff, t5tapeon, .. t8tapeoff, t8tapeon
    VECTOR_BEGIN_OFFSET = 4,            // the float vector: its first element's address at +4 (stock reads [R7 + 4])
    FLOAT_2_31 = 0x4f000000,            // 2147483648.0f, what 0x8d5168f4 divides by
    LOG_BUDGET = 8                      // F2: the F1b log dropped the gauge's line with a budget of four
};

static const char LOG_FILE[] = "../src/gui/op1f_screens.cpp";

extern "C" i32 op1f_ext_params[3 * STRIP_SLOTS] asm("op1f_ext_params");    // build_b_mix.cpp: volumes 0-3, pans 4-7, mute gains 8-11
extern "C" int op1f_track_unmuted(int ch) asm("op1f_track_unmuted");       // build_b_mix.cpp: six tracks

extern "C" u32 op1f_help_ext_frames asm("op1f_help_ext_frames");            // help screen updates with track 5/6 selected
u32 op1f_help_ext_frames;
extern "C" u32 op1f_signalflow_ext_reads asm("op1f_signalflow_ext_reads");  // signal-flow level reads of track 5/6
u32 op1f_signalflow_ext_reads;

static u32 log_budget = LOG_BUDGET;

static void say(int level, int line, const char* what, u32 a, u32 b, u32 c) {
    if (!log_budget) return;
    --log_budget;
    STOCK_LOG(level, LOG_FILE, line, what, a, b, c);
}

// ---- the help screen -------------------------------------------------------------------------------------

static const char* const LEGEND_IDS[LEGEND_NODES] = { "t5tapeoff", "t5tapeon", "t6tapeoff", "t6tapeon", "t7tapeoff", "t7tapeon", "t8tapeoff", "t8tapeon" };
static void* legend_screen;                     // the HelpScreen the nodes were looked up on
static u8* legend_nodes[LEGEND_NODES];           // 0 when the artwork has no such node (the device's file, say)

// ---- the tape gauge (Build F) ----------------------------------------------------------------------------

extern "C" u32 __attribute__((weak)) op1f_tape_free_chunks asm("op1f_tape_free_chunks");   // build_b_storage.cpp (F1)

enum {
    GAUGE_STEPS = 13,                   // roll sizes per reel, as the artwork tool draws them
    GAUGE_DIGITS = 4,                   // MM:SS
    CHUNKS_PER_PARTITION = 8192,
    CHUNK_FRAMES = 16384,
    SAMPLE_RATE = 44100
};
// A roll shows step k when its amount, in chunks, is at least ROLL_THRESHOLD[k]: 8192 x ((k - 0.5) / 12)^2, the
// roll's area following the amount as the artwork's radii do; step 0 is an empty reel.
static const u32 ROLL_THRESHOLD[GAUGE_STEPS] = { 0, 14, 128, 356, 697, 1152, 1721, 2403, 3200, 4110, 5134, 6272, 7523 };
static u8* gauge_roll[2][GAUGE_STEPS];           // left (free), right (used)
static u8* gauge_digit[GAUGE_DIGITS][10];
static u32 gauge_found;
static char gauge_id[16];

static u32 udiv(u32 n, u32 d) {                  // restoring shift-subtract division (no libgcc routine); d == 0 yields 0
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static u8* gauge_node(u8* screen, const char* prefix, u32 a, u32 b) {   // the node "<prefix><a><b>", a and b one digit each
    char* p = gauge_id;
    while (*prefix) *p++ = *prefix++;
    *p++ = (char)('0' + a);
    *p++ = (char)('0' + b);
    *p = 0;
    u8* node = (u8*)STOCK_FIND_NODE(screen, gauge_id, 1);
    if (node) ++gauge_found;
    return node;
}

static void gauge_setup(u8* screen) {
    gauge_found = 0;
    for (u32 k = 0; k < GAUGE_STEPS; ++k) {                          // gaugel00..gaugel12, gauger00..gauger12
        u32 tens = k >= 10 ? 1u : 0u;
        gauge_roll[0][k] = gauge_node(screen, "gaugel", tens, k - 10 * tens);
        gauge_roll[1][k] = gauge_node(screen, "gauger", tens, k - 10 * tens);
    }
    for (u32 p = 0; p < GAUGE_DIGITS; ++p)                           // gauged00..gauged39: position, digit
        for (u32 d = 0; d < 10; ++d) gauge_digit[p][d] = gauge_node(screen, "gauged", p, d);
    say(3, 6, "help gauge: %u of %u nodes found", gauge_found, 2 * GAUGE_STEPS + 10 * GAUGE_DIGITS, 0);
}

static u32 roll_step(u32 chunks) {
    u32 k = 0;
    while (k + 1 < GAUGE_STEPS && chunks >= ROLL_THRESHOLD[k + 1]) ++k;
    return k;
}

static void gauge_update() {
    if (!gauge_found) return;
    u32 free = (&op1f_tape_free_chunks != 0) ? op1f_tape_free_chunks : 0u;
    if (free > CHUNKS_PER_PARTITION) free = CHUNKS_PER_PARTITION;
    u32 used = CHUNKS_PER_PARTITION - free;
    u32 step[2] = { roll_step(free), roll_step(used) };
    for (u32 side = 0; side < 2; ++side)
        for (u32 k = 0; k < GAUGE_STEPS; ++k)
            if (gauge_roll[side][k]) gauge_roll[side][k][NODE_DISPLAY] = (u8)(k == step[side] ? 1 : 0);
    u32 seconds = udiv(free * CHUNK_FRAMES, SAMPLE_RATE);           // at most 3,043 for a whole partition
    u32 minutes = udiv(seconds, 60);
    seconds -= minutes * 60;
    if (minutes > 99) minutes = 99;
    u32 shown[GAUGE_DIGITS] = { udiv(minutes, 10), minutes - udiv(minutes, 10) * 10, udiv(seconds, 10), seconds - udiv(seconds, 10) * 10 };
    for (u32 p = 0; p < GAUGE_DIGITS; ++p)
        for (u32 d = 0; d < 10; ++d)
            if (gauge_digit[p][d]) gauge_digit[p][d][NODE_DISPLAY] = (u8)(d == shown[p] ? 1 : 0);
}

extern "C" i32 op1f_help_track_ext(u8* screen) asm("op1f_help_track_ext");
extern "C" i32 op1f_help_track_ext(u8* screen) {
    i32 track = SELECTED_TRACK;
    if (legend_screen != (void*)screen) {                            // once per screen object: the nodes by id
        legend_screen = screen;
        u32 found = 0;
        for (u32 i = 0; i < LEGEND_NODES; ++i) {
            legend_nodes[i] = (u8*)STOCK_FIND_NODE(screen, LEGEND_IDS[i], 1);
            if (legend_nodes[i]) ++found;
        }
        say(3, 1, "help legend: %u of %u nodes for tracks 5-8 found", found, LEGEND_NODES, 0);
        gauge_setup(screen);                                         // Build F
    }
    for (u32 j = 0; j < EXT_TRACKS; ++j) {                           // the lit copy for the selected track, the dim one otherwise
        int lit = track == (i32)(STOCK_TRACKS + j);
        if (legend_nodes[2 * j]) legend_nodes[2 * j][NODE_DISPLAY] = (u8)(lit ? 0 : 1);
        if (legend_nodes[2 * j + 1]) legend_nodes[2 * j + 1][NODE_DISPLAY] = (u8)(lit ? 1 : 0);
    }
    gauge_update();                                                  // Build F: the reels and the free time
    if (track >= STOCK_TRACKS && track < STOCK_TRACKS + EXT_TRACKS) ++op1f_help_ext_frames;
    return track;                                                   // as the getter: 0-3 light their stock box, 4-5 none
}

// ---- the signal-flow screen -------------------------------------------------------------------------------

static u32 ext_level_float[EXT_TRACKS] __attribute__((aligned(4)));   // float bit patterns of tracks 5-6's volume

extern "C" i32 op1f_signalflow_level_ext(u32 track, const u8* vector, u32* slot) asm("op1f_signalflow_level_ext");
extern "C" i32 op1f_signalflow_level_ext(u32 track, const u8* vector, u32* slot) {
    i32 unmuted = op1f_track_unmuted((int)track);
    u32 j = track - STOCK_TRACKS;
    if (j < EXT_TRACKS && vector && slot) {
        ext_level_float[j] = STOCK_FLOAT_DIV(STOCK_FLOAT_FROM_INT(op1f_ext_params[j]), FLOAT_2_31);
        const u8* begin = *(const u8* const*)(vector + VECTOR_BEGIN_OFFSET);
        u32 index = (u32)((const u8*)&ext_level_float[j] - begin) >> 2;   // stock adds index << 2 to begin: the module's float
        *slot = index;
        if (op1f_signalflow_ext_reads == 0) {
            say(3, 2, "signal flow: track %d level %08x read through index %08x", track + 1, ext_level_float[j], index);
        }
        ++op1f_signalflow_ext_reads;
    }
    return unmuted;
}

// ---- the signal-flow meters of tracks 5 and 6 (E4) ---------------------------------------------------------

typedef void* (*VuVectorFn)();                                     // 0x8d516c90: the four-float VU vector, in dB
typedef u32 (*FloatCurveFn)(u32 x);                                // 0x8f0061d4: 20 log10 x
typedef u32 (*FloatAddFn)(u32 a, u32 b);                           // 0x040218e2: a + b
typedef u32 (*DbToAmpFn)(u32 db);                                  // 0x8f0061a8: 10 ^ (db / 20)
typedef i32 (*ToFixedFn)(u32 amp);                                 // 0x8fc356a0: ((amp × 1.4) − 1) × 1024, as an int
typedef void (*ApplyMatrixFn)(void* node, const u32* matrix);      // 0x8c03dbbe: the node takes the transform
typedef void (*BoundsFn)(void* node, i32* box);                    // a node's vtable slot 3: its box, min x, max x, min y, max y
typedef u8* (*ColourPointerFn)(void* screen, const char* id, int optional);   // 0x8fc25ebc: the colour bytes of the node with that id
typedef u32 (*FloatMulFn)(u32 a, u32 b);                           // 0x04021ac0: a × b
typedef u32 (*FloatDivFn2)(u32 a, u32 b);                          // 0x040219e4: a / b
typedef i32 (*FloatToIntFn)(u32 a);                                // 0x04020fd8: the int of a float, as stock's meter code uses it

#define STOCK_VU_VALUES ((VuVectorFn)0x8d516c90u)
#define STOCK_VU_CURVE ((FloatCurveFn)0x8f0061d4u)
#define STOCK_FLOAT_ADD ((FloatAddFn)0x040218e2u)
#define STOCK_FLOAT_MUL ((FloatMulFn)0x04021ac0u)
#define STOCK_FLOAT_DIV2 ((FloatDivFn2)0x040219e4u)
#define STOCK_FLOAT_TO_INT ((FloatToIntFn)0x04020fd8u)
#define STOCK_DB_TO_AMP ((DbToAmpFn)0x8f0061a8u)
#define STOCK_METER_FIXED ((ToFixedFn)0x8fc356a0u)
#define STOCK_APPLY_MATRIX ((ApplyMatrixFn)0x8c03dbbeu)
#define STOCK_COLOUR_POINTER ((ColourPointerFn)0x8fc25ebcu)
#define METER_SMOOTH_GAIN (*(volatile i32*)0x84c9752cu)             // stock's smoothing gain word, read per meter as stock does
#define SIGNALFLOW_PALETTE ((const volatile u32*)0x84c96ac8u)       // the screen's colour words: +0x20 bright, +0x28 dim, +0x38 over the threshold
#define LIGHT_THRESHOLD (*(const volatile u32*)0x848d43e4u)         // 0.9f: a level at or above it takes the colour at +0x38

enum {
    METERS = 4,                         // F2: the meters of tracks 5-8 (2 until F1b)
    FLOAT_6 = 0x40c00000,               // 6.0f
    FLOAT_3_3 = 0x40533333,             // 3.3f
    FLOAT_80 = 0x42a00000,              // 80.0f: the bracket's colour runs from dim at -80 dB to bright at 0 dB
    FLOAT_1024 = 0x44800000,            // 1024.0f
    FLOAT_32768 = 0x47000000,           // 32768.0f
    SMOOTH_KEEP = 921,
    FIXED_ONE = 1024,
    MATRIX_WORDS = 9,
    VTABLE_BOUNDS = 3,
    PALETTE_BRIGHT = 8,                 // word index of +0x20
    PALETTE_DIM = 10,                   // +0x28
    PALETTE_OVER = 14,                  // +0x38
    METER_LOG_BUDGET = 24,
    METER_LOG_EVERY = 128               // frames between two diagnostic lines, about three seconds
};

extern "C" unsigned short op1f_ext_vu[STRIP_SLOTS] asm("op1f_ext_vu");              // build_b_mix.cpp: the block peaks of tracks 5-8
extern "C" void op1f_matrix_call(i32 x, i32 y, i32 value, u32* matrix) asm("op1f_matrix_call");   // build_e_screens.s
extern "C" u32 op1f_signalflow_meter_frames asm("op1f_signalflow_meter_frames");       // updates that reshaped the two meters
// Build G (G1, Build G's duck unit): the duck page drawn on every update of this screen; weak, so earlier images link unchanged.
extern "C" void __attribute__((weak)) op1f_duck_screen(u8* screen, const u8* vector, const u32* ext_db) asm("op1f_duck_screen");
static u32 ext_db[METERS];                  // this update's dB floats of tracks 5-8, for the duck page
u32 op1f_signalflow_meter_frames;

static const char* const METER_IDS[METERS][3] = { { "tapevu5", "tapevubase5", "tape4" }, { "tapevu6", "tapevubase6", "tape5" },
                                                  { "tapevu7", "tapevubase7", "tape6" }, { "tapevu8", "tapevubase8", "tape7" } };
static void* meter_screen;                  // the SignalFlowScreen the nodes were looked up on
static u8* meter_line[METERS];              // 0 when the artwork has no such meter (the device's file, say)
static u8* meter_bracket[METERS];           // the colour bytes of the bracket into the EQ (E5), 0 when missing
static i32 meter_anchor[METERS][2];         // the base dot's box: max x, max y, as stock keeps per meter
static i32 meter_value[METERS];             // the smoothed value, as stock's table keeps per meter
static u32 meter_log_budget = METER_LOG_BUDGET;
static int float_below(u32 a, u32 b);

// The bracket's colour, as stock's loop colours the brackets of tracks 1-4 (E5): a level at or above 0.9 takes the
// palette's word at +0x38; otherwise the colour runs from the dim word (+0x28) at -80 dB and below to the bright
// word (+0x20) at 0 dB, each byte blended as (bright × k + dim × (gain − k)) >> 10 with k = (level + 80) / 80 × 1024
// capped at the gain word.
static u32 bracket_colour(u32 db) {
    u32 over_or_nan = (db & 0x7fffffffu) >= 0x7f800001u || (LIGHT_THRESHOLD & 0x7fffffffu) >= 0x7f800001u;
    if (!over_or_nan && !float_below(db, LIGHT_THRESHOLD)) return SIGNALFLOW_PALETTE[PALETTE_OVER];
    u32 t = STOCK_FLOAT_ADD(FLOAT_80, db);
    i32 k = 0;
    if (!(t & 0x80000000u) && (t & 0x7fffffffu) < 0x7f800001u && t != 0) {
        k = STOCK_FLOAT_TO_INT(STOCK_FLOAT_MUL(STOCK_FLOAT_DIV2(t, FLOAT_80), FLOAT_1024));
    }
    i32 gain = METER_SMOOTH_GAIN;
    if (k > gain) k = gain;
    u32 bright = SIGNALFLOW_PALETTE[PALETTE_BRIGHT];
    u32 dim = SIGNALFLOW_PALETTE[PALETTE_DIM];
    u32 out = 0;
    for (u32 i = 0; i < 4; ++i) {
        u32 b = (bright >> (8 * i)) & 0xffu;
        u32 d = (dim >> (8 * i)) & 0xffu;
        u32 c = ((b * (u32)k + d * (u32)(gain - k)) >> 10) & 0xffu;
        out |= c << (8 * i);
    }
    return out;
}

static void paint(u8* colour, u32 word) {    // the four bytes in the word's memory order, as stock writes them
    colour[0] = (u8)word;
    colour[1] = (u8)(word >> 8);
    colour[2] = (u8)(word >> 16);
    colour[3] = (u8)(word >> 24);
}

static int float_below(u32 a, u32 b) {      // a < b, by sign and magnitude as stock compares the level with 3.3
    i32 sa = (a & 0x80000000u) ? -(i32)(a & 0x7fffffffu) : (i32)(a & 0x7fffffffu);
    i32 sb = (b & 0x80000000u) ? -(i32)(b & 0x7fffffffu) : (i32)(b & 0x7fffffffu);
    return sa < sb;
}

static void meter_setup(u8* screen) {
    meter_screen = screen;
    u32 found = 0;
    for (u32 j = 0; j < METERS; ++j) {
        meter_line[j] = (u8*)STOCK_FIND_NODE(screen, METER_IDS[j][0], 1);
        u8* dot = (u8*)STOCK_FIND_NODE(screen, METER_IDS[j][1], 1);
        meter_bracket[j] = STOCK_FIND_NODE(screen, METER_IDS[j][2], 1) ? STOCK_COLOUR_POINTER(screen, METER_IDS[j][2], 0) : 0;
        meter_value[j] = 0;
        if (meter_line[j] && dot) {
            i32 box[4] = { 0x7fffffff, (i32)0x80000000, 0x7fffffff, (i32)0x80000000 };
            BoundsFn bounds = (*(BoundsFn**)dot)[VTABLE_BOUNDS];
            bounds(dot, box);
            meter_anchor[j][0] = box[1];
            meter_anchor[j][1] = box[3];
            ++found;
        } else {
            meter_line[j] = 0;
        }
    }
    u32 brackets = 0;
    for (u32 j = 0; j < METERS; ++j) if (meter_bracket[j]) ++brackets;
    say(3, 3, "signal flow meters: %u of %u found, meter 5 anchored at %08x", found, METERS, (u32)meter_anchor[0][0]);
    say(3, 5, "signal flow brackets: %u found, palette bright %08x dim %08x", brackets,
        SIGNALFLOW_PALETTE[PALETTE_BRIGHT], SIGNALFLOW_PALETTE[PALETTE_DIM]);
}

extern "C" void* op1f_signalflow_vu_ext(u8* screen) asm("op1f_signalflow_vu_ext");
extern "C" void* op1f_signalflow_vu_ext(u8* screen) {
    void* vector = STOCK_VU_VALUES();                                // stock's four, cleared as read, for stock's loop
    if (meter_screen != (void*)screen) meter_setup(screen);
    int report = meter_log_budget && (op1f_signalflow_meter_frames % METER_LOG_EVERY) == 0;
    for (u32 j = 0; j < METERS; ++j) {
        if (!meter_line[j] && !meter_bracket[j]) continue;
        i32 peak = op1f_ext_vu[j];
        op1f_ext_vu[j] = 0;
        u32 db = STOCK_VU_CURVE(STOCK_FLOAT_DIV(STOCK_FLOAT_FROM_INT(peak), FLOAT_32768));
        ext_db[j] = db;
        i32 value = 0;
        if (meter_line[j]) {
            u32 t = STOCK_FLOAT_ADD(FLOAT_6, db);
            u32 level = (float_below(t, FLOAT_3_3) && (t & 0x7fffffffu) < 0x7f800001u) ? t : (u32)FLOAT_3_3;
            i32 fixed = STOCK_METER_FIXED(STOCK_DB_TO_AMP(level));
            value = ((METER_SMOOTH_GAIN - SMOOTH_KEEP) * fixed + meter_value[j] * SMOOTH_KEEP) >> 10;   // stock's 64-bit products fit 32 bits here: the gain word is near 1024 and the values are within a few thousand
            meter_value[j] = value;
            u32 matrix[MATRIX_WORDS];
            op1f_matrix_call(meter_anchor[j][0], meter_anchor[j][1] - FIXED_ONE, value, matrix);
            STOCK_APPLY_MATRIX(meter_line[j], matrix);
        }
        u32 word = 0;
        if (meter_bracket[j]) {
            word = bracket_colour(db);                                 // E5: the bracket lit by the level, as stock's
            paint(meter_bracket[j], word);
        }
        if (report && j == 0) {
            --meter_log_budget;
            STOCK_LOG(3, LOG_FILE, 4, "signal flow meter 5: peak %u level %08x value %d bracket %08x", (u32)peak, db, value, word);
        }
    }
    ++op1f_signalflow_meter_frames;
    if (&op1f_duck_screen != 0) op1f_duck_screen(screen, (const u8*)vector, ext_db);   // Build G: the duck page
    return vector;
}
