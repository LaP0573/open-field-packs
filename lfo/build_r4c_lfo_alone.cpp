// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4c (base 27), the pack lfo version 11: the LFOs' own file unit of J5i (which
// stays as it was for the packs before) with one change and nothing else: the sign that the chain unit wrote its file
// is that unit's own word, op1f_lfo_file_chain_saved, which its save calls once the file is written (fx train 13), in
// the place of a run of the synth's slots 0 to 7 collected in a row. The run held only while every unit of fx train
// asked for slots in an order that never made one, and a save at a start that left out a slot the chain did not know
// yet gave none, after which an ask of that slot gave a false one. Now a slot collected says only what the chain's copy
// holds, in any order, and the sign comes from the save alone, whatever slots it holds. fx train and lfo are of
// interface 3 from these versions on, so that neither runs beside the other's version of before.
//
// What follows is J5i's description, which holds but where it names the run through the slots as the sign: read the
// chain unit's word there.
//
// Build J packs J5i: J5h's unit for the LFOs without the effect train (kept as that step's) with the two mends its
// read-out asked for:
//   * the own file is written when the chain unit has written its file, not after a time on the clock: the chain
//     unit's save waits thirty frames, and a frame at a start lasts seconds, so two seconds on the clock were over
//     first. The chain unit collects the eight slots in order when it writes its file (op1f_lfo_file_to_chain, slot
//     0 to 7): that run is the sign, and the own file follows in the same frame. Where no file of the chain unit's
//     is to come (that unit away, or nothing of its own to write) the own file is written half a second after the
//     last change, as before without the chain unit.
//   * the start's line is two lines, the verdict first: the logger cuts a line at 260 characters with its prefix.
// What follows is J5h's text with those two changes.
//
// Build J packs J5h: the LFOs without the pack `fx train` (OS 1.7.0+0).
//
// Until J5h the effect train's units did five things for the four LFOs of a sound (build_i_lfo4.cpp,
// Build I's LFO presets unit): their handlers on the synth controller's slots asked the LFO unit first; their frame
// call ran the LFO units' frame work and kept the millisecond clock; their run on the audio thread called the fx
// side of the LFOs; their file carried the LFOs' settings; and their two screen units showed the LFO page and the
// presets' screens. Since J5h the two screen units are `kit`'s, and this unit holds the rest for a unit that runs
// the pack `lfo` without the pack `fx train`:
//
//   the keys and knobs   the synth controller's four handlers (its vtable 0x8401364c: key-down +0xc, shifted
//                        key-down +0x10, key-up +0x14, the encoders +0x18), as table entries marked "only when the
//                        pack fx train is away". Each asks the LFO unit as the chain's handlers do and hands every
//                        other key to stock's own handler.
//   the fx side          op1f_lfo_alone_fx (build_j5h_lfo_alone.s) before stock's effect call, marked the same way.
//   the frame and clock  op1f_lfo_alone_frame, which `kit`'s frame hook calls where no chain unit is there.
//
// And, whatever runs, the LFOs' own settings file.
//
// The settings. A synth sound's four LFOs are fifty integers (build_i_lfo4.h), which the chain unit keeps in its
// file user/op1f.json after the tag 98 of each synth slot's element, and which it hands to the LFO unit at its load
// and asks back at its save. That copy stays, so that the packs before J5h find current values. Since J5h the same
// integers stand in a file of the LFOs' own, user/lfo4.json, with one number more: the sum of the chain file's copy
// as it was last known when the own file was written. At a start the unit has the chain's copy (handed over by the
// chain unit, or read from the chain's file here when that unit is away: reading only) and its own file:
//
//   the chain's copy still has the sum the own file names   the own file is the newer, or the same: it is taken
//   the sums differ, or there is no own file                something wrote the chain's copy since (a pack of
//                                                           before J5h, or the unit went off between the two
//                                                           writes): the chain's copy is taken, and the own file
//                                                           written from it
//
// With the chain unit away only the own file is written, and the sum it names stays the chain copy's, which nothing
// changes then: when the chain unit is back, its copy is the one the own file names, the own file wins, and the
// chain's file is written anew. The own file is written after the chain's (J5i: in the frame in which the chain
// unit has collected all eight slots for its file, right after that unit's part of the frame; at the shutdown
// countdown the chain's frame call saves before it calls the LFO unit's), so a unit that goes off between the two
// leaves a chain copy the own file does not name: the chain's copy wins, and it is the newer. The rule is K1's for
// the tape's two map files. The choice is made at the LFO unit's first
// frame call after the chain unit has read its file (op1f_chain_file_loaded), which on the unit is the first.
//
// The LFO under the lamp. That one is stock's own LFO, and which effect wagon it turns is not among the fifty
// integers: the chain unit keeps it (its tags 96 and 97 in each slot's element), and when the lamp moves the two
// units pass the choices between them. While the chain unit is away nothing passes: every LFO keeps its choice in
// its own wagon's record (build_i_lfo4.cpp, picks), the lamp's too. So that this holds across a restart and across
// the chain unit's return, the own file carries the lamp LFO's choice for each sound while it is written without
// the chain unit (-1 when that unit is there and keeps it), and at a start:
//
//   the chain unit away    the lamp's record gets the own file's choice where the own file won and holds one,
//                          else the chain file's (its tags 96 and 97, read here)
//   the chain unit there   where the own file won and holds one, the chain unit is handed it: it is the choice of
//                          the LFO that is under the lamp now, wherever the lamp was when the chain unit last saved
//
// user/lfo4.json: a json array of integers: 7401, the sum's high and low halves, then for each of the eight synth
// slots a 1 or a 0 (the slot has more than stock's one LFO), its fifty integers, the lamp LFO's effect wagon (0 the
// one under the effect lamp, 1 to 4 a knob's, -1 kept by the chain unit) and its midi kind's four (four bits each).
// The path has 14 characters: short enough to live inside a stock string object.
//
// GUI thread only: the handlers, the frame call, the file. The audio thread's part is the stub's call of
// op1f_lfo4_block, which is the LFO unit's.
// Freestanding like the other units.

#include "build_i_lfo4.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*KeyEventFn)(void* self, const int* event);
typedef int (*KeyUpFn)(void* self, int key);
typedef int (*EncoderFn)(void* self, int index, int delta);
typedef int (*JsonSizeFn)(void* json);                              // 0x8c084c22
typedef void* (*JsonAtFn)(void* json, int index);                   // 0x8c084ee0
typedef int (*JsonIntFn)(void* json);                               // 0x8c084840
typedef void (*JsonCtorFn)(void* json, int kind);                   // 0x8c085ba8(json, kind): 0 is null, which the first push turns into an array
typedef void (*JsonFromIntFn)(void* json, int value);               // 0x8c083b1c
typedef void (*JsonPushFn)(void* array, void* value);               // 0x8c085d54
typedef void (*JsonDtorFn)(void* json);                             // 0x8c0844e0
typedef int (*JsonReadFileFn)(const char* path, void* json);        // 0x8c0534d0: the file parsed into the json, non-zero on success
typedef int (*JsonWriteFileFn)(void* path_string, void* json);      // 0x8c053168: the json written to the path, a stock string object; 0 when the write did not land
typedef void (*StringCtorFn)(void* out, const char* text);          // 0x8c03e434: a stock string object from text
typedef int (*FileSizeFn)(const char* path);                        // 0x8c051f9c: the file's size, -1 when it is not there (and no line written)

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_SYNTH_KEY_DOWN ((KeyEventFn)0x8c0cfe7cu)
#define STOCK_SYNTH_KEY_SHIFT ((KeyEventFn)0x8c0cf904u)
#define STOCK_SYNTH_KEY_UP ((KeyUpFn)0x8c0cf364u)
#define STOCK_SYNTH_ENCODER ((EncoderFn)0x8c0d5c10u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#ifdef OP1F_HOST
extern unsigned char mock_power_popup;                              // the PC run's stand-ins for the three things below that are the hardware's or the linker's
extern unsigned int mock_cycles;
extern int mock_chain_there;
#define POWER_POPUP (mock_power_popup)
#else
#define POWER_POPUP (*(volatile u8*)0x84c9b3e4u)                    // 1 while the shutdown countdown is on the screen
#endif

static const char LOG_FILE[] = "../src/main/op1f_lfo_alone.cpp";
static const char OWN_PATH[] = "user/lfo4.json";                    // 14 characters
static const char CHAIN_PATH[] = "user/op1f.json";                  // the chain unit's file (Build H's chain unit, FILE_PATH)

enum {
    SLOTS = 8,
    INTS = OP1F_LFO4_STORE_INTS,                                    // fifty
    MIN = OP1F_LFO4_STORE_MIN,                                      // forty-nine: what a file of I6's has
    WAGONS = 4,
    MIDI_DESTS = 4,
    FILE_TAG = 7401,
    FILE_HEAD = 3,                                                  // the tag, the sum's two halves
    FILE_SLOT = 1 + INTS + 2,                                       // the flag, the fifty integers, the lamp LFO's effect wagon and its midi four
    FILE_INTS = FILE_HEAD + SLOTS * FILE_SLOT,
    JSON_ARRAY_KIND = 6,                                            // the kind byte (json + 4) of an array
    // the chain unit's file as its reader takes it (Build H's chain unit, load_file): the root's tag, its seventeen
    // elements at the least, a synth slot's element with its tag, its 48 integers, the pairs 96 and 97, the tag 98
    CHAIN_ROOT_TAG_A = 91, CHAIN_ROOT_TAG_B = 93, CHAIN_ROOT_MIN = 1 + 2 * SLOTS,
    CHAIN_SLOT_TAG = 92, CHAIN_SLOT_INTS = 48, CHAIN_FX_TAG = 96, CHAIN_MIDI_TAG = 97, CHAIN_LFO4_TAG = 98, CHAIN_LFO4_AT = CHAIN_SLOT_INTS + 4,
    OWN_DELAY_MS = 500,                                             // J5i: where no file of the chain unit's is to come first, the own file is written this long after the last change
    OWN_WAIT_FRAMES = 90,                                           // J5i: where one is, the own file waits for it: three times the chain unit's thirty frames, counted in frames as that unit counts, then no longer
    CYCLES_PER_MS = 400000,
    WRITE_LINES = 8
};

struct Json { u32 words[4]; };                                      // a stock json value is 12 bytes; 16 for alignment
struct Copy { int values[SLOTS][INTS]; u8 has[SLOTS]; };            // the eight synth slots' integers; has: the slot has more than stock's one LFO
struct Lamp { int pick[SLOTS]; int midi[SLOTS]; };                  // the effect wagon the LFO under the lamp turns (0 the effect lamp's, 1 to 4 a knob's; -1: not kept here) and its midi kind's four, four bits each

// build_i_lfo4.cpp, Build I's LFO presets unit: the same pack
extern "C" int op1f_lfotrain_key_down(void* self, const int* event, int shifted) asm("op1f_lfotrain_key_down");
extern "C" void op1f_lfotrain_key_up(int key) asm("op1f_lfotrain_key_up");
extern "C" int op1f_lfotrain_encoder(void* self, int index, int delta) asm("op1f_lfotrain_encoder");
extern "C" void op1f_lfo4_frame() asm("op1f_lfo4_frame");
extern "C" void op1f_lfo4_store_set(int slot, const int* values) asm("op1f_lfo4_store_set");
extern "C" int op1f_lfo4_store_get(int slot, int* values) asm("op1f_lfo4_store_get");
extern "C" void op1f_lfo4_lamp_picks(int slot, int write, u8* pick, u8* midi) asm("op1f_lfo4_lamp_picks");
extern "C" void op1f_lfopresets_frame() asm("op1f_lfopresets_frame");
extern "C" void op1f_lfopresets_scan() asm("op1f_lfopresets_scan");
// Build H's chain unit, when the pack fx train runs
extern "C" void __attribute__((weak)) op1f_chain_touch(int host) asm("op1f_chain_touch");
extern "C" u32 __attribute__((weak)) op1f_chain_ms() asm("op1f_chain_ms");
extern "C" int __attribute__((weak)) op1f_chain_file_loaded() asm("op1f_chain_file_loaded");
extern "C" void __attribute__((weak)) op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) asm("op1f_chain_lfo_picks");

static Copy chain_copy;                 // the chain file's copy as last known: handed over or read at the start, then as the chain unit last collected it
static int decided;                     // the start's choice between the two copies is made
static int early;                       // the store was touched before that
static int own_dirty;                   // the own file is to be written
static u32 own_dirty_ms;
static int own_waits;                   // J5i: and it waits for the chain unit's file, which is written first
static int own_wait_frames;             // frame calls it has waited
static int own_now;                     // the chain unit has written its file: the own file is written at this frame call
static int own_early;                   // the wait was given up: the own file is written once more when the chain unit's file does come, to name its sum
static u32 own_ms, own_cycle_acc, own_last_cycles, own_frames;      // the clock where the chain unit's is away
static u32 handed, write_lines;
static const char* took = "nothing yet";

extern "C" u32 op1f_lfo_file_writes asm("op1f_lfo_file_writes");    // the own file written
extern "C" u32 op1f_lfo_file_failed asm("op1f_lfo_file_failed");    // of those, writes that did not land
extern "C" u32 op1f_lfo_alone_keys asm("op1f_lfo_alone_keys");      // keys and turns the stand-in handlers saw
extern "C" u32 op1f_lfo_file_gave_up asm("op1f_lfo_file_gave_up");  // J5i: waits for the chain unit's file that were given up (expected at 0)
u32 op1f_lfo_file_writes, op1f_lfo_file_failed, op1f_lfo_alone_keys, op1f_lfo_file_gave_up;

// The effect train's units are there: the pack fx train runs, and the base has filled the chain unit's names in.
static int chain_there() {
#ifdef OP1F_HOST
    return mock_chain_there;
#else
    return &op1f_chain_touch != 0;
#endif
}

static u32 cycles() {
#ifdef OP1F_HOST
    return mock_cycles;
#else
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
#endif
}

// ---------------------------------------------------------------- the clock

extern "C" u32 op1f_lfo_alone_ms() asm("op1f_lfo_alone_ms");
extern "C" u32 op1f_lfo_alone_ms() { return own_ms; }

static u32 now_ms() { return chain_there() ? op1f_chain_ms() : own_ms; }

// ---------------------------------------------------------------- the settings

// lfo-file-begin
// The sum of a copy: FNV-1a over each slot's flag and, where it is set, its fifty integers.
static u32 sum_of(const Copy* c) {
    u32 h = 2166136261u;
    for (int s = 0; s < SLOTS; ++s) {
        h = (h ^ (u32)(c->has[s] ? 1 : 0)) * 16777619u;
        if (!c->has[s]) continue;
        for (int i = 0; i < INTS; ++i) h = (h ^ (u32)c->values[s][i]) * 16777619u;
    }
    return h;
}

static int same_copy(const Copy* a, const Copy* b) {
    for (int s = 0; s < SLOTS; ++s) {
        if ((a->has[s] != 0) != (b->has[s] != 0)) return 0;
        if (!a->has[s]) continue;
        for (int i = 0; i < INTS; ++i) if (a->values[s][i] != b->values[s][i]) return 0;
    }
    return 1;
}

static void clear_copy(Copy* c) {
    for (int s = 0; s < SLOTS; ++s) {
        c->has[s] = 0;
        for (int i = 0; i < INTS; ++i) c->values[s][i] = 0;
    }
}
// lfo-file-end

static int is_array(void* j) { return j != 0 && *((u8*)j + 4) == JSON_ARRAY_KIND; }
static int int_at(void* array, int i) { void* e = STOCK_JSON_AT(array, i); return e ? STOCK_JSON_INT(e) : 0; }

// A copy into the store. A slot the copy has nothing for is put back to what stock has (its one LFO on the first
// knob, the three others empty, the knobs' own order: the integers the chain unit hands over for an instrument
// without LFOs of ours) where `over` says the store holds something for it.
static void apply(const Copy* c, const Copy* over) {
    for (int s = 0; s < SLOTS; ++s) {
        if (c->has[s]) {
            op1f_lfo4_store_set(s, c->values[s]);
        } else if (over != 0 && over->has[s]) {
            int plain[INTS];
            for (int i = 0; i < INTS; ++i) plain[i] = 0;
            plain[1] = 1;
            plain[MIN] = -1;
            op1f_lfo4_store_set(s, plain);
        }
    }
}

// The chain unit's file, read here when that unit is away, by the tests of that unit's own reader (load_file): for
// each synth slot the effect wagon of the LFO under the lamp (after the tag 96) and its midi kind's four (after the
// tag 97), and the fifty integers after the tag 98. Reading only.
static void read_chain_file(Copy* out, Lamp* lamp) {
    clear_copy(out);
    for (int s = 0; s < SLOTS; ++s) { lamp->pick[s] = 0; lamp->midi[s] = 0; }
    if (STOCK_FILE_SIZE(CHAIN_PATH) <= 0) return;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(CHAIN_PATH, &root);
    int tag = read && is_array(&root) ? int_at(&root, 0) : 0;
    if (read && is_array(&root) && STOCK_JSON_SIZE(&root) >= CHAIN_ROOT_MIN && (tag == CHAIN_ROOT_TAG_A || tag == CHAIN_ROOT_TAG_B)) {
        for (int s = 0; s < SLOTS; ++s) {
            void* e = STOCK_JSON_AT(&root, 1 + s);
            if (!is_array(e)) continue;
            int size = STOCK_JSON_SIZE(e);
            if (size < CHAIN_SLOT_INTS + 2 || int_at(e, 0) != CHAIN_SLOT_TAG || int_at(e, CHAIN_SLOT_INTS) != CHAIN_FX_TAG) continue;
            lamp->pick[s] = int_at(e, CHAIN_SLOT_INTS + 1);
            if (size < CHAIN_SLOT_INTS + 4 || int_at(e, CHAIN_SLOT_INTS + 2) != CHAIN_MIDI_TAG) continue;
            lamp->midi[s] = int_at(e, CHAIN_SLOT_INTS + 3);
            if (size < CHAIN_LFO4_AT + 1 + MIN || int_at(e, CHAIN_LFO4_AT) != CHAIN_LFO4_TAG) continue;
            int have = size - (CHAIN_LFO4_AT + 1);
            for (int i = 0; i < INTS; ++i) out->values[s][i] = i < have ? int_at(e, CHAIN_LFO4_AT + 1 + i) : -1;   // what a file of I6's does not have reads as -1
            out->has[s] = 1;
        }
    }
    STOCK_JSON_DTOR(&root);
}

// The own file. Non-zero when it is there and whole.
static int read_own(Copy* out, Lamp* lamp, u32* seen) {
    clear_copy(out);
    for (int s = 0; s < SLOTS; ++s) { lamp->pick[s] = -1; lamp->midi[s] = 0; }
    if (STOCK_FILE_SIZE(OWN_PATH) <= 0) return 0;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int ok = STOCK_JSON_READ_FILE(OWN_PATH, &root) && is_array(&root) && STOCK_JSON_SIZE(&root) == FILE_INTS && int_at(&root, 0) == FILE_TAG;
    if (ok) {
        *seen = (((u32)int_at(&root, 1) & 0xffffu) << 16) | ((u32)int_at(&root, 2) & 0xffffu);
        for (int s = 0; s < SLOTS; ++s) {
            int at = FILE_HEAD + s * FILE_SLOT;
            out->has[s] = (u8)(int_at(&root, at) != 0);
            for (int i = 0; i < INTS; ++i) out->values[s][i] = out->has[s] ? int_at(&root, at + 1 + i) : 0;
            int pick = int_at(&root, at + 1 + INTS);
            lamp->pick[s] = (pick >= 0 && pick <= WAGONS) ? pick : -1;
            lamp->midi[s] = lamp->pick[s] >= 0 ? int_at(&root, at + 2 + INTS) : 0;
        }
    }
    STOCK_JSON_DTOR(&root);
    return ok;
}

// A choice of effect wagon and the midi kind's four as the two units pass them: a byte and four, each held to 0..4.
static void unpack(int pick, int packed, u8* p, u8* m) {
    *p = (u8)((pick >= 0 && pick <= WAGONS) ? pick : 0);
    for (int i = 0; i < MIDI_DESTS; ++i) {
        int one = (packed >> (4 * i)) & 15;
        m[i] = (u8)(one <= WAGONS ? one : 0);
    }
}

static void push_int(Json* array, int value) {
    Json item;
    STOCK_JSON_FROM_INT(&item, value);
    STOCK_JSON_PUSH(array, &item);
    STOCK_JSON_DTOR(&item);
}

// The own file from the store, naming the sum of the chain's copy as it is known now. The lamp LFO's choice of
// effect wagon rides in it while the chain unit, which keeps it otherwise, is away.
static void write_own() {
    own_dirty = 0;
    own_waits = 0;
    own_now = 0;
    own_wait_frames = 0;
    u32 seen = sum_of(&chain_copy);
    int away = !chain_there();
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    push_int(&root, FILE_TAG);
    push_int(&root, (int)(seen >> 16));
    push_int(&root, (int)(seen & 0xffffu));
    int slots = 0;
    for (int s = 0; s < SLOTS; ++s) {
        int values[INTS];
        int have = op1f_lfo4_store_get(s, values);
        push_int(&root, have ? 1 : 0);
        for (int i = 0; i < INTS; ++i) push_int(&root, have ? values[i] : 0);
        if (have) ++slots;
        u8 pick = 0, midi[MIDI_DESTS] = { 0, 0, 0, 0 };
        if (away) op1f_lfo4_lamp_picks(s, 0, &pick, midi);
        int packed = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) packed |= (int)midi[i] << (4 * i);
        push_int(&root, away ? (int)pick : -1);
        push_int(&root, away ? packed : 0);
    }
    u32 path[12];
    for (int i = 0; i < 12; ++i) path[i] = 0;
    STOCK_STRING_CTOR(path, OWN_PATH);
    int written = STOCK_JSON_WRITE_FILE(path, &root);
    STOCK_JSON_DTOR(&root);
    ++op1f_lfo_file_writes;
    if (!written) ++op1f_lfo_file_failed;
    if (write_lines < (u32)WRITE_LINES || !written) {
        ++write_lines;
        STOCK_LOG(written ? 3 : 1, LOG_FILE, 2, "lfo file: %s %s, %d sound(s) with more than stock's LFO, the chain file's copy named by its sum %08x (%u writes, %u failed)",
                  OWN_PATH, written ? "written" : "NOT written", slots, seen, op1f_lfo_file_writes, op1f_lfo_file_failed);
    }
}

// The own file is to be written. after_chain: the same change made the chain unit's file due (that unit was touched
// for it), so the own file waits for that file: the wait ends at the chain unit's word that it wrote it (R4c), at the
// countdown, or after OWN_WAIT_FRAMES frame calls without either. Without it, half a second after the last change.
static void own_touch(int after_chain) {
    own_dirty = 1;
    own_dirty_ms = now_ms();
    if (after_chain && chain_there()) {
        own_waits = 1;
        own_wait_frames = 0;
        own_now = 0;
    }
}

// The start's choice between the chain file's copy and the own file (the unit's head has the rule).
static void decide() {
    decided = 1;
    int chain = chain_there();
    static Lamp file_lamp;                                          // the chain file's word on the LFO under the lamp, read here when that unit is away
    if (!chain) read_chain_file(&chain_copy, &file_lamp);           // nobody handed it over
    u32 chain_sum = sum_of(&chain_copy);
    int chain_slots = 0;
    for (int s = 0; s < SLOTS; ++s) if (chain_copy.has[s]) ++chain_slots;
    static Copy own;                                                // not on the stack: 1,600 bytes
    static Lamp own_lamp;
    u32 own_seen = 0;
    int have_own = read_own(&own, &own_lamp, &own_seen);
    int own_wins = have_own && own_seen == chain_sum;
    int kept = 0;                                                   // sounds whose lamp LFO's effect wagon the own file holds: it was written while the chain unit was away
    for (int s = 0; s < SLOTS; ++s) if (own_wins && own_lamp.pick[s] >= 0) ++kept;
    if (own_wins) {
        int differs = !same_copy(&own, &chain_copy);
        if (!chain) apply(&own, 0);                                 // the store is as stock has it: the own file's sounds into it
        else if (differs) apply(&own, &chain_copy);                 // over what the chain unit handed over
        if (chain && (differs || kept != 0)) {
            for (int s = 0; s < SLOTS; ++s) {                       // the chain unit is back: each sound's lamp LFO as the own file holds it
                if (own_lamp.pick[s] < 0 || &op1f_chain_lfo_picks == 0) continue;
                u8 pick, midi[MIDI_DESTS];
                unpack(own_lamp.pick[s], own_lamp.midi[s], &pick, midi);
                op1f_chain_lfo_picks(0, (u32)s, 1, &pick, midi);
            }
            op1f_chain_touch(0);                                    // the chain's file follows, and the own file names its new sum after it
            own_touch(1);
        }
        took = differs ? "the own file, the newer" : "the own file, the same as the chain file's copy";
    } else {
        if (!chain) apply(&chain_copy, 0);
        if (chain_slots != 0 || have_own) own_touch(0);             // the own file is written from the store, naming this copy's sum; nothing of the chain unit's is written for it
        took = have_own ? "the chain file's copy, written since the own file was" : (chain_slots ? "the chain file's copy: there is no own file yet" : "neither: every sound is as stock has it");
    }
    if (!chain) {
        for (int s = 0; s < SLOTS; ++s) {                           // nothing passes the lamp LFO's choice on now: into the lamp's record, where it stays
            int mine = own_wins && own_lamp.pick[s] >= 0;
            u8 pick, midi[MIDI_DESTS];
            unpack(mine ? own_lamp.pick[s] : file_lamp.pick[s], mine ? own_lamp.midi[s] : file_lamp.midi[s], &pick, midi);
            op1f_lfo4_lamp_picks(s, 1, &pick, midi);
        }
    }
    if (early) {                                                    // a change made before this: the own file follows it (the LFO unit touched the chain unit for it too)
        early = 0;
        own_touch(1);
    }
    STOCK_LOG(3, LOG_FILE, 1, "lfo file: at the start: taken: %s", took);   // J5i: two lines, the verdict first (the logger cuts a line at 260 characters with its prefix)
    STOCK_LOG(3, LOG_FILE, 5, "lfo file: at the start the chain unit is %s; its file's copy has %d sound(s) (sum %08x, %u handed over); the own file %s (it names %08x, %d lamp choice(s) in it)",
              chain ? "there" : "away", chain_slots, chain_sum, handed, have_own ? "is there" : "is not there", own_seen, kept);
}

// The LFO unit: the chain unit handed a slot's integers over. At the start that is the chain file's copy; later an
// instrument's, and the own file follows.
extern "C" void op1f_lfo_file_from_chain(int slot, const int* values) asm("op1f_lfo_file_from_chain");
extern "C" void op1f_lfo_file_from_chain(int slot, const int* values) {
    if (slot < 0 || slot >= SLOTS) return;
    if (decided) { own_touch(1); return; }                          // an instrument's: the chain unit marks its own file for it (instrument_event)
    for (int i = 0; i < INTS; ++i) chain_copy.values[slot][i] = values[i];
    chain_copy.has[slot] = 1;
    ++handed;
}

// The LFO unit: the chain unit collected a slot's integers, for its file or for an instrument: what its copy holds.
extern "C" void op1f_lfo_file_to_chain(int slot, const int* values, int have) asm("op1f_lfo_file_to_chain");
extern "C" void op1f_lfo_file_to_chain(int slot, const int* values, int have) {
    if (slot < 0 || slot >= SLOTS || !decided) return;              // R4c: no order of collected slots is a sign (the chain unit's word below is)
    chain_copy.has[slot] = (u8)(have ? 1 : 0);
    for (int i = 0; i < INTS; ++i) chain_copy.values[slot][i] = have ? values[i] : 0;
}

// R4c: the chain unit's save wrote its file now (fx train 13's save_file, right after the write, in that unit's frame
// work before it calls the LFO unit's frame): the own file is written at this frame call, after it, and once more
// where a wait was given up. Whatever slots the save held.
extern "C" void op1f_lfo_file_chain_saved() asm("op1f_lfo_file_chain_saved");
extern "C" void op1f_lfo_file_chain_saved() {
    if (own_waits) { own_waits = 0; own_now = 1; }
    if (own_early) { own_early = 0; own_dirty = 1; own_now = 1; }
}

// The LFO unit: the store changed.
extern "C" void op1f_lfo_file_touch() asm("op1f_lfo_file_touch");
extern "C" void op1f_lfo_file_touch() {
    if (decided) own_touch(1);                                      // the LFO unit's touch() touches the chain unit first, where there is one
    else early = 1;
}

// The LFO unit's frame call, first thing: the start's choice once, then the own file's write when it is due.
extern "C" void op1f_lfo_file_frame() asm("op1f_lfo_file_frame");
extern "C" void op1f_lfo_file_frame() {
    if (!decided) {
        if (chain_there() && &op1f_chain_file_loaded != 0 && !op1f_chain_file_loaded()) return;   // the chain unit has not read its file: its copy is not handed over yet
        decide();
    }
    if (!own_dirty) return;
    if (POWER_POPUP != 0 || own_now) {                              // the countdown (the chain unit's save came first in this frame), or that unit has just written its file
        write_own();
        return;
    }
    if (own_waits) {                                                // J5i: the chain unit's file first, however long a frame lasts
        if (++own_wait_frames < (int)OWN_WAIT_FRAMES) return;
        own_waits = 0;                                              // it did not come: the own file without it, and once more when it does
        own_early = 1;
        ++op1f_lfo_file_gave_up;
        if (op1f_lfo_file_gave_up <= 4u)
            STOCK_LOG(2, LOG_FILE, 4, "lfo file: the chain unit's file did not come in %d frames: the own file is written without it, and again when it does", (int)OWN_WAIT_FRAMES);
    }
    if (now_ms() - own_dirty_ms >= (u32)OWN_DELAY_MS) write_own();
}

// ---------------------------------------------------------------- the frame call where the chain unit is away

// From `kit`'s frame hook, which calls the chain unit's frame function when there is one and this otherwise: the
// clock as the chain unit keeps it (the core's cycles, whatever the frame rate), the LFO presets' folder and files
// once, then the two frame functions the chain unit's calls at its end.
extern "C" void op1f_lfo_alone_frame() asm("op1f_lfo_alone_frame");
extern "C" void op1f_lfo_alone_frame() {
    u32 now = cycles();
    if (own_frames != 0) {
        own_cycle_acc += now - own_last_cycles;
        while (own_cycle_acc >= (u32)CYCLES_PER_MS) { own_cycle_acc -= (u32)CYCLES_PER_MS; ++own_ms; }
    }
    own_last_cycles = now;
    if (own_frames == 0) {
        STOCK_LOG(3, LOG_FILE, 3, "lfo alone: the pack fx train is away: the LFOs' own keys, fx side, frame call and clock are in");
        op1f_lfopresets_scan();
    }
    ++own_frames;
    op1f_lfo4_frame();
    op1f_lfopresets_frame();
}

// ---------------------------------------------------------------- the synth controller's handlers where the chain unit is away

// Each as the chain unit's handler of the same slot does for the LFOs (Build H's chain unit, op1f_fx_key_down and its
// three fellows): the LFO unit is asked first and answers -1 for what is not the LFO page's; stock's handler has
// the rest.
extern "C" int op1f_lfo_alone_key_down(void* self, const int* event) asm("op1f_lfo_alone_key_down");
extern "C" int op1f_lfo_alone_key_down(void* self, const int* event) {
    ++op1f_lfo_alone_keys;
    int taken = op1f_lfotrain_key_down(self, event, 0);
    if (taken >= 0) return taken;
    return STOCK_SYNTH_KEY_DOWN(self, event);
}

extern "C" int op1f_lfo_alone_key_shift(void* self, const int* event) asm("op1f_lfo_alone_key_shift");
extern "C" int op1f_lfo_alone_key_shift(void* self, const int* event) {
    ++op1f_lfo_alone_keys;
    int taken = op1f_lfotrain_key_down(self, event, 1);
    if (taken >= 0) return taken;
    return STOCK_SYNTH_KEY_SHIFT(self, event);
}

extern "C" int op1f_lfo_alone_key_up(void* self, int key) asm("op1f_lfo_alone_key_up");
extern "C" int op1f_lfo_alone_key_up(void* self, int key) {
    op1f_lfotrain_key_up(key);                                      // the LFO page sees its knobs released; stock's bookkeeping runs as ever
    return STOCK_SYNTH_KEY_UP(self, key);
}

extern "C" int op1f_lfo_alone_encoder(void* self, int index, int delta) asm("op1f_lfo_alone_encoder");
extern "C" int op1f_lfo_alone_encoder(void* self, int index, int delta) {
    ++op1f_lfo_alone_keys;
    int taken = op1f_lfotrain_encoder(self, index, delta);
    if (taken >= 0) return taken;
    return STOCK_SYNTH_ENCODER(self, index, delta);
}
