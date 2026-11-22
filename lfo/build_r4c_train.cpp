// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4c (base 27): the effect train's page and the UFOs' page, in fx train (version 13) and lfo (version 11). A
// sibling of fx train 12's train unit, which stays as it was for the packs before, with one change and nothing else:
// the page step runs from the frame only while the unit is in the instrument's mode (the frame's `mode` is the host
// the step works for), whether the base took the flag OP1F_PAGE_STAYS or not. Under base 26 the step ran in every mode
// while the base kept the page, and a request it made there became the page of the mode the unit was in: a saved
// effect preset shows "saved" for a second and then wants the presets' list, so the mixer key pressed within that
// second put the list over the mixer (bases 21 and 24 did the same). Now the step waits while the unit is away, the
// page stays as it was asked, and at the first frame back the base shows it at once and the step asks for what is
// wanted then. Under a base that refused the flag the word is cleared as before (that base closed the page), and the
// page is asked again at the first frame back.
//
// What follows is the elder's description, which holds but for its last sentence on the base that took the flag,
// rewritten.
//
// Step R4b: the effect train's page and the UFOs' page, in fx train (version 12) and lfo (version 10). A sibling of
// fx train 11's train unit, which stays as it was for the packs before: everything of it is here as it was, and two
// things are added to the page step, each for the flash seen on the unit going back from the mixer to the train (the
// synth's own page for some 200 ms):
//
//   The page stays with its mode. The page is asked of the base with the flag OP1F_PAGE_STAYS (module/
//   build_r4b_card.h): from base 26 on, the base keeps it while the unit visits another mode, and the firmware's own
//   screen request shows it again at once when the unit comes back. A base before 26 answers 0 for any flag: the page
//   is then asked again at once with no flag, and from then on with none (page_ask).
//   The page asked again at the first frame back, under a base that refused the flag. While the base's mode (the
//   frame's `mode`) is not the instrument the page step works for, the word of the document last asked is cleared and
//   the page step waits: such a base has closed the page for the mode change, so at the first frame back in the
//   instrument's mode the step asks for it again (one frame of the mode's own page, where the heal waited four). Where
//   the base took the flag, the word is left as it was and the step waits too (R4c): the base keeps the page for its
//   mode, and nothing is asked while the unit is in another.
//
// What follows is the elder unit's description, which holds.
//
// Step R2: the effect train's page and the UFOs' page, in the packs that show them. A sibling of
// Build H's train page unit, which stays as it was (the base packs 17 to 19 hold it): every function that draws and
// moves the page is that unit's, instruction for instruction. What changes is where the page's document comes from
// and how the page gets onto the screen.
//
//   The document. Made at the pack's first frame call from the pack's own fxtrain.svg (op1f_picture with the pack's
//   card), where the elder unit made it from the base's copy. No screen is registered: the document stands in a
//   holder of sixteen words whose second word is the document and the rest zero, which is all the firmware's lookup
//   of a part reads of a screen. So every function that finds and dresses the page's parts runs as it was.
//
//   The page. Where the synth's and the drum's screen answers chose the train (the elder unit's op1f_fx_screen_synth
//   and op1f_fx_screen_drum), page_wanted(host) answers the document to show, in the same order: for the synth, the
//   effect presets' save screen or list while the presets' screens exist; else the LFO presets' save screen or list;
//   else the train's document while the UFOs' page is open; else while the train is open; else none. For the drum,
//   the effect presets' screens, else the train, else none. The page step asks the base for the page
//   (op1f_page_show with the pack's card) or closes it (op1f_page_close) only when what is wanted changed. It runs
//   once a frame from the pack's frame function, and in fx train also from the chain's key handlers on the
//   firmware's key thread (op1f_train_page_now), so that the page follows a key at the next frame. While another
//   pack's page lies on top, a change of document waits until that page closes; the train or the UFOs opened (T3,
//   T4) come over it, as they always did. The two threads share single words: the document last asked, the page
//   last seen open (the train or the UFOs), and the frame's word that another pack's page is on top. Where they
//   meet half way, the next frame asks again from what the units say then. The step counts; the frame says it.
//
//   Two packs. This one text is compiled into fx train and, through lfo 9's train unit with
//   OP1F_R2_LFO_COPY defined, into lfo, which shows the UFOs where fx train is away. lfo's copy stands back while
//   fx train runs (the chain's view is there): it makes no document and asks for no page, and its
//   op1f_train_available answers fx train's. The names both copies define are wanted by neither pack of the other,
//   so neither offers them; the names only one copy has differ by copy.
//
// What follows is the elder unit's description of the drawing and its motion, which holds.
//
//   * Every frame (from the pack's frame function, after the chain's frame work) the train is dressed from the
//     chain's view with display bytes (+0x5c) and matrices (0x8c03dbbe), which the renderer draws on the next frame:
//     the four wagons placed in signal order, each knob's chassis and the character of its effect translated to an
//     eased position (a moved wagon hops over its neighbour, a wagon dropping onto its siding slides down), the
//     labels in the knob colours, an uncoupled wagon on its siding (its chassis and label in the dark step of its
//     knob's colour since H3b, its character dim), the links as unit lines mapped onto the coupling bars (skipping
//     the sidings), the lamp on the mirrored wagon, the hint row under shift (0x8fc24ca4), the parameter card after
//     a knob turn, the dial's needle rotated by the cost; a refused pick sags the rail under the wagon (the sag
//     group), squashes its wheels (the sketched chassis) and names the refused effect in red while FULL shows and the
//     needle stands red at the stop (H3).
//   * The motion (H3): the train runs while the instrument sounds (the chain unit's bus peak, held for half a
//     second): the ground, the near hills and the far ridge slide right at their own speeds and wrap on their tiles,
//     the drivers' spokes and every wheel turn, the rod rides the crank pins, the smoke puffs grow and trail, the
//     headlamp's beam shows; the bell and the driver's arm swing, the windmill and the clouds move with a breeze, and
//     every character lives whether the train runs or not: the cow's tail, chewing and blinking, the terminal's
//     cursor, the saucer's bob, lights and rising figure, the delay's spreading rings, the grid's running point, the
//     race car's shake and flame, the handset and the crow, the bag and its hit marks, the box car's bounce, the
//     ray gun's flicker and burst, the vocoder's breath, the tumbleweed rolling. Angles are Q10 degrees from a
//     91-entry sine table; rotations are proper rotations about a point (regular matrices, the third column 0, 0,
//     1024 as the renderer needs it); a child's matrix composes with its group's translation.
//   * H3b, the six fixes of the H3 test: the needle turned about the origin and translated to the dial (H3 turned it
//     about the dial as if it were drawn there, and it left the screen); the train at 100 pixels a second with the
//     wheels rolling true; a parked wagon's chassis, wheels and name in the dark step of its knob's colour; the die
//     out of the hint row; the hop 8 frames and 22 pixels high over a slide of half the way per frame, with a landing
//     bounce; the needle's matrix logged once as applied, and a test of the PC's modelling the
//     matrix helpers against the artwork's geometry (the preview cannot catch a module matrix error).
//   * H4: the same effect may ride on several wagons, so the artwork carries every character, and the empty wagon's
//     tumbleweed, four times (the ids of the copies 2 to 4 end in _2, _3, _4, after the dim copy's d) and the
//     dressing hands a type's copies out in position order; every copy's moving parts take the same transforms.
//   * H5: the presets list (build_r2_presets.cpp) shows over the train while the chain says the list is open; the
//     train stays open under it and shows again when the list closes. H5b: the save screen, while a save is shown
//     (op1f_chain_screen_wanted tells which).
//   * H4b: the windmills' sails turn about the artwork's hub, y 110.359 (H3 to H4 turned them about y 100, the far
//     ridge's base, and the sails circled the tower's top); the test checks the hub against the artwork tool's
//     windmill_hub.
// The drawing and the frame: the display thread. The page step: the display thread and the key thread.

#include "build_r4b_card.h"
#ifdef OP1F_HOST
#include "mock_page_r2.h"                                   // the PC's stand-ins for the firmware (for tests this tree does not hold)
#endif
#include "build_h_chain.h"
#include "build_i_lfo4.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);   // 0x8fc25cc0: 0 when optional and missing
typedef void (*ApplyMatrixFn)(void* node, const u32* matrix);               // 0x8c03dbbe
typedef int (*ShiftFn)();                                                   // 0x8fc24ca4: non-zero while shift is held

#ifndef OP1F_HOST
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_APPLY_MATRIX ((ApplyMatrixFn)0x8c03dbbeu)
#define STOCK_SHIFT ((ShiftFn)0x8fc24ca4u)
#define RD32(a) (*(volatile u32*)(a))
#endif

// The pack's own card: fx train's, or lfo's in lfo's copy (tools/pack_build.py writes it from the recipe).
#ifdef OP1F_R2_LFO_COPY
#define OWN_CARD op1f_card_lfo
extern "C" const Op1fCard op1f_card_lfo OP1F_NAMED("op1f_card_lfo");
#else
#define OWN_CARD op1f_card_fx_train
extern "C" const Op1fCard op1f_card_fx_train OP1F_NAMED("op1f_card_fx_train");
#endif

// I6: the LFO train on T4 is this page with other cargo; build_i_lfo4.cpp says what its wagons carry
extern "C" void __attribute__((weak)) op1f_lfotrain_view(Op1fLfoTrainView* out) OP1F_NAMED("op1f_lfotrain_view");
extern "C" int __attribute__((weak)) op1f_lfotrain_open() OP1F_NAMED("op1f_lfotrain_open");
extern "C" int __attribute__((weak)) op1f_presets_available() OP1F_NAMED("op1f_presets_available");   // build_r2_presets.cpp: the presets' list and save documents exist
extern "C" int __attribute__((weak)) op1f_lfopresets_screen_wanted() OP1F_NAMED("op1f_lfopresets_screen_wanted");   // the LFO presets (I8): the save screen's id, the list's id, or 0 for the LFO page or stock
// J5h: the LFOs stand without the pack `fx train`, and the chain unit's three names are optional here: with that unit
// away the page shows the UFOs alone, from the LFO unit's view, and where neither unit is there no document is made.
extern "C" void __attribute__((weak)) op1f_chain_view(int host, Op1fChainView* out) OP1F_NAMED("op1f_chain_view");
extern "C" int __attribute__((weak)) op1f_chain_train_open(int host) OP1F_NAMED("op1f_chain_train_open");
extern "C" int __attribute__((weak)) op1f_chain_screen_wanted(int host) OP1F_NAMED("op1f_chain_screen_wanted");
// R2: what the presets unit of the same pack has made: the list's document for its id, the save screen's for its,
// 0 for anything else and while they are not made (build_r2_presets.cpp). Named by copy.
#ifdef OP1F_R2_LFO_COPY
#define PRESETS_DOCUMENT op1f_lfo_presets_document
extern "C" void* op1f_lfo_presets_document(int id) OP1F_NAMED("op1f_lfo_presets_document");
#else
#define PRESETS_DOCUMENT op1f_fx_presets_document
extern "C" void* op1f_fx_presets_document(int id) OP1F_NAMED("op1f_fx_presets_document");
#endif
#ifdef OP1F_R2_LFO_COPY
// R2: fx train's copy of this unit, there while fx train runs: its document is made, and so are its presets'.
extern "C" int __attribute__((weak)) op1f_fx_train_page_ready() OP1F_NAMED("op1f_fx_train_page_ready");
#endif

static const char LOG_FILE[] = "../src/main/op1f_train.cpp";
static const u32 MODE_GUI = 0x84d21088u;

enum { TRAIN_SCREEN_ID = 77, HOSTS = 2, WAGONS = OP1F_CHAIN_WAGONS, TYPES = 11, LFO_KINDS = 6, CARGOS = TYPES + LFO_KINDS, NONE_INDEX = CARGOS, LABEL_DARK0 = 4, LABEL_RED = 8, LABEL_VARIANTS = 9 };   // a label's variants: the four knob colours, their four dark steps (a parked wagon, H3b), red
// The names and the card's words are CARGOS things: the eleven effects (the firmware's enumeration), then the six LFO kinds (stock's numbering). I6 put the kinds on wagons; since I7 they ride under the UFOs' glass.
enum { COPIES = 4 };                                                                // H4: each character four times, one per wagon that may carry its effect
enum { NODE_DISPLAY = 0x5c, MATRIX_WORDS = 9, Q = 1024, FULL_TURN = 360 * Q };
enum { CARD_FRAMES = 60, PRINT_FRAMES = 232, RUN_HOLD_FRAMES = 30 };
enum { HOP_FRAMES = 12, HOP_ARC_FRAMES = 8, HOP_HEIGHT = 22, BOUNCE_HEIGHT = 5 };   // H3b: the hop's arc of 22 pixels over 8 frames, then a 5-pixel landing bounce over 4 (H3: 18 frames, 18 pixels, no bounce)
enum { RUN_PEAK = 1 << 18 };                                                       // the bus sample above which the instrument counts as sounding
// the artwork's geometry (the artwork tool's), in pixels (and Q10 where marked)
enum { FIRST_X = 118, PITCH = 80, WAGON_W = 70, FLOOR_Y = 144, SIDING_DY = 11, SAG_DY = 4, LOCO_RIGHT = 110, DIAL_X = 70, DIAL_Y = 107 };
enum { WHEEL_Y_Q = 158524 };                                                       // 154.81 (RAIL_Y - SMALL_R)
enum { MILL_X = 330, MILL_HUB_Y_Q = 113008 };                                     // the windmills' hub: WINDMILL_X and ridge_y(330, 10, 480, 120) - 15 = 110.359 in the artwork tool (H4b; H3 to H4 had y 100 and the sails circled the tower)
enum { DRIVER_CY_Q = 154880, DRIVER1_X = 54, DRIVER2_X = 76, SMALL1_X = 32, SMALL2_X = 100, PIN_R_Q = 4608 };   // 151.25; the crank pins at 4.5
enum { GROUND_TILE_Q = 22 * Q, HILL_TILE_Q = 480 * Q, FAR_TILE_Q = 240 * Q };
enum { GROUND_STEP = 1766, HILL_STEP = 963, FAR_STEP = 482 };                        // Q10 pixels per frame at 58 frames per second: 100, 54.5 and 27.3 pixels per second (H3b; H3 ran at 55, 30 and 15)
enum { WHEEL_STEP = 11564, SMALL_STEP = 19506, SAIL_STEP = 794 };                    // Q10 degrees per frame: the drivers (radius 8.75) and the small wheels (5.187) rolling true at the ground speed, 1.82 and 3.07 turns per second (a test of the PC's checks the rates); the mill a turn in 8 s
enum { PH025_STEP = 25423, PH08_STEP = 7945, PH1_STEP = 6355, PH2_STEP = 3178, PH4_STEP = 1589 };   // the characters' cycles of 0.25, 0.8, 1, 2 and 4 seconds

typedef char lfo_kinds_are_the_lfo_units[((int)LFO_KINDS == (int)OP1F_LFO_KINDS && (int)WAGONS == (int)OP1F_LFO_WAGONS) ? 1 : -1];
// The names of what a wagon or a UFO carries are stock's own and are read where stock keeps them: the eleven
// effects' in the table 0x8490e288 and the six LFO kinds' in the table 0x84d256c4, each of eight-byte entries, the
// name's address and the number. This table says where each name's address stands; find_fmt and find_copy read the
// name through it. The ids of this page's picture are made with the same names (the artwork tool). "none" is
// this unit's own word.
#ifndef OP1F_HOST
#define FX_NAME_ENTRY(type) ((const char* const*)(0x8490e288u + 8u * (type)))
#define LFO_NAME_ENTRY(kind) ((const char* const*)(0x84d256c4u + 8u * (kind)))
#endif
static const char* const NONE_NAME = "none";
static const char* const* const NAMES[CARGOS + 1] = { FX_NAME_ENTRY(0), FX_NAME_ENTRY(1), FX_NAME_ENTRY(2), FX_NAME_ENTRY(3), FX_NAME_ENTRY(4), FX_NAME_ENTRY(5),
                                                      FX_NAME_ENTRY(6), FX_NAME_ENTRY(7), FX_NAME_ENTRY(8), FX_NAME_ENTRY(9), FX_NAME_ENTRY(10),
                                                      LFO_NAME_ENTRY(0), LFO_NAME_ENTRY(1), LFO_NAME_ENTRY(2), LFO_NAME_ENTRY(3), LFO_NAME_ENTRY(4), LFO_NAME_ENTRY(5),
                                                      &NONE_NAME };
static const short SIN_Q10[91] = { 0, 18, 36, 54, 71, 89, 107, 125, 143, 160, 178, 195, 213, 230, 248, 265, 282, 299, 316, 333, 350, 367, 384, 400, 416, 433, 449, 465, 481, 496,
                                   512, 527, 543, 558, 573, 587, 602, 616, 630, 644, 658, 672, 685, 698, 711, 724, 737, 749, 761, 773, 784, 796, 807, 818, 828, 839, 849, 859, 868, 878,
                                   887, 896, 904, 912, 920, 928, 935, 943, 949, 956, 962, 968, 974, 979, 984, 989, 994, 998, 1002, 1005, 1008, 1011, 1014, 1016, 1018, 1020, 1022, 1023, 1023, 1024, 1024 };

// ---------------------------------------------------------------- the screen and its nodes

static u32 screen_object[16];                     // R2: a holder, no screen: the second word the document, the rest 0 (the lookup reads that word and no other)
static int screen_built, screen_failed;
static u32 nodes_found, nodes_wanted;
static u32 frames_shown, frames_period, frames_running, print_countdown, card_shows, hops, bypass_frames, lfo_frames;
static u32 last_matrix[9];                        // the matrix rotate_about last applied, and the needle's (H3b: logged once as applied, for the read-out)
static u32 needle_matrix[9];
static int needle_pct, matrices_logged;
static int last_shown_host = -1;
static u32 last_peak;

struct Part { u8* n[COPIES][2]; };                // a moving part in the normal and the dim character, per copy (H4)

static u8* n_chassis[WAGONS];
static u8* n_chassisdim[WAGONS];
static u8* n_chassissq[WAGONS];
static u8* n_wheel[WAGONS][2];
static u8* n_wheeld[WAGONS][2];
static u8* n_car[TYPES][COPIES];                  // the characters, four copies each (H4)
static u8* n_car_none[COPIES];
static u8* n_dim[TYPES][COPIES];
static u8* n_label[CARGOS][LABEL_VARIANTS];        // the four knob colours, their dark steps (a parked wagon's), red
static u8* n_label_none;
static u8* n_siding[WAGONS];
static u8* n_sag[WAGONS];
static u8* n_link[WAGONS];
static u8* n_lamp;
static u8* n_hint[WAGONS];
static u8* n_beam;
static u8* n_smoke;
static u8* n_puff[4];
static u8* n_needle;
static u8* n_needlered;
static u8* n_full;
static u8* n_card;
static u8* n_cardbg[WAGONS];
static u8* n_cardknob[WAGONS];
static u8* n_cardval[CARGOS + 1];
static u8* n_cardlabel;                           // the card's word over the value: fx, or lfo on the LFO train (I6)
static u8* n_cardlabel_lfo;
static int lfo_mode;                              // this frame dresses the LFO page (the UFOs)
static u8* n_ground;
static u8* n_hills;
static u8* n_far;
static u8* n_clouds;
static u8* n_sails[2];
static u8* n_spokes[2];
static u8* n_small[2];
static u8* n_rod;
static u8* n_bell;
static u8* n_arm;
static u8* n_weed[COPIES];
static Part p_tail, p_muzzle, p_eye, p_blink, p_cursor, p_saucer, p_light[5], p_figure, p_ring[3], p_gridpoint, p_carbody, p_flame;
static Part p_handset, p_crowl, p_crowr, p_bag, p_hits, p_boxcar, p_zigzag, p_burst, p_fdot, p_waves;

// the motion's state
static u32 ph_ground, ph_hills, ph_far, ph_wheel, ph_small, ph_sails, ph025, ph08, ph1, ph2, ph4;
static int run_hold;
static i32 ax[WAGONS], ay[WAGONS];               // each knob's eased position, Q10 pixels
static i32 last_target_x[WAGONS];
static int hop[WAGONS];
static int positions_seeded;

static char id_buffer[40];

static char* put(char* p, const char* s) { while (*s) *p++ = *s++; return p; }
static char* put_digit(char* p, int d) { *p++ = (char)('0' + d); return p; }

static u8* find(void* screen, const char* id) {
    ++nodes_wanted;
    u8* n = (u8*)STOCK_FIND_NODE(screen, id, 1);
    if (n) ++nodes_found;
    return n;
}

static u8* find_fmt(void* screen, const char* prefix, const char* const* name, int number, const char* suffix) {    // name: where the name's address stands, or 0
    char* p = put(id_buffer, prefix);
    if (name && *name) p = put(p, *name);
    if (number >= 0) p = put_digit(p, number);
    if (suffix) p = put(p, suffix);
    *p = 0;
    return find(screen, id_buffer);
}

static const char* const COPY_SUFFIX[COPIES] = { "", "_2", "_3", "_4" };          // H4: the id's tail per copy

static u8* find_copy(void* screen, const char* prefix, const char* const* name, int number, const char* suffix, int copy) {
    char* p = put(id_buffer, prefix);
    if (name && *name) p = put(p, *name);
    if (number >= 0) p = put_digit(p, number);
    if (suffix) p = put(p, suffix);
    p = put(p, COPY_SUFFIX[copy]);
    *p = 0;
    return find(screen, id_buffer);
}

static void find_part(void* screen, Part* part, const char* name, int number) {
    for (int c = 0; c < COPIES; ++c) {
        part->n[c][0] = find_copy(screen, name, 0, number, 0, c);
        part->n[c][1] = find_copy(screen, name, 0, number, "d", c);
    }
}

static void show(u8* n, int on) { if (n) n[NODE_DISPLAY] = (u8)(on ? 1 : 0); }
static void show_part(const Part& p, int on) { for (int c = 0; c < COPIES; ++c) { show(p.n[c][0], on); show(p.n[c][1], on); } }

static void apply(u8* n, const u32* m) { if (n) STOCK_APPLY_MATRIX(n, m); }

static void place(u8* n, i32 tx_q, i32 ty_q) {                                     // a translation, Q10 pixels
    u32 m[MATRIX_WORDS] = { Q, 0, 0, 0, Q, 0, (u32)tx_q, (u32)ty_q, Q };
    apply(n, m);
}

static void place_part(const Part& p, i32 tx_q, i32 ty_q) { for (int c = 0; c < COPIES; ++c) { place(p.n[c][0], tx_q, ty_q); place(p.n[c][1], tx_q, ty_q); } }

static void segment(u8* n, i32 x1, i32 y1, i32 x2, i32 y2) {                      // the unit line (0,0)-(1,0) onto a bar, Q10; flat or leaning (never upright)
    u32 m[MATRIX_WORDS] = { (u32)(x2 - x1), (u32)(y2 - y1), 0, 0, Q, 0, (u32)x1, (u32)y1, Q };
    apply(n, m);
}

static u32 step_wrap(u32 ph, u32 step, u32 period) {                              // a phase advanced and folded, no division (the module links no runtime)
    ph += step;
    while (ph >= period) ph -= period;
    return ph;
}

static i32 phase_unit(u32 ph) { return (i32)(((u32)ph * 2912u) >> 20); }         // a Q10-degree phase (0..368639) as 0..1023

static int sin_q(int deg) {                                                        // any angle in degrees, folded, Q10
    while (deg < 0) deg += 360;
    while (deg >= 360) deg -= 360;
    int s = deg < 180 ? 1 : -1;
    if (deg >= 180) deg -= 180;
    int v = deg <= 90 ? SIN_Q10[deg] : SIN_Q10[180 - deg];
    return s * v;
}

static int cos_q(int deg) { return sin_q(deg + 90); }

static int phase_deg(u32 ph) { return (int)(ph >> 10); }

// A proper rotation by `deg` counter-clockwise on the screen about (cx, cy) in Q10 pixels, then a translation by
// (dx, dy) in Q10: x' = c·x + s·y + e, y' = -s·x + c·y + f (regular at every angle, the third column 0, 0, 1024).
// Two conventions, never mixed (H3's needle took one for the other and left the dial): a node drawn in place turns
// about a point of its own geometry (the spokes, the bell, the tail: cx, cy its pivot, dx = dy = 0); a node drawn at
// the origin (the needle, a unit line) turns about the origin and is translated to its place (cx = cy = 0, dx, dy the
// place). A test of the PC's models this function and checks the needle's tip against the dial.
static void rotate_about(u8* n, int deg, i32 cx_q, i32 cy_q, i32 dx_q, i32 dy_q) {
    if (!n) return;
    i32 c = cos_q(deg), s = sin_q(deg);
    i32 e = cx_q - ((c * cx_q + s * cy_q) >> 10) + dx_q;
    i32 f = cy_q - ((-s * cx_q + c * cy_q) >> 10) + dy_q;
    u32 m[MATRIX_WORDS] = { (u32)c, (u32)(-s), 0, (u32)s, (u32)c, 0, (u32)e, (u32)f, Q };
    for (int i = 0; i < MATRIX_WORDS; ++i) last_matrix[i] = m[i];
    STOCK_APPLY_MATRIX(n, m);
}

static void rotate_part(const Part& p, int deg, i32 cx_q, i32 cy_q, i32 dx_q, i32 dy_q) {
    for (int c = 0; c < COPIES; ++c) {
        rotate_about(p.n[c][0], deg, cx_q, cy_q, dx_q, dy_q);
        rotate_about(p.n[c][1], deg, cx_q, cy_q, dx_q, dy_q);
    }
}

static void scale_about(u8* n, i32 sx_q, i32 sy_q, i32 cx_q, i32 cy_q, i32 dx_q, i32 dy_q) {   // a scale about a point, Q10
    if (!n) return;
    i32 e = cx_q - ((sx_q * cx_q) >> 10) + dx_q;
    i32 f = cy_q - ((sy_q * cy_q) >> 10) + dy_q;
    u32 m[MATRIX_WORDS] = { (u32)sx_q, 0, 0, 0, (u32)sy_q, 0, (u32)e, (u32)f, Q };
    STOCK_APPLY_MATRIX(n, m);
}

static void scale_part(const Part& p, i32 sx_q, i32 sy_q, i32 cx_q, i32 cy_q, i32 dx_q, i32 dy_q) {
    for (int c = 0; c < COPIES; ++c) {
        scale_about(p.n[c][0], sx_q, sy_q, cx_q, cy_q, dx_q, dy_q);
        scale_about(p.n[c][1], sx_q, sy_q, cx_q, cy_q, dx_q, dy_q);
    }
}

static void unit_circle(u8* n, i32 r_q, i32 cx_q, i32 cy_q) {                     // the unit circle grown to r and placed
    if (!n) return;
    u32 m[MATRIX_WORDS] = { (u32)r_q, 0, 0, 0, (u32)r_q, 0, (u32)cx_q, (u32)cy_q, Q };
    STOCK_APPLY_MATRIX(n, m);
}

static void find_ufos(void* s);                    // I7, with the UFOs' code below

static void find_nodes() {
    void* s = screen_object;
    static const char* const SUFFIX[LABEL_VARIANTS] = { "_1", "_2", "_3", "_4", "_d1", "_d2", "_d3", "_d4", "_red" };
    for (int k = 0; k < WAGONS; ++k) {
        n_chassis[k] = find_fmt(s, "fxchassis", 0, k + 1, 0);
        n_chassisdim[k] = find_fmt(s, "fxchassisdim", 0, k + 1, 0);
        n_chassissq[k] = find_fmt(s, "fxchassissq", 0, k + 1, 0);
        n_wheel[k][0] = find_fmt(s, "fxwheel", 0, k + 1, "a");
        n_wheel[k][1] = find_fmt(s, "fxwheel", 0, k + 1, "b");
        n_wheeld[k][0] = find_fmt(s, "fxwheeld", 0, k + 1, "a");
        n_wheeld[k][1] = find_fmt(s, "fxwheeld", 0, k + 1, "b");
        n_siding[k] = find_fmt(s, "fxsiding", 0, k + 1, 0);
        n_sag[k] = find_fmt(s, "fxsag", 0, k + 1, 0);
        n_link[k] = find_fmt(s, "fxlink", 0, k + 1, 0);
        n_hint[k] = find_fmt(s, "fxhint", 0, k + 1, 0);
        n_cardbg[k] = find_fmt(s, "fxcardbg", 0, k + 1, 0);
        n_cardknob[k] = find_fmt(s, "fxcardknob", 0, k + 1, 0);
        n_puff[k] = find_fmt(s, "fxpuff", 0, k + 1, 0);
    }
    for (int t = 0; t <= CARGOS; ++t) n_cardval[t] = find_fmt(s, "fxcardval_", NAMES[t], -1, 0);
    n_cardlabel = find(s, "fxcardlabel");
    n_cardlabel_lfo = find(s, "fxcardlabel_lfo");
    for (int t = 0; t < CARGOS; ++t) {
        for (int c = 0; t < TYPES && c < COPIES; ++c) {                              // H4: four copies of each character (the effects': the LFO kinds have names only)
            n_car[t][c] = find_copy(s, "fxcar_", NAMES[t], -1, 0, c);
            n_dim[t][c] = find_copy(s, "fxdim_", NAMES[t], -1, 0, c);
        }
        for (int c = 0; c < LABEL_VARIANTS; ++c) n_label[t][c] = (c == LABEL_RED && t >= TYPES) ? 0 : find_fmt(s, "fxlabel_", NAMES[t], -1, SUFFIX[c]);   // no pick of an LFO kind is refused: no red name
    }
    for (int c = 0; c < COPIES; ++c) {
        n_car_none[c] = find_copy(s, "fxcar_none", 0, -1, 0, c);
        n_weed[c] = find_copy(s, "fxweed", 0, -1, 0, c);
    }
    n_label_none = find(s, "fxlabel_none_dim");
    n_lamp = find(s, "fxlamp");
    n_beam = find(s, "fxbeam");
    n_smoke = find(s, "fxsmoke");
    n_needle = find(s, "fxneedle");
    n_needlered = find(s, "fxneedlered");
    n_full = find(s, "fxfull");
    n_card = find(s, "fxcard");
    n_ground = find(s, "fxground");
    n_hills = find(s, "fxhills");
    n_far = find(s, "fxfar");
    n_clouds = find(s, "fxclouds");
    n_sails[0] = find(s, "fxsails1");
    n_sails[1] = find(s, "fxsails2");
    n_spokes[0] = find(s, "fxspokes1");
    n_spokes[1] = find(s, "fxspokes2");
    n_small[0] = find(s, "fxsmall1");
    n_small[1] = find(s, "fxsmall2");
    n_rod = find(s, "fxrod");
    n_bell = find(s, "fxbell");
    n_arm = find(s, "fxarm");
    find_part(s, &p_tail, "fxtail", -1);
    find_part(s, &p_muzzle, "fxmuzzle", -1);
    find_part(s, &p_eye, "fxeye", -1);
    find_part(s, &p_blink, "fxblink", -1);
    find_part(s, &p_cursor, "fxcursor", -1);
    find_part(s, &p_saucer, "fxsaucer", -1);
    for (int i = 0; i < 5; ++i) find_part(s, &p_light[i], "fxlight", i + 1);
    find_part(s, &p_figure, "fxfigure", -1);
    for (int i = 0; i < 3; ++i) find_part(s, &p_ring[i], "fxring", i + 1);
    find_part(s, &p_gridpoint, "fxgridpoint", -1);
    find_part(s, &p_carbody, "fxcarbody", -1);
    find_part(s, &p_flame, "fxflame", -1);
    find_part(s, &p_handset, "fxhandset", -1);
    find_part(s, &p_crowl, "fxcrowl", -1);
    find_part(s, &p_crowr, "fxcrowr", -1);
    find_part(s, &p_bag, "fxbag", -1);
    find_part(s, &p_hits, "fxhits", -1);
    find_part(s, &p_boxcar, "fxboxcar", -1);
    find_part(s, &p_zigzag, "fxzigzag", -1);
    find_part(s, &p_burst, "fxburst", -1);
    find_part(s, &p_fdot, "fxfdot", -1);
    find_part(s, &p_waves, "fxwaves", -1);
    find_ufos(s);                                                                  // I7
}

// Everything the frame places is hidden until it is placed (the parser has no display attribute: every node parses
// shown, B3f's lesson).
static void hide_wagons() {
    for (int k = 0; k < WAGONS; ++k) {
        show(n_chassis[k], 0); show(n_chassisdim[k], 0); show(n_chassissq[k], 0); show(n_siding[k], 0); show(n_sag[k], 0);
        show(n_link[k], 0); show(n_hint[k], 0);
    }
    for (int c = 0; c < COPIES; ++c) show(n_car_none[c], 0);
    for (int t = 0; t < CARGOS; ++t) {
        for (int c = 0; t < TYPES && c < COPIES; ++c) { show(n_car[t][c], 0); show(n_dim[t][c], 0); }
        for (int c = 0; c < LABEL_VARIANTS; ++c) show(n_label[t][c], 0);
    }
    show(n_label_none, 0); show(n_lamp, 0);
}

static __attribute__((noinline)) void hide_ufos();               // R2: out of line in both copies, as the elder unit has it

static void hide_variants() {
    hide_wagons();
    hide_ufos();
    for (int t = 0; t <= CARGOS; ++t) show(n_cardval[t], 0);
    show(n_cardlabel_lfo, 0);
    show(n_full, 0); show(n_card, 0); show(n_needlered, 0); show(n_smoke, 0); show(n_beam, 0);
    rotate_about(n_needle, 180, 0, 0, DIAL_X * Q, DIAL_Y * Q);                     // drawn at the origin: turned there, then translated to the dial (H3b)
}

static void build_screen() {
    if (screen_built || screen_failed) return;
    for (int i = 0; i < 16; ++i) screen_object[i] = 0;             // R2: a holder, registered nowhere
    void* document = op1f_picture(&OWN_CARD, "fxtrain.svg");       // R2: from the pack's own picture, once (the base keeps the answer)
    if (document == 0) {
        screen_failed = 1;
        STOCK_LOG(1, LOG_FILE, 1, "train art: '%s' could not make its fxtrain.svg, the train stays off (T3 runs stock)", OWN_CARD.name);
        return;
    }
    screen_object[1] = (u32)document;
    find_nodes();
    hide_variants();
    screen_built = 1;
    STOCK_LOG(3, LOG_FILE, 2, "train art: the page's document %08x made from '%s''s fxtrain.svg, %u of %u nodes found", (u32)document, OWN_CARD.name, nodes_found, nodes_wanted);
}

// ---------------------------------------------------------------- the motion

static void advance_phases(int running) {
    if (running) {
        ph_ground = step_wrap(ph_ground, GROUND_STEP, GROUND_TILE_Q);
        ph_hills = step_wrap(ph_hills, HILL_STEP, HILL_TILE_Q);
        ph_far = step_wrap(ph_far, FAR_STEP, FAR_TILE_Q);
        ph_wheel = step_wrap(ph_wheel, WHEEL_STEP, FULL_TURN);
        ph_small = step_wrap(ph_small, SMALL_STEP, FULL_TURN);
    }
    ph_sails = step_wrap(ph_sails, SAIL_STEP, FULL_TURN);
    ph025 = step_wrap(ph025, PH025_STEP, FULL_TURN);
    ph08 = step_wrap(ph08, PH08_STEP, FULL_TURN);
    ph1 = step_wrap(ph1, PH1_STEP, FULL_TURN);
    ph2 = step_wrap(ph2, PH2_STEP, FULL_TURN);
    ph4 = step_wrap(ph4, PH4_STEP, FULL_TURN);
}

static void dress_world(int running) {
    place(n_ground, (i32)ph_ground, 0);
    place(n_hills, (i32)ph_hills, 0);
    place(n_far, (i32)ph_far, 0);
    place(n_clouds, (i32)ph_hills, 0);
    int sails = phase_deg(ph_sails);
    rotate_about(n_sails[0], sails, (MILL_X - 480) * Q, MILL_HUB_Y_Q, 0, 0);        // drawn in place: the hubs at x 330 - 480 (the left tile) and 330, y 110.359 (H4b)
    rotate_about(n_sails[1], sails, MILL_X * Q, MILL_HUB_Y_Q, 0, 0);
    int wheel = phase_deg(ph_wheel), small = phase_deg(ph_small);
    rotate_about(n_spokes[0], wheel, DRIVER1_X * Q, DRIVER_CY_Q, 0, 0);
    rotate_about(n_spokes[1], wheel, DRIVER2_X * Q, DRIVER_CY_Q, 0, 0);
    place(n_rod, ((PIN_R_Q * cos_q(wheel)) >> 10) - PIN_R_Q, -((PIN_R_Q * sin_q(wheel)) >> 10));
    rotate_about(n_small[0], small, SMALL1_X * Q, WHEEL_Y_Q, 0, 0);
    rotate_about(n_small[1], small, SMALL2_X * Q, WHEEL_Y_Q, 0, 0);
    rotate_about(n_bell, (8 * sin_q(phase_deg(ph08))) >> 10, 56 * Q, 100 * Q, 0, 0);
    rotate_about(n_arm, (28 * sin_q(phase_deg(ph1))) >> 10, 104 * Q, 110 * Q + Q / 2, 0, 0);
    show(n_beam, running);
    show(n_smoke, running);
    if (running) {
        for (int k = 0; k < 4; ++k) {                                               // each puff ages from 0 to 1 over four seconds, a quarter apart
            i32 age = phase_unit(step_wrap(ph4, (u32)k * (FULL_TURN / 4), FULL_TURN));   // 0..1023
            i32 px = 47 * Q + 56 * age;                                             // the plume streams further back at the faster speed (H3b; H3: 30 right, 46 up)
            i32 py = 76 * Q - 38 * age;
            i32 r = (i32)(2.4 * Q) + ((i32)(5.6 * Q) * age >> 10);
            unit_circle(n_puff[k], r, px, py);
        }
    }
}

static void dress_characters() {
    int d08 = phase_deg(ph08), d1 = phase_deg(ph1), d2 = phase_deg(ph2), d4 = phase_deg(ph4), d025 = phase_deg(ph025);
    // the cow: the tail swishes, the muzzle chews, the eye blinks at the end of every fourth second
    rotate_part(p_tail, (18 * sin_q(d2)) >> 10, 64 * Q, 116 * Q, 0, 0);
    place_part(p_muzzle, 0, sin_q(d08));
    int blink = d4 > 342;
    show_part(p_eye, !blink);
    show_part(p_blink, blink);
    // the terminal's cursor
    show_part(p_cursor, d1 < 198);
    // the mother's saucer bobs, its lights run, the figure rises in the beam
    place_part(p_saucer, 0, 2 * sin_q(d2));
    int lit = (d1 * 57) >> 12;                                                     // d1 / 72 without a division
    for (int i = 0; i < 5; ++i) show_part(p_light[i], i == lit);
    place_part(p_figure, 0, -3 * (Q + cos_q(d4)));
    // the delay's rings spread like the repeats
    for (int k = 0; k < 3; ++k) {
        i32 u = phase_unit(step_wrap(ph2, (u32)k * (FULL_TURN / 3), FULL_TURN));     // 0..1023
        i32 r = 2 * Q + ((i32)(10.5 * Q) * u >> 10);
        for (int c = 0; c < COPIES; ++c) { unit_circle(p_ring[k].n[c][0], r, 35 * Q, 125 * Q); unit_circle(p_ring[k].n[c][1], r, 35 * Q, 125 * Q); }
    }
    // the grid's point runs the crate's front edges
    {
        static const i32 CX[4] = { 12 * Q, 48 * Q, 60 * Q, 24 * Q };
        static const i32 CY[4] = { 112 * Q, 112 * Q, (i32)(103.6 * Q), (i32)(103.6 * Q) };
        i32 u = (i32)(((u32)ph4 * 5825u) >> 19);                                  // 0..4095 (ph4 / 90 without a division): the corner in the top two bits, the way between in the rest
        int i = (u >> 10) & 3, j = (i + 1) & 3;
        i32 t = u & 1023;
        place_part(p_gridpoint, CX[i] + (((CX[j] - CX[i]) * t) >> 10), CY[i] + (((CY[j] - CY[i]) * t) >> 10));
    }
    // the race car shakes, its flame flickers
    place_part(p_carbody, 0, (frames_shown & 1) ? (i32)(0.6 * Q) : 0);
    scale_part(p_flame, Q + ((sin_q(d025) * Q / 2) >> 10), Q, 6 * Q, 0, 0, 0);
    // the phone's handset swings, the crow turns its head
    rotate_part(p_handset, (7 * sin_q(d2)) >> 10, 56 * Q, 120 * Q, 0, 0);
    int left = d4 < 198;
    show_part(p_crowl, left);
    show_part(p_crowr, !left);
    // the punch bag swings, the hit marks flash at the top of the swing
    rotate_part(p_bag, (9 * sin_q(d1)) >> 10, 35 * Q, 100 * Q, 0, 0);
    show_part(p_hits, sin_q(d1) > 717);
    // the box car bounces on its coils
    rotate_part(p_boxcar, (2 * sin_q(d08 + 108)) >> 10, 35 * Q, 122 * Q, 0, (3 * sin_q(d08)) >> 1);
    // the ray gun's zigzag flickers, its burst turns, the dot blinks
    show_part(p_zigzag, (frames_shown & 7) != 3);
    rotate_part(p_burst, d2 * 2, (i32)(-3.8 * Q), (i32)(94.0 * Q), 0, 0);
    show_part(p_fdot, d1 < 180);
    // the vocoder breathes
    scale_part(p_waves, Q + ((sin_q(d1) * 102) >> 10), Q + ((sin_q(d1) * 102) >> 10), 27 * Q, 108 * Q, 0, 0);
    // the tumbleweed rolls the empty wagon: two turns per crossing, a little hop
    {
        i32 u = phase_unit(ph4);                                                   // 0..1023
        int turn = (int)(((u32)u * 720u) >> 10);
        i32 bounce = (sin_q((u * 540) >> 10) * 2);
        if (bounce < 0) bounce = -bounce;
        for (int c = 0; c < COPIES; ++c) rotate_about(n_weed[c], turn, 8 * Q, (i32)(138.5 * Q), 54 * u, -bounce);
    }
}

// The card after a knob turn: the knob's colour and the value's word (an effect, an LFO kind, none), under the word
// fx or lfo. Counted once a showing (I6 counted at a turn's age of 0 frames, which the LFO unit's age never is).
static int card_counted;

static void dress_card(int turn_knob, int turn_age, int value) {
    int card = turn_knob >= 0 && turn_knob < WAGONS && turn_age < CARD_FRAMES;
    if (card && !card_counted) ++card_shows;
    card_counted = card;
    for (int k = 0; k < WAGONS; ++k) {
        show(n_cardbg[k], card && k == turn_knob);
        show(n_cardknob[k], card && k == turn_knob);
    }
    for (int t = 0; t <= CARGOS; ++t) show(n_cardval[t], card && t == value);
    show(n_cardlabel, !lfo_mode);
    show(n_cardlabel_lfo, lfo_mode);
    show(n_card, card);
}

// ---------------------------------------------------------------- I7: the LFO page, four UFOs
//
// The design since I6: the LFOs run side by side, so no train: four saucers, one per knob, the kind of LFO
// under each glass, on a backdrop of their own. The same page: the train's world and the UFOs' are two sets of
// groups of the one document, each shown by its display bytes, as the flow screen and the duck page share theirs
// since G1b.
//
// A saucer and what rides under its glass are drawn about their own origin (UFO_OX, UFO_OY) and placed by one
// matrix: turned about that point (the tilt) and moved to their place. What moves under the glass is a node of its
// own inside the kind's drawing, moved in the saucer's frame (the renderer composes the two). The names are I6's
// labels, in the labels' own frame. The constants are the artwork tool's (a test holds them together).
enum { UFO_FIRST_X = 72, UFO_PITCH = 112, UFO_HOVER_Y = 92, UFO_OX = 60, UFO_OY = 70, UFO_LAND_DY = 20, UFO_EMPTY_DY = 8, UFO_NAME_DY = 190 - 78, UFO_LABEL_X = 35 };
enum { UFO_ARC = 34, UFO_ARC_FRAMES = 16, UFO_LIGHTS = 7, UFO_STARS = 6, UFO_POOL_Y = 6, UFO_POOL_DY = 3, UFO_WORLD = 6, UFO_LEGS_AT = 870 };
enum { UFO_GYRO_X = 61440, UFO_GYRO_Y = 46592, UFO_PLUG_X = 74035, UFO_PLUG_Y = 39834, UFO_BOAT_X = 61440, UFO_BOAT_Y = 44515, UFO_DIE_X = 70656, UFO_DIE_Y = 52224, UFO_PUCK_TRAVEL = 19497 };   // Q10: ufo_pivots()
enum { UFO_DISH_X = 240, UFO_DISH_Y_Q = 164864, UFO_RANDOM_FRAMES = 35, UFO_SHOOT_EVERY = 350, UFO_SHOOT_FRAMES = 41 };
enum { KIND_TREMOLO = 0, KIND_VALUE = 1, KIND_RANDOM = 2, KIND_ELEMENT = 3, KIND_MIDI = 4, KIND_VELOCITY = 5 };
static const i32 UFO_GROUND_Q[WAGONS] = { 183736, 180614, 180614, 183736 };       // the moon's edge under the four places (ufo_ground_y)
static const i32 UFO_LIGHT_X[UFO_LIGHTS] = { -35328, -23552, -11776, 0, 11776, 23552, 35328 };   // the rim's lights from the hull's middle (UFO_LIGHTS in the tool)
static const i32 UFO_LIGHT_Y[UFO_LIGHTS] = { 4116, 2683, 1823, 1536, 1823, 2683, 4116 };

static u8* n_fxworld[UFO_WORLD];                  // the train's world: hidden for the UFOs
static u8* n_ufoscene;
static u8* n_ufoshoot;
static u8* n_ufodish;
static u8* n_ufoflag[2];
static u8* n_ufotw[UFO_STARS];
static u8* n_ufobeam[2];
static u8* n_ufopool;
static u8* n_ufo[WAGONS][2];                      // a knob's saucer in its colour, and in the colour's dark step
static u8* n_ufolegs[WAGONS][2];
static u8* n_ufoblink[WAGONS];
static u8* n_ufolite[WAGONS][2];
static u8* n_ufoem[LFO_KINDS][COPIES][2];         // a kind under the glass, four copies, lit and dim
static u8* n_ufonone[WAGONS];
static u8* n_ufoarrow[WAGONS];
static Part p_ufonote, p_ufoboat, p_ufodie, p_ufogyro, p_ufoplug, p_ufopuck;
static i32 ufo_x[WAGONS], ufo_land[WAGONS], ufo_kick[WAGONS], ufo_level[WAGONS], ufo_level_to[WAGONS];
static int ufo_place[WAGONS], ufo_arc[WAGONS], ufo_wait[WAGONS];
static int ufo_seeded, scene_is_ufo = -1, ufo_light, ufo_logged;
static u32 ufo_frames, ufo_notes_seen, ufo_lcg = 12345u, ufo_shoot_from, ufo_moves, ufo_landings;
static i32 ufo_puck;

static void find_ufos(void* s) {
    static const char* const WORLD[UFO_WORLD] = { "fxsky", "fxclouds", "fxfar", "fxhills", "fxline", "fxloco" };
    for (int i = 0; i < UFO_WORLD; ++i) n_fxworld[i] = find(s, WORLD[i]);
    n_ufoscene = find(s, "ufoscene");
    n_ufoshoot = find(s, "ufoshoot");
    n_ufodish = find(s, "ufodish");
    n_ufoflag[0] = find(s, "ufoflag1");
    n_ufoflag[1] = find(s, "ufoflag2");
    for (int i = 0; i < UFO_STARS; ++i) n_ufotw[i] = find_fmt(s, "ufotw", 0, i + 1, 0);
    n_ufobeam[0] = find(s, "ufobeama");
    n_ufobeam[1] = find(s, "ufobeamb");
    n_ufopool = find(s, "ufopool");
    for (int k = 0; k < WAGONS; ++k) {
        n_ufo[k][0] = find_fmt(s, "ufo", 0, k + 1, 0);
        n_ufo[k][1] = find_fmt(s, "ufod", 0, k + 1, 0);
        n_ufolegs[k][0] = find_fmt(s, "ufolegs", 0, k + 1, 0);
        n_ufolegs[k][1] = find_fmt(s, "ufolegsd", 0, k + 1, 0);
        n_ufoblink[k] = find_fmt(s, "ufoblink", 0, k + 1, 0);
        n_ufolite[k][0] = find_fmt(s, "ufolite", 0, k + 1, "a");
        n_ufolite[k][1] = find_fmt(s, "ufolite", 0, k + 1, "b");
        n_ufonone[k] = find_fmt(s, "ufonone", 0, k + 1, 0);
        n_ufoarrow[k] = find_fmt(s, "ufoarrow", 0, k + 1, 0);
    }
    for (int kind = 0; kind < LFO_KINDS; ++kind) {
        for (int c = 0; c < COPIES; ++c) {
            n_ufoem[kind][c][0] = find_copy(s, "ufoem_", NAMES[TYPES + kind], -1, 0, c);
            n_ufoem[kind][c][1] = find_copy(s, "ufoed_", NAMES[TYPES + kind], -1, 0, c);
        }
    }
    find_part(s, &p_ufonote, "ufonote", -1);
    find_part(s, &p_ufoboat, "ufoboat", -1);
    find_part(s, &p_ufodie, "ufodie", -1);
    find_part(s, &p_ufogyro, "ufogyro", -1);
    find_part(s, &p_ufoplug, "ufoplug", -1);
    find_part(s, &p_ufopuck, "ufopuck", -1);
}

static void hide_ufos() {
    show(n_ufoscene, 0); show(n_ufoshoot, 0); show(n_ufobeam[0], 0); show(n_ufobeam[1], 0); show(n_ufopool, 0);
    for (int k = 0; k < WAGONS; ++k) { show(n_ufo[k][0], 0); show(n_ufo[k][1], 0); show(n_ufonone[k], 0); show(n_ufoarrow[k], 0); }
    for (int kind = 0; kind < LFO_KINDS; ++kind) {
        for (int c = 0; c < COPIES; ++c) { show(n_ufoem[kind][c][0], 0); show(n_ufoem[kind][c][1], 0); }
        for (int c = 0; c < LABEL_VARIANTS; ++c) show(n_label[TYPES + kind][c], 0);
    }
}

// The one or the other scene: the world's groups by their display bytes, and the other scene's own nodes out.
static void show_scene(int ufo) {
    for (int i = 0; i < UFO_WORLD; ++i) show(n_fxworld[i], !ufo);
    show(n_ufoscene, ufo);
    if (ufo) hide_wagons();
    else hide_ufos();
}

// A part under the glass, in the lit copies only: under a saucer that stands, nothing moves.
static void place_lit(const Part& p, i32 tx_q, i32 ty_q) { for (int c = 0; c < COPIES; ++c) place(p.n[c][0], tx_q, ty_q); }
static void rotate_lit(const Part& p, int deg, i32 cx_q, i32 cy_q, i32 dx_q, i32 dy_q) { for (int c = 0; c < COPIES; ++c) rotate_about(p.n[c][0], deg, cx_q, cy_q, dx_q, dy_q); }

// How a saucer flies is how its kind of LFO moves a sound: the value kind sways, the
// tremolo kind shivers, the random kind jumps from height to height, the element kind tilts, the midi kind steps, the
// velocity kind kicks up at each note and settles. dy in Q10 pixels, the tilt in degrees.
static void ufo_flight(int kind, int k, i32* dy, int* tilt) {
    int d2 = phase_deg(ph2) + 97 * k, d4 = phase_deg(ph4) + 77 * k;
    *dy = 0;
    *tilt = 0;
    if (kind == KIND_VALUE) {
        *dy = 5 * sin_q(d2);
    } else if (kind == KIND_TREMOLO) {
        *dy = ((1638 * sin_q(phase_deg(ph025) + 40 * k)) >> 10) + ((1536 * sin_q(d4)) >> 10);
    } else if (kind == KIND_RANDOM) {
        if (--ufo_wait[k] <= 0) {
            ufo_wait[k] = UFO_RANDOM_FRAMES;
            ufo_lcg = ufo_lcg * 1664525u + 1013904223u;
            ufo_level_to[k] = (i32)((ufo_lcg >> 20) & 0xfffu) * 3 - 6 * Q;       // six pixels either way
        }
        ufo_level[k] += (ufo_level_to[k] - ufo_level[k]) >> 1;
        *dy = ufo_level[k];
    } else if (kind == KIND_ELEMENT) {
        *dy = (1536 * sin_q(d2)) >> 10;
        *tilt = (7 * sin_q(d4)) >> 10;
    } else if (kind == KIND_MIDI) {
        static const i32 STEP[4] = { -5 * Q, -1536, 3 * Q, 0 };
        int d = d4;
        while (d >= 360) d -= 360;
        ufo_level[k] += (STEP[(d * 91) >> 13] - ufo_level[k]) >> 1;                // a step a second (d / 90 without a division)
        *dy = ufo_level[k];
    } else if (kind == KIND_VELOCITY) {
        *dy = -9 * ufo_kick[k];
    }
}

static void dress_ufo(const Op1fLfoTrainView& v) {
    int shift = STOCK_SHIFT() != 0;
    int d08 = phase_deg(ph08), d1 = phase_deg(ph1), d2 = phase_deg(ph2), d4 = phase_deg(ph4);
    ++ufo_frames;
    int note = v.notes != ufo_notes_seen;                                          // a note started: the velocity kind's kick, and its puck
    ufo_notes_seen = v.notes;
    if (note) ufo_puck = Q;
    else ufo_puck -= ufo_puck >> 3;
    if ((ufo_frames & 3u) == 0 && ++ufo_light >= UFO_LIGHTS) ufo_light = 0;       // the rim's light a step every fourth frame
    int shown_em[LFO_KINDS][COPIES][2], shown_name[LFO_KINDS][LABEL_VARIANTS], used[LFO_KINDS];
    for (int kind = 0; kind < LFO_KINDS; ++kind) {
        used[kind] = 0;
        for (int c = 0; c < COPIES; ++c) { shown_em[kind][c][0] = 0; shown_em[kind][c][1] = 0; }
        for (int c = 0; c < LABEL_VARIANTS; ++c) shown_name[kind][c] = 0;
    }
    int beam = 0;
    i32 beam_x = 0, beam_ground = 0;
    for (int p = 0; p < WAGONS; ++p) {
        int k = v.order[p] & 3;
        int kind = v.kind[k];
        int has = kind >= 0 && kind < LFO_KINDS;
        int off = has && !v.on[k];
        int dark = off || !has;
        i32 target_x = (UFO_FIRST_X + UFO_PITCH * p) * Q;
        if (!ufo_seeded) { ufo_x[k] = target_x; ufo_place[k] = p; ufo_land[k] = off ? Q : 0; ufo_arc[k] = 0; }
        if (ufo_place[k] != p) {                                                   // moved: the turned one flies over its neighbour, which slides under it
            if (v.turn_move && v.turn_knob == k && v.turn_age <= 3) { ufo_arc[k] = UFO_ARC_FRAMES; ++ufo_moves; }
            ufo_place[k] = p;
        }
        ufo_x[k] += (target_x - ufo_x[k]) >> 2;
        i32 land_to = off ? Q : 0;
        if (ufo_land[k] != land_to && (ufo_land[k] == 0 || ufo_land[k] == Q)) ++ufo_landings;
        ufo_land[k] += (land_to - ufo_land[k]) >> 3;
        if (land_to - ufo_land[k] < 8 && ufo_land[k] - land_to < 8) ufo_land[k] = land_to;
        if (note && kind == KIND_VELOCITY) ufo_kick[k] = Q;
        else ufo_kick[k] -= ufo_kick[k] >> 3;
        i32 dy = 0;
        int tilt = 0;
        if (has && !off) ufo_flight(kind, k, &dy, &tilt);
        else if (!has) dy = UFO_EMPTY_DY * Q + ((819 * sin_q(d4 + 90 * k)) >> 10);  // an empty one hangs a little lower, nearly still
        i32 fly = Q - ufo_land[k];
        dy = (dy * fly) >> 10;
        tilt = (tilt * fly) >> 10;
        i32 hover = UFO_HOVER_Y * Q + dy;
        i32 stand = UFO_GROUND_Q[p] - UFO_LAND_DY * Q;
        i32 y = hover + (((stand - hover) * ufo_land[k]) >> 10);
        if (ufo_arc[k] > 0) {                                                      // the arc of a move: a half turn of the sine in sixteen frames
            y -= UFO_ARC * sin_q(((UFO_ARC_FRAMES - ufo_arc[k]) * 45) >> 2);
            --ufo_arc[k];
        }
        i32 x = ufo_x[k];
        int variant = dark ? 1 : 0;
        rotate_about(n_ufo[k][variant], tilt, UFO_OX * Q, UFO_OY * Q, x - UFO_OX * Q, y - UFO_OY * Q);
        if (!ufo_logged && k == 0) {                                               // the first saucer's matrix as applied, once, for the read-out
            ufo_logged = 1;
            const u32* m = last_matrix;
            STOCK_LOG(3, LOG_FILE, 6, "ufo matrices: knob 0 at (%d, %d) x1024, tilt %d = [%d %d %d %d %d %d %d %d %d]; its own middle (%d, %d)",
                      x, y, tilt, (i32)m[0], (i32)m[1], (i32)m[2], (i32)m[3], (i32)m[4], (i32)m[5], (i32)m[6], (i32)m[7], (i32)m[8], (int)UFO_OX, (int)UFO_OY);
        }
        show(n_ufo[k][variant], 1);
        show(n_ufo[k][1 - variant], 0);
        show(n_ufolegs[k][variant], ufo_land[k] > UFO_LEGS_AT);
        if (!dark) {                                                               // its lights: the antenna's, and two that run round the rim
            int i = ufo_light + 2 * k;
            while (i >= UFO_LIGHTS) i -= UFO_LIGHTS;
            int j = i == 0 ? UFO_LIGHTS - 1 : i - 1;
            place(n_ufolite[k][0], UFO_LIGHT_X[i], UFO_LIGHT_Y[i]);
            place(n_ufolite[k][1], UFO_LIGHT_X[j], UFO_LIGHT_Y[j]);
            show(n_ufoblink[k], sin_q(d1 + 90 * k) > 300);
        }
        if (has) {                                                                 // the kind under the glass, with the saucer's own matrix; its name in the knob's colour
            int c = used[kind] < COPIES ? used[kind]++ : COPIES - 1;
            rotate_about(n_ufoem[kind][c][off], tilt, UFO_OX * Q, UFO_OY * Q, x - UFO_OX * Q, y - UFO_OY * Q);
            shown_em[kind][c][off] = 1;
            int colour = off ? LABEL_DARK0 + k : k;
            place(n_label[TYPES + kind][colour], x - UFO_LABEL_X * Q, UFO_NAME_DY * Q);
            shown_name[kind][colour] = 1;
        } else {
            place(n_ufonone[k], x - UFO_LABEL_X * Q, UFO_NAME_DY * Q);
        }
        show(n_ufonone[k], !has);
        place(n_ufoarrow[k], x - UFO_LABEL_X * Q, UFO_NAME_DY * Q);
        show(n_ufoarrow[k], shift);
        if (k == v.lamp && has && !off && ufo_arc[k] == 0 && ufo_land[k] == 0) {   // the searchlight: under the LFO whose page T4 opens, while it hovers in its place
            beam = 1;
            beam_x = x;
            beam_ground = UFO_GROUND_Q[p];
        }
    }
    ufo_seeded = 1;
    for (int kind = 0; kind < LFO_KINDS; ++kind) {
        for (int c = 0; c < COPIES; ++c) { show(n_ufoem[kind][c][0], shown_em[kind][c][0]); show(n_ufoem[kind][c][1], shown_em[kind][c][1]); }
        for (int c = 0; c < LABEL_VARIANTS; ++c) show(n_label[TYPES + kind][c], shown_name[kind][c]);
    }
    int phase = (ufo_frames >> 3) & 1;                                             // the beam's dashes crawl: two phases by turns
    if (beam) {
        place(n_ufobeam[phase], beam_x - UFO_OX * Q, 0);
        place(n_ufopool, beam_x - UFO_OX * Q, beam_ground + UFO_POOL_DY * Q - UFO_POOL_Y * Q);
    }
    show(n_ufobeam[phase], beam);
    show(n_ufobeam[1 - phase], 0);
    show(n_ufopool, beam);
    // what moves under the glass
    place_lit(p_ufonote, 0, (2560 * sin_q(d08)) >> 10);                             // the note bobs
    rotate_lit(p_ufoboat, (8 * sin_q(d2)) >> 10, UFO_BOAT_X, UFO_BOAT_Y, 0, (1536 * sin_q(d2 + 90)) >> 10);   // the boat rocks and rides
    {
        i32 u = phase_unit(ph1);                                                   // the die: a throw a second
        rotate_lit(p_ufodie, phase_deg(ph1), UFO_DIE_X, UFO_DIE_Y, 0, -9 * ((4 * u * (1023 - u)) >> 10));
    }
    rotate_lit(p_ufogyro, (14 * sin_q(d4)) >> 10, UFO_GYRO_X, UFO_GYRO_Y, 0, 0);    // the gimbal leans
    rotate_lit(p_ufoplug, (9 * sin_q(d2)) >> 10, UFO_PLUG_X, UFO_PLUG_Y, 0, 0);     // the plug swings
    place_lit(p_ufopuck, 0, -((UFO_PUCK_TRAVEL * ufo_puck) >> 10));                 // the puck: up at a note, down again
    // the backdrop: stars that twinkle, the dish, the flag, a shooting star now and then
    for (int i = 0; i < UFO_STARS; ++i) show(n_ufotw[i], sin_q(2 * d4 + 61 * i) > 700);
    rotate_about(n_ufodish, (28 * sin_q(d4)) >> 10, UFO_DISH_X * Q, UFO_DISH_Y_Q, 0, 0);
    show(n_ufoflag[0], d1 < 180);
    show(n_ufoflag[1], d1 >= 180);
    u32 since = ufo_frames - ufo_shoot_from;
    if (since >= (u32)UFO_SHOOT_EVERY) { ufo_shoot_from = ufo_frames; since = 0; }
    if (since < (u32)UFO_SHOOT_FRAMES) place(n_ufoshoot, 130 * Q + (i32)since * 2997, 10 * Q + (i32)since * 749);
    show(n_ufoshoot, since < (u32)UFO_SHOOT_FRAMES);
    dress_card(v.turn_move ? -1 : v.turn_knob, v.turn_age, v.turn_knob >= 0 && v.turn_knob < WAGONS && v.kind[v.turn_knob] >= 0 && v.kind[v.turn_knob] < LFO_KINDS ? TYPES + v.kind[v.turn_knob] : NONE_INDEX);
}

// ---------------------------------------------------------------- the page, dressed from the chain

static void dress(int host, const Op1fChainView& v, int running) {
    int shift = STOCK_SHIFT() != 0;
    int shown_label[CARGOS][LABEL_VARIANTS];
    for (int t = 0; t < CARGOS; ++t) for (int c = 0; c < LABEL_VARIANTS; ++c) shown_label[t][c] = 0;
    int shown_car[CARGOS][COPIES], shown_dim[CARGOS][COPIES], shown_none[COPIES], used[CARGOS], used_none = 0;   // H4: a type's copies handed out in position order
    for (int t = 0; t < CARGOS; ++t) { used[t] = 0; for (int c = 0; c < COPIES; ++c) { shown_car[t][c] = 0; shown_dim[t][c] = 0; } }
    for (int c = 0; c < COPIES; ++c) shown_none[c] = 0;
    int shown_none_label = 0, links = 0, lamp_placed = 0;
    if (!positions_seeded) {                                                       // the first frame starts the wagons where they stand
        for (int p = 0; p < WAGONS; ++p) { int k = v.order[p] & 3; ax[k] = (FIRST_X + PITCH * p) * Q; ay[k] = 0; last_target_x[k] = FIRST_X + PITCH * p; }
        positions_seeded = 1;
    }
    i32 prev_right = LOCO_RIGHT * Q, prev_y = (FLOOR_Y * Q) + (5 * Q) / 2;
    int wheel_deg = phase_deg(ph_small);
    for (int p = 0; p < WAGONS; ++p) {
        int k = v.order[p] & 3;
        int t = v.type[k];
        int has = t >= 0 && t < TYPES;
        int off = has && !v.coupled[k];
        int refused = v.full && v.refused_knob == k && v.refused_type >= 0 && v.refused_type < TYPES;
        i32 target_x = FIRST_X + PITCH * p;
        i32 target_y = off ? SIDING_DY : (refused ? SAG_DY : 0);
        if (last_target_x[k] != target_x) {                                        // a move: the turned wagon hops, the other slides
            if (v.turn_move && v.turn_knob == k && v.turn_age <= 2) { hop[k] = HOP_FRAMES; ++hops; }
            last_target_x[k] = target_x;
        }
        ax[k] += (target_x * Q - ax[k]) >> 1;                                       // the slide: half the way per frame (H3b; H3 eased five sixteenths)
        ay[k] += (target_y * Q - ay[k]) >> 1;
        i32 hop_q = 0;
        if (hop[k] > 0) {                                                           // the hop (H3b): an arc of 22 pixels over 8 frames, then a 5-pixel landing bounce over 4
            int i = HOP_FRAMES - hop[k];
            if (i < HOP_ARC_FRAMES) hop_q = -HOP_HEIGHT * sin_q((i * 45) >> 1);     // 22.5 degrees per frame, the half turn in 8
            else hop_q = -BOUNCE_HEIGHT * sin_q((i - HOP_ARC_FRAMES) * 45);          // 45 degrees per frame, the half turn in 4
            --hop[k];
        }
        i32 x = ax[k], y = ay[k] + hop_q;
        // the chassis: normal, dim on the siding, sketched with squashed wheels under a refused pick
        place(n_chassis[k], x, y);
        place(n_chassisdim[k], x, y);
        place(n_chassissq[k], x, y);
        show(n_chassis[k], !off && !refused);
        show(n_chassisdim[k], off);
        show(n_chassissq[k], refused && !off);
        for (int w = 0; w < 2; ++w) {                                              // the wheels turn with the train (local centres)
            i32 cx_q = (w == 0 ? 14 : WAGON_W - 14) * Q;
            rotate_about(n_wheel[k][w], wheel_deg, cx_q, WHEEL_Y_Q, 0, 0);
            rotate_about(n_wheeld[k][w], wheel_deg, cx_q, WHEEL_Y_Q, 0, 0);
        }
        // the character: the effect's, dim on the siding, the tumbleweed on an empty wagon
        if (has) {
            int c = used[t] < COPIES ? used[t]++ : COPIES - 1;
            if (off) { place(n_dim[t][c], x, y); shown_dim[t][c] = 1; }
            else { place(n_car[t][c], x, y); shown_car[t][c] = 1; }
        } else {
            int c = used_none < COPIES ? used_none++ : COPIES - 1;
            place(n_car_none[c], x, y);
            shown_none[c] = 1;
        }
        // the label: the knob's colour, its dark step on the siding (H3b), the refused effect's name in red during the flash
        if (refused) {
            place(n_label[v.refused_type][LABEL_RED], x, hop_q);
            shown_label[v.refused_type][LABEL_RED] = 1;
        } else if (has) {
            int colour = off ? LABEL_DARK0 + k : k;
            place(n_label[t][colour], x, hop_q);
            shown_label[t][colour] = 1;
        } else {
            place(n_label_none, x, hop_q);
            shown_none_label = 1;
        }
        show(n_siding[p], off);
        show(n_sag[p], refused && !off);
        if (!off) {                                                                // the link from the wagon before (the locomotive first), spanning the sidings
            i32 this_y = (FLOOR_Y * Q) + (5 * Q) / 2 + ay[k];
            segment(n_link[links], prev_right + Q + Q / 2, prev_y, x - Q - Q / 2, this_y);
            show(n_link[links], 1);
            ++links;
            prev_right = x + WAGON_W * Q;
            prev_y = this_y;
        }
        if (k == v.mirror) { place(n_lamp, x, y); lamp_placed = 1; }
        place(n_hint[k], x, 0);
        show(n_hint[k], shift);
    }
    for (int i = links; i < WAGONS; ++i) show(n_link[i], 0);
    for (int c = 0; c < COPIES; ++c) show(n_car_none[c], shown_none[c]);
    for (int t = 0; t < CARGOS; ++t) {
        for (int c = 0; t < TYPES && c < COPIES; ++c) { show(n_car[t][c], shown_car[t][c]); show(n_dim[t][c], shown_dim[t][c]); }
        for (int c = 0; c < LABEL_VARIANTS; ++c) show(n_label[t][c], shown_label[t][c]);
    }
    show(n_label_none, shown_none_label);
    show(n_lamp, lamp_placed);
    // the dial: the needle from the stop at the left (0 %) to the stop at the right (100 % of the ceiling)
    int pct = v.cost_percent < 0 ? 0 : (v.cost_percent > 100 ? 100 : v.cost_percent);
    int deg = 180 - ((pct * 1843 + 512) >> 10);                                     // 1.8 degrees per percent, no division (the module links no runtime)
    rotate_about(n_needle, deg, 0, 0, DIAL_X * Q, DIAL_Y * Q);                      // drawn at the origin: turned about it, then translated to the dial (H3b; H3 turned it about the dial and it left the screen)
    for (int i = 0; i < MATRIX_WORDS; ++i) needle_matrix[i] = last_matrix[i];
    needle_pct = pct;
    rotate_about(n_needlered, v.full ? 0 : deg, 0, 0, DIAL_X * Q, DIAL_Y * Q);
    show(n_needle, !v.full);
    show(n_needlered, v.full);
    show(n_full, v.full);
    dress_card(v.turn_knob, v.turn_age, v.turn_knob >= 0 && v.turn_knob < WAGONS && v.type[v.turn_knob] >= 0 && v.type[v.turn_knob] < TYPES ? v.type[v.turn_knob] : NONE_INDEX);
    dress_world(running);
    dress_characters();
}

// ---------------------------------------------------------------- the frame, the page

// The train's document is made (T3 and T4 open a page only then). In lfo's copy while fx train runs, fx train's
// answer: the page is that pack's.
extern "C" int op1f_train_available() OP1F_NAMED("op1f_train_available");
extern "C" int op1f_train_available() {
#ifdef OP1F_R2_LFO_COPY
    if (&op1f_chain_view != 0) return &op1f_fx_train_page_ready != 0 ? op1f_fx_train_page_ready() : 0;
#endif
    return screen_built;
}

#ifndef OP1F_R2_LFO_COPY
// R2: for lfo's copy, which answers by it while this pack runs.
extern "C" int op1f_fx_train_page_ready() OP1F_NAMED("op1f_fx_train_page_ready");
extern "C" int op1f_fx_train_page_ready() { return screen_built; }
#endif

// R2: the document the page shows for the host now, 0 for none (the mode's own screen), in the order the
// controllers' screen answers had (H5, H5b, I6, I8): the presets' save screen or list over the train, the LFO
// presets' over the UFOs, the UFOs, the train.
static void* page_wanted(int host) {
    if (host == 0) {
        if (&op1f_presets_available != 0 && op1f_presets_available() && &op1f_chain_screen_wanted != 0) { int id = op1f_chain_screen_wanted(0); if (id) return PRESETS_DOCUMENT(id); }   // H5: the list over the train; H5b: the save screen over both
        if (&op1f_presets_available != 0 && op1f_presets_available() && &op1f_lfopresets_screen_wanted != 0) { int id = op1f_lfopresets_screen_wanted(); if (id) return PRESETS_DOCUMENT(id); }   // I8: the same two for the LFO presets, over the LFO page
        if (screen_built && &op1f_lfotrain_open != 0 && op1f_lfotrain_open()) return (void*)screen_object[1];   // I6: the LFO train on T4 is the same page with other cargo
        if (screen_built && &op1f_chain_train_open != 0 && op1f_chain_train_open(0)) return (void*)screen_object[1];
        return 0;
    }
    if (host == 1) {
        if (&op1f_presets_available != 0 && op1f_presets_available() && &op1f_chain_screen_wanted != 0) { int id = op1f_chain_screen_wanted(1); if (id) return PRESETS_DOCUMENT(id); }
        if (screen_built && &op1f_chain_train_open != 0 && op1f_chain_train_open(1)) return (void*)screen_object[1];
        return 0;
    }
    return 0;
}

// R2: which of the two is open for the host: 2 the UFOs, 1 the train, 0 neither. A change to one of them is a page
// opened by a key (T3, T4), which comes over another pack's page as it always did.
static u32 page_view(int host) {
    if (host == 0 && &op1f_lfotrain_open != 0 && op1f_lfotrain_open()) return 2u;
    if ((host == 0 || host == 1) && &op1f_chain_train_open != 0 && op1f_chain_train_open(host)) return 1u;
    return 0u;
}

// R2: the page step's words, shared by the display thread and the key thread: single words, each written whole.
static void* volatile page_asked;                  // the document last asked of the base, 0 for none (or closed)
static volatile u32 page_view_was;                 // page_view at the last step
static volatile u32 page_other;                    // the frame's word: another pack's page lies on top
static volatile u32 page_steps, page_shows, page_closes, page_waits, page_refused;   // since the start, for the frame's line
static void* volatile page_refused_doc;            // the document the base last refused, 0 once another was taken
static volatile u32 page_flags = OP1F_PAGE_STAYS;  // R4b: the flag the page is asked with, 0 once a base answered 0 to it
static volatile u32 page_unflagged, page_away;     // R4b: asks a base took only without the flag; frames the step waited out of its mode
static u32 page_said[7], page_countdown;
// The heals (the display thread's alone): what the frame's `page` says against what the step asked.
static u32 heal_closes, heal_shows;                // since the start, for the frame's line
static u32 heal_stale_shown, heal_stale_missing;   // frames in a row a heal's state was seen
static u32 heal_round;                             // heals made for the page shown or the document wanted now
static void* heal_for;                             // the document wanted when heal_round counted
enum { HEAL_SHOWN_FRAMES = 2, HEAL_MISSING_FRAMES = 4, HEAL_MOST = 3 };

// The PC's tests run the other thread's step between the word and the call here; nothing on the unit.
#ifndef PAGE_SEAM
#define PAGE_SEAM() ((void)0)
#endif

// R4b: the page asked of the base with the flag OP1F_PAGE_STAYS, and asked again at once with none when a base
// answers 0 to it (a base before 26 knows no flag); from then on with none. 1 when the base took the request.
static int page_ask(void* want) {
    u32 flags = page_flags;
    if (op1f_page_show(&OWN_CARD, want, flags)) return 1;
    if (flags == 0 || !op1f_page_show(&OWN_CARD, want, 0)) return 0;
    page_flags = 0;
    ++page_unflagged;
    return 1;
}

// R2: the page asked of the base or closed, only when what is wanted changed. From the frame and from a key.
static __attribute__((noinline)) void page_step(int host) {
    ++page_steps;
    void* want = page_wanted(host);
    u32 view = page_view(host), was = page_view_was;
    page_view_was = view;
    void* asked = page_asked;
    if (want == 0) {
        if (asked == 0) return;
        page_asked = 0;
        PAGE_SEAM();
        ++page_closes;
        op1f_page_close(&OWN_CARD);                                                // closed also from under another pack's page
        return;
    }
    int over = view != 0 && view != was && page_other != 0;                        // the train or the UFOs opened: over the other pack's page
    if (want == asked && !over) return;
    if (page_other != 0 && !over) { ++page_waits; return; }                         // another pack's page on top: shown when it closes
    page_asked = want;
    PAGE_SEAM();
    if (page_ask(want)) { ++page_shows; page_refused_doc = 0; }                  // R4b: with the flag, or without
    else { ++page_refused; page_refused_doc = want; }                              // not asked again for the same document
}

// R2: the frame's step after the page step (the display thread). The two threads' steps each write the word before
// they call the base, so where one runs between the other's word and its call, the base can be left showing a page
// nothing wants, or not showing one the word says was asked. The frame's `page` says what the GUI shows, so:
//   shown, not wanted: nothing is wanted for the mode and the frame names this pack's page HEAL_SHOWN_FRAMES frames
//     in a row (a close, from a key or the frame, is taken at the base's next frame, before the page is named): the
//     page closed again;
//   wanted, not shown: the word holds what is wanted, the frame names no page of any pack and the screen is not the
//     base's (its own top page names none) HEAL_MISSING_FRAMES frames in a row (a request is taken at the base's next
//     frame and named from the frame after, two frames at most): asked again, unless the base refused that document.
//     Only while the disk mode is closed and the frame's mode is the host the step works for: a page the base closed
//     for a mode change is not asked again in the new mode.
// At most HEAL_MOST heals for one page shown or one document wanted, so a base that will not is not asked for ever.
static void page_heal(const Op1fFrame* now, int host) {
    void* want = page_wanted(host);
    void* asked = page_asked;
    if (want != heal_for) { heal_for = want; heal_round = 0; }
    if (want == 0 && asked == 0 && now->page == &OWN_CARD) {
        if (++heal_stale_shown >= (u32)HEAL_SHOWN_FRAMES && heal_round < (u32)HEAL_MOST) {
            heal_stale_shown = 0;
            ++heal_round;
            ++heal_closes;
            op1f_page_close(&OWN_CARD);
        }
    } else {
        heal_stale_shown = 0;
    }
    u32 disk = now->bytes >= 36u ? now->disk : 0u;                                // the disk mode open: the computer's screen, no page of ours asked
    int same_mode = host >= 0 && now->mode == (u32)host;                           // the base's mode word and the one the step works from agree
    if (want != 0 && want == asked && now->page == 0 && now->screen != (u32)TRAIN_SCREEN_ID && page_other == 0 && want != page_refused_doc &&
        disk == 0 && same_mode) {
        if (++heal_stale_missing >= (u32)HEAL_MISSING_FRAMES && heal_round < (u32)HEAL_MOST) {
            heal_stale_missing = 0;
            ++heal_round;
            ++heal_shows;
            if (!page_ask(want)) page_refused_doc = want;                       // R4b: with the flag, or without
        }
    } else {
        heal_stale_missing = 0;
        if (now->page == &OWN_CARD && want != 0) heal_round = 0;                    // shown: a later stuck page has its heals again
    }
}

#ifdef OP1F_R2_LFO_COPY
#define PAGE_FRAME op1f_lfo_page_frame
extern "C" void op1f_lfo_page_frame(const Op1fFrame* now) OP1F_NAMED("op1f_lfo_page_frame");
#else
#define PAGE_FRAME op1f_train_page_frame
extern "C" void op1f_train_page_frame(const Op1fFrame* now) OP1F_NAMED("op1f_train_page_frame");

// R2: the page step after a key, from the chain's key handlers and the LFO presets' (the firmware's key thread).
// Writes no line.
extern "C" void op1f_train_page_now(int host) OP1F_NAMED("op1f_train_page_now");
extern "C" void op1f_train_page_now(int host) { page_step(host); }
#endif

// R2: once a frame from the pack's frame function, after the page is dressed: whether another pack's page is on top,
// the page step, and a line when its counts moved (at most every PRINT_FRAMES frames).
extern "C" void PAGE_FRAME(const Op1fFrame* now) {
#ifdef OP1F_R2_LFO_COPY
    if (&op1f_chain_view != 0) return;                                             // fx train runs: the page is its
#endif
    page_other = now->page != 0 && now->page != &OWN_CARD ? 1u : 0u;
    int mode = (int)RD32(MODE_GUI);
    int host = mode == 0 || mode == 1 ? mode : -1;
    if (host >= 0 && now->mode != (u32)host) {                                     // R4c: another mode: the step waits for the instrument's
        if (page_flags == 0) page_asked = 0;                                       // R4b: a base that refused the flag closed the page: asked again at the first frame back
        ++page_away;
    } else {
        page_step(host);
    }
    page_heal(now, host);
    if (++page_countdown < (u32)PRINT_FRAMES) return;
    page_countdown = 0;
    u32 now_counts[7] = { page_steps, page_shows, page_closes, page_waits, page_refused, heal_shows, heal_closes };
    int moved = 0;
    for (int i = 1; i < 7; ++i) if (now_counts[i] != page_said[i]) moved = 1;
    if (!moved) return;
    for (int i = 0; i < 7; ++i) page_said[i] = now_counts[i];
    STOCK_LOG(3, LOG_FILE, 7, "train page: '%s' %u shown, %u closed, %u waited under another pack's page, %u refused by the base, %u asked again and %u closed again out of step; %u steps since the start",
              OWN_CARD.name, now_counts[1], now_counts[2], now_counts[3], now_counts[4], now_counts[5], now_counts[6], now_counts[0]);
}

extern "C" void op1f_train_frame() OP1F_NAMED("op1f_train_frame");
extern "C" void op1f_train_frame() {
#ifdef OP1F_R2_LFO_COPY
    if (&op1f_chain_view != 0) return;                                             // R2: fx train runs: its copy makes and dresses the page
#endif
    int chain = &op1f_chain_view != 0;                                             // J5h: the effect train's unit is there (the pack fx train runs)
    if (!chain && &op1f_lfotrain_view == 0) return;                                // neither the train nor the LFOs: no screen is made, nothing is said
    build_screen();
    int mode = (int)RD32(MODE_GUI);
    int host = (mode == 0 || mode == 1) ? mode : -1;
    Op1fChainView view;
    view.known = 0; view.open = 0; view.bypassed = 0; view.peak = 0; view.cost_percent = 0;   // what is read of it below when no unit fills it
    int have_view = 0;
    if (host >= 0 && chain) {
        op1f_chain_view(host, &view);
        have_view = view.known;
        if (view.peak > last_peak) last_peak = view.peak;
        if (view.peak >= (u32)RUN_PEAK) run_hold = RUN_HOLD_FRAMES;
        else if (run_hold > 0) --run_hold;
    } else if (run_hold > 0) {
        --run_hold;
    }
    int running = run_hold > 0;
    lfo_mode = 0;
    Op1fLfoTrainView lv;
    lv.open = 0;
    if (&op1f_lfotrain_view != 0) op1f_lfotrain_view(&lv);                         // I7: the LFO page, the UFOs (I6 dressed the train's wagons from this view)
    lfo_mode = host == 0 && (have_view || !chain) && lv.open != 0;                 // J5h: without the train's unit the LFO unit's view alone says so
    if (screen_built && host >= 0 && ((have_view && view.open) || lfo_mode)) {     // with the train's unit there: have_view && (view.open || lfo_mode), as before
        if (scene_is_ufo != lfo_mode) {                                            // the other scene: its world shown, the first one's nodes out
            show_scene(lfo_mode);
            scene_is_ufo = lfo_mode;
        }
        advance_phases(running);
        if (lfo_mode) { dress_ufo(lv); ++lfo_frames; }
        else dress(host, view, running);
        if (!matrices_logged && !lfo_mode) {                                                    // H3b: the needle's matrix as applied, once, for the read-out (its tip from the words: x2 = 4 pixels)
            matrices_logged = 1;
            const u32* m = needle_matrix;
            i32 tip_x = (((i32)m[0] * 4 + (i32)m[6]) * 100) >> 10, tip_y = (((i32)m[1] * 4 + (i32)m[7]) * 100) >> 10;
            STOCK_LOG(3, LOG_FILE, 5, "train matrices: needle at %d%% = [%d %d %d %d %d %d %d %d %d], tip x100 (%d, %d), the dial at (%d, %d)",
                      needle_pct, (i32)m[0], (i32)m[1], (i32)m[2], (i32)m[3], (i32)m[4], (i32)m[5], (i32)m[6], (i32)m[7], (i32)m[8], tip_x, tip_y, (int)DIAL_X, (int)DIAL_Y);
        }
        ++frames_shown;
        ++frames_period;
        if (running) ++frames_running;
        if (view.bypassed) ++bypass_frames;
        last_shown_host = host;
    } else {
        positions_seeded = 0;                                                      // the next opening places the wagons where they stand
        ufo_seeded = 0;
        for (int k = 0; k < WAGONS; ++k) hop[k] = 0;
    }
    if (++print_countdown >= (u32)PRINT_FRAMES) {
        print_countdown = 0;
        if (frames_period) {
            STOCK_LOG(3, LOG_FILE, 4, "train: %u frames shown this period (%u in all, %u running, %u as the lfo page), host %d, cost %d%%, peak %u, %u card shows, %u hops, %u ufo moves, %u landings and take-offs",
                      frames_period, frames_shown, frames_running, lfo_frames, last_shown_host, have_view ? view.cost_percent : -1, last_peak, card_shows, hops, ufo_moves, ufo_landings);
            frames_period = 0;
            last_peak = 0;
        }
    }
}
