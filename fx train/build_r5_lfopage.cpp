// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R5 (base 28), fx train 14: the LFO pages of Build I's LFO page unit (which stays as it was for the packs
// before), with one change: where the LFO turns a wagon that carries another pack's effect (not the first wagon, whose
// ride the base names), the name over a knob is that effect's own name of the value (the chain unit's
// op1f_chain_rack_value), and nothing past its values. Everything else is the elder's; its description holds.
//
// Build I images I2 to I4: the stock LFO pages whose destination box can stand on an effect wagon.
//
// The pages are stock's own, singletons the GUI registers by id and draws from their artwork: the value LFO's
// screen 32 (0x84c9a708, built by 0x8fc591f0 from singlelfo.svg, the render 0x8fc599d4; I2), since I3 the random
// LFO's 33 (rndlfo.svg), the element LFO's 34 (reroutelfo.svg) and the velocity LFO's 36 (velocitylfo.svg), and
// since I4 the midi LFO's 35 (midilfo.svg, the render 0x8fc568f8): the controllers' page table maps the slot's LFO
// type to the screen 31 + type (0x8fc25af4). Two things are added to each, and nothing of it is replaced:
//
//   * A marker under the destination's box says which effect wagon the LFO's effect destination turns: a small
//     wagon in that wagon's knob colour. The four markers are groups of the module's copy of the page's artwork
//     (the artwork tool appends them to stock's file; build_b_art serves the copy to the SVG loader). The
//     OS's parser has no display attribute, so the module hides the four when it first finds them, and shows one
//     while the page is up, the slot's LFO is the page's type and its destination is the effect. The midi page has
//     four destination boxes, the continuous controllers 1 to 4, each with four markers and a wagon of its own.
//
//   * The parameter's name over the knob picture. Each page's render asks 0x8d51750c(kind, index) for it (the
//     value page at 0x8fc5bbf6, the element page at 0x8fc59468, the random page at 0x8fc5a6e8, the velocity page
//     at 0x8fc5cd6c; the kind is the destination's class, 2 the effect, as each page works it out from the
//     destination's value); for the effect that function reads the slot's effect type (0x11904794 / 0x119047b4 +
//     4 x slot) and returns the word at 0x840af848 + 16 x type + 4 x index. The four CALL.a are retargeted to
//     op1f_lfo_param_name, which answers the same word for the effect type of the wagon the LFO turns when that is
//     not the wagon under the lamp, a dash when that wagon is empty, and stock's function for everything else.
//     The midi page asks for no name: it shows the parameter as a knob picture alone.
//
// GUI thread only: the frame call comes from the chain unit's frame, the name call from the pages' renders.
#include "build_h_chain.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);
typedef u32* (*ScreensFn)();
typedef const char* (*ParamNameFn)(int kind, int index);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_SCREENS ((ScreensFn)0x8fc261fcu)                 // the registry: 86 screen pointers by id (0x84c960f4)
#define STOCK_PARAM_NAME ((ParamNameFn)0x8d51750cu)            // (kind, index): R0 the kind, R1 the index (the pages' calls; the prologue R7 = R0, R6 = R1)
#define RD32(a) (*(volatile u32*)(a))

enum { PAGES = 4, NODE_DISPLAY = 0x5c, WAGONS = 4, FX_TYPES = 11, NAME_KIND_FX = 2, PRINT_FRAMES = 232, PRINT_LINES = 24 };
enum { MIDI_SCREEN = 35, MIDI_DESTS = 4, MIDI_PRINT_LINES = 8 };   // I4: the midi LFO's page, its four destination boxes
static const int PAGE_SCREEN[PAGES] = { 32, 33, 34, 36 };      // the value, random, element and velocity LFOs' screens
static const int PAGE_LFO_TYPE[PAGES] = { 1, 2, 3, 5 };        // the LFO type each shows: the screen 31 + type
// A page's LFO kind by stock's own name, read where stock keeps the six (packs L3; until then this unit carried the
// four it needs): entries of eight bytes at 0x84d256c4, the name's address and the kind, filled by one of the
// constructors the start code runs before main (0x8fca6224). This table says where each page's name's address stands.
#define LFO_NAME_ENTRY(kind) ((const char* const*)(0x84d256c4u + 8u * (kind)))
static const char* const* const PAGE_NAME[PAGES] = { LFO_NAME_ENTRY(1), LFO_NAME_ENTRY(2), LFO_NAME_ENTRY(3), LFO_NAME_ENTRY(5) };   // PAGE_LFO_TYPE's kinds
static const u32 CURRENT_SCREEN = 0x848d2540u;                 // the id of the screen the GUI shows (0x8fc23a1c sets it)
static const u32 GUI_HOST = 0x0802fea8u;                       // 0 the synth, else the drum: what 0x8fc239f4() returns and the name function chooses its arrays by
static const u32 FX_PARAM_NAMES = 0x840af848u;                 // four name pointers per effect type
static const u32 NAME_DASH = 0x840b086cu;                      // stock's "-"
static const char LOG_FILE[] = "op1f_lfopage";
static const char* const MARKER_ID[WAGONS] = { "lfw0", "lfw1", "lfw2", "lfw3" };
extern "C" const char* op1f_chain_rack_value(int host, int knob, int index) asm("op1f_chain_rack_value");   // R5: the chain unit's
static const char* const MIDI_MARKER_ID[MIDI_DESTS][WAGONS] = {    // by destination box, then by the wagon's knob
    { "lfm00", "lfm01", "lfm02", "lfm03" }, { "lfm10", "lfm11", "lfm12", "lfm13" }, { "lfm20", "lfm21", "lfm22", "lfm23" }, { "lfm30", "lfm31", "lfm32", "lfm33" },
};

static u8* marker[PAGES][WAGONS];
static u8 searched[PAGES];
static u8* midi_marker[MIDI_DESTS][WAGONS];
static u8 midi_searched;
static u32 shown_frames, names_given, printed_frames, printed_names, frames, lines;
static u32 midi_frames, midi_printed, midi_lines;

static void show(u8* node, int on) { if (node) node[NODE_DISPLAY] = (u8)(on ? 1 : 0); }

// A page's markers, once its screen is registered and its document loaded: found and hidden from the first frame.
static void search(int p) {
    u32* screens = STOCK_SCREENS();
    void* screen = screens ? (void*)screens[PAGE_SCREEN[p]] : 0;
    if (screen == 0 || ((u32*)screen)[1] == 0) return;         // not registered, or its document not loaded yet (find_node asserts without one): the next frame
    searched[p] = 1;
    int found = 0;
    for (int k = 0; k < WAGONS; ++k) {
        marker[p][k] = (u8*)STOCK_FIND_NODE(screen, MARKER_ID[k], 1);
        if (marker[p][k]) ++found;
        show(marker[p][k], 0);                                 // hidden from the first frame
    }
    STOCK_LOG(3, LOG_FILE, 1, "lfo page: the %s LFO's screen %d at %08x, %d of %d wagon markers found in its artwork", *PAGE_NAME[p], PAGE_SCREEN[p], (u32)screen, found, (int)WAGONS);
}

// I4: the midi page's sixteen, the same way.
static void search_midi() {
    u32* screens = STOCK_SCREENS();
    void* screen = screens ? (void*)screens[MIDI_SCREEN] : 0;
    if (screen == 0 || ((u32*)screen)[1] == 0) return;
    midi_searched = 1;
    int found = 0;
    for (int b = 0; b < MIDI_DESTS; ++b) {
        for (int k = 0; k < WAGONS; ++k) {
            midi_marker[b][k] = (u8*)STOCK_FIND_NODE(screen, MIDI_MARKER_ID[b][k], 1);
            if (midi_marker[b][k]) ++found;
            show(midi_marker[b][k], 0);
        }
    }
    STOCK_LOG(3, LOG_FILE, 3, "lfo page: the midi LFO's screen %d at %08x, %d of %d wagon markers found in its artwork", (int)MIDI_SCREEN, (u32)screen, found, (int)(MIDI_DESTS * WAGONS));
}

extern "C" void op1f_lfopage_frame() asm("op1f_lfopage_frame");
extern "C" void op1f_lfopage_frame() {
    for (int p = 0; p < PAGES; ++p) if (!searched[p]) search(p);
    if (!midi_searched) search_midi();
    int host = RD32(GUI_HOST) ? 1 : 0;
    Op1fLfoFxView v;
    op1f_lfofx_view(host, &v);
    u32 current = RD32(CURRENT_SCREEN);
    int any = 0;
    for (int p = 0; p < PAGES; ++p) {
        int up = searched[p] && current == (u32)PAGE_SCREEN[p] && v.known && v.lfo_type == PAGE_LFO_TYPE[p] && v.to_fx && v.type >= 0;
        for (int k = 0; k < WAGONS; ++k) show(marker[p][k], up && k == v.knob);
        if (up) any = 1;
    }
    if (any) ++shown_frames;
    Op1fLfoMidiView m;                                         // I4: the midi page, a marker a destination box
    op1f_lfofx_midi_view(host, &m);
    int midi_up = midi_searched && current == (u32)MIDI_SCREEN && m.known && m.midi;
    int midi_any = 0;
    for (int b = 0; b < MIDI_DESTS; ++b) {
        int on = midi_up && m.to_fx[b] && m.type[b] >= 0;
        for (int k = 0; k < WAGONS; ++k) show(midi_marker[b][k], on && k == m.knob[b]);
        if (on) midi_any = 1;
    }
    if (midi_any) ++midi_frames;
    if (++frames >= (u32)PRINT_FRAMES) {                       // a line for the read-out when a page was up or a name was given this period (the first twenty-four)
        frames = 0;
        if ((shown_frames != printed_frames || names_given != printed_names) && lines < (u32)PRINT_LINES) {
            ++lines;
            STOCK_LOG(3, LOG_FILE, 2, "lfo page: %u frames with a wagon marker shown (the marker of knob %d), %u names given for another wagon's effect; %s, lfo type %d, screen %u, destination %s",
                      shown_frames, v.knob, names_given, host ? "drum" : "synth", v.lfo_type, current, v.to_fx ? "the effect" : "not the effect");
            printed_frames = shown_frames;
            printed_names = names_given;
        }
        if (midi_frames != midi_printed && midi_lines < (u32)MIDI_PRINT_LINES) {
            ++midi_lines;
            STOCK_LOG(3, LOG_FILE, 4, "lfo page: midi, %u frames with a wagon marker shown; %s, the four destinations to the effect %d %d %d %d, on the wagons of knobs %d %d %d %d",
                      midi_frames, host ? "drum" : "synth", m.to_fx[0], m.to_fx[1], m.to_fx[2], m.to_fx[3], m.knob[0], m.knob[1], m.knob[2], m.knob[3]);
            midi_printed = midi_frames;
        }
    }
}

// The pages' call for the parameter's name, in place of stock's.
extern "C" const char* op1f_lfo_param_name(int kind, int index) asm("op1f_lfo_param_name");
extern "C" const char* op1f_lfo_param_name(int kind, int index) {
    if (kind == NAME_KIND_FX && index >= 0) {
        Op1fLfoFxView v;
        op1f_lfofx_view(RD32(GUI_HOST) ? 1 : 0, &v);
        if (v.known && v.steps && v.knob != v.mirror) {                         // another wagon than the lamp's: its effect's names, not the slot effect's
            ++names_given;
            if (v.type < 0 || v.type >= FX_TYPES) return (const char*)NAME_DASH;
            const char* rack = op1f_chain_rack_value(RD32(GUI_HOST) ? 1 : 0, v.knob, index);   // R5: a pack's effect on that wagon: its own names
            if (rack) return rack[0] ? rack : (const char*)NAME_DASH;
            return *(const char* const*)(FX_PARAM_NAMES + 16u * (u32)v.type + 4u * (u32)(index & 3));   // stock's own indexing (0x8d516480)
        }
    }
    return STOCK_PARAM_NAME(kind, index);
}
