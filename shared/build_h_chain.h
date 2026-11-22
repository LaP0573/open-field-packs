// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build H images H2 and H3: what the chain unit (Build H's chain unit) tells the train page (Build H's train page unit) each
// frame. Plain integers only; the train unit reads it, the chain unit fills it from its own state. H5 adds the
// presets list's view for Build H's presets unit (the chain unit keeps the store, the file and the keys; the presets
// unit the screen and its labels); H5b the save screen's state.
#ifndef OP1F_BUILD_H_CHAIN_H
#define OP1F_BUILD_H_CHAIN_H

enum { OP1F_CHAIN_WAGONS = 4 };

struct Op1fChainView {
    int open;                              // the train is open for this host (T3 opened it, nothing closed it)
    int known;                             // the current slot's chain is the module's own (initialised)
    int slot;                              // the GUI's current sound slot
    int order[OP1F_CHAIN_WAGONS];          // the knob (0..3) at each position, left to right
    int type[OP1F_CHAIN_WAGONS];           // per knob: the wagon's effect type (0..10, the firmware's enumeration) or -1 for none
    int coupled[OP1F_CHAIN_WAGONS];        // per knob: the effect runs (a wagon on the line); 0 on the siding
    int mirror;                            // the knob whose wagon stock's slot shows (T3 opens its page; the lamp)
    int full;                              // the refusal flash is on (a pick over the ceiling)
    int refused_knob;                      // H3: the knob whose pick was refused (valid while full), else -1
    int refused_type;                      // H3: the effect type that did not fit (valid while full), else -1
    int cost_percent;                      // the chain's summed cost as a percentage of the ceiling (may exceed 100)
    int turn_knob;                         // the knob last turned on the train, or -1
    int turn_age;                          // frames since that turn
    int turn_move;                         // H3: that turn was a move (shift held), not a pick
    int bypassed;                          // H3: shift + T3 parked every wagon on its siding (always 0 since H5: the bypass gave its key to the presets list)
    unsigned int peak;                     // H3: the instrument bus's largest sample since the last frame (the train runs on sound)
};

extern "C" void op1f_chain_view(int host, Op1fChainView* out) asm("op1f_chain_view");
extern "C" int op1f_chain_train_open(int host) asm("op1f_chain_train_open");

// H5: the chain presets list, a screen of the GUI's own at the free id 37
// (H6d; H5 to H6c had 85, which the GUI's render loop draws after the power popup's 82, so the list covered the
// shutdown countdown: every screen of the module's own takes a free id below 82, section 22.13),
// dressed by Build H's presets unit from this view every frame; H5b adds the save screen at the free id 38 (stock's
// save page redrawn: the host's box with the preset's number, "saving" with the clock while shift + T3 is held,
// "saved!" with the check after).
enum { OP1F_PRESETS_SCREEN_ID = 37, OP1F_SAVE_SCREEN_ID = 38, OP1F_PRESET_ROWS = 8, OP1F_PRESET_NAME = 12, OP1F_SAVE_SPOKES = 8 };   // H6d: the list at 37 (below the power popup's 82)
enum { OP1F_POWER_POPUP_SCREEN_ID = 82 };                                   // the countdown popup: the render loop draws the registered screens in id order, so a module screen's id stays below it
enum { OP1F_SAVE_NONE = 0, OP1F_SAVE_SAVING = 1, OP1F_SAVE_SAVED = 2 };

struct Op1fPresetsView {
    int open;                                   // the list shows for this host (shift + T3 on the train opened it)
    int count;                                  // presets in the store
    int first;                                  // the window's first preset
    int cursor;                                 // the highlighted preset
    int loaded;                                 // the preset the current chain was loaded from or saved as, or -1 once the chain changed
    int editing;                                // the rename editor is open: the edited preset on the window's first row, the cells over it
    int caret;                                  // the editor's cell (0..11)
    const char* row[OP1F_PRESET_ROWS];          // the window's texts in the font's lowercase: a name, saved, deleted, list full, no presets, or "" for an empty row
    const char* wagon[OP1F_CHAIN_WAGONS];       // the highlighted preset's effect per knob, "-" for an empty wagon, " off" appended to a parked one
    const char* cell[OP1F_PRESET_NAME];         // the editor's cells, one character each (a space shows nothing)
    int save_state;                             // H5b: OP1F_SAVE_NONE, SAVING (the hold runs) or SAVED (the confirmation stands)
    int save_host;                              // the host whose chain is being saved (0 synth, 1 drum): its box on the save screen
    int save_number;                            // the preset's number, 1-based (the one being made while saving)
    int save_progress;                          // 0..OP1F_SAVE_SPOKES while saving: the clock's spokes lit
};

extern "C" void op1f_presets_view(int host, Op1fPresetsView* out) asm("op1f_presets_view");
extern "C" int op1f_chain_list_open(int host) asm("op1f_chain_list_open");
extern "C" int op1f_chain_screen_wanted(int host) asm("op1f_chain_screen_wanted");   // H5b: the save screen's id, the list's id, or 0 for the train or stock

// I2: what the chain unit tells the value LFO's page (Build I's LFO page unit)
// about the stock LFO's effect destination: which effect wagon it turns.
struct Op1fLfoFxView {
    int known;                             // the current slot's chain is the module's own
    int lfo_type;                          // the slot's LFO type (0 tremolo, 1 value, 2 random, 3 element, 4 midi, 5 velocity)
    int steps;                             // it is a type whose destination knob steps through the wagons (I2 the value LFO; I3 the random, element and velocity LFOs)
    int to_fx;                             // its destination is the effect (the destination word inside the type's effect range)
    int knob;                              // the knob (0..3) of the effect wagon it turns: the chosen one while it carries an effect, else the lamp's
    int type;                              // that wagon's effect type (0..10), or -1 when it is empty
    int mirror;                            // the lamp's knob (the wagon stock's own effect page shows)
};

extern "C" void op1f_lfofx_view(int host, Op1fLfoFxView* out) asm("op1f_lfofx_view");

// I4 (section 13): the midi LFO has four destinations, each with a wagon of its own when it is the effect.
struct Op1fLfoMidiView {
    int known;                             // the current slot's chain is the module's own
    int midi;                              // the slot's LFO is the midi type
    int to_fx[4];                          // destination 1..4 is the effect
    int knob[4];                           // the knob (0..3) of the effect wagon it turns: the chosen one while it carries an effect, else the lamp's
    int type[4];                           // that wagon's effect type (0..10), or -1 when it is empty
};

extern "C" void op1f_lfofx_midi_view(int host, Op1fLfoMidiView* out) asm("op1f_lfofx_midi_view");

#endif
