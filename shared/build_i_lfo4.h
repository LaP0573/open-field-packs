// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// The LFO unit (build_i_lfo4.cpp): what it tells the page each frame, the size of what it keeps for a sound in the
// files, and the names the pack's other units call. Since lfo 12 (step R15)
// a sound is a place: 0 to 7 a synth slot, 8 to 15 a drum slot (8 + slot). Plain integers only.
#ifndef OP1F_BUILD_I_LFO4_H
#define OP1F_BUILD_I_LFO4_H

enum { OP1F_LFO_WAGONS = 4, OP1F_LFO_KINDS = 6, OP1F_LFO4_PLACES = 16, OP1F_LFO4_DRUM_PLACE = 8 };
// A place's integers (the chain file after the tag 98 for a synth slot, the LFOs' own files): the lamp's knob, then
// for each of the four knobs the kind plus one (0 none), on or off, the effect wagon it turns (as I2's choice), the
// midi kind's four packed four bits each (as I4's), and the eight parameters as stock's integers.
// I7 adds the order of the four on the page as one more integer (the knob at each place, two bits a place); a file
// of I6's has the first forty-nine and reads as the knobs' own order. lfo 12: that integer carries
// OP1F_LFO4_NOT_RIDING more when the place did not ride "ufo" at the save (the flag "kept, not riding").
enum { OP1F_LFO4_WAGON_INTS = 12, OP1F_LFO4_STORE_MIN = 1 + OP1F_LFO_WAGONS * OP1F_LFO4_WAGON_INTS, OP1F_LFO4_STORE_INTS = OP1F_LFO4_STORE_MIN + 1 };
enum { OP1F_LFO4_NOT_RIDING = 256 };

struct Op1fLfoTrainView {
    int open;                              // the UFO page is on the screen (op1f_lfo4_shown said it is up)
    int slot;                              // lfo 12: the place asked for, 0 to 15 (8 + slot for a drum sound)
    int lamp;                              // the knob whose LFO is the unit's own: T4 opens its page (the searchlight)
    int order[OP1F_LFO_WAGONS];            // I7: the knob (0..3) at each place, left to right
    int kind[OP1F_LFO_WAGONS];             // per knob: 0 tremolo, 1 value, 2 random, 3 element, 4 midi, 5 velocity; -1 an empty wagon
    int on[OP1F_LFO_WAGONS];               // per knob: the LFO runs (a wagon on the line); 0 on the siding
    int turn_knob;                         // the knob last turned on the page, or -1
    int turn_age;                          // frames since that turn
    int turn_move;                         // I7: that turn was a move (shift held), not a pick
    unsigned int notes;                    // I7: notes started on the synth since the boot (the velocity kind's UFO kicks at each)
};

// lfo 12 (section 9). The ride bits, bit n for place n, read from the base once a frame (op1f_lfo4_frame).
extern "C" volatile unsigned int op1f_lfo4_rides asm("op1f_lfo4_rides");
// The place on the screen: the synth's slot in the synth mode, 8 + the drum's slot in the drum mode; -1 in another.
extern "C" int op1f_lfo4_place() asm("op1f_lfo4_place");
// A key on the UFO page of a place (what: OP1F_DOWN, OP1F_DOWN_SHIFT, OP1F_UP), and a turn (knob 0..3): 1 taken, 0
// not. The key thread.
extern "C" int op1f_lfo4_key(int place, int key, int what) asm("op1f_lfo4_key");
extern "C" int op1f_lfo4_turn(int place, int knob, int delta, int shifted) asm("op1f_lfo4_turn");
// The page wanted since the last read, for lfo_frame to apply (op1f_layer_page): 0 nothing, OWN the UFO page, UNIT the
// stock page of the searchlight's LFO. Cleared by the read.
enum { OP1F_LFO4_WANT_NONE = 0, OP1F_LFO4_WANT_OWN = 1, OP1F_LFO4_WANT_UNIT = 2 };
extern "C" int op1f_lfo4_take_page() asm("op1f_lfo4_take_page");
// T4 tapped on the UFO page (the presets unit tells it): the searchlight LFO's stock page, when it has a kind.
extern "C" void op1f_lfo4_open_searchlight(int place) asm("op1f_lfo4_open_searchlight");
extern "C" void op1f_lfo4_view(int place, Op1fLfoTrainView* out) asm("op1f_lfo4_view");
// The base's `shown` of the layer's page passed on: up not 0 when the UFO page came up for the place, 0 when it went.
extern "C" void op1f_lfo4_shown(int place, int up) asm("op1f_lfo4_shown");
// The ride starts the loaded stores ask for: a place lfo_frame should start now (op1f_layer_start), or -1; and the
// answer it got.
extern "C" int op1f_lfo4_ride_due() asm("op1f_lfo4_ride_due");
extern "C" void op1f_lfo4_ride_answer(int place, int answer) asm("op1f_lfo4_ride_answer");

// I8 (section 21): a sound's four LFOs as a preset holds them (the presets unit keeps the library, the files and the
// list; this unit hands the four out and takes them back). The order on the page (the knob at each place), then for
// each knob fifteen integers: the kind (-1 none, else 0..5), on or off, the effect wagon it turns (0 the one under the
// effect train's lamp, 1..4 a knob's), the midi kind's four the same way, and the eight parameters as stock's
// integers. The lamp of the LFO page is not a preset's: it stays on its knob at a load.
enum { OP1F_LFO_PRESET_MIDI = 4, OP1F_LFO_PRESET_PARAMS = 8, OP1F_LFO_PRESET_WAGON_INTS = 3 + OP1F_LFO_PRESET_MIDI + OP1F_LFO_PRESET_PARAMS,
       OP1F_LFO_PRESET_INTS = OP1F_LFO_WAGONS + OP1F_LFO_WAGONS * OP1F_LFO_PRESET_WAGON_INTS };
extern "C" void op1f_lfo4_preset_capture(int* values) asm("op1f_lfo4_preset_capture");          // the place on the screen, OP1F_LFO_PRESET_INTS integers
extern "C" int op1f_lfo4_preset_apply(const int* values) asm("op1f_lfo4_preset_apply");         // onto the place on the screen; answers how many parameters were held to their knob's range
extern "C" const char* op1f_lfo4_kind_name(int kind) asm("op1f_lfo4_kind_name");                // stock's own name of a kind, "none" for anything else
extern "C" int op1f_lfo4_default(int kind, int i) asm("op1f_lfo4_default");                     // a new LFO's parameter i, the catalog's
extern "C" int op1f_lfo4_release_knobs() asm("op1f_lfo4_release_knobs");                        // the page's knobs count as let go (the presets list opens over the page); answers which were down, a bit a knob

#endif
