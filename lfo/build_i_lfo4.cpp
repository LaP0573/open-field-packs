// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build I images I6 and I7: four LFOs per sound on the synth.
//
// The rule: four LFOs per sound, each stock's own LFO in every part, shown as a
// train on T4. One of the four is stock's slot LFO as it is: its kind word, its eight words, its on-off byte, its
// document, its page. That is the wagon under the train's lamp. The three others are objects of stock's LFO
// classes that this unit builds at its own addresses and calls where stock calls its own, each with its own eight
// words and its own on-off. Image I5 proved such objects the same as stock's at every call (the study's 16.11).
//
// The audio side, on the synth:
//
//   * the envelope's part of a voice (the apply with the class 1) and, for the velocity kind, the level's method,
//     from the stub at 0x11a04334 (build_i_lfo4.s), after stock's own LFO has had its call;
//   * the engine's part of a voice (the class 0), from the stub at 0x11a045c0;
//   * the fx side (the class 2), from the chain's run (op1f_fx_chain_run in Build H's chain unit), which every block of
//     the second stage reaches after stock's own fx-side call and before any effect wagon runs; the chain unit's
//     op1f_lfofx_extra aims each LFO at the effect wagon of its choice, as I2 to I4 do for stock's LFO;
//   * the note-on, from a wrapper of the reset 0x8d504496 at its three calls.
//
// Every class is called for every LFO, as stock calls its own whatever the destination: the kinds advance their
// state in each call (the random kind's generator, the value kind's oscillator at the fx side). What an LFO adds to
// the scratch 0x11904754 or to the voice's buffers stays there: that is the modulation, on top of what stock's LFO
// added before it. Around each call the LFO's own copy of the words its kind keeps outside the object goes in and
// comes out again (KIND[].keep; the study's 14.4), so stock's LFO and its page see only their own. When a call
// moved the scratch, stock's page is sent its copy as stock sends it (0x8d504854, which does nothing while one is
// waiting to be read).
//
// The GUI side: the store (per synth slot the lamp's knob, four wagons and their order on the page), the instances
// built for the slot on the screen, the page's keys, the move of the lamp, which hands stock another wagon's LFO
// through stock's own setters and its record's apply, and the log. The page is the effect train's own screen,
// dressed by Build H's train page unit from op1f_lfotrain_view: since I7 four UFOs, the wagons of this unit's words.
//
// The keys since I7 are the effect train's, key for key (since I6): a
// knob's turn picks the kind, a short press opens that LFO's page (stock's own for its kind; stock's list of kinds
// for an empty one), a hold switches it off or on, shift + a turn moves it to the next place, T4 on the page opens
// the page of the LFO under the searchlight. I6 moved the lamp at a press and opened the page on T4 only.
//
// I8 (the study's section 21): presets for the four LFOs, kept as files and listed under shift + T4 on the page by
// Build I's LFO presets unit, exactly as the effect chain's presets are under shift + T3 on the effect train. This unit
// hands a sound's four LFOs out as a preset holds them (op1f_lfo4_preset_capture) and takes a preset's four back
// (op1f_lfo4_preset_apply: the lamp stays on its knob, that knob's LFO goes onto stock's slot as move_lamp puts one
// there, the three others into the store, the instances are built again), and passes the page's keys to the list
// first while it shows.
//
// Threads: the two stubs, the block call and (by I5's read-out) the reset's wrapper run on the audio thread, which
// runs a block to its end before the GUI thread runs again; everything else here is the GUI thread's. The audio
// thread reads an instance through its published pointer only; the GUI thread takes the pointer away a frame before
// it builds. Nothing is logged from the audio thread.

#include "build_i_lfo4.h"

typedef unsigned int u32;
typedef int i32;
typedef short i16;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*CtorFn)(void* self);
typedef int (*ApplyFn)(void* self, u32* block, int cls);
typedef void (*NoteOnFn)(void* self, u32* block);
typedef int (*VelocityFn)(void* self, u32* block, int v);
typedef void (*ResetFn)(u32 voice);
typedef void (*PageCopyFn)(u32 slot, u32 type, int cls);          // 0x8d504854: the scratch and the three words to the page, unless one is waiting
typedef void (*LfoPickFn)(void* record, int id);                  // 0x8d4f3a54: the catalog's lfo_types entry of that id onto the record (lfo_type, lfo_params), then its apply
typedef void (*OnSetFn)(u32 slot, int value);                     // 0x8d504308: the synth slot's LFO on byte
typedef void (*SetWordsDocFn)(void* record, const u32* words);    // 0x8d4f3c84: the document's lfo_params (each word shifted right by sixteen), then the change notice
typedef void (*SetValueDocFn)(void* record, int value);           // 0x8d4f3d0c: the document's lfo_active, then the change notice
typedef void (*RecordFn)(void* record);                           // the record's virtuals
typedef void (*ControllerFn)(void* self);                         // the controller's virtuals without an argument
typedef int (*KeyEventFn)(void* self, const int* event);
typedef void (*ListFn)(void* self, int slot);                      // the controller's virtual at +0x40: stock's list of LFO kinds, as shift + T4 opens it (0x8c0cf130)
typedef void (*JsonCtorFn)(void* json, int kind);                 // 0x8c085ba8
typedef void (*JsonFromCstrFn)(void* json, const char* text);     // 0x8c084430
typedef void* (*JsonLookupFn)(void* json, const char* key);       // 0x8c085e84: the member, inserted when missing
typedef void (*JsonAssignFn)(void* target, void* value);          // 0x8c0845b8
typedef void (*JsonDtorFn)(void* json);                           // 0x8c0844e0

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_RESET_SYNTH ((ResetFn)0x8d504496u)               // (voice): the note-on of the voice's LFO object and of the fx-side one, while the slot's LFO is on
#define STOCK_PAGE_COPY ((PageCopyFn)0x8d504854u)
#define STOCK_LFO_PICK ((LfoPickFn)0x8d4f3a54u)
#define STOCK_LFO_ON_SET ((OnSetFn)0x8d504308u)
#define STOCK_SET_LFO_WORDS_DOC ((SetWordsDocFn)0x8d4f3c84u)
#define STOCK_SET_LFO_ACTIVE_DOC ((SetValueDocFn)0x8d4f3d0cu)
#define STOCK_SYNTH_KEY_DOWN ((KeyEventFn)0x8c0cfe7cu)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define RD16S(a) (*(volatile i16*)(a))

enum { KINDS = 6, VOICES = 6, OBJ_WORDS = 15, KEEP_WORDS = 12, KEEP_REGIONS = 2, PARAMS = 8, WAGONS = OP1F_LFO_WAGONS, SLOTS = 8, MIDI_DESTS = 4 };
enum { KIND_TREMOLO, KIND_VALUE, KIND_RANDOM, KIND_ELEMENT, KIND_MIDI, KIND_VELOCITY, KIND_NONE = -1 };
enum { CLASS_ENGINE = 0, CLASS_ENVELOPE = 1, CLASS_FX = 2 };   // the apply's third argument at the synth's three places
enum { FRAME_VOICE_NEXT = 0x30 };                              // stock's frame in the voice loop: the voice's number plus one at FP - 0x30 (0x11a0421c)
enum { KEY_T1 = 6, KEY_T4 = 9, KEY_CLICK0 = 54, KEY_PRESS0 = 61, PAGE_LFO = 3, CONTROLLER_PAGE = 0x28, CONTROLLER_LFO_LIST = 0x40, CONTROLLER_LFO_ONOFF = 0x58, RECORD_LFO_APPLY = 0x14 };
enum { ORDER_AS_KNOBS = 0xe4 };                                // the knob at each place, two bits a place: 0, 1, 2, 3
enum { HOLD_MS = 400, CLEAN_MS = 120, SWALLOW_MS = 300 };      // the effect train's own times (Build H's chain unit, H4b)
enum { PRINT_FRAMES = 232, EVENT_LINES = 160, SUMMARY_LINES = 240, CHANGE_LINES = 240, POPUP_LINES = 60, FAST_SUMMARIES = 30, LINES_A_SUMMARY = 3, SETTLE_FRAMES = 30 };
enum { BY_PERIOD, BY_CHANGE, BY_POPUP, BUDGETS };              // a summary's budget: the three kinds never spend each other's lines

static const u32 BUILT_SLOT = 0x84a01524u;                    // the sound slot the audio thread plays
static const u32 GUI_SLOT = 0x11904704u;                      // the synth slot on the screen
static const u32 MODE_GUI = 0x84d21088u;                      // 0 while the synth is the mode on the screen
static const u32 LFO_ON = 0x84a01554u;                        // + slot: the slot's LFO is on (a byte)
static const u32 LFO_TYPE = 0x119035a0u;                      // + 4 x slot: the slot's LFO kind
static const u32 LFO_BLOCK = 0x119037e0u;                     // + 32 x slot: the slot's eight LFO words
static const u32 ENGINE_TYPE = 0x11904714u;                   // + 4 x slot: the slot's engine, for the engine page's copy (0x11a04c68)
static const u32 FX_TYPE = 0x11904794u;                       // + 4 x slot: the slot's effect, for the effect page's copy (0x11a054f0)
static const u32 SYNTH_RECORDS = 0x080235dcu;                 // + 52 x slot: the slot's record (its table at +0, its document at +8)
static const u32 RANDOM_LATCH = 0x84026dc8u;                  // a byte the stage sets at its start (0x11a0413a): the first random LFO apply of the block takes it and leaves a value for the page (0x8d50bcb4)
static const u32 LFO_DESCRIPTORS = 0x840af940u;               // 48 bytes a kind, 6 a parameter: the lowest and highest value as signed half-words, filled at boot
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;             // 1 while the shutdown countdown is on the screen
static const char LOG_FILE[] = "op1f_lfo4";
static const char LFO_TYPE_KEY[] = "lfo_type";

// The six kinds, in stock's numbering (the study's 14.1): the constructors (this), the classes of a synth voice's
// object and of the fx-side one (the factories 0x8f00cfae and 0x8f00cf00 write the class after the constructor;
// the tremolo and midi constructors write their own, the same word), the object's size in words, and the words the
// kind keeps outside its objects (14.4), which each LFO must have for itself. A kind's name is stock's own and is read
// where stock keeps the six (packs L3; until then this table carried them): entries of eight bytes at 0x84d256c4, the
// name's address and the kind, in stock's numbering. That table is not in the image: one of the constructors the start
// code runs before main fills it (0x8fca6224). These are the names stock's lookup 0x8fca618c knows, so a name written
// into a sound's document is one it takes. `name` says where the name's address stands.
#define LFO_NAME_ENTRY(kind) ((const char* const*)(0x84d256c4u + 8u * (kind)))
struct Kind {
    const char* const* name;
    u32 ctor_voice, ctor_fx, vtable_voice, vtable_fx, words;
    struct { u32 addr, words; } keep[KEEP_REGIONS];
};
static const Kind KIND[KINDS] = {
    { LFO_NAME_ENTRY(0), 0x8d509cf0u, 0,           0x840addf8u, 0,           7,  { { 0x080267acu, 11 }, { 0x840addecu, 1 } } },   // 28 bytes; no fx side; the struct the page draws from (the apply 0x8d508c4a, the note-on 0x8d508b88) and the step
    { LFO_NAME_ENTRY(1), 0x8d50d2c0u, 0x8d50d2c0u, 0x840ae384u, 0x840ae3bcu, 3,  { { 0x08026a14u, 3 },  { 0, 0 } } },             // 12 bytes; the oscillator all voices share in the FREE form
    { LFO_NAME_ENTRY(2), 0x8d50cae8u, 0x8d50cae8u, 0x840ae220u, 0x840ae258u, 13, { { 0, 0 },            { 0, 0 } } },             // 52 bytes; the generator's seed is everybody's and the latch is stock's: see lfo_apply
    { LFO_NAME_ENTRY(3), 0x8d50b6e0u, 0x8d50b6e0u, 0x840ae078u, 0x840ae0b0u, 14, { { 0, 0 },            { 0, 0 } } },             // 56 bytes; its source buffer lives within one call
    { LFO_NAME_ENTRY(4), 0x8d50b0feu, 0x8d50b3e4u, 0x840adf14u, 0x840adf4cu, 1,  { { 0x08026818u, 2 },  { 0, 0 } } },             // 4 bytes; four half-words the voices' direct volume reads
    { LFO_NAME_ENTRY(5), 0x8d50dec8u, 0x8d50dec8u, 0x840ae518u, 0x840ae550u, 15, { { 0x08026ac4u, 1 },  { 0, 0 } } },             // 60 bytes; a byte for the page
};
// A new LFO's eight parameters: content/op1_factory.json, lfo_types[].default_params (what stock's own pick takes
// from the catalog, 0x8d4f3a54).
static const i16 DEFAULTS[KINDS][PARAMS] = {
    { 16000, 0, 0, 16000, 0, 0, 0, 0 }, { 1000, 4000, 2000, 2000, 0, 0, 0, 0 }, { 2000, 4000, 2000, 24000, 0, 0, 0, 0 },
    { 4688, 9886, 2000, 10640, 0, 0, 0, 0 }, { 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 }, { 32767, 9886, 2000, 1024, 0, 0, 0, 0 } };

// ---------------------------------------------------------------- the store (the GUI thread's)

// A wagon that is not under the lamp: an LFO of the module's own. The wagon under the lamp is stock's slot LFO, read
// where stock keeps it; of its entry here only the kind's sign is used (KIND_NONE: the owner emptied it, which holds
// while stock's LFO stays off).
struct LfoWagon {
    i32 kind;                                                  // 0..5, or KIND_NONE
    i32 on;
    u32 words[PARAMS];                                         // the eight parameters, as stock's block holds them
    u8 pick;                                                   // the effect wagon its effect destination turns: 0 the one under the effect train's lamp, 1..4 a knob's (as I2's choice)
    u8 midi_pick[MIDI_DESTS];                                  // the same for the midi kind's four destinations (as I4's)
};
struct SlotStore {
    i32 lamp;                                                  // the knob whose wagon is stock's slot LFO
    u8 order[WAGONS];                                          // I7: the knob at each place on the page, left to right
    LfoWagon wagon[WAGONS];
};
static SlotStore store[SLOTS];
static int store_ready;

// ---------------------------------------------------------------- the instances (built by the GUI thread, run by the audio thread)

struct Inst {
    u32 voice[VOICES][OBJ_WORDS];
    u32 fx[OBJ_WORDS];
    u32 keep[KEEP_WORDS];                                      // its own copy of the kind's words outside the objects
    u32 block[PARAMS];
    i32 kind;
    u32 slot;
    u32 has_fx;
    u8 pick;
    u8 midi_pick[MIDI_DESTS];
};
static Inst inst[WAGONS];                                      // by the wagon's knob; the lamp's is not used
static Inst* volatile active[WAGONS];                          // published: the audio thread reads the pointer once a use
extern "C" volatile u32 op1f_lfo4_live asm("op1f_lfo4_live");
volatile u32 op1f_lfo4_live;                                   // how many are published: the stubs' one test (build_i_lfo4.s)

struct Time { volatile u32 calls, cycles, most; };
static Time time_env, time_eng, time_fx;                       // the whole of each call from the stub, when an LFO was called
static struct {
    volatile u32 env, eng, fx, note_ons, busy, bad_voice, blocks;
} counts;
static volatile u32 note_busy;                                 // inside the note-ons of the reset's wrapper
static volatile u32 notes_started;                             // I7: every call of the reset, for the page (the velocity kind's UFO)

// Build H's chain unit. Since J5h each of the seven is optional: where the
// effect train's units are away (the pack `lfo` without the pack `fx train`) the LFOs work on the sound's one
// effect, with the answers of the wrappers below and of J5h's unit for the LFOs without the effect train. With those units there, their
// own seven are used, as since I6.
extern "C" int __attribute__((weak)) op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) asm("op1f_lfofx_extra");
extern "C" void __attribute__((weak)) op1f_lfofx_extra_counts(u32* out) asm("op1f_lfofx_extra_counts");
extern "C" u32 __attribute__((weak)) op1f_chain_ms() asm("op1f_chain_ms");
extern "C" void __attribute__((weak)) op1f_chain_touch(int host) asm("op1f_chain_touch");
extern "C" void __attribute__((weak)) op1f_chain_close_train(int host) asm("op1f_chain_close_train");
extern "C" void __attribute__((weak)) op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) asm("op1f_chain_lfo_picks");
extern "C" int __attribute__((weak)) op1f_chain_lfo_knob(int host, u32 slot, int pick) asm("op1f_chain_lfo_knob");   // I8: the knob of the effect wagon a choice turns now
// J5h's unit for the LFOs without the effect train (J5h): the clock where the chain's is away, and the LFOs' own settings file. Optional, so
// that the packs before J5h still build this unit by itself.
extern "C" u32 __attribute__((weak)) op1f_lfo_alone_ms() asm("op1f_lfo_alone_ms");
extern "C" void __attribute__((weak)) op1f_lfo_file_touch() asm("op1f_lfo_file_touch");                              // the store changed: the own file follows
extern "C" void __attribute__((weak)) op1f_lfo_file_from_chain(int slot, const int* values) asm("op1f_lfo_file_from_chain");   // the chain unit handed a slot's integers over
extern "C" void __attribute__((weak)) op1f_lfo_file_to_chain(int slot, const int* values, int have) asm("op1f_lfo_file_to_chain");   // the chain unit collected a slot's integers for its file
extern "C" void __attribute__((weak)) op1f_lfo_file_frame() asm("op1f_lfo_file_frame");                              // once a frame: the start's choice between the two copies, the own file's write
// Build I's LFO presets unit (I8): the presets list over the page. The key functions answer -1 for a key that is not the list's.
extern "C" int __attribute__((weak)) op1f_lfopresets_key_down(int key, int shifted) asm("op1f_lfopresets_key_down");
extern "C" void __attribute__((weak)) op1f_lfopresets_key_up(int key) asm("op1f_lfopresets_key_up");
extern "C" int __attribute__((weak)) op1f_lfopresets_turn(int knob, int delta) asm("op1f_lfopresets_turn");
// Build H's train page unit: the train's screen exists
extern "C" int __attribute__((weak)) op1f_train_available() asm("op1f_train_available");

// The core clock. The memory clobber keeps the read where it is written (I5: the compiler may move a volatile asm
// past a call).
static u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}

static void account(Time* t, u32 dt) {
    t->calls = t->calls + 1;
    t->cycles = t->cycles + dt;
    if (dt > t->most) t->most = dt;
}

// lfo4-alone-begin
// J5h: what the chain unit does for this one, with an answer for when it is away. GUI thread, but for none of them.

// The millisecond clock of the holds: the chain's frame call keeps one; without it the LFOs' own frame call does.
static u32 now_ms() {
    return &op1f_chain_ms != 0 ? op1f_chain_ms() : (&op1f_lfo_alone_ms != 0 ? op1f_lfo_alone_ms() : 0u);
}

// The store changed: the chain's file is written a moment later (its copy of the LFOs' integers rides in it), and
// the LFOs' own file after it.
static void touch() {
    if (&op1f_chain_touch != 0) op1f_chain_touch(0);
    if (&op1f_lfo_file_touch != 0) op1f_lfo_file_touch();
}

// The effect wagon stock's slot LFO turns, read or written in the chain unit. Without that unit there is one effect
// and no choice to make: a read leaves what the caller holds and a write goes nowhere, so every wagon's record, the
// lamp's too, keeps the choice it holds, wherever the lamp moves. The LFOs' own file puts the lamp's there at a start
// and carries it (op1f_lfo4_lamp_picks), and hands it to the chain unit when that unit is back.
static void picks(u32 slot, int write, u8* pick, u8* midi) {
    if (&op1f_chain_lfo_picks != 0) op1f_chain_lfo_picks(0, slot, write, pick, midi);
}

// The knob of the effect wagon a choice turns now, for a preset. Without the chain unit: the knob the choice names,
// or the first for none (a sound as stock has it carries its one effect on the first wagon).
static int knob_of(u32 slot, int pick) {
    if (&op1f_chain_lfo_knob != 0) return op1f_chain_lfo_knob(0, slot, pick);
    return (pick > 0 && pick <= WAGONS) ? pick - 1 : 0;
}
// lfo4-alone-end

// ---------------------------------------------------------------- the audio thread

// The kind's words outside its objects: the LFO's own in before a call, and out again after it (the same exchange).
static void swap_keep(Inst* in) {
    u32* mine = in->keep;
    const Kind* kind = &KIND[in->kind];
    for (int r = 0; r < KEEP_REGIONS; ++r) {
        volatile u32* live = (volatile u32*)kind->keep[r].addr;
        u32 n = kind->keep[r].words;
        for (u32 i = 0; i < n; ++i) {
            u32 t = live[i];
            live[i] = *mine;
            *mine++ = t;
        }
    }
}

// One apply of an LFO's object, as stock calls its own. The random kind's latch is held clear for the call: an LFO
// that is not stock's leaves no value for stock's page. Its draws come from the generator the whole firmware shares,
// as stock's do.
static int lfo_apply(Inst* in, u32* object, int cls) {
    u8 latch = 0;
    if (in->kind == KIND_RANDOM) { latch = RD8(RANDOM_LATCH); *(volatile u8*)RANDOM_LATCH = 0; }
    int moved = ((ApplyFn)((u32*)object[0])[2])(object, in->block, cls);
    if (in->kind == KIND_RANDOM) *(volatile u8*)RANDOM_LATCH = latch;
    return moved;
}

// The stub at 0x11a04334: the envelope's part of a voice, after stock's own LFO has had its call. Answers the
// voice's level, which the velocity kind changes.
extern "C" int op1f_lfo4_env(u32* frame, int level) asm("op1f_lfo4_env");
extern "C" int op1f_lfo4_env(u32* frame, int level) {
    u32 t_in = cycles();
    if (note_busy) { counts.busy = counts.busy + 1; return level; }   // a note-on is under way on another thread: its LFOs wait a block
    u32 voice = *(volatile u32*)((u32)frame - (u32)FRAME_VOICE_NEXT) - 1u;
    if (voice >= (u32)VOICES) { counts.bad_voice = counts.bad_voice + 1; return level; }
    u32 slot = RD32(BUILT_SLOT) & 7u;
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = active[k];
        if (in == 0 || in->slot != slot) continue;
        u32* object = in->voice[voice];
        swap_keep(in);
        moved |= lfo_apply(in, object, CLASS_ENVELOPE);
        if (in->kind == KIND_VELOCITY) level = ((VelocityFn)((u32*)object[0])[4])(object, in->block, (i16)level);   // stock hands the method the velocity as a signed half-word and takes the low half of its answer (0x11a04cb6, 0x11a0459c)
        swap_keep(in);
        called = 1;
    }
    if (!called) return level;
    if (moved) STOCK_PAGE_COPY(slot, 0, CLASS_ENVELOPE);       // as stock after its own apply (0x11a04cc8)
    counts.env = counts.env + 1;
    account(&time_env, cycles() - t_in);
    return level;
}

// The stub at 0x11a045c0: the engine's part of a voice, the same way. The direct destinations write the voice's
// buffers, which are ready here (stock's own call was just before).
extern "C" void op1f_lfo4_eng(u32* frame) asm("op1f_lfo4_eng");
extern "C" void op1f_lfo4_eng(u32* frame) {
    u32 t_in = cycles();
    if (note_busy) { counts.busy = counts.busy + 1; return; }
    u32 voice = *(volatile u32*)((u32)frame - (u32)FRAME_VOICE_NEXT) - 1u;
    if (voice >= (u32)VOICES) { counts.bad_voice = counts.bad_voice + 1; return; }
    u32 slot = RD32(BUILT_SLOT) & 7u;
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = active[k];
        if (in == 0 || in->slot != slot) continue;
        swap_keep(in);
        moved |= lfo_apply(in, in->voice[voice], CLASS_ENGINE);
        swap_keep(in);
        called = 1;
    }
    if (!called) return;
    if (moved) STOCK_PAGE_COPY(slot, RD32(ENGINE_TYPE + 4u * slot), CLASS_ENGINE);   // as stock (0x11a04c66)
    counts.eng = counts.eng + 1;
    account(&time_eng, cycles() - t_in);
}

// The chain's run calls this first, once a block of the second stage: the fx side of each LFO, on the effect wagon
// of its choice.
extern "C" void op1f_lfo4_block(int host) asm("op1f_lfo4_block");
extern "C" void op1f_lfo4_block(int host) {
    if (host != 0 || op1f_lfo4_live == 0) return;              // the drum's turn comes with its own image
    u32 t_in = cycles();
    counts.blocks = counts.blocks + 1;
    if (note_busy) { counts.busy = counts.busy + 1; return; }
    u32 slot = RD32(BUILT_SLOT) & 7u;
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = active[k];
        if (in == 0 || in->slot != slot || !in->has_fx) continue;
        u8 latch = 0;
        if (in->kind == KIND_RANDOM) { latch = RD8(RANDOM_LATCH); *(volatile u8*)RANDOM_LATCH = 0; }
        swap_keep(in);
        if (&op1f_lfofx_extra != 0) moved |= op1f_lfofx_extra(0, in->fx, in->block, in->pick, in->midi_pick);
        else moved |= ((ApplyFn)((u32*)in->fx[0])[2])(in->fx, in->block, CLASS_FX);   // J5h: the chain unit is away: stock's apply on the scratch as it stands, for the sound's one effect
        swap_keep(in);
        if (in->kind == KIND_RANDOM) *(volatile u8*)RANDOM_LATCH = latch;
        called = 1;
    }
    if (!called) return;
    if (moved) STOCK_PAGE_COPY(slot, RD32(FX_TYPE + 4u * slot), CLASS_FX);   // as stock (0x11a054e4): the lamp's effect moved on the scratch
    counts.fx = counts.fx + 1;
    account(&time_fx, cycles() - t_in);
}

// In place of the reset 0x8d504496 at its three calls: stock's reset as it is, then the note-on of each of the
// module's LFOs for the same voice (its voice object, then its fx-side one, as stock's reset does for its own).
extern "C" void op1f_lfo4_reset_synth(u32 voice) asm("op1f_lfo4_reset_synth");
extern "C" void op1f_lfo4_reset_synth(u32 voice) {
    STOCK_RESET_SYNTH(voice);
    notes_started = notes_started + 1;
    if (op1f_lfo4_live == 0 || voice >= (u32)VOICES) return;
    u32 slot = RD32(BUILT_SLOT) & 7u;
    note_busy = 1;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = active[k];
        if (in == 0 || in->slot != slot) continue;
        swap_keep(in);
        ((NoteOnFn)((u32*)in->voice[voice][0])[3])(in->voice[voice], in->block);
        if (in->has_fx) ((NoteOnFn)((u32*)in->fx[0])[3])(in->fx, in->block);
        swap_keep(in);
    }
    note_busy = 0;
    counts.note_ons = counts.note_ons + 1;
}

// ---------------------------------------------------------------- the GUI thread: the store

static struct {
    int open;                                                  // the LFO train is on the screen
    u32 frames;
    int want, stage;                                           // the instances are to be built again: 0 take them away, 1 build and publish
    int built_slot;                                            // the slot they stand for, or -1
    u32 built;                                                 // instances built since the start
    void* self;                                                // the synth controller, from the last key event
    int pressed[WAGONS], armed[WAGONS], acted[WAGONS];
    u32 press_ms[WAGONS], last_turn_ms[WAGONS], swallow_until[WAGONS];
    int turn_knob, turn_move;
    u32 turn_frame;
    u32 picks, lamp_moves, holds, taps, opens, page_opens, list_opens, moves, loads;
    u32 event_lines, summary_lines[BUDGETS], summaries, summary_at;
    int print_countdown, summary_due, popup_seen;
} lt;

static u32 gui_slot() { return RD32(GUI_SLOT) & 7u; }
static int stock_on(u32 slot) { return RD8(LFO_ON + slot) != 0; }
static int stock_kind(u32 slot) { u32 k = RD32(LFO_TYPE + 4u * slot); return k < (u32)KINDS ? (int)k : KIND_NONE; }   // 6: a name stock's lookup did not know (0x8fca618c)
static void* record_of(u32 slot) { return (void*)(SYNTH_RECORDS + 52u * slot); }

// Every slot starts as stock has it: its one LFO on the first wagon, under the lamp, the three others empty.
static void ensure_store() {
    if (store_ready) return;
    store_ready = 1;
    lt.built_slot = -1;
    lt.turn_knob = -1;
    for (int s = 0; s < SLOTS; ++s) {
        store[s].lamp = 0;
        for (int k = 0; k < WAGONS; ++k) store[s].wagon[k].kind = k == 0 ? 0 : KIND_NONE;
        for (int k = 0; k < WAGONS; ++k) store[s].order[k] = (u8)k;
    }
}

// A wagon's kind and on-off as the page shows them: stock's own for the wagon under the lamp.
static int wagon_kind(u32 slot, int k) {
    const SlotStore* s = &store[slot];
    if (k != s->lamp) return s->wagon[k].kind;
    return (s->wagon[k].kind < 0 && !stock_on(slot)) ? KIND_NONE : stock_kind(slot);
}

static int wagon_on(u32 slot, int k) {
    if (wagon_kind(slot, k) < 0) return 0;
    return k == store[slot].lamp ? stock_on(slot) : (store[slot].wagon[k].on != 0);
}

static const char* kind_name(int kind) { return (kind >= 0 && kind < KINDS) ? *KIND[kind].name : "none"; }

// A stored parameter held to its knob's range as the descriptor row stands in memory (the rows are filled at boot;
// an entry not filled holds nothing), as the chain unit holds an effect's (H6j).
static int hold_param(int kind, int i, int value) {
    const volatile i16* entry = (const volatile i16*)(LFO_DESCRIPTORS + 48u * (u32)kind + 6u * (u32)i);
    int lo = entry[0], hi = entry[1];
    if (lo >= hi) return value;
    return value < lo ? lo : (value > hi ? hi : value);
}

// A synth slot's integers (build_i_lfo4.h) into the store. J5h: the plain setter, which the LFOs' own file uses too.
extern "C" void op1f_lfo4_store_set(int slot, const int* values) asm("op1f_lfo4_store_set");
extern "C" void op1f_lfo4_store_set(int slot, const int* values) {
    ensure_store();
    if (slot < 0 || slot >= SLOTS) return;
    SlotStore* s = &store[slot];
    int lamp = values[0];
    s->lamp = (lamp >= 0 && lamp < WAGONS) ? lamp : 0;
    for (int k = 0; k < WAGONS; ++k) {
        const int* v = values + 1 + k * OP1F_LFO4_WAGON_INTS;
        LfoWagon* w = &s->wagon[k];
        int kind = v[0] - 1;
        w->kind = (kind >= 0 && kind < KINDS) ? kind : KIND_NONE;
        w->on = v[1] != 0;
        w->pick = (u8)((v[2] >= 0 && v[2] <= WAGONS) ? v[2] : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) {
            int one = (v[3] >> (4 * i)) & 15;
            w->midi_pick[i] = (u8)(one <= WAGONS ? one : 0);
        }
        for (int i = 0; i < PARAMS; ++i) w->words[i] = (u32)(w->kind >= 0 ? hold_param(w->kind, i, v[4 + i]) : 0) << 16;
    }
    int packed = values[OP1F_LFO4_STORE_MIN], seen = 0;        // I7: the order; anything but the four knobs once each reads as the knobs' own order (a file of I6's has none)
    for (int k = 0; k < WAGONS; ++k) seen |= 1 << ((packed >> (2 * k)) & 3);
    if (packed < 0 || packed > 255 || seen != 15) packed = ORDER_AS_KNOBS;
    for (int k = 0; k < WAGONS; ++k) s->order[k] = (u8)((packed >> (2 * k)) & 3);
    ++lt.loads;
    lt.want = 1;
}

// The chain unit hands a slot's integers over: from its file at its load, or from an instrument that was loaded.
// J5h: the LFOs' own file is told first (at the start it holds the chain's copy against its own; later it follows).
extern "C" void op1f_lfo4_store_load(int slot, const int* values) asm("op1f_lfo4_store_load");
extern "C" void op1f_lfo4_store_load(int slot, const int* values) {
    if (&op1f_lfo_file_from_chain != 0) op1f_lfo_file_from_chain(slot, values);
    op1f_lfo4_store_set(slot, values);
}

// The store's integers for a slot. 0 while the slot is as stock has it (nothing is written for such a sound).
// J5h: the plain getter, which the LFOs' own file uses too.
extern "C" int op1f_lfo4_store_get(int slot, int* values) asm("op1f_lfo4_store_get");
extern "C" int op1f_lfo4_store_get(int slot, int* values) {
    ensure_store();
    if (slot < 0 || slot >= SLOTS) return 0;
    const SlotStore* s = &store[slot];
    int packed = 0;
    for (int k = 0; k < WAGONS; ++k) packed |= (int)s->order[k] << (2 * k);
    int plain = s->lamp == 0 && s->wagon[0].kind >= 0 && packed == ORDER_AS_KNOBS;
    for (int k = 1; k < WAGONS; ++k) if (s->wagon[k].kind >= 0) plain = 0;
    if (plain) return 0;
    values[0] = s->lamp;
    values[OP1F_LFO4_STORE_MIN] = packed;
    for (int k = 0; k < WAGONS; ++k) {
        int* v = values + 1 + k * OP1F_LFO4_WAGON_INTS;
        const LfoWagon* w = &s->wagon[k];
        int mine = k != s->lamp && w->kind >= 0;               // the lamp's wagon is stock's: its entry says only whether it was emptied
        v[0] = w->kind < 0 ? 0 : w->kind + 1;
        v[1] = mine && w->on ? 1 : 0;
        v[2] = mine ? w->pick : 0;
        v[3] = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) v[3] |= (mine ? (int)w->midi_pick[i] : 0) << (4 * i);
        for (int i = 0; i < PARAMS; ++i) v[4 + i] = mine ? (int)w->words[i] >> 16 : 0;
    }
    return 1;
}

// The chain unit collects a slot's integers: for its file at its save, or for an instrument that is saved.
// J5h: the LFOs' own file is told what the chain's copy now holds.
extern "C" int op1f_lfo4_store_save(int slot, int* values) asm("op1f_lfo4_store_save");
extern "C" int op1f_lfo4_store_save(int slot, int* values) {
    int have = op1f_lfo4_store_get(slot, values);
    if (&op1f_lfo_file_to_chain != 0) op1f_lfo_file_to_chain(slot, values, have);
    return have;
}

// J5h: the effect wagon the LFO under the lamp turns, and its midi kind's four, as this unit's record of the lamp's
// wagon holds them: read, or written. With the chain unit there they are that unit's to keep (picks(), its tags 96
// and 97) and this record is not looked at. While it is away nothing moves them, so the record is where they stay:
// the LFOs' own file (J5h's unit for the LFOs without the effect train) puts them here at a start and carries them in its file.
extern "C" void op1f_lfo4_lamp_picks(int slot, int write, u8* pick, u8* midi) asm("op1f_lfo4_lamp_picks");
extern "C" void op1f_lfo4_lamp_picks(int slot, int write, u8* pick, u8* midi) {
    ensure_store();
    if (slot < 0 || slot >= SLOTS) return;
    LfoWagon* w = &store[slot].wagon[store[slot].lamp];
    if (write) {
        w->pick = (u8)(*pick <= WAGONS ? *pick : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) w->midi_pick[i] = (u8)(midi[i] <= WAGONS ? midi[i] : 0);
    } else {
        *pick = w->pick;
        for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = w->midi_pick[i];
    }
}

// ---------------------------------------------------------------- the GUI thread: the instances

static void take_away() {
    op1f_lfo4_live = 0;
    for (int k = 0; k < WAGONS; ++k) active[k] = 0;
}

// One LFO of the module's own for the slot: a voice's object for each of the six voices and, where the kind has one,
// the fx-side object, built from zeroed memory by stock's constructors with the class the factory gives (I5's
// read-out: the same words as stock's own leave). The words its kind keeps outside the objects start as they stand
// (a state stock's code made).
static void build_one(int k, u32 slot, const LfoWagon* w) {
    Inst* in = &inst[k];
    const Kind* kind = &KIND[w->kind];
    for (int v = 0; v < VOICES; ++v) {
        for (int i = 0; i < OBJ_WORDS; ++i) in->voice[v][i] = 0;
        ((CtorFn)kind->ctor_voice)(in->voice[v]);
        in->voice[v][0] = kind->vtable_voice;
    }
    for (int i = 0; i < OBJ_WORDS; ++i) in->fx[i] = 0;
    if (kind->vtable_fx) {
        ((CtorFn)kind->ctor_fx)(in->fx);
        in->fx[0] = kind->vtable_fx;
    }
    u32* mine = in->keep;
    for (int r = 0; r < KEEP_REGIONS; ++r) {
        const volatile u32* live = (const volatile u32*)kind->keep[r].addr;
        for (u32 i = 0; i < kind->keep[r].words; ++i) *mine++ = live[i];
    }
    for (int i = 0; i < PARAMS; ++i) in->block[i] = w->words[i];
    in->kind = w->kind;
    in->slot = slot;
    in->has_fx = kind->vtable_fx != 0;
    in->pick = w->pick;
    for (int i = 0; i < MIDI_DESTS; ++i) in->midi_pick[i] = w->midi_pick[i];
    ++lt.built;
}

static void build_all(u32 slot) {
    const SlotStore* s = &store[slot];
    u32 live = 0;
    for (int k = 0; k < WAGONS; ++k) {
        const LfoWagon* w = &s->wagon[k];
        if (k == s->lamp || w->kind < 0 || !w->on) continue;
        build_one(k, slot, w);
        active[k] = &inst[k];
        ++live;
    }
    op1f_lfo4_live = live;
}

// ---------------------------------------------------------------- the GUI thread: the train's grammar

static void event_line(const char* what, u32 slot, int knob, int a, int b) {
    lt.summary_due = 1;
    lt.summary_at = lt.frames + (u32)SETTLE_FRAMES;                 // one summary for a run of changes: half a second after the last
    if (lt.event_lines >= (u32)EVENT_LINES) return;
    ++lt.event_lines;
    STOCK_LOG(3, LOG_FILE, 1, "lfo train: s%u knob %d %s: %s -> %s; lamp on knob %d, wagons %s/%s/%s/%s, on %d%d%d%d",
              slot, knob, what, kind_name(a), kind_name(b), store[slot].lamp,
              kind_name(wagon_kind(slot, 0)), kind_name(wagon_kind(slot, 1)), kind_name(wagon_kind(slot, 2)), kind_name(wagon_kind(slot, 3)),
              wagon_on(slot, 0), wagon_on(slot, 1), wagon_on(slot, 2), wagon_on(slot, 3));
}

static void set_text(void* doc, const char* key, const char* text) {
    u32 item[4];
    STOCK_JSON_CTOR(item, 0);
    STOCK_JSON_FROM_CSTR(item, text);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(doc, key), item);
    STOCK_JSON_DTOR(item);
}

// A turn of a wagon's knob: the kind before or after, none below tremolo. Under the lamp the pick is stock's own
// (the catalog's entry onto the record, which switches nothing off; the LFO is switched on, as stock's list does),
// and none is stock's LFO switched off. On another wagon a new LFO starts as stock starts one: the catalog's
// defaults, switched on, its effect destination on the wagon under the effect train's lamp.
static void pick_kind(int k, int delta) {
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    int now = wagon_kind(slot, k);
    int next = now + (delta > 0 ? 1 : -1);
    if (next < KIND_NONE || next >= KINDS) return;             // the ends of the list
    LfoWagon* w = &s->wagon[k];
    if (k == s->lamp) {
        void* record = record_of(slot);
        if (next < 0) {
            STOCK_LFO_ON_SET(slot, 0);
            STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        } else {
            STOCK_LFO_PICK(record, next + 1);                  // the catalog's ids are the kinds plus one (content/op1_factory.json)
            STOCK_LFO_ON_SET(slot, 1);
            STOCK_SET_LFO_ACTIVE_DOC(record, 1);
        }
        w->kind = next;
    } else {
        w->kind = next;
        w->on = next >= 0;
        w->pick = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) w->midi_pick[i] = 0;
        for (int i = 0; i < PARAMS; ++i) w->words[i] = next >= 0 ? (u32)(i32)DEFAULTS[next][i] << 16 : 0;
        lt.want = 1;
    }
    ++lt.picks;
    touch();
    event_line("turned", slot, k, now, next);
}

// Shift + a turn: the UFO to the next place on the page, as a wagon on the effect train. The LFOs run side by side,
// so the order is the page's alone.
static void move_place(int k, int delta) {
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    int at = 0;
    for (int p = 0; p < WAGONS; ++p) if (s->order[p] == (u8)k) at = p;
    int to = at + (delta > 0 ? 1 : -1);
    if (to < 0 || to >= WAGONS) return;
    s->order[at] = s->order[to];
    s->order[to] = (u8)k;
    ++lt.moves;
    touch();
    int kind = wagon_kind(slot, k);
    event_line(delta > 0 ? "moved right" : "moved left", slot, k, kind, kind);
}

// The hold: the wagon's LFO off or on. Under the lamp it is stock's own on-off (the controller's virtual, which
// writes the byte and the document); an empty wagon has nothing to switch.
static void toggle(int k) {
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    int kind = wagon_kind(slot, k);
    if (kind < 0) return;
    if (k == s->lamp) {
        if (lt.self == 0) return;
        ((ControllerFn)(*(u32**)lt.self)[CONTROLLER_LFO_ONOFF / 4])(lt.self);
    } else {
        s->wagon[k].on = !s->wagon[k].on;
        lt.want = 1;
    }
    ++lt.holds;
    touch();
    event_line(wagon_on(slot, k) ? "held, on" : "held, off", slot, k, kind, kind);
}

// The short press: the lamp moves to the wagon. What stock holds goes into the record of the wagon the lamp leaves,
// which becomes one of the module's three; the other wagon's LFO goes onto stock's slot through the document
// (lfo_type as the kind's name through the json string setter, the key having no setter of its own, as fx_type;
// lfo_params and lfo_active through stock's two setters) and the record's own LFO apply, which is how stock puts a
// document's LFO on the stage (0x8d4f53a0: the kind word, the eight words, the on byte, the objects built). An
// empty wagon under the lamp is stock's LFO switched off.
static void move_lamp(int to) {
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    int from = s->lamp;
    if (to == from) return;
    void* record = record_of(slot);
    LfoWagon* a = &s->wagon[from];
    int a_kind = wagon_kind(slot, from);
    a->kind = a_kind;
    a->on = stock_on(slot);
    for (int i = 0; i < PARAMS; ++i) a->words[i] = RD32(LFO_BLOCK + 32u * slot + 4u * (u32)i);
    picks(slot, 0, &a->pick, a->midi_pick);
    LfoWagon* b = &s->wagon[to];
    if (b->kind >= 0) {
        set_text((u8*)record + 8, LFO_TYPE_KEY, *KIND[b->kind].name);
        STOCK_SET_LFO_WORDS_DOC(record, b->words);
        STOCK_SET_LFO_ACTIVE_DOC(record, b->on ? 1 : 0);
        ((RecordFn)(*(u32**)record)[RECORD_LFO_APPLY / 4])(record);
        picks(slot, 1, &b->pick, b->midi_pick);
    } else {
        u8 none = 0, nones[MIDI_DESTS] = { 0, 0, 0, 0 };
        STOCK_LFO_ON_SET(slot, 0);
        STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        picks(slot, 1, &none, nones);
    }
    s->lamp = to;
    lt.want = 1;
    ++lt.lamp_moves;
    touch();
    event_line("pressed, the lamp moved here", slot, to, a_kind, b->kind);
}

// A short press on a knob, and T4 on the page for the LFO under the searchlight: that LFO's page. The lamp moves to
// it first, so it is stock's own, and stock shows its page for its kind: stock's T4 switches the LFO off or on when
// its page is the one showing, so the page word is moved off it first (as the effect train does for T3). An empty
// one has no page: stock's list of kinds opens for it, as shift + T4 opens it (the controller's virtual at +0x40,
// the twin of the effect browser the effect train opens on an empty wagon), with the LFO page behind it.
static void open_page(void* self, int k) {
    u32 slot = gui_slot();
    if (k != store[slot].lamp) move_lamp(k);
    lt.open = 0;
    int* page = (int*)((u8*)self + CONTROLLER_PAGE);
    if (wagon_kind(slot, k) < 0) {
        ++lt.list_opens;
        *page = PAGE_LFO;
        ((ListFn)(*(u32**)self)[CONTROLLER_LFO_LIST / 4])(self, (int)slot);
        return;
    }
    ++lt.page_opens;
    if (*page == PAGE_LFO) *page = 0;
    int event[4] = { KEY_T4, 0, 0, 0 };
    STOCK_SYNTH_KEY_DOWN(self, event);
}

// The synth controller's key-down (shifted: with shift held), before the effect train's own handling. -1: not the
// LFO train's key.
extern "C" int op1f_lfotrain_key_down(void* self, const int* event, int shifted) asm("op1f_lfotrain_key_down");
extern "C" int op1f_lfotrain_key_down(void* self, const int* event, int shifted) {
    ensure_store();
    int key = event[0];
    lt.self = self;
    if (!lt.open) {
        if (key != KEY_T4 || shifted || &op1f_train_available == 0 || !op1f_train_available()) return -1;   // no page to show: T4 is stock's
        lt.open = 1;
        ++lt.opens;
        if (&op1f_chain_close_train != 0) op1f_chain_close_train(0);
        return 1;
    }
    if (&op1f_lfopresets_key_down) {                           // I8: shift + T4 on the page is the presets' key (a tap opens the list, a hold saves); the list, while it shows, takes the knobs and T4
        int taken = op1f_lfopresets_key_down(key, shifted);
        if (taken >= 0) return taken;
    }
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) {      // a knob pressed: the hold's timer starts (the frame call fires it), unless the knob was just turned
        int k = key - KEY_PRESS0;
        u32 now = now_ms();
        lt.pressed[k] = 1;
        lt.press_ms[k] = now;
        lt.acted[k] = 0;
        lt.armed[k] = !shifted && now - lt.last_turn_ms[k] >= (u32)CLEAN_MS;
        return 1;
    }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) {      // the click stock makes of a release without a turn: the short press, unless a hold just acted
        int k = key - KEY_CLICK0;
        if ((i32)(lt.swallow_until[k] - now_ms()) > 0) { lt.swallow_until[k] = now_ms(); return 1; }
        if (!shifted) { ++lt.taps; open_page(self, k); }
        return 1;
    }
    if (key == KEY_T4) {
        if (!shifted) open_page(self, store[gui_slot()].lamp);
        return 1;
    }
    if (key >= KEY_T1 && key < KEY_T4) lt.open = 0;             // T1 to T3: the train leaves the screen; the effect train's handling and stock's go on
    return -1;
}

extern "C" void op1f_lfotrain_key_up(int key) asm("op1f_lfotrain_key_up");
extern "C" void op1f_lfotrain_key_up(int key) {
    if (&op1f_lfopresets_key_up) op1f_lfopresets_key_up(key);  // I8: T4 let go (the tap of shift + T4), and the list's own knobs
    if (key < KEY_PRESS0 || key >= KEY_PRESS0 + WAGONS) return;
    int k = key - KEY_PRESS0;
    if (lt.pressed[k] && lt.acted[k]) lt.swallow_until[k] = now_ms() + (u32)SWALLOW_MS;   // the click that follows a hold is not a short press
    lt.pressed[k] = 0;
}

// A knob turned while the page is open (index 4 to 7: with shift, the move). -1 while the page is closed.
extern "C" int op1f_lfotrain_encoder(void* self, int index, int delta) asm("op1f_lfotrain_encoder");
extern "C" int op1f_lfotrain_encoder(void* self, int index, int delta) {
    if (!lt.open) return -1;
    ensure_store();
    lt.self = self;
    if (delta == 0) return 1;
    if (&op1f_lfopresets_turn && op1f_lfopresets_turn(index & 3, delta)) return 1;   // I8: the list's scroll, caret and letter
    int k = index & 3;
    if (lt.pressed[k]) lt.acted[k] = 1;                        // a turn while pressed: no hold (stock sends no click either)
    lt.last_turn_ms[k] = now_ms();
    if (index >= 0 && index < WAGONS) pick_kind(k, delta);
    else if (index >= WAGONS && index < 2 * WAGONS) move_place(k, delta);
    else return 1;
    lt.turn_knob = k;
    lt.turn_frame = lt.frames;
    lt.turn_move = index >= WAGONS;
    return 1;
}

extern "C" int op1f_lfotrain_open() { return lt.open; }

extern "C" void op1f_lfotrain_view(Op1fLfoTrainView* out) {
    ensure_store();
    u32 slot = gui_slot();
    out->open = lt.open;
    out->slot = (int)slot;
    out->lamp = store[slot].lamp;
    for (int p = 0; p < WAGONS; ++p) out->order[p] = store[slot].order[p];
    for (int k = 0; k < WAGONS; ++k) {
        out->kind[k] = wagon_kind(slot, k);
        out->on[k] = wagon_on(slot, k);
    }
    out->turn_knob = lt.turn_knob;
    out->turn_age = lt.turn_knob >= 0 ? (int)(lt.frames - lt.turn_frame) : 0x7fffffff;
    out->turn_move = lt.turn_move;
    out->notes = notes_started;
}

// ---------------------------------------------------------------- the GUI thread: what the presets ask (I8)

extern "C" const char* op1f_lfo4_kind_name(int kind) { return kind_name(kind); }

extern "C" int op1f_lfo4_default(int kind, int i) { return (kind >= 0 && kind < KINDS && i >= 0 && i < PARAMS) ? DEFAULTS[kind][i] : 0; }

// The presets list opens over the page: a knob held at that moment fires no hold under the list. Answers which
// knobs were down (a bit a knob), so the list can pass over the click stock makes of their release.
extern "C" int op1f_lfotrain_release_knobs() {
    int down = 0;
    for (int k = 0; k < WAGONS; ++k) {
        if (lt.pressed[k]) down |= 1 << k;
        lt.pressed[k] = 0;
        lt.acted[k] = 0;
    }
    return down;
}

// The four LFOs of the sound on the screen, as a preset holds them (build_i_lfo4.h): what the page shows for each
// knob, the wagon under the lamp read where stock keeps it. The effect wagon an LFO turns is written as the knob of
// the wagon it turns now (op1f_chain_lfo_knob: the chosen one while it carries an effect, else the one under the
// effect train's lamp), never as "the wagon under the lamp": a preset loaded onto another sound beside the effect
// preset it was made with then turns the same wagon, wherever that sound's effect lamp stands.
extern "C" void op1f_lfo4_preset_capture(int* values) {
    ensure_store();
    u32 slot = gui_slot();
    const SlotStore* s = &store[slot];
    for (int p = 0; p < WAGONS; ++p) values[p] = s->order[p];
    for (int k = 0; k < WAGONS; ++k) {
        int* v = values + WAGONS + k * OP1F_LFO_PRESET_WAGON_INTS;
        int kind = wagon_kind(slot, k);
        u8 pick = 0, midi[MIDI_DESTS] = { 0, 0, 0, 0 };
        u32 words[PARAMS];
        if (k == s->lamp) {
            pick = s->wagon[k].pick;                           // J5h: what the lamp's record keeps while the chain unit is away; with that unit there its own word takes its place
            for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = s->wagon[k].midi_pick[i];
            picks(slot, 0, &pick, midi);
            for (int i = 0; i < PARAMS; ++i) words[i] = RD32(LFO_BLOCK + 32u * slot + 4u * (u32)i);
        } else {
            const LfoWagon* w = &s->wagon[k];
            pick = w->pick;
            for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = w->midi_pick[i];
            for (int i = 0; i < PARAMS; ++i) words[i] = w->words[i];
        }
        v[0] = kind;
        v[1] = kind >= 0 ? wagon_on(slot, k) : 0;
        v[2] = kind >= 0 ? knob_of(slot, pick) + 1 : 0;
        for (int i = 0; i < MIDI_DESTS; ++i) v[3 + i] = kind == KIND_MIDI ? knob_of(slot, midi[i]) + 1 : 0;
        for (int i = 0; i < PARAMS; ++i) v[3 + MIDI_DESTS + i] = kind >= 0 ? (int)words[i] >> 16 : 0;
    }
}

// A preset's four LFOs onto the sound on the screen. The lamp stays on its knob; when the preset leaves that knob
// empty and another carries an LFO, it moves to the first one in the page's order, as a loaded effect preset moves
// the effect train's lamp off an empty wagon (H6g), so T4 opens the page of an LFO of the preset. The LFO of the
// lamp's knob goes onto stock's slot as move_lamp puts one there (the document's three keys, then the record's own
// LFO apply; an empty one is stock's LFO switched off), the three others into the store, and the instances are built
// again at the next frames. Every parameter is held to a half-word and to its knob's range as the descriptor row
// stands in memory (hold_param, as the store's load). Answers how many were held.
extern "C" int op1f_lfo4_preset_apply(const int* values) {
    ensure_store();
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    int held = 0, seen = 0, kinds[WAGONS];
    for (int p = 0; p < WAGONS; ++p) seen |= 1 << (values[p] & 3);
    for (int p = 0; p < WAGONS; ++p) s->order[p] = (u8)(seen == 15 ? (values[p] & 3) : p);   // anything but the four knobs once each: the knobs' own order
    for (int k = 0; k < WAGONS; ++k) {
        int kind = values[WAGONS + k * OP1F_LFO_PRESET_WAGON_INTS];
        kinds[k] = (kind >= 0 && kind < KINDS) ? kind : KIND_NONE;
    }
    int lamp = s->lamp;
    if (kinds[lamp] < 0) {
        for (int p = 0; p < WAGONS; ++p) {
            int k = s->order[p];
            if (kinds[k] >= 0) { lamp = k; break; }
        }
    }
    for (int k = 0; k < WAGONS; ++k) {
        const int* v = values + WAGONS + k * OP1F_LFO_PRESET_WAGON_INTS;
        LfoWagon* w = &s->wagon[k];
        w->kind = kinds[k];
        w->on = w->kind >= 0 && v[1] != 0;
        w->pick = (u8)((w->kind >= 0 && v[2] >= 0 && v[2] <= WAGONS) ? v[2] : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) w->midi_pick[i] = (u8)((w->kind == KIND_MIDI && v[3 + i] >= 0 && v[3 + i] <= WAGONS) ? v[3 + i] : 0);
        for (int i = 0; i < PARAMS; ++i) {
            int value = w->kind >= 0 ? v[3 + MIDI_DESTS + i] : 0;
            int kept = value < -32767 ? -32767 : (value > 32767 ? 32767 : value);
            if (w->kind >= 0) kept = hold_param(w->kind, i, kept);
            if (kept != value) ++held;
            w->words[i] = (u32)kept << 16;
        }
    }
    s->lamp = lamp;
    void* record = record_of(slot);
    LfoWagon* b = &s->wagon[lamp];
    if (b->kind >= 0) {
        set_text((u8*)record + 8, LFO_TYPE_KEY, *KIND[b->kind].name);
        STOCK_SET_LFO_WORDS_DOC(record, b->words);
        STOCK_SET_LFO_ACTIVE_DOC(record, b->on ? 1 : 0);
        ((RecordFn)(*(u32**)record)[RECORD_LFO_APPLY / 4])(record);
        picks(slot, 1, &b->pick, b->midi_pick);
    } else {
        u8 none = 0, nones[MIDI_DESTS] = { 0, 0, 0, 0 };
        STOCK_LFO_ON_SET(slot, 0);
        STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        picks(slot, 1, &none, nones);
    }
    lt.want = 1;
    touch();
    event_line("preset loaded", slot, lamp, b->kind, b->kind);
    return held;
}

// ---------------------------------------------------------------- the GUI thread: the frame

// Restoring shift-subtract division (no library routine). d == 0 yields 0.
static u32 udiv(u32 n, u32 d) {
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

struct Seen { u32 calls, cycles; };
static Seen seen_env, seen_eng, seen_fx;
static u32 seen_activity;

// calls, average and most of one timer since the last line; the most is cleared.
static void interval(Time* t, Seen* s, u32* out) {
    u32 calls = t->calls, cyc = t->cycles;
    u32 dcalls = calls - s->calls;
    out[0] = dcalls;
    out[1] = udiv(cyc - s->cycles, dcalls);
    out[2] = t->most;
    t->most = 0;
    s->calls = calls;
    s->cycles = cyc;
}

// The counts and the cost: half a second after the train's state last changed, when the shutdown countdown comes up,
// and every four seconds in which the LFOs were called (every half minute once thirty such summaries are out). Each
// of the three has a budget of its own, so the last lines of a session are never lost to the others (I5's lesson).
static void summary(u32 slot, const char* why, int budget) {
    static const u32 LIMIT[BUDGETS] = { SUMMARY_LINES, CHANGE_LINES, POPUP_LINES };
    if (lt.summary_lines[budget] >= LIMIT[budget]) return;
    lt.summary_lines[budget] += (u32)LINES_A_SUMMARY;
    STOCK_LOG(3, LOG_FILE, 2, "lfo4 (%s): s%u, %u running, %u built; calls: %u envelope, %u engine, %u fx blocks, %u note-ons; %u busy, %u bad voices",
              why, slot, op1f_lfo4_live, lt.built, counts.env, counts.eng, counts.fx, counts.note_ons, counts.busy, counts.bad_voice);
    u32 e[3], g[3], f[3], x[3];
    interval(&time_env, &seen_env, e);
    interval(&time_eng, &seen_eng, g);
    interval(&time_fx, &seen_fx, f);
    x[0] = x[1] = x[2] = 0;
    if (&op1f_lfofx_extra_counts != 0) op1f_lfofx_extra_counts(x);   // J5h: the chain unit's counts, when it is there
    STOCK_LOG(3, LOG_FILE, 3, "lfo4 time: env %u/%u over %u, eng %u/%u over %u, fx %u/%u over %u cycles (average/most a call, calls since the last line)",
              e[1], e[2], e[0], g[1], g[2], g[0], f[1], f[2], f[0]);
    STOCK_LOG(3, LOG_FILE, 4, "lfo4 fx side: %u wagon turns, %u parked or not built, %u on the scratch (the lamp's effect or another destination)", x[0], x[1], x[2]);
}

extern "C" void op1f_lfo4_frame() asm("op1f_lfo4_frame");
extern "C" void op1f_lfo4_frame() {
    ensure_store();
    if (&op1f_lfo_file_frame != 0) op1f_lfo_file_frame();   // J5h: the start's choice between the chain's copy and the own file, and the own file's write
    ++lt.frames;
    u32 slot = gui_slot();
    SlotStore* s = &store[slot];
    if (s->wagon[s->lamp].kind < 0 && stock_on(slot)) {        // the wagon under the lamp was emptied and stock's LFO is on again (a sound loaded, stock's own list): it carries that LFO
        s->wagon[s->lamp].kind = 0;
        touch();
    }
    if (lt.open && RD32(MODE_GUI) != 0u) lt.open = 0;          // another mode on the screen
    u32 now = now_ms();
    for (int k = 0; k < WAGONS; ++k) {                         // the holds
        if (lt.open && lt.pressed[k] && lt.armed[k] && !lt.acted[k] && now - lt.press_ms[k] >= (u32)HOLD_MS) {
            lt.acted[k] = 1;
            toggle(k);
        }
    }
    if ((int)slot != lt.built_slot) lt.want = 1;               // another sound on the screen: its LFOs
    if (lt.want) {
        if (lt.stage == 0) {                                   // taken away now, built at the next frame: no block can be inside an object that is being built
            take_away();
            lt.stage = 1;
        } else {
            lt.stage = 0;
            lt.want = 0;
            lt.built_slot = (int)slot;
            build_all(slot);
        }
    }
    int popup = RD8(POWER_POPUP_SHOWN) != 0;
    if (popup && !lt.popup_seen) summary(slot, "power popup", BY_POPUP);
    lt.popup_seen = popup;
    if (lt.summary_due && (i32)(lt.frames - lt.summary_at) >= 0 && lt.stage == 0 && !lt.want) {
        lt.summary_due = 0;
        summary(slot, "change", BY_CHANGE);
    }
    if (++lt.print_countdown >= PRINT_FRAMES) {
        lt.print_countdown = 0;
        u32 activity = counts.env + counts.eng + counts.fx + counts.note_ons;
        if (activity != seen_activity) {
            ++lt.summaries;
            if (lt.summaries <= (u32)FAST_SUMMARIES || (lt.summaries & 7u) == 0) {
                seen_activity = activity;
                summary(slot, "period", BY_PERIOD);
            }
        }
    }
}
