// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build I images I6 and I7: what the LFO unit (build_i_lfo4.cpp)
// tells the page (Build H's train page unit) each frame, and the size of what it keeps in the chain file
// (Build H's chain unit). Plain integers only.
#ifndef OP1F_BUILD_I_LFO4_H
#define OP1F_BUILD_I_LFO4_H

enum { OP1F_LFO_WAGONS = 4, OP1F_LFO_KINDS = 6 };
// A synth slot in the chain file, after the tag 98: the lamp's knob, then for each of the four knobs the kind plus
// one (0 none), on or off, the effect wagon it turns (as I2's choice), the midi kind's four packed four bits each
// (as I4's), and the eight parameters as stock's integers.
// I7 adds the order of the four on the page as one more integer (the knob at each place, two bits a place); a file
// of I6's has the first forty-nine and reads as the knobs' own order.
enum { OP1F_LFO4_WAGON_INTS = 12, OP1F_LFO4_STORE_MIN = 1 + OP1F_LFO_WAGONS * OP1F_LFO4_WAGON_INTS, OP1F_LFO4_STORE_INTS = OP1F_LFO4_STORE_MIN + 1 };

struct Op1fLfoTrainView {
    int open;                              // the LFO page is on the screen (T4 opened it, nothing closed it)
    int slot;                              // the GUI's current synth slot
    int lamp;                              // the knob whose LFO is stock's own: T4 opens its page (the searchlight)
    int order[OP1F_LFO_WAGONS];            // I7: the knob (0..3) at each place, left to right
    int kind[OP1F_LFO_WAGONS];             // per knob: 0 tremolo, 1 value, 2 random, 3 element, 4 midi, 5 velocity; -1 an empty wagon
    int on[OP1F_LFO_WAGONS];               // per knob: the LFO runs (a wagon on the line); 0 on the siding
    int turn_knob;                         // the knob last turned on the train, or -1
    int turn_age;                          // frames since that turn
    int turn_move;                         // I7: that turn was a move (shift held), not a pick
    unsigned int notes;                    // I7: notes started on the synth since the boot (the velocity kind's UFO kicks at each)
};

extern "C" void op1f_lfotrain_view(Op1fLfoTrainView* out) asm("op1f_lfotrain_view");
extern "C" int op1f_lfotrain_open() asm("op1f_lfotrain_open");

// I8 (section 21): a sound's four LFOs as a preset holds them (Build I's LFO presets unit keeps the library, the files
// and the list; this unit hands the four out and takes them back). The order on the page (the knob at each place),
// then for each knob fifteen integers: the kind (-1 none, else 0..5), on or off, the effect wagon it turns (0 the one
// under the effect train's lamp, 1..4 a knob's), the midi kind's four the same way, and the eight parameters as
// stock's integers. The lamp of the LFO page is not a preset's: it stays on its knob at a load.
enum { OP1F_LFO_PRESET_MIDI = 4, OP1F_LFO_PRESET_PARAMS = 8, OP1F_LFO_PRESET_WAGON_INTS = 3 + OP1F_LFO_PRESET_MIDI + OP1F_LFO_PRESET_PARAMS,
       OP1F_LFO_PRESET_INTS = OP1F_LFO_WAGONS + OP1F_LFO_WAGONS * OP1F_LFO_PRESET_WAGON_INTS };
extern "C" void op1f_lfo4_preset_capture(int* values) asm("op1f_lfo4_preset_capture");          // the sound on the screen, OP1F_LFO_PRESET_INTS integers
extern "C" int op1f_lfo4_preset_apply(const int* values) asm("op1f_lfo4_preset_apply");         // onto the sound on the screen; answers how many parameters were held to their knob's range
extern "C" const char* op1f_lfo4_kind_name(int kind) asm("op1f_lfo4_kind_name");                // stock's own name of a kind, "none" for anything else
extern "C" int op1f_lfo4_default(int kind, int i) asm("op1f_lfo4_default");                     // a new LFO's parameter i, the catalog's
extern "C" int op1f_lfotrain_release_knobs() asm("op1f_lfotrain_release_knobs");                // the page's knobs count as let go (the presets list opens over the page); answers which were down, a bit a knob

#endif
