// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// The LFO unit: four LFOs per sound, on synth and drum sounds (lfo 12, step R15; the history before it: I6 to I8).
//
// The rule: four LFOs per sound, each stock's own LFO in every part. One of the four
// is the unit's own LFO of the sound as it is: its kind word, its eight words, its on-off byte, its document, its
// page. That is the wagon under the lamp (the searchlight). The three others are objects of stock's LFO classes that
// this unit builds at its own addresses and calls where stock calls its own, each with its own eight words and its own
// on-off. Image I5 proved such objects the same as stock's at every call (the study's 16.11).
//
// lfo 12: the four are the entry "ufo" in a sound's own list of LFO kinds, a layer of base 37 (module/build_r6e_card.h,
// "LFOs"). A sound is a place: 0 to 7 a synth slot, 8 to 15 a drum slot (8 + slot). The three extra LFOs play only
// while their place rides "ufo" (op1f_lfo4_rides, read once a frame from op1f_riding): a set of instances is built
// for the synth's slot on the screen and one for the drum's, each while its place rides, and taken away at the next
// frame when the ride ends. The store keeps all sixteen places whatever rides. A place's order integer carries
// OP1F_LFO4_NOT_RIDING more when it does not ride at a save; a load without it that is more than stock's one LFO
// makes a ride start due, which the pack's frame function starts (op1f_lfo4_ride_due, op1f_lfo4_ride_answer).
//
// The audio side, on the synth (I6, as it was):
//
//   * the envelope's part of a voice (the apply with the class 1) and, for the velocity kind, the level's method,
//     from the stub at 0x11a04334 (build_i_lfo4.s), after stock's own LFO has had its call;
//   * the engine's part of a voice (the class 0), from the stub at 0x11a045c0;
//   * the fx side (the class 2), from op1f_lfo4_block(0): fx train's chain run calls it, or the pack's own stub where
//     fx train is away; fx train's op1f_lfofx_extra aims each LFO at the effect wagon of its choice;
//   * the note-on, from a wrapper of the reset 0x8d504496 at its three calls.
//
// On the drum: the instances (eight voices, built with the synth's constructors and the drum's classes) and
// op1f_lfo4_drum_live; its calls are drum_block (op1f_lfo4_block(1)) and the drum's stubs.
//
// Every class is called for every LFO, as stock calls its own whatever the destination. Around each call the LFO's
// own copy of the words its kind keeps outside the object goes in and comes out again (KIND[].keep; the study's
// 14.4), so stock's LFO and its page see only their own. When a call moved the scratch, stock's page is sent its copy
// as stock sends it (0x8d504854, which does nothing while one is waiting to be read).
//
// The GUI side: the store, the instances, the move of the lamp (which hands stock another wagon's LFO through stock's
// own setters and its record's apply), the UFO page's grammar for a place (op1f_lfo4_key, op1f_lfo4_turn: a knob's
// turn picks the kind, shift + a turn moves it, a click moves the lamp there and asks for its stock page, a hold
// switches it off or on; T4 on the page, as the presets unit tells a tap, asks for the searchlight's stock page, and
// T4 there asks for the UFOs again), the pages wanted (op1f_lfo4_take_page, which the pack's frame function applies),
// and the log. The presets unit (I8) is asked first for every key of the page, as since I8.
//
// Threads: the stubs, the block call and the reset's wrapper run on the audio thread, which runs a block to its end
// before the GUI thread runs again; op1f_lfo4_key and op1f_lfo4_turn on the key thread (never for the keys 13 to 53,
// which may come on the audio thread); everything else on the display thread. The audio thread reads an instance
// through its published pointer only; the display thread takes the pointer away a frame before it builds. Nothing is
// logged from the audio thread. Nothing is divided (the module links no runtime).

#include "build_i_lfo4.h"
#include "build_r6e_card.h"

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
typedef void (*OnSetFn)(u32 slot, int value);                     // 0x8d504308 (synth), 0x8d5042d8 (drum): the slot's LFO on byte
typedef void (*SetWordsDocFn)(void* record, const u32* words);    // 0x8d4f3c84: the document's lfo_params (each word shifted right by sixteen), then the change notice
typedef void (*SetValueDocFn)(void* record, int value);           // 0x8d4f3d0c: the document's lfo_active, then the change notice
typedef void (*RecordFn)(void* record);                           // the record's virtuals
typedef void (*ControllerFn)(void* self);                         // the controller's virtuals without an argument
typedef void (*JsonCtorFn)(void* json, int kind);                 // 0x8c085ba8
typedef void (*JsonFromCstrFn)(void* json, const char* text);     // 0x8c084430
typedef void* (*JsonLookupFn)(void* json, const char* key);       // 0x8c085e84: the member, inserted when missing
typedef void (*JsonAssignFn)(void* target, void* value);          // 0x8c0845b8
typedef void (*JsonDtorFn)(void* json);                           // 0x8c0844e0

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_RESET_SYNTH ((ResetFn)0x8d504496u)               // (voice): the note-on of the voice's LFO object and of the fx-side one, while the slot's LFO is on
#define STOCK_RESET_DRUM ((ResetFn)0x8d50441au)                // (voice): the same for the drum (its objects 0x84a03868, its fx side [0x84a01574]), called at 0x8f0075fc and 0x8f008338
#define STOCK_PAGE_COPY ((PageCopyFn)0x8d504854u)
#define STOCK_LFO_PICK ((LfoPickFn)0x8d4f3a54u)
#define STOCK_LFO_ON_SET ((OnSetFn)0x8d504308u)
#define STOCK_DRUM_LFO_ON_SET ((OnSetFn)0x8d5042d8u)
#define STOCK_SET_LFO_WORDS_DOC ((SetWordsDocFn)0x8d4f3c84u)
#define STOCK_SET_LFO_ACTIVE_DOC ((SetValueDocFn)0x8d4f3d0cu)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))

enum { KINDS = 6, VOICES = 6, DRUM_VOICES = 8, OBJ_WORDS = 15, KEEP_WORDS = 12, KEEP_REGIONS = 2, PARAMS = 8, WAGONS = OP1F_LFO_WAGONS, MIDI_DESTS = 4 };
enum { PLACES = OP1F_LFO4_PLACES, DRUM_PLACE = OP1F_LFO4_DRUM_PLACE, HOSTS = 2 };   // a place: 0..7 a synth slot, 8..15 a drum slot; a host: 0 the synth, 1 the drum
enum { KIND_TREMOLO, KIND_VALUE, KIND_RANDOM, KIND_ELEMENT, KIND_MIDI, KIND_VELOCITY, KIND_NONE = -1 };
enum { CLASS_ENGINE = 0, CLASS_ENVELOPE = 1, CLASS_FX = 2 };   // the apply's third argument at the synth's three places
enum { CLASS_DRUM_VOICE = 3, CLASS_DRUM_SECOND = 4, CLASS_DRUM_FX = 5 };   // and at the drum's: a voice (0x8d503d5c), the fx-side object after the voices (0x11a04072), the fx site (0x11a0402c)
enum { FRAME_VOICE_NEXT = 0x30 };                              // stock's frame in the voice loop: the voice's number plus one at FP - 0x30 (0x11a0421c)
enum { KEY_T1 = 6, KEY_T4 = 9, KEY_NOTES_FIRST = 13, KEY_NOTES_LAST = 53, KEY_CLICK0 = 54, KEY_PRESS0 = 61, KEYS_KEPT = 128 };
enum { PAGE_LFO = 3, CONTROLLER_PAGE = 0x28, CONTROLLER_LFO_ONOFF = 0x58, RECORD_LFO_APPLY = 0x14 };
enum { ORDER_AS_KNOBS = 0xe4 };                                // the knob at each place, two bits a place: 0, 1, 2, 3
enum { HOLD_MS = 400, CLEAN_MS = 120, SWALLOW_MS = 300 };      // the effect train's own times (Build H's chain unit, H4b)
enum { RIDE_DELAY_FRAMES = 5, RIDE_TRY_FRAMES = 120, RIDE_LINES = 40 };   // 5: at least four of the base's frames also for a load fx train makes in its frame before this unit's (R15's review)   // a ride start after a load (design section 3)
enum { PRINT_FRAMES = 232, EVENT_LINES = 160, SUMMARY_LINES = 240, CHANGE_LINES = 240, POPUP_LINES = 60, FAST_SUMMARIES = 30, LINES_A_SUMMARY = 4, SETTLE_FRAMES = 30 };
enum { BY_PERIOD, BY_CHANGE, BY_POPUP, BUDGETS };              // a summary's budget: the three kinds never spend each other's lines

// The synth's places (OS 1.7.0).
static const u32 BUILT_SLOT = 0x84a01524u;                    // the sound slot the audio thread plays
static const u32 GUI_SLOT = 0x11904704u;                      // the synth slot on the screen
static const u32 LFO_ON = 0x84a01554u;                        // + slot: the slot's LFO is on (a byte)
static const u32 LFO_TYPE = 0x119035a0u;                      // + 4 x slot: the slot's LFO kind
static const u32 LFO_BLOCK = 0x119037e0u;                     // + 32 x slot: the slot's eight LFO words
static const u32 ENGINE_TYPE = 0x11904714u;                   // + 4 x slot: the slot's engine, for the engine page's copy (0x11a04c68)
static const u32 FX_TYPE = 0x11904794u;                       // + 4 x slot: the slot's effect, for the effect page's copy (0x11a054f0)
static const u32 SYNTH_RECORDS = 0x080235dcu;                 // + 52 x slot: the slot's record (its table at +0, its document at +8)
static const u32 SYNTH_CONTROLLER = 0x08018804u;              // the synth's controller, a static object: its first word its table (the LFO on-off at +0x58, the page word at +0x28)
// The drum's (OS 1.7.0, read 2026-11-24: the design's section 5). Its records' LFO apply is at the same place of
// their table, and the document setters, the pick and the page copy are the synth's.
static const u32 DRUM_BUILT_SLOT = 0x84a01528u;
static const u32 DRUM_GUI_SLOT = 0x11904708u;
static const u32 DRUM_LFO_ON = 0x84a01564u;
static const u32 DRUM_LFO_TYPE = 0x119035c0u;
static const u32 DRUM_LFO_BLOCK = 0x11903ae0u;
static const u32 DRUM_ENGINE_TYPE = 0x11904734u;
static const u32 DRUM_FX_TYPE = 0x119047b4u;
static const u32 DRUM_RECORDS = 0x0802377cu;
static const u32 DRUM_CONTROLLER = 0x080188f4u;
static const u32 CURRENT_MODE = 0x8400f978u;                  // the mode: 0 synth, 1 drum, 2 tape, 3 mixer (the base's `mode`)
static const u32 RANDOM_LATCH = 0x84026dc8u;                  // a byte the stage sets at its start (0x11a0413a): the first random LFO apply of the block takes it and leaves a value for the page (0x8d50bcb4)
static const u32 LFO_DESCRIPTORS = 0x840af940u;               // 48 bytes a kind, 6 a parameter: the lowest and highest value as signed half-words, filled at boot
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;             // 1 while the shutdown countdown is on the screen
static const char LOG_FILE[] = "op1f_lfo4";
static const char LFO_TYPE_KEY[] = "lfo_type";

// A host's places, by the host (0 the synth, 1 the drum).
struct Host { u32 gui_slot, built_slot, lfo_on, lfo_type, lfo_block, records, engine_type, fx_type, controller; };
static const Host HOST[HOSTS] = {
    { GUI_SLOT, BUILT_SLOT, LFO_ON, LFO_TYPE, LFO_BLOCK, SYNTH_RECORDS, ENGINE_TYPE, FX_TYPE, SYNTH_CONTROLLER },
    { DRUM_GUI_SLOT, DRUM_BUILT_SLOT, DRUM_LFO_ON, DRUM_LFO_TYPE, DRUM_LFO_BLOCK, DRUM_RECORDS, DRUM_ENGINE_TYPE, DRUM_FX_TYPE, DRUM_CONTROLLER },
};

// The six kinds, in stock's numbering (the study's 14.1): the constructors (this), the classes of a synth voice's
// object and of the fx-side one (the factories 0x8f00cfae and 0x8f00cf00 write the class after the constructor;
// the tremolo and midi constructors write their own, the same word), the object's size in words, and the words the
// kind keeps outside its objects (14.4), which each LFO must have for itself. A kind's name is stock's own and is read
// where stock keeps the six (packs L3): entries of eight bytes at 0x84d256c4, the name's address and the kind, in
// stock's numbering, filled by one of the constructors the start code runs before main (0x8fca6224). These are the
// names stock's lookup 0x8fca618c knows, so a name written into a sound's document is one it takes.
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
// A drum voice's object (the study's 14.1): built by the kind's synth constructor, then given the drum voice's class;
// the midi kind has a drum constructor of its own, which writes that class. 0: the synth voice's (the tremolo's one
// class serves both voices). The fx side is the synth's object. The drum's kinds keep no other words outside their
// objects than KIND[].keep (read 2026-11-24: the classes' apply and note-on are the synth's but for the midi kind's,
// 0x8d50a160 and the empty 0x8d509f28, which leave 0x08026818 alone, so its exchange there undoes itself).
static const u32 DRUM_CTOR[KINDS] = { 0, 0, 0, 0, 0x8d50b30au, 0 };
static const u32 DRUM_CLASS[KINDS] = { 0, 0x840ae3a0u, 0x840ae23cu, 0x840ae094u, 0x840adf30u, 0x840ae534u };
// A new LFO's eight parameters: content/op1_factory.json, lfo_types[].default_params (what stock's own pick takes
// from the catalog, 0x8d4f3a54).
static const i16 DEFAULTS[KINDS][PARAMS] = {
    { 16000, 0, 0, 16000, 0, 0, 0, 0 }, { 1000, 4000, 2000, 2000, 0, 0, 0, 0 }, { 2000, 4000, 2000, 24000, 0, 0, 0, 0 },
    { 4688, 9886, 2000, 10640, 0, 0, 0, 0 }, { 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 }, { 32767, 9886, 2000, 1024, 0, 0, 0, 0 } };

// ---------------------------------------------------------------- the store (the GUI thread's)

// A wagon that is not under the lamp: an LFO of the module's own. The wagon under the lamp is the unit's own LFO of
// the place, read where stock keeps it; of its entry here only the kind's sign is used (KIND_NONE: the owner emptied
// it, which holds while stock's LFO stays off).
struct LfoWagon {
    i32 kind;                                                  // 0..5, or KIND_NONE
    i32 on;
    u32 words[PARAMS];                                         // the eight parameters, as stock's block holds them
    u8 pick;                                                   // the effect wagon its effect destination turns: 0 the one under the effect train's lamp, 1..4 a knob's (as I2's choice)
    u8 midi_pick[MIDI_DESTS];                                  // the same for the midi kind's four destinations (as I4's)
};
struct SlotStore {
    i32 lamp;                                                  // the knob whose wagon is the unit's own LFO
    u8 order[WAGONS];                                          // I7: the knob at each place on the page, left to right
    LfoWagon wagon[WAGONS];
};
static SlotStore store[PLACES];                                // by the place: 0..7 the synth's slots, 8..15 the drum's
static int store_ready;

// lfo 12: the ride starts the loads ask for (design section 3), by the place.
static struct {
    u8 due[PLACES];
    u8 no_flag[PLACES];                                        // its start was given up: no flag is written for it until it rides or is loaded again (R15's review: else an old sound stayed on one LFO for good)
    u32 due_at[PLACES], first_due[PLACES];                    // the frame it is due from; the frame it was first due
    u32 started, given_up, lines;
} rd;

// ---------------------------------------------------------------- the instances (built by the GUI thread, run by the audio thread)

struct Inst {
    u32 voice[DRUM_VOICES][OBJ_WORDS];                         // the synth uses the first six
    u32 fx[OBJ_WORDS];
    u32 keep[KEEP_WORDS];                                      // its own copy of the kind's words outside the objects
    u32 block[PARAMS];
    i32 kind;
    u32 slot;
    u32 has_fx;
    u8 pick;
    u8 midi_pick[MIDI_DESTS];
};
static Inst inst[WAGONS];                                      // the synth's, by the wagon's knob; the lamp's is not used
static Inst* volatile active[WAGONS];                          // published: the audio thread reads the pointer once a use
extern "C" volatile u32 op1f_lfo4_live asm("op1f_lfo4_live");
volatile u32 op1f_lfo4_live;                                   // how many are published: the stubs' one test (build_i_lfo4.s)
static Inst dinst[WAGONS];                                     // lfo 12: the drum's, the same way
static Inst* volatile dactive[WAGONS];
extern "C" volatile u32 op1f_lfo4_drum_live asm("op1f_lfo4_drum_live");
volatile u32 op1f_lfo4_drum_live;                              // how many of the drum's are published: the drum stubs' one test
volatile u32 op1f_lfo4_rides;                                  // the ride bits (build_i_lfo4.h)

struct Time { volatile u32 calls, cycles, most; };
static Time time_env, time_eng, time_fx;                       // the whole of each call from the stub, when an LFO was called
static struct {
    volatile u32 env, eng, fx, note_ons, busy, bad_voice, blocks;
} counts;
static volatile u32 note_busy;                                 // inside the note-ons of either reset's wrapper: the kinds' words outside their objects are one set for both hosts
static volatile u32 notes_started;                             // I7: every call of the reset, for the page (the velocity kind's UFO)
static struct {
    volatile u32 s1, s2, s3, c4, fx, note_ons, busy, bad_voice;
} dcounts;                                                     // lfo 12: the drum's calls that reached an LFO of the module's
static volatile u32 drum_notes_started;                        // lfo 12: every call of the drum's reset, for a drum place's page

// The pack's card (build_r15_lfo_card.s): its ask 0 is the layer "ufo".
extern "C" const Op1fCard op1f_card_lfo OP1F_NAMED("op1f_card_lfo");

// fx train's chain unit. Since J5h each is optional: where fx train is away
// the LFOs work on the sound's one effect, with the answers of the wrappers below and of the LFOs' own file.
extern "C" int __attribute__((weak)) op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) asm("op1f_lfofx_extra");
extern "C" void __attribute__((weak)) op1f_lfofx_extra_counts(u32* out) asm("op1f_lfofx_extra_counts");
extern "C" u32 __attribute__((weak)) op1f_chain_ms() asm("op1f_chain_ms");
extern "C" void __attribute__((weak)) op1f_chain_touch(int host) asm("op1f_chain_touch");
extern "C" void __attribute__((weak)) op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) asm("op1f_chain_lfo_picks");
extern "C" int __attribute__((weak)) op1f_chain_lfo_knob(int host, u32 slot, int pick) asm("op1f_chain_lfo_knob");   // I8: the knob of the effect wagon a choice turns now
// The LFOs' own file unit: the clock where the chain's is away, and the settings files. Optional, so that a set
// without it still builds this unit.
extern "C" u32 __attribute__((weak)) op1f_lfo_alone_ms() asm("op1f_lfo_alone_ms");
extern "C" void __attribute__((weak)) op1f_lfo_file_touch() asm("op1f_lfo_file_touch");                              // the store changed: the own file follows
extern "C" void __attribute__((weak)) op1f_lfo_file_from_chain(int slot, const int* values) asm("op1f_lfo_file_from_chain");   // the chain unit handed a slot's integers over
extern "C" void __attribute__((weak)) op1f_lfo_file_to_chain(int slot, const int* values, int have) asm("op1f_lfo_file_to_chain");   // the chain unit collected a slot's integers for its file
extern "C" void __attribute__((weak)) op1f_lfo_file_frame() asm("op1f_lfo_file_frame");                              // once a frame: the start's choice between the two copies, the own file's write
// The presets unit (I8): the list over the page; asked first for every key of the page. -1: not the list's key.
extern "C" int __attribute__((weak)) op1f_lfopresets_key_down(int key, int shifted) asm("op1f_lfopresets_key_down");
extern "C" void __attribute__((weak)) op1f_lfopresets_key_up(int key) asm("op1f_lfopresets_key_up");
extern "C" int __attribute__((weak)) op1f_lfopresets_turn(int knob, int delta) asm("op1f_lfopresets_turn");

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

static int host_of(int place) { return place >= DRUM_PLACE ? 1 : 0; }
static u32 slot_of(int place) { return (u32)place & 7u; }

// lfo4-alone-begin
// J5h: what the chain unit does for this one, with an answer for when it is away. GUI thread, but for none of them.

// The millisecond clock of the holds: the chain's frame call keeps one; without it the LFOs' own frame call does.
static u32 now_ms() {
    return &op1f_chain_ms != 0 ? op1f_chain_ms() : (&op1f_lfo_alone_ms != 0 ? op1f_lfo_alone_ms() : 0u);
}

// A place's store changed: the chain's file of its host is written a moment later (its copy of the LFOs' integers
// rides in it), and the LFOs' own files after it.
static void touch(int place) {
    if (&op1f_chain_touch != 0) op1f_chain_touch(host_of(place));
    if (&op1f_lfo_file_touch != 0) op1f_lfo_file_touch();
}

// The effect wagon the unit's own LFO of a place turns, read or written in the chain unit. Without that unit there is
// one effect and no choice to make: a read leaves what the caller holds and a write goes nowhere, so every wagon's
// record, the lamp's too, keeps the choice it holds, wherever the lamp moves. The LFOs' own file puts the lamp's there
// at a start and carries it (op1f_lfo4_lamp_picks), and hands it to the chain unit when that unit is back.
static void picks(int place, int write, u8* pick, u8* midi) {
    if (&op1f_chain_lfo_picks != 0) op1f_chain_lfo_picks(host_of(place), slot_of(place), write, pick, midi);
}

// The knob of the effect wagon a choice turns now, for a preset. Without the chain unit: the knob the choice names,
// or the first for none (a sound as stock has it carries its one effect on the first wagon).
static int knob_of(int place, int pick) {
    if (&op1f_chain_lfo_knob != 0) return op1f_chain_lfo_knob(host_of(place), slot_of(place), pick);
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

// ---- lfo 12: the drum (the design's section 5). The stubs of build_r15_lfo4_drum.s call these from the drum's stage
// 0x11a03670 with the voice from R7 (0 to 7), each after the place stock calls its own LFO: S1 0x11a037c8 after the
// class 3 of kinds 1 to 5, S2 0x11a03858 after the velocity method, S3 0x11a0387a with both buffers filled (before
// stock's tremolo and the render), C4 0x11a03a48 after the class 4 on the fx-side object; F5 0x11a03e34 is
// op1f_lfo4_block(1). The kinds' words outside their objects are the synth's (read 2026-11-24): the drum voice's
// classes differ from the synth voice's in their destructors only, but for the midi kind, whose drum apply 0x8d50a160
// never touches the four half-words 0x08026818 (its exchange in swap_keep undoes itself) and whose note-on is empty.

static u32 drum_slot() { return RD32(HOST[1].built_slot) & 7u; }

// Class 3 on the voice's object of each drum LFO whose kind is the tremolo (tremolo 1) or another (0), and stock's page
// copy after a moved apply with the arguments 0x8d503d5c gives it: the drum slot and the synth's engine table at that
// slot, as stock reads it (0x8d503dba). 1 when an LFO was called.
static int drum_class3(u32 voice, int tremolo) {
    u32 slot = drum_slot();
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = dactive[k];
        if (in == 0 || in->slot != slot || (in->kind == KIND_TREMOLO) != (tremolo != 0)) continue;
        swap_keep(in);
        moved |= lfo_apply(in, in->voice[voice], CLASS_DRUM_VOICE);
        swap_keep(in);
        called = 1;
    }
    if (moved) STOCK_PAGE_COPY(slot, RD32(ENGINE_TYPE + 4u * slot), CLASS_DRUM_VOICE);   // as stock (0x8d503dcc)
    return called;
}

// S1: kinds 1 to 5, on the scratch that holds the voice's key words, read next for its pitch and handed to the drum
// engine's render (0x11a0387e).
extern "C" int op1f_lfo4_drum_s1(int voice) asm("op1f_lfo4_drum_s1");
extern "C" int op1f_lfo4_drum_s1(int voice) {
    if (op1f_lfo4_drum_live == 0) return 0;
    if (note_busy) { dcounts.busy = dcounts.busy + 1; return 0; }
    if ((u32)voice >= (u32)DRUM_VOICES) { dcounts.bad_voice = dcounts.bad_voice + 1; return 0; }
    int called = drum_class3((u32)voice, 0);
    if (called) dcounts.s1 = dcounts.s1 + 1;
    return called;
}

// S2: the velocity kind's method on the voice's level, through each such LFO. Stock calls the method whatever the kind
// (0x11a0411c), the five others answering the level as it came (0x8d50851c); it takes a signed half-word and answers
// in the low half, which the stub puts back into R2.
extern "C" int op1f_lfo4_drum_s2(int voice, int level) asm("op1f_lfo4_drum_s2");
extern "C" int op1f_lfo4_drum_s2(int voice, int level) {
    if (op1f_lfo4_drum_live == 0 || note_busy || (u32)voice >= (u32)DRUM_VOICES) return level;
    u32 slot = drum_slot();
    int called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = dactive[k];
        if (in == 0 || in->slot != slot || in->kind != KIND_VELOCITY) continue;
        u32* object = in->voice[voice];
        swap_keep(in);
        level = ((VelocityFn)((u32*)object[0])[4])(object, in->block, (i16)level);
        swap_keep(in);
        called = 1;
    }
    if (called) dcounts.s2 = dcounts.s2 + 1;
    return level;
}

// S3: the tremolo kind, on the pitch buffer 0x11903c04 just filled (as stock's own at 0x11a04104).
extern "C" void op1f_lfo4_drum_s3(int voice) asm("op1f_lfo4_drum_s3");
extern "C" void op1f_lfo4_drum_s3(int voice) {
    if (op1f_lfo4_drum_live == 0) return;
    if (note_busy) { dcounts.busy = dcounts.busy + 1; return; }
    if ((u32)voice >= (u32)DRUM_VOICES) { dcounts.bad_voice = dcounts.bad_voice + 1; return; }
    if (drum_class3((u32)voice, 1)) dcounts.s3 = dcounts.s3 + 1;
}

// C4, once a block after the voices: class 4 on the fx-side object of each LFO that has one, on the scratch as stock's
// left it (the words of 0x119038e0 + 32 x slot, which 0x8f00467c takes at 0x11a03c2e).
extern "C" void op1f_lfo4_drum_c4() asm("op1f_lfo4_drum_c4");
extern "C" void op1f_lfo4_drum_c4() {
    if (op1f_lfo4_drum_live == 0) return;
    if (note_busy) { dcounts.busy = dcounts.busy + 1; return; }
    u32 slot = drum_slot();
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = dactive[k];
        if (in == 0 || in->slot != slot || !in->has_fx) continue;
        swap_keep(in);
        moved |= lfo_apply(in, in->fx, CLASS_DRUM_SECOND);
        swap_keep(in);
        called = 1;
    }
    if (!called) return;
    if (moved) STOCK_PAGE_COPY(slot, 0, CLASS_DRUM_SECOND);   // as stock (0x11a04090)
    dcounts.c4 = dcounts.c4 + 1;
}

// op1f_lfo4_block(1), once a block: class 5 on the fx-side object of each LFO that has one. With fx train's units there,
// from their chain's run at the drum's end site (0x11a03e68), on the effect wagon of the LFO's choice; without them,
// from the stub at 0x11a03e34, stock's apply on the scratch as it stands (the fx words 0x119039e0 + 32 x slot), for
// the sound's one effect, which reads it next (0x11a03ff8).
static void drum_block() {
    if (op1f_lfo4_drum_live == 0) return;
    if (note_busy) { dcounts.busy = dcounts.busy + 1; return; }
    u32 slot = drum_slot();
    int moved = 0, called = 0;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = dactive[k];
        if (in == 0 || in->slot != slot || !in->has_fx) continue;
        u8 latch = 0;
        if (in->kind == KIND_RANDOM) { latch = RD8(RANDOM_LATCH); *(volatile u8*)RANDOM_LATCH = 0; }
        swap_keep(in);
        if (&op1f_lfofx_extra != 0) moved |= op1f_lfofx_extra(1, in->fx, in->block, in->pick, in->midi_pick);
        else moved |= ((ApplyFn)((u32*)in->fx[0])[2])(in->fx, in->block, CLASS_DRUM_FX);
        swap_keep(in);
        if (in->kind == KIND_RANDOM) *(volatile u8*)RANDOM_LATCH = latch;
        called = 1;
    }
    if (!called) return;
    if (moved) STOCK_PAGE_COPY(slot, RD32(HOST[1].fx_type + 4u * slot), CLASS_DRUM_FX);   // as stock (0x11a04058)
    dcounts.fx = dcounts.fx + 1;
}

// The chain's run calls this first, once a block of the second stage (or the pack's own stub where fx train is
// away): the fx side of each LFO, on the effect wagon of its choice. Host 1 is the drum's.
extern "C" void op1f_lfo4_block(int host) asm("op1f_lfo4_block");
extern "C" void op1f_lfo4_block(int host) {
    if (host == 1) { drum_block(); return; }
    if (host != 0 || op1f_lfo4_live == 0) return;
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

// lfo 12: in place of the drum's reset 0x8d50441a at its two calls (0x8f0075fc; 0x8f008338, whose caller has checked
// the voice under 8): stock's reset as it is, then the note-on of each drum LFO of the module's for the same voice,
// its voice object and then its fx-side one, as stock's reset does for its own.
extern "C" void op1f_lfo4_reset_drum(u32 voice) asm("op1f_lfo4_reset_drum");
extern "C" void op1f_lfo4_reset_drum(u32 voice) {
    STOCK_RESET_DRUM(voice);
    drum_notes_started = drum_notes_started + 1;
    if (op1f_lfo4_drum_live == 0 || voice >= (u32)DRUM_VOICES) return;
    u32 slot = drum_slot();
    note_busy = 1;
    for (int k = 0; k < WAGONS; ++k) {
        Inst* in = dactive[k];
        if (in == 0 || in->slot != slot) continue;
        swap_keep(in);
        ((NoteOnFn)((u32*)in->voice[voice][0])[3])(in->voice[voice], in->block);
        if (in->has_fx) ((NoteOnFn)((u32*)in->fx[0])[3])(in->fx, in->block);
        swap_keep(in);
    }
    note_busy = 0;
    dcounts.note_ons = dcounts.note_ons + 1;
}

// ---------------------------------------------------------------- the GUI thread: the store

static struct {
    int open, open_place;                                      // the UFO page is on the screen (op1f_lfo4_shown), and for which place it came up
    u32 frames;
    u32 rides_seen;                                            // the ride bits at the last frame: a change touches the place's files
    int want[HOSTS], stage[HOSTS];                             // a host's instances are to be built again: 0 take them away, 1 build and publish
    int built_place[HOSTS];                                    // the place they stand for, or -1
    u32 built;                                                 // instances built since the start
    volatile int page_want;                                    // the page asked for since the last op1f_lfo4_take_page
    int choice;                                                // the page last asked for (OP1F_LFO4_WANT_OWN or _UNIT)
    u32 down_taken[KEYS_KEPT / 32];                            // the keys whose down was taken: their up is too
    int pressed[WAGONS], armed[WAGONS], acted[WAGONS];
    u32 press_ms[WAGONS], last_turn_ms[WAGONS], swallow_until[WAGONS];
    int turn_knob, turn_move;
    u32 turn_frame;
    u32 picks, lamp_moves, holds, taps, opens, page_opens, backs, moves, loads;
    u32 event_lines, summary_lines[BUDGETS], summaries, summary_at;
    int print_countdown, summary_due, popup_seen;
} lt;

static int valid(int place) { return place >= 0 && place < PLACES; }
static int riding(int place) { return valid(place) && ((op1f_lfo4_rides >> place) & 1u) != 0; }
static const Host* host_at(int place) { return &HOST[host_of(place)]; }
static int gui_place(int host) { return (host ? DRUM_PLACE : 0) + (int)(RD32(HOST[host].gui_slot) & 7u); }
static int stock_on(int place) { return RD8(host_at(place)->lfo_on + slot_of(place)) != 0; }
static int stock_kind(int place) { u32 k = RD32(host_at(place)->lfo_type + 4u * slot_of(place)); return k < (u32)KINDS ? (int)k : KIND_NONE; }   // 6: a name stock's lookup did not know (0x8fca618c)
static u32 stock_word(int place, int i) { return RD32(host_at(place)->lfo_block + 32u * slot_of(place) + 4u * (u32)i); }
static void* record_of(int place) { return (void*)(host_at(place)->records + 52u * slot_of(place)); }
static void* controller_of(int place) { return (void*)host_at(place)->controller; }

static void lfo_on_set(int place, int value) {
    if (host_of(place)) STOCK_DRUM_LFO_ON_SET(slot_of(place), value);
    else STOCK_LFO_ON_SET(slot_of(place), value);
}

// Every place starts as stock has it: its one LFO on the first wagon, under the lamp, the three others empty.
static void ensure_store() {
    if (store_ready) return;
    store_ready = 1;
    for (int h = 0; h < HOSTS; ++h) lt.built_place[h] = -1;
    lt.open_place = -1;
    lt.turn_knob = -1;
    lt.choice = OP1F_LFO4_WANT_OWN;
    for (int s = 0; s < PLACES; ++s) {
        store[s].lamp = 0;
        for (int k = 0; k < WAGONS; ++k) store[s].wagon[k].kind = k == 0 ? 0 : KIND_NONE;
        for (int k = 0; k < WAGONS; ++k) store[s].order[k] = (u8)k;
    }
}

// A wagon's kind and on-off as the page shows them: the unit's own for the wagon under the lamp.
static int wagon_kind(int place, int k) {
    const SlotStore* s = &store[place];
    if (k != s->lamp) return s->wagon[k].kind;
    return (s->wagon[k].kind < 0 && !stock_on(place)) ? KIND_NONE : stock_kind(place);
}

static int wagon_on(int place, int k) {
    if (wagon_kind(place, k) < 0) return 0;
    return k == store[place].lamp ? stock_on(place) : (store[place].wagon[k].on != 0);
}

static const char* kind_name(int kind) { return (kind >= 0 && kind < KINDS) ? *KIND[kind].name : "none"; }
static char host_letter(int place) { return host_of(place) ? 'd' : 's'; }

// A stored parameter held to its knob's range as the descriptor row stands in memory (the rows are filled at boot;
// an entry not filled holds nothing), as the chain unit holds an effect's (H6j).
static int hold_param(int kind, int i, int value) {
    const volatile i16* entry = (const volatile i16*)(LFO_DESCRIPTORS + 48u * (u32)kind + 6u * (u32)i);
    int lo = entry[0], hi = entry[1];
    if (lo >= hi) return value;
    return value < lo ? lo : (value > hi ? hi : value);
}

static int packed_order(const SlotStore* s) {
    int packed = 0;
    for (int k = 0; k < WAGONS; ++k) packed |= (int)s->order[k] << (2 * k);
    return packed;
}

// A place as stock alone has it: the unit's one LFO on the first wagon (not emptied), the three others empty, the
// knobs' own order. Nothing is written for such a sound, and its load asks for no ride.
static int plain_store(const SlotStore* s) {
    int plain = s->lamp == 0 && s->wagon[0].kind >= 0 && packed_order(s) == ORDER_AS_KNOBS;
    for (int k = 1; k < WAGONS; ++k) if (s->wagon[k].kind >= 0) plain = 0;
    return plain;
}

// A place's integers (build_i_lfo4.h) into the store: the plain setter, which the LFOs' own files use too. lfo 12: the
// order integer's flag "kept, not riding" is kept apart; a store that is more than stock's one LFO and has no flag
// makes a ride start due for its place, RIDE_DELAY_FRAMES frames on (design section 3).
extern "C" void op1f_lfo4_store_set(int slot, const int* values) asm("op1f_lfo4_store_set");
extern "C" void op1f_lfo4_store_set(int slot, const int* values) {
    ensure_store();
    if (slot < 0 || slot >= PLACES) return;
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
    int packed = values[OP1F_LFO4_STORE_MIN], seen = 0;        // I7: the order; anything but the four knobs once each reads as the knobs' own order (a file of I6's has none: -1)
    int kept = packed >= OP1F_LFO4_NOT_RIDING && packed < 2 * OP1F_LFO4_NOT_RIDING;   // lfo 12: the flag, only on an order of 256..511
    if (kept) packed -= OP1F_LFO4_NOT_RIDING;
    for (int k = 0; k < WAGONS; ++k) seen |= 1 << ((packed >> (2 * k)) & 3);
    if (packed < 0 || packed > 255 || seen != 15) packed = ORDER_AS_KNOBS;
    for (int k = 0; k < WAGONS; ++k) s->order[k] = (u8)((packed >> (2 * k)) & 3);
    ++lt.loads;
    lt.want[host_of(slot)] = 1;
    rd.due[slot] = 0;
    rd.no_flag[slot] = 0;
    if (!kept && !plain_store(s)) {
        rd.due[slot] = 1;
        rd.due_at[slot] = rd.first_due[slot] = lt.frames + (u32)RIDE_DELAY_FRAMES;
    }
}

// The chain unit hands a synth slot's integers over: from its file at its load, or from an instrument that was
// loaded. J5h: the LFOs' own file is told first (at the start it holds the chain's copy against its own; later it
// follows).
extern "C" void op1f_lfo4_store_load(int slot, const int* values) asm("op1f_lfo4_store_load");
extern "C" void op1f_lfo4_store_load(int slot, const int* values) {
    if (&op1f_lfo_file_from_chain != 0) op1f_lfo_file_from_chain(slot, values);
    op1f_lfo4_store_set(slot, values);
}

// The store's integers for a place. 0 while the place is as stock has it (nothing is written for such a sound). The
// plain getter, which the LFOs' own files use too. lfo 12: the order carries OP1F_LFO4_NOT_RIDING more when the place
// does not ride "ufo" and no ride start is due for it.
extern "C" int op1f_lfo4_store_get(int slot, int* values) asm("op1f_lfo4_store_get");
extern "C" int op1f_lfo4_store_get(int slot, int* values) {
    ensure_store();
    if (slot < 0 || slot >= PLACES) return 0;
    const SlotStore* s = &store[slot];
    if (plain_store(s)) return 0;
    values[0] = s->lamp;
    values[OP1F_LFO4_STORE_MIN] = packed_order(s) + (riding(slot) || rd.due[slot] || rd.no_flag[slot] ? 0 : (int)OP1F_LFO4_NOT_RIDING);
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

// The chain unit collects a synth slot's integers: for its file at its save, or for an instrument that is saved.
// J5h: the LFOs' own file is told what the chain's copy now holds.
extern "C" int op1f_lfo4_store_save(int slot, int* values) asm("op1f_lfo4_store_save");
extern "C" int op1f_lfo4_store_save(int slot, int* values) {
    int have = op1f_lfo4_store_get(slot, values);
    if (&op1f_lfo_file_to_chain != 0) op1f_lfo_file_to_chain(slot, values, have);
    return have;
}

// J5h: the effect wagon the LFO under the lamp turns, and its midi kind's four, as this unit's record of the lamp's
// wagon holds them: read, or written, for a place 0..15. With the chain unit there they are that unit's to keep
// (picks(), its tags 96 and 97) and this record is not looked at. While it is away nothing moves them, so the record
// is where they stay: the LFOs' own files put them here at a start and carry them.
extern "C" void op1f_lfo4_lamp_picks(int slot, int write, u8* pick, u8* midi) asm("op1f_lfo4_lamp_picks");
extern "C" void op1f_lfo4_lamp_picks(int slot, int write, u8* pick, u8* midi) {
    ensure_store();
    if (slot < 0 || slot >= PLACES) return;
    LfoWagon* w = &store[slot].wagon[store[slot].lamp];
    if (write) {
        w->pick = (u8)(*pick <= WAGONS ? *pick : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) w->midi_pick[i] = (u8)(midi[i] <= WAGONS ? midi[i] : 0);
    } else {
        *pick = w->pick;
        for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = w->midi_pick[i];
    }
}

// ---------------------------------------------------------------- the GUI thread: the ride starts (design section 3)

static void ride_line(int place, const char* what, int answer) {
    if (rd.lines >= (u32)RIDE_LINES) return;
    ++rd.lines;
    STOCK_LOG(3, LOG_FILE, 5, "lfo ufo: %c%u: the ride a load asked for %s (answer %d, %u frames after it was due; %u started, %u given up)",
              host_letter(place), slot_of(place), what, answer, lt.frames - rd.first_due[place], rd.started, rd.given_up);
}

// A place whose ride the pack's frame function should start now (op1f_layer_start), or -1. A place that rides
// already needs none.
extern "C" int op1f_lfo4_ride_due() {
    for (int p = 0; p < PLACES; ++p) {
        if (!rd.due[p]) continue;
        if (riding(p)) { rd.due[p] = 0; continue; }
        if ((i32)(lt.frames - rd.due_at[p]) >= 0) return p;
    }
    return -1;
}

// op1f_layer_start's answer for that place: 0 it rides; OP1F_NOT_NOW due again at the next frame, until
// RIDE_TRY_FRAMES frames after it was first due; anything else given up.
extern "C" void op1f_lfo4_ride_answer(int place, int answer) {
    if (!valid(place) || !rd.due[place]) return;
    if (answer == 0) {
        rd.due[place] = 0;
        ++rd.started;
        ride_line(place, "started", answer);
        return;
    }
    if (answer == OP1F_NOT_NOW && lt.frames - rd.first_due[place] < (u32)RIDE_TRY_FRAMES) {
        rd.due_at[place] = lt.frames + 1u;
        return;
    }
    rd.due[place] = 0;
    rd.no_flag[place] = 1;
    ++rd.given_up;
    ride_line(place, "given up", answer);
}

// ---------------------------------------------------------------- the GUI thread: the instances

static void take_away(int host) {
    if (host) {
        op1f_lfo4_drum_live = 0;
        for (int k = 0; k < WAGONS; ++k) dactive[k] = 0;
    } else {
        op1f_lfo4_live = 0;
        for (int k = 0; k < WAGONS; ++k) active[k] = 0;
    }
}

// One LFO of the module's own for a place: a voice's object for each of the host's voices (six on the synth, eight on
// the drum) and, where the kind has one, the fx-side object, built from zeroed memory by stock's constructors with the
// class the factory gives (I5's read-out: the same words as stock's own leave). The words its kind keeps outside the
// objects start as they stand (a state stock's code made).
// The wagon's kind is read once (R15's review: the key thread may change it while a set is built): a kind that is
// none by then builds nothing (answers 0).
static int build_one(int host, int k, u32 slot, const LfoWagon* w) {
    Inst* in = host ? &dinst[k] : &inst[k];
    int kind_now = *(const volatile i32*)&w->kind;
    if (kind_now < 0 || kind_now >= KINDS) return 0;
    const Kind* kind = &KIND[kind_now];
    int voices = host ? (int)DRUM_VOICES : (int)VOICES;
    u32 ctor = (host && DRUM_CTOR[kind_now]) ? DRUM_CTOR[kind_now] : kind->ctor_voice;
    u32 vtable = (host && DRUM_CLASS[kind_now]) ? DRUM_CLASS[kind_now] : kind->vtable_voice;
    for (int v = 0; v < voices; ++v) {
        for (int i = 0; i < OBJ_WORDS; ++i) in->voice[v][i] = 0;
        ((CtorFn)ctor)(in->voice[v]);
        in->voice[v][0] = vtable;
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
    in->kind = kind_now;
    in->slot = slot;
    in->has_fx = kind->vtable_fx != 0;
    in->pick = w->pick;
    for (int i = 0; i < MIDI_DESTS; ++i) in->midi_pick[i] = w->midi_pick[i];
    ++lt.built;
    return 1;
}

static void build_all(int host, int place) {
    const SlotStore* s = &store[place];
    u32 live = 0;
    for (int k = 0; k < WAGONS; ++k) {
        const LfoWagon* w = &s->wagon[k];
        if (k == s->lamp || w->kind < 0 || !w->on) continue;
        if (!build_one(host, k, slot_of(place), w)) continue;
        if (host) dactive[k] = &dinst[k]; else active[k] = &inst[k];
        ++live;
    }
    if (host) op1f_lfo4_drum_live = live; else op1f_lfo4_live = live;
}

// ---------------------------------------------------------------- the GUI thread: the page's grammar

static void event_line(const char* what, int place, int knob, int a, int b) {
    lt.summary_due = 1;
    lt.summary_at = lt.frames + (u32)SETTLE_FRAMES;                 // one summary for a run of changes: half a second after the last
    if (lt.event_lines >= (u32)EVENT_LINES) return;
    ++lt.event_lines;
    STOCK_LOG(3, LOG_FILE, 1, "lfo ufo: %c%u knob %d %s: %s -> %s; lamp on knob %d, wagons %s/%s/%s/%s, on %d%d%d%d",
              host_letter(place), slot_of(place), knob, what, kind_name(a), kind_name(b), store[place].lamp,
              kind_name(wagon_kind(place, 0)), kind_name(wagon_kind(place, 1)), kind_name(wagon_kind(place, 2)), kind_name(wagon_kind(place, 3)),
              wagon_on(place, 0), wagon_on(place, 1), wagon_on(place, 2), wagon_on(place, 3));
}

static void set_text(void* doc, const char* key, const char* text) {
    u32 item[4];
    STOCK_JSON_CTOR(item, 0);
    STOCK_JSON_FROM_CSTR(item, text);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(doc, key), item);
    STOCK_JSON_DTOR(item);
}

static void want_page(int which) {
    lt.choice = which;
    lt.page_want = which;
}

// A turn of a wagon's knob: the kind before or after, none below tremolo. Under the lamp the pick is stock's own
// (the catalog's entry onto the record, which switches nothing off; the LFO is switched on, as stock's list does),
// and none is the unit's LFO switched off; the ride goes on either way. On another wagon a new LFO starts as stock
// starts one: the catalog's defaults, switched on, its effect destination on the wagon under the effect train's lamp.
static void pick_kind(int place, int k, int delta) {
    SlotStore* s = &store[place];
    int now = wagon_kind(place, k);
    int next = now + (delta > 0 ? 1 : -1);
    if (next < KIND_NONE || next >= KINDS) return;             // the ends of the list
    LfoWagon* w = &s->wagon[k];
    if (k == s->lamp) {
        void* record = record_of(place);
        if (next < 0) {
            lfo_on_set(place, 0);
            STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        } else {
            STOCK_LFO_PICK(record, next + 1);                  // the catalog's ids are the kinds plus one (content/op1_factory.json)
            lfo_on_set(place, 1);
            STOCK_SET_LFO_ACTIVE_DOC(record, 1);
        }
        w->kind = next;
    } else {
        w->kind = next;
        w->on = next >= 0;
        w->pick = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) w->midi_pick[i] = 0;
        for (int i = 0; i < PARAMS; ++i) w->words[i] = next >= 0 ? (u32)(i32)DEFAULTS[next][i] << 16 : 0;
        lt.want[host_of(place)] = 1;
    }
    ++lt.picks;
    touch(place);
    event_line("turned", place, k, now, next);
}

// Shift + a turn: the UFO to the next place on the page. The LFOs run side by side, so the order is the page's alone.
static void move_place(int place, int k, int delta) {
    SlotStore* s = &store[place];
    int at = 0;
    for (int p = 0; p < WAGONS; ++p) if (s->order[p] == (u8)k) at = p;
    int to = at + (delta > 0 ? 1 : -1);
    if (to < 0 || to >= WAGONS) return;
    s->order[at] = s->order[to];
    s->order[to] = (u8)k;
    ++lt.moves;
    touch(place);
    int kind = wagon_kind(place, k);
    event_line(delta > 0 ? "moved right" : "moved left", place, k, kind, kind);
}

// The hold: the wagon's LFO off or on. Under the lamp it is the unit's own on-off: the controller's virtual on the
// host's static controller (it writes the byte and the document, for the slot on the screen); an empty wagon has
// nothing to switch.
static void toggle(int place, int k) {
    SlotStore* s = &store[place];
    int kind = wagon_kind(place, k);
    if (kind < 0) return;
    if (k == s->lamp) {
        void* self = controller_of(place);
        ((ControllerFn)(*(u32**)self)[CONTROLLER_LFO_ONOFF / 4])(self);
    } else {
        s->wagon[k].on = !s->wagon[k].on;
        lt.want[host_of(place)] = 1;
    }
    ++lt.holds;
    touch(place);
    event_line(wagon_on(place, k) ? "held, on" : "held, off", place, k, kind, kind);
}

// The lamp moves to a wagon. What the unit holds goes into the record of the wagon the lamp leaves, which becomes one
// of the module's three; the other wagon's LFO goes onto the unit's slot through the document (lfo_type as the kind's
// name through the json string setter, the key having no setter of its own, as fx_type; lfo_params and lfo_active
// through stock's two setters) and the record's own LFO apply, which is how stock puts a document's LFO on the stage
// (0x8d4f64c0: the kind word, the eight words, the on byte, the objects built). An empty wagon under the lamp is the
// unit's LFO switched off.
static void move_lamp(int place, int to) {
    SlotStore* s = &store[place];
    int from = s->lamp;
    if (to == from) return;
    void* record = record_of(place);
    LfoWagon* a = &s->wagon[from];
    int a_kind = wagon_kind(place, from);
    a->kind = a_kind;
    a->on = stock_on(place);
    for (int i = 0; i < PARAMS; ++i) a->words[i] = stock_word(place, i);
    picks(place, 0, &a->pick, a->midi_pick);
    LfoWagon* b = &s->wagon[to];
    if (b->kind >= 0) {
        set_text((u8*)record + 8, LFO_TYPE_KEY, *KIND[b->kind].name);
        STOCK_SET_LFO_WORDS_DOC(record, b->words);
        STOCK_SET_LFO_ACTIVE_DOC(record, b->on ? 1 : 0);
        ((RecordFn)(*(u32**)record)[RECORD_LFO_APPLY / 4])(record);
        picks(place, 1, &b->pick, b->midi_pick);
    } else {
        u8 none = 0, nones[MIDI_DESTS] = { 0, 0, 0, 0 };
        lfo_on_set(place, 0);
        STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        picks(place, 1, &none, nones);
    }
    s->lamp = to;
    lt.want[host_of(place)] = 1;
    ++lt.lamp_moves;
    touch(place);
    event_line("pressed, the lamp moved here", place, to, a_kind, b->kind);
}

// A knob's click on the page: the lamp moves to that wagon and its stock page is asked for. An empty wagon: nothing
// (the unit's list of kinds, which lfo 11 opened for it, would end the ride).
static void click(int place, int k) {
    if (wagon_kind(place, k) < 0) return;
    if (k != store[place].lamp) move_lamp(place, k);
    ++lt.page_opens;
    want_page(OP1F_LFO4_WANT_UNIT);
}

// T4 tapped on the UFO page (the presets unit tells it; without that unit the page's own T4): the stock page of the
// searchlight's LFO, when it has a kind.
extern "C" void op1f_lfo4_open_searchlight(int place) {
    ensure_store();
    if (!valid(place) || wagon_kind(place, store[place].lamp) < 0) return;
    ++lt.page_opens;
    want_page(OP1F_LFO4_WANT_UNIT);
}

// A key-down on the UFO page (design section 4): the presets unit first (its list, its save, T4's tap and hold), then
// the page's own. 0 for T1 to T3 (stock leaves the page) and for every key the page does not use.
static int page_key(int place, int key, int shifted) {
    if (&op1f_lfopresets_key_down) {
        int taken = op1f_lfopresets_key_down(key, shifted);
        if (taken >= 0) return taken != 0;
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
        if (!shifted) { ++lt.taps; click(place, k); }
        return 1;
    }
    if (key == KEY_T4) {                                       // the presets unit away (or leaving T4): the down is the tap
        if (!shifted) op1f_lfo4_open_searchlight(place);
        return 1;
    }
    return 0;
}

// Not on the UFO page: T4 down while the stock page of the searchlight's LFO shows after a tap (the page asked for
// last is the unit's, and the host's controller stands on its LFO page): the UFOs again.
static int back_to_own(int place) {
    if (lt.choice != OP1F_LFO4_WANT_UNIT) return 0;
    if (RD32(host_at(place)->controller + (u32)CONTROLLER_PAGE) != (u32)PAGE_LFO) return 0;   // T1 to T3 on the screen: stock's T4 brings the UFOs (the base's page for a ride)
    ++lt.backs;
    want_page(OP1F_LFO4_WANT_OWN);
    return 1;
}

// A key for a place (what: OP1F_DOWN, OP1F_DOWN_SHIFT, OP1F_UP), on the key thread: 1 taken. The keys 13 to 53 (the
// keyboard, shift) are never the page's: they may come on the audio thread. A place that does not ride "ufo": 0 (the
// up of a down taken before its ride ended is still taken). Shift + T4: 0 always (the unit's list of kinds).
extern "C" int op1f_lfo4_key(int place, int key, int what) {
    if (key < 0 || key >= KEYS_KEPT || (key >= KEY_NOTES_FIRST && key <= KEY_NOTES_LAST)) return 0;
    u32 bit = 1u << (key & 31);
    u32* taken = &lt.down_taken[key >> 5];
    if (what == OP1F_UP) {
        int mine = (*taken & bit) != 0;
        *taken &= ~bit;
        if (&op1f_lfopresets_key_up) op1f_lfopresets_key_up(key);   // I8: T4 let go (its tap), and the list's own knobs
        if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) {
            int k = key - KEY_PRESS0;
            if (lt.pressed[k] && lt.acted[k]) lt.swallow_until[k] = now_ms() + (u32)SWALLOW_MS;   // the click that follows a hold is not a short press
            lt.pressed[k] = 0;
        }
        return mine;
    }
    if ((what != OP1F_DOWN && what != OP1F_DOWN_SHIFT) || !riding(place)) return 0;
    ensure_store();
    int shifted = what == OP1F_DOWN_SHIFT;
    int answer;
    if (shifted && key == KEY_T4) answer = 0;
    else if (lt.open) answer = page_key(place, key, shifted);
    else answer = key == KEY_T4 && !shifted && back_to_own(place);
    if (answer) *taken |= bit;
    return answer;
}

// A knob turned on the UFO page of a place (knob 0..3; shifted: the move), on the key thread: 1 taken.
extern "C" int op1f_lfo4_turn(int place, int knob, int delta, int shifted) {
    if (!riding(place) || !lt.open || knob < 0 || knob >= WAGONS) return 0;
    ensure_store();
    if (delta == 0) return 1;
    if (&op1f_lfopresets_turn && op1f_lfopresets_turn(knob, delta)) return 1;   // I8: the list's scroll, caret and letter
    int k = knob;
    if (lt.pressed[k]) lt.acted[k] = 1;                        // a turn while pressed: no hold (stock sends no click either)
    lt.last_turn_ms[k] = now_ms();
    if (shifted) move_place(place, k, delta);
    else pick_kind(place, k, delta);
    lt.turn_knob = k;
    lt.turn_frame = lt.frames;
    lt.turn_move = shifted != 0;
    return 1;
}

// The page asked for since the last read (0 nothing, OP1F_LFO4_WANT_OWN, OP1F_LFO4_WANT_UNIT), for lfo_frame's
// op1f_layer_page. Cleared by the read.
extern "C" int op1f_lfo4_take_page() {
    unsigned was = op1f_mask();                                // the key thread writes the wish: read and cleared as one (R15's review)
    int w = lt.page_want;
    lt.page_want = OP1F_LFO4_WANT_NONE;
    op1f_unmask(was);
    return w;
}

// The base's `shown` of the layer's page: up for the place, or gone. While it is up the page's keys are this unit's.
extern "C" void op1f_lfo4_shown(int place, int up) {
    if (up) {
        if (!lt.open) ++lt.opens;
        lt.open = 1;
        lt.open_place = place;
        lt.choice = OP1F_LFO4_WANT_OWN;                         // the UFO page is the one the base shows
    } else {
        lt.open = 0;
    }
}

// The place on the screen: the synth's slot in the synth mode, 8 + the drum's slot in the drum mode; -1 in another.
extern "C" int op1f_lfo4_place() {
    u32 mode = RD32(CURRENT_MODE);
    if (mode == 0u) return gui_place(0);
    if (mode == 1u) return gui_place(1);
    return -1;
}

// The place a preset acts on: the one on the screen, or the synth's in another mode.
static int screen_place() {
    int place = op1f_lfo4_place();
    return place >= 0 ? place : gui_place(0);
}

extern "C" void op1f_lfo4_view(int place, Op1fLfoTrainView* out) {
    ensure_store();
    if (!valid(place)) place = gui_place(0);
    out->open = lt.open;
    out->slot = place;
    out->lamp = store[place].lamp;
    for (int p = 0; p < WAGONS; ++p) out->order[p] = store[place].order[p];
    for (int k = 0; k < WAGONS; ++k) {
        out->kind[k] = wagon_kind(place, k);
        out->on[k] = wagon_on(place, k);
    }
    out->turn_knob = lt.turn_knob;
    out->turn_age = lt.turn_knob >= 0 ? (int)(lt.frames - lt.turn_frame) : 0x7fffffff;
    out->turn_move = lt.turn_move;
    out->notes = host_of(place) ? drum_notes_started : notes_started;
}

// ---------------------------------------------------------------- the GUI thread: what the presets ask (I8)

extern "C" const char* op1f_lfo4_kind_name(int kind) { return kind_name(kind); }

extern "C" int op1f_lfo4_default(int kind, int i) { return (kind >= 0 && kind < KINDS && i >= 0 && i < PARAMS) ? DEFAULTS[kind][i] : 0; }

// The presets list opens over the page: a knob held at that moment fires no hold under the list. Answers which
// knobs were down (a bit a knob), so the list can pass over the click stock makes of their release.
extern "C" int op1f_lfo4_release_knobs() {
    int down = 0;
    for (int k = 0; k < WAGONS; ++k) {
        if (lt.pressed[k]) down |= 1 << k;
        lt.pressed[k] = 0;
        lt.acted[k] = 0;
    }
    return down;
}

// The four LFOs of the place on the screen, as a preset holds them (build_i_lfo4.h): what the page shows for each
// knob, the wagon under the lamp read where stock keeps it. The effect wagon an LFO turns is written as the knob of
// the wagon it turns now (op1f_chain_lfo_knob: the chosen one while it carries an effect, else the one under the
// effect train's lamp), never as "the wagon under the lamp": a preset loaded onto another sound beside the effect
// preset it was made with then turns the same wagon, wherever that sound's effect lamp stands.
extern "C" void op1f_lfo4_preset_capture(int* values) {
    ensure_store();
    int place = screen_place();
    const SlotStore* s = &store[place];
    for (int p = 0; p < WAGONS; ++p) values[p] = s->order[p];
    for (int k = 0; k < WAGONS; ++k) {
        int* v = values + WAGONS + k * OP1F_LFO_PRESET_WAGON_INTS;
        int kind = wagon_kind(place, k);
        u8 pick = 0, midi[MIDI_DESTS] = { 0, 0, 0, 0 };
        u32 words[PARAMS];
        if (k == s->lamp) {
            pick = s->wagon[k].pick;                           // J5h: what the lamp's record keeps while the chain unit is away; with that unit there its own word takes its place
            for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = s->wagon[k].midi_pick[i];
            picks(place, 0, &pick, midi);
            for (int i = 0; i < PARAMS; ++i) words[i] = stock_word(place, i);
        } else {
            const LfoWagon* w = &s->wagon[k];
            pick = w->pick;
            for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = w->midi_pick[i];
            for (int i = 0; i < PARAMS; ++i) words[i] = w->words[i];
        }
        v[0] = kind;
        v[1] = kind >= 0 ? wagon_on(place, k) : 0;
        v[2] = kind >= 0 ? knob_of(place, pick) + 1 : 0;
        for (int i = 0; i < MIDI_DESTS; ++i) v[3 + i] = kind == KIND_MIDI ? knob_of(place, midi[i]) + 1 : 0;
        for (int i = 0; i < PARAMS; ++i) v[3 + MIDI_DESTS + i] = kind >= 0 ? (int)words[i] >> 16 : 0;
    }
}

// A preset's four LFOs onto the place on the screen. The lamp stays on its knob; when the preset leaves that knob
// empty and another carries an LFO, it moves to the first one in the page's order, as a loaded effect preset moves
// the effect train's lamp off an empty wagon (H6g), so T4 opens the page of an LFO of the preset. The LFO of the
// lamp's knob goes onto the unit's slot as move_lamp puts one there (the document's three keys, then the record's own
// LFO apply; an empty one is the unit's LFO switched off), the three others into the store, and the instances are
// built again at the next frames. Every parameter is held to a half-word and to its knob's range as the descriptor row
// stands in memory (hold_param, as the store's load). Answers how many were held.
extern "C" int op1f_lfo4_preset_apply(const int* values) {
    ensure_store();
    int place = screen_place();
    SlotStore* s = &store[place];
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
    void* record = record_of(place);
    LfoWagon* b = &s->wagon[lamp];
    if (b->kind >= 0) {
        set_text((u8*)record + 8, LFO_TYPE_KEY, *KIND[b->kind].name);
        STOCK_SET_LFO_WORDS_DOC(record, b->words);
        STOCK_SET_LFO_ACTIVE_DOC(record, b->on ? 1 : 0);
        ((RecordFn)(*(u32**)record)[RECORD_LFO_APPLY / 4])(record);
        picks(place, 1, &b->pick, b->midi_pick);
    } else {
        u8 none = 0, nones[MIDI_DESTS] = { 0, 0, 0, 0 };
        lfo_on_set(place, 0);
        STOCK_SET_LFO_ACTIVE_DOC(record, 0);
        picks(place, 1, &none, nones);
    }
    lt.want[host_of(place)] = 1;
    touch(place);
    event_line("preset loaded", place, lamp, b->kind, b->kind);
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

// The counts and the cost: half a second after the page's state last changed, when the shutdown countdown comes up,
// and every four seconds in which the LFOs were called (every half minute once thirty such summaries are out). Each
// of the three has a budget of its own, so the last lines of a session are never lost to the others (I5's lesson).
static void summary(int place, const char* why, int budget) {
    static const u32 LIMIT[BUDGETS] = { SUMMARY_LINES, CHANGE_LINES, POPUP_LINES };
    if (lt.summary_lines[budget] >= LIMIT[budget]) return;
    lt.summary_lines[budget] += (u32)LINES_A_SUMMARY;
    STOCK_LOG(3, LOG_FILE, 2, "lfo4 (%s): %c%u, rides %x, %u+%u running, %u built; calls: %u env, %u eng, %u fx, %u note-ons; %u busy, %u bad voices",
              why, host_letter(place), slot_of(place), op1f_lfo4_rides, op1f_lfo4_live, op1f_lfo4_drum_live, lt.built,
              counts.env, counts.eng, counts.fx, counts.note_ons, counts.busy, counts.bad_voice);
    u32 e[3], g[3], f[3], x[3];
    interval(&time_env, &seen_env, e);
    interval(&time_eng, &seen_eng, g);
    interval(&time_fx, &seen_fx, f);
    x[0] = x[1] = x[2] = 0;
    if (&op1f_lfofx_extra_counts != 0) op1f_lfofx_extra_counts(x);   // J5h: the chain unit's counts, when it is there
    STOCK_LOG(3, LOG_FILE, 3, "lfo4 time: env %u/%u over %u, eng %u/%u over %u, fx %u/%u over %u cycles (average/most a call, calls since the last line)",
              e[1], e[2], e[0], g[1], g[2], g[0], f[1], f[2], f[0]);
    STOCK_LOG(3, LOG_FILE, 4, "lfo4 fx side: %u wagon turns, %u parked or not built, %u on the scratch (the lamp's effect or another destination)", x[0], x[1], x[2]);
    STOCK_LOG(3, LOG_FILE, 5, "lfo4 drum: calls %u s1, %u s2, %u s3, %u c4, %u fx blocks, %u note-ons; %u busy, %u bad voices (since the start)",
              dcounts.s1, dcounts.s2, dcounts.s3, dcounts.c4, dcounts.fx, dcounts.note_ons, dcounts.busy, dcounts.bad_voice);
}

// Once a frame, on the display thread: the ride bits, the own files' frame, the holds of the page, each host's
// instances for its place on the screen while it rides, and the log.
extern "C" void op1f_lfo4_frame() asm("op1f_lfo4_frame");
extern "C" void op1f_lfo4_frame() {
    u32 rides = op1f_riding(&op1f_card_lfo, 0);                // lfo 12: bit n for place n; read once a frame (it walks the cards)
    op1f_lfo4_rides = rides;
    ensure_store();
    ++lt.frames;                                               // before the own files' frame: a load there is a load of this frame, tried RIDE_DELAY_FRAMES frames later (R15's PC test)
    u32 turned = rides ^ lt.rides_seen;                        // a ride begun or ended: the place's files follow, so a sound left on one LFO is saved with the flag (R15's PC test: else a restart put it back on ufo)
    lt.rides_seen = rides;
    for (int p = 0; p < PLACES; ++p) {
        if (!((turned >> p) & 1u)) continue;
        if ((rides >> p) & 1u) rd.no_flag[p] = 0;              // it rides: from now on its flag follows its ride again
        touch(p);
    }
    if (&op1f_lfo_file_frame != 0) op1f_lfo_file_frame();   // J5h: the start's choice between the chain's copy and the own file, and the own file's write
    for (int h = 0; h < HOSTS; ++h) {                          // the wagon under the lamp was emptied and the unit's LFO is on again (a sound loaded, the unit's own list): it carries that LFO
        int p = gui_place(h);
        SlotStore* s = &store[p];
        if (s->wagon[s->lamp].kind < 0 && stock_on(p)) {
            s->wagon[s->lamp].kind = 0;
            touch(p);
        }
    }
    int place = op1f_lfo4_place();
    u32 now = now_ms();
    for (int k = 0; k < WAGONS; ++k) {                         // the holds, for the place on the screen while its page is up
        if (lt.open && riding(place) && lt.pressed[k] && lt.armed[k] && !lt.acted[k] && now - lt.press_ms[k] >= (u32)HOLD_MS) {
            lt.acted[k] = 1;
            toggle(place, k);
        }
    }
    for (int h = 0; h < HOSTS; ++h) {                          // each host's set: built while its place on the screen rides, taken away when the ride ends or the place changes
        int p = gui_place(h);
        int target = riding(p) ? p : -1;
        if (target != lt.built_place[h]) lt.want[h] = 1;
        if (!lt.want[h]) continue;
        if (lt.stage[h] == 0) {                                // taken away now, built at the next frame: no block can be inside an object that is being built
            take_away(h);
            lt.stage[h] = 1;
        } else {
            lt.stage[h] = 0;
            lt.want[h] = 0;
            lt.built_place[h] = target;
            if (target >= 0) build_all(h, target);
        }
    }
    int shown = place >= 0 ? place : gui_place(0);
    int popup = RD8(POWER_POPUP_SHOWN) != 0;
    if (popup && !lt.popup_seen) summary(shown, "power popup", BY_POPUP);
    lt.popup_seen = popup;
    if (lt.summary_due && (i32)(lt.frames - lt.summary_at) >= 0 && lt.stage[0] == 0 && lt.stage[1] == 0 && !lt.want[0] && !lt.want[1]) {
        lt.summary_due = 0;
        summary(shown, "change", BY_CHANGE);
    }
    if (++lt.print_countdown >= PRINT_FRAMES) {
        lt.print_countdown = 0;
        u32 activity = counts.env + counts.eng + counts.fx + counts.note_ons + dcounts.s1 + dcounts.s3 + dcounts.c4 + dcounts.fx + dcounts.note_ons;
        if (activity != seen_activity) {
            ++lt.summaries;
            if (lt.summaries <= (u32)FAST_SUMMARIES || (lt.summaries & 7u) == 0) {
                seen_activity = activity;
                summary(shown, "period", BY_PERIOD);
            }
        }
    }
}
