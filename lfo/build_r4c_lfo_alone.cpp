// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// The pack lfo version 12 (step R15): the LFOs' own files, and the frame call
// and clock where the pack fx train is away. lfo 11's unit (step R4c) with the drum's file added and the synth
// controller's four handlers gone: lfo 12 takes its keys and knobs through the base's input (lfo_input).
//
// Where fx train is away:
//   the fx side          op1f_lfo_alone_fx (build_j5h_lfo_alone.s) before stock's effect call, marked in the pack's
//                        table "only when the pack fx train is away".
//   the frame and clock  op1f_lfo_alone_frame, which the base's frame hook calls where no chain unit is.
//
// The synth's file, user/lfo4.json, keeps lfo 11's form (lfo 11 refuses any other: it is the way back). A synth slot's
// four LFOs are fifty integers (build_i_lfo4.h; lfo 12's order integer may carry 256 more, "kept, not riding", which
// passes here untouched). fx train keeps the same integers in user/op1f.json after each synth slot's tag 98; the own
// file names that copy's sum as last known. At the start (the first frame call after the chain unit read its file):
//   the chain's copy still has the sum the own file names   the own file is the newer, or the same: it is taken
//   the sums differ, or there is no own file                the chain's copy is taken, the own file written from it
// The own file is written after the chain's: at the chain unit's word that it wrote its file
// (op1f_lfo_file_chain_saved), at the countdown, or 90 frame calls after a change without the word; half a second
// after the last change where no chain file is to come. A unit that goes off between the two writes leaves a chain
// copy the own file does not name, and that copy, the newer, wins (K1's rule for the tape's map files).
// The LFO under the lamp is stock's own; the effect wagon it turns is fx train's to keep (tags 96 and 97), and while
// that unit is away the LFO unit's record of the lamp's wagon keeps it, which the own file carries (-1 while the chain
// unit is there). At the start, away: the record gets the own file's choice where the own file won and holds one,
// else the chain file's; there: where the own file won and holds one, the chain unit is handed it.
//
// The drum's file, user/lfo4d.json: the drum slots' places of the LFO unit's store (8 + slot). fx train keeps no copy
// of a drum sound's LFOs, so nothing is chosen against: read at the start into the store (missing or refused: the drum
// sounds as stock has them); written half a second after a change of the store, by a mark of its own that waits for
// no word of fx train's, and only when what it would hold differs from what it holds. The drum lamp LFO's effect
// wagon rides in it as in the synth's file (fx train keeps it in its drum slots' elements); a choice the file hands
// to the chain unit at the start is dropped from the file only after that unit's word (or 90 frame calls).
//
// user/lfo4.json: a json array of 427 integers: 7401, the sum's high and low halves, then for each synth slot a 1 or a
// 0 (more than stock's one LFO), its fifty integers, the lamp LFO's effect wagon (0 the one under the effect lamp, 1 to
// 4 a knob's, -1 kept by the chain unit) and its midi kind's four (four bits each).
// user/lfo4d.json: 425 integers: 7402, then the same 53 for each drum slot.
// Both paths fit inside a stock string object (15 characters at most). GUI thread only; freestanding.

#include "build_i_lfo4.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
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
static const char DRUM_PATH[] = "user/lfo4d.json";                  // 15 characters: the most a stock string object holds inside itself
static const char CHAIN_PATH[] = "user/op1f.json";                  // the chain unit's file (Build H's chain unit, FILE_PATH)

enum {
    SLOTS = 8,
    DRUM_AT = OP1F_LFO4_DRUM_PLACE,                                 // a drum slot's place in the LFO unit's store: 8 + the slot
    HOST_DRUM = 1,                                                  // the chain unit's host of the drum sounds
    INTS = OP1F_LFO4_STORE_INTS,                                    // fifty
    MIN = OP1F_LFO4_STORE_MIN,                                      // forty-nine: what a file of I6's has
    WAGONS = 4,
    MIDI_DESTS = 4,
    FILE_TAG = 7401,
    FILE_HEAD = 3,                                                  // the tag, the sum's two halves
    DRUM_TAG = 7402,
    DRUM_HEAD = 1,                                                  // the tag alone
    FILE_SLOT = 1 + INTS + 2,                                       // the flag, the fifty integers, the lamp LFO's effect wagon and its midi four
    FILE_INTS = FILE_HEAD + SLOTS * FILE_SLOT,                      // 427
    DRUM_INTS = DRUM_HEAD + SLOTS * FILE_SLOT,                      // 425
    JSON_ARRAY_KIND = 6,                                            // the kind byte (json + 4) of an array
    // the chain unit's file as its reader takes it (Build H's chain unit, load_file): the root's tag, its seventeen
    // elements at the least (the eight synth slots', then the eight drum slots'), a slot's element with its tag, its
    // 48 integers, the pairs 96 and 97, and on a synth slot's the tag 98
    CHAIN_ROOT_TAG_A = 91, CHAIN_ROOT_TAG_B = 93, CHAIN_ROOT_MIN = 1 + 2 * SLOTS,
    CHAIN_SLOT_TAG = 92, CHAIN_SLOT_INTS = 48, CHAIN_FX_TAG = 96, CHAIN_MIDI_TAG = 97, CHAIN_LFO4_TAG = 98, CHAIN_LFO4_AT = CHAIN_SLOT_INTS + 4,
    OWN_DELAY_MS = 500,                                             // J5i: where no file of the chain unit's is to come first, the own file is written this long after the last change
    OWN_WAIT_FRAMES = 90,                                           // J5i: where one is, the own file waits for it: three times the chain unit's thirty frames, counted in frames as that unit counts, then no longer
    CYCLES_PER_MS = 400000,
    WRITE_LINES = 8
};

struct Json { u32 words[4]; };                                      // a stock json value is 12 bytes; 16 for alignment
struct Copy { int values[SLOTS][INTS]; u8 has[SLOTS]; };            // eight slots' integers; has: the slot has more than stock's one LFO
struct Lamp { int pick[SLOTS]; int midi[SLOTS]; };                  // the effect wagon the LFO under the lamp turns (0 the effect lamp's, 1 to 4 a knob's; -1: not kept here) and its midi kind's four, four bits each

// build_i_lfo4.cpp, Build I's LFO presets unit: the same pack. The store's places: 0 to 7 the synth slots, 8 to 15 the drum's.
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
static Copy drum_held;                  // R15: what the drum's file holds (read at the start, or as last written); a write only when the store's differs
static Lamp drum_held_lamp;
static int drum_dirty;                  // R15: the drum's file is to be written (if its content changed)
static u32 drum_dirty_ms;
static int drum_waits;                  // R15: lamp choices handed to the chain unit at the start: the file keeps them until that unit's word
static int drum_wait_frames;
static u32 drum_lines;

extern "C" u32 op1f_lfo_file_writes asm("op1f_lfo_file_writes");    // the own file written
extern "C" u32 op1f_lfo_file_failed asm("op1f_lfo_file_failed");    // of those, writes that did not land
extern "C" u32 op1f_lfo_file_gave_up asm("op1f_lfo_file_gave_up");  // J5i: waits for the chain unit's file that were given up (expected at 0)
extern "C" u32 op1f_lfo_drum_writes asm("op1f_lfo_drum_writes");    // R15: the drum's file written
extern "C" u32 op1f_lfo_drum_failed asm("op1f_lfo_drum_failed");    // R15: of those, writes that did not land
u32 op1f_lfo_file_writes, op1f_lfo_file_failed, op1f_lfo_file_gave_up, op1f_lfo_drum_writes, op1f_lfo_drum_failed;

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

static int same_lamp(const Lamp* a, const Lamp* b) {
    for (int s = 0; s < SLOTS; ++s) if (a->pick[s] != b->pick[s] || a->midi[s] != b->midi[s]) return 0;
    return 1;
}

// Loops, not a struct's copy: nothing here calls memcpy.
static void copy_into(Copy* to, Lamp* to_lamp, const Copy* from, const Lamp* from_lamp) {
    for (int s = 0; s < SLOTS; ++s) {
        to->has[s] = from->has[s];
        for (int i = 0; i < INTS; ++i) to->values[s][i] = from->values[s][i];
        to_lamp->pick[s] = from_lamp->pick[s];
        to_lamp->midi[s] = from_lamp->midi[s];
    }
}

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

// A slot's element of the chain unit's file: the effect wagon of the LFO under the lamp (after the tag 96) and its midi
// kind's four (after the tag 97). The element's size where it has both, else 0 (the wagon may be read without the four).
static int chain_slot(void* e, int* pick, int* midi) {
    if (!is_array(e)) return 0;
    int size = STOCK_JSON_SIZE(e);
    if (size < CHAIN_SLOT_INTS + 2 || int_at(e, 0) != CHAIN_SLOT_TAG || int_at(e, CHAIN_SLOT_INTS) != CHAIN_FX_TAG) return 0;
    *pick = int_at(e, CHAIN_SLOT_INTS + 1);
    if (size < CHAIN_SLOT_INTS + 4 || int_at(e, CHAIN_SLOT_INTS + 2) != CHAIN_MIDI_TAG) return 0;
    *midi = int_at(e, CHAIN_SLOT_INTS + 3);
    return size;
}

// The chain unit's file, read here when that unit is away, by the tests of that unit's own reader (load_file): for
// each synth slot the lamp LFO's effect wagon and midi four, and the fifty integers after the tag 98; for each drum
// slot (R15) the lamp LFO's effect wagon and midi four. Reading only.
static void read_chain_file(Copy* out, Lamp* lamp, Lamp* drums) {
    clear_copy(out);
    for (int s = 0; s < SLOTS; ++s) { lamp->pick[s] = 0; lamp->midi[s] = 0; drums->pick[s] = 0; drums->midi[s] = 0; }
    if (STOCK_FILE_SIZE(CHAIN_PATH) <= 0) return;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(CHAIN_PATH, &root);
    int tag = read && is_array(&root) ? int_at(&root, 0) : 0;
    if (read && is_array(&root) && STOCK_JSON_SIZE(&root) >= CHAIN_ROOT_MIN && (tag == CHAIN_ROOT_TAG_A || tag == CHAIN_ROOT_TAG_B)) {
        for (int s = 0; s < SLOTS; ++s) {
            void* e = STOCK_JSON_AT(&root, 1 + s);
            int size = chain_slot(e, &lamp->pick[s], &lamp->midi[s]);
            if (size < CHAIN_LFO4_AT + 1 + MIN || int_at(e, CHAIN_LFO4_AT) != CHAIN_LFO4_TAG) continue;
            int have = size - (CHAIN_LFO4_AT + 1);
            for (int i = 0; i < INTS; ++i) out->values[s][i] = i < have ? int_at(e, CHAIN_LFO4_AT + 1 + i) : -1;   // what a file of I6's does not have reads as -1
            out->has[s] = 1;
        }
        for (int s = 0; s < SLOTS; ++s) chain_slot(STOCK_JSON_AT(&root, 1 + SLOTS + s), &drums->pick[s], &drums->midi[s]);
    }
    STOCK_JSON_DTOR(&root);
}

// An own file (the synth's, head FILE_HEAD with the sum, or the drum's, head DRUM_HEAD): 1 when it is there and whole,
// 0 when it is not there, -1 when it is refused (another size or tag). The integers are taken as they stand.
static int read_own(const char* path, int tag, int head, Copy* out, Lamp* lamp, u32* seen) {
    clear_copy(out);
    for (int s = 0; s < SLOTS; ++s) { lamp->pick[s] = -1; lamp->midi[s] = 0; }
    if (STOCK_FILE_SIZE(path) <= 0) return 0;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int ok = STOCK_JSON_READ_FILE(path, &root) && is_array(&root) && STOCK_JSON_SIZE(&root) == head + SLOTS * FILE_SLOT && int_at(&root, 0) == tag;
    if (ok) {
        if (head == FILE_HEAD && seen != 0) *seen = (((u32)int_at(&root, 1) & 0xffffu) << 16) | ((u32)int_at(&root, 2) & 0xffffu);
        for (int s = 0; s < SLOTS; ++s) {
            int at = head + s * FILE_SLOT;
            out->has[s] = (u8)(int_at(&root, at) != 0);
            for (int i = 0; i < INTS; ++i) out->values[s][i] = out->has[s] ? int_at(&root, at + 1 + i) : 0;
            int pick = int_at(&root, at + 1 + INTS);
            lamp->pick[s] = (pick >= 0 && pick <= WAGONS) ? pick : -1;
            lamp->midi[s] = lamp->pick[s] >= 0 ? int_at(&root, at + 2 + INTS) : 0;
        }
    }
    STOCK_JSON_DTOR(&root);
    return ok ? 1 : -1;
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

// What an own file holds of the eight places from `first` on: the store's integers and, while the chain unit, which
// keeps it otherwise, is away, the lamp LFO's choice of effect wagon (-1 where the record gave none). Answers how many
// places have more than stock's one LFO.
static int collect(int first, Copy* c, Lamp* l) {
    int away = !chain_there(), slots = 0;
    for (int s = 0; s < SLOTS; ++s) {
        int values[INTS];
        int have = op1f_lfo4_store_get(first + s, values);
        c->has[s] = (u8)(have ? 1 : 0);
        for (int i = 0; i < INTS; ++i) c->values[s][i] = have ? values[i] : 0;
        if (have) ++slots;
        u8 pick = 0xff, midi[MIDI_DESTS] = { 0, 0, 0, 0 };
        if (away) op1f_lfo4_lamp_picks(first + s, 0, &pick, midi);
        int packed = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) packed |= (int)midi[i] << (4 * i);
        l->pick[s] = (away && pick <= WAGONS) ? (int)pick : -1;
        l->midi[s] = l->pick[s] >= 0 ? packed : 0;
    }
    return slots;
}

// An own file written: the tag, the sum's two halves where the head has them, then each place's 53 integers.
static int write_file(const char* path, int tag, int head, u32 seen, const Copy* c, const Lamp* l) {
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    push_int(&root, tag);
    if (head == FILE_HEAD) {
        push_int(&root, (int)(seen >> 16));
        push_int(&root, (int)(seen & 0xffffu));
    }
    for (int s = 0; s < SLOTS; ++s) {
        push_int(&root, c->has[s] ? 1 : 0);
        for (int i = 0; i < INTS; ++i) push_int(&root, c->has[s] ? c->values[s][i] : 0);
        push_int(&root, l->pick[s]);
        push_int(&root, l->pick[s] >= 0 ? l->midi[s] : 0);
    }
    u32 path_string[12];
    for (int i = 0; i < 12; ++i) path_string[i] = 0;
    STOCK_STRING_CTOR(path_string, path);
    int written = STOCK_JSON_WRITE_FILE(path_string, &root);
    STOCK_JSON_DTOR(&root);
    return written;
}

// The own file from the store, naming the sum of the chain's copy as it is known now. The lamp LFO's choice of
// effect wagon rides in it while the chain unit, which keeps it otherwise, is away.
static void write_own() {
    own_dirty = 0;
    own_waits = 0;
    own_now = 0;
    own_wait_frames = 0;
    u32 seen = sum_of(&chain_copy);
    static Copy c;                                                  // not on the stack: 1,600 bytes
    static Lamp l;
    int slots = collect(0, &c, &l);
    int written = write_file(OWN_PATH, FILE_TAG, FILE_HEAD, seen, &c, &l);
    ++op1f_lfo_file_writes;
    if (!written) ++op1f_lfo_file_failed;
    if (write_lines < (u32)WRITE_LINES || !written) {
        ++write_lines;
        STOCK_LOG(written ? 3 : 1, LOG_FILE, 2, "lfo file: %s %s, %d sound(s) with more than stock's LFO, the chain file's copy named by its sum %08x (%u writes, %u failed)",
                  OWN_PATH, written ? "written" : "NOT written", slots, seen, op1f_lfo_file_writes, op1f_lfo_file_failed);
    }
}

// R15: the drum's file from the store, where what it would hold differs from what it holds.
static void write_drum() {
    drum_dirty = 0;
    static Copy c;
    static Lamp l;
    int slots = collect(DRUM_AT, &c, &l);
    if (same_copy(&c, &drum_held) && same_lamp(&l, &drum_held_lamp)) return;
    int kept = 0;
    for (int s = 0; s < SLOTS; ++s) if (l.pick[s] >= 0) ++kept;
    int written = write_file(DRUM_PATH, DRUM_TAG, DRUM_HEAD, 0, &c, &l);
    ++op1f_lfo_drum_writes;
    if (written) copy_into(&drum_held, &drum_held_lamp, &c, &l);    // a write that did not land is made again at the next change
    else ++op1f_lfo_drum_failed;
    if (drum_lines < (u32)WRITE_LINES || !written) {
        ++drum_lines;
        STOCK_LOG(written ? 3 : 1, LOG_FILE, 6, "lfo file: %s %s, %d drum sound(s) with more than stock's LFO, %d lamp choice(s) in it (%u writes, %u failed)",
                  DRUM_PATH, written ? "written" : "NOT written", slots, kept, op1f_lfo_drum_writes, op1f_lfo_drum_failed);
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

// R15: the drum's file is to be written, half a second after the last change, whatever the chain unit does.
static void drum_touch() {
    drum_dirty = 1;
    drum_dirty_ms = now_ms();
}

// R15: the drum's file at the start, with the synth's choice: into the store as it stands, nothing to choose against.
// The drum lamp LFO's effect wagon: away, into the lamp's record (the file's, else the chain file's); there, the
// file's handed to the chain unit, and the file drops it after that unit's word.
static void drum_start(int chain, const Lamp* chain_drums) {
    static Copy file;
    static Lamp file_lamp;
    int got = read_own(DRUM_PATH, DRUM_TAG, DRUM_HEAD, &file, &file_lamp, 0);
    int sounds = 0, kept = 0, given = 0;
    for (int s = 0; s < SLOTS; ++s) {
        if (got > 0 && file.has[s]) { op1f_lfo4_store_set(DRUM_AT + s, file.values[s]); ++sounds; }
        int mine = got > 0 && file_lamp.pick[s] >= 0;
        if (mine) ++kept;
        u8 pick, midi[MIDI_DESTS];
        if (!chain) {
            unpack(mine ? file_lamp.pick[s] : chain_drums->pick[s], mine ? file_lamp.midi[s] : chain_drums->midi[s], &pick, midi);
            op1f_lfo4_lamp_picks(DRUM_AT + s, 1, &pick, midi);
        } else if (mine && &op1f_chain_lfo_picks != 0) {
            unpack(file_lamp.pick[s], file_lamp.midi[s], &pick, midi);
            op1f_chain_lfo_picks(HOST_DRUM, (u32)s, 1, &pick, midi);
            ++given;
        }
    }
    if (given) {
        op1f_chain_touch(HOST_DRUM);                                // the chain's file takes them first
        drum_waits = 1;
        drum_wait_frames = 0;
    }
    static Copy now;
    static Lamp now_lamp;
    collect(DRUM_AT, &now, &now_lamp);
    if (got > 0) {
        copy_into(&drum_held, &drum_held_lamp, &file, &file_lamp);
        if (!same_copy(&now, &file) || !same_lamp(&now_lamp, &file_lamp)) drum_touch();   // what the start changed (a choice handed over, an integer held to its range)
    } else {
        copy_into(&drum_held, &drum_held_lamp, &now, &now_lamp);    // nothing on the disk to mend: a write at the first change
    }
    if (got > 0)
        STOCK_LOG(3, LOG_FILE, 7, "lfo file: at the start: %s read: %d drum sound(s) with more than stock's LFO, %d lamp choice(s) in it, %d handed to the chain unit",
                  DRUM_PATH, sounds, kept, given);
    else
        STOCK_LOG(got < 0 ? 2 : 3, LOG_FILE, 7, "lfo file: at the start: %s %s (a json array of %d integers, %d first): the drum sounds are as stock has them",
                  DRUM_PATH, got < 0 ? "refused: it is not" : "is not there", (int)DRUM_INTS, (int)DRUM_TAG);
}

// The start's choice between the chain file's copy and the own file (the unit's head has the rule).
static void decide() {
    decided = 1;
    int chain = chain_there();
    static Lamp file_lamp;                                          // the chain file's word on the LFO under the lamp, read here when that unit is away
    static Lamp file_drums;                                         // R15: the same for the drum slots
    if (!chain) read_chain_file(&chain_copy, &file_lamp, &file_drums);   // nobody handed it over
    u32 chain_sum = sum_of(&chain_copy);
    int chain_slots = 0;
    for (int s = 0; s < SLOTS; ++s) if (chain_copy.has[s]) ++chain_slots;
    static Copy own;                                                // not on the stack: 1,600 bytes
    static Lamp own_lamp;
    u32 own_seen = 0;
    int have_own = read_own(OWN_PATH, FILE_TAG, FILE_HEAD, &own, &own_lamp, &own_seen) > 0;
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
        drum_touch();
    }
    STOCK_LOG(3, LOG_FILE, 1, "lfo file: at the start: taken: %s", took);   // J5i: two lines, the verdict first (the logger cuts a line at 260 characters with its prefix)
    STOCK_LOG(3, LOG_FILE, 5, "lfo file: at the start the chain unit is %s; its file's copy has %d sound(s) (sum %08x, %u handed over); the own file %s (it names %08x, %d lamp choice(s) in it)",
              chain ? "there" : "away", chain_slots, chain_sum, handed, have_own ? "is there" : "is not there", own_seen, kept);
    drum_start(chain, &file_drums);
}

// The LFO unit: the chain unit handed a slot's integers over. At the start that is the chain file's copy; later an
// instrument's, and the own file follows. Synth slots only (fx train carries no drum LFOs).
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
// where a wait was given up. Whatever slots the save held. R15: the drum lamp choices handed over are in it now.
extern "C" void op1f_lfo_file_chain_saved() asm("op1f_lfo_file_chain_saved");
extern "C" void op1f_lfo_file_chain_saved() {
    if (own_waits) { own_waits = 0; own_now = 1; }
    if (own_early) { own_early = 0; own_dirty = 1; own_now = 1; }
    drum_waits = 0;
}

// The LFO unit: the store changed (any place: the synth's file follows as in lfo 11, the drum's by its own mark).
extern "C" void op1f_lfo_file_touch() asm("op1f_lfo_file_touch");
extern "C" void op1f_lfo_file_touch() {
    if (decided) { own_touch(1); drum_touch(); }                    // the LFO unit's touch() touches the chain unit first, where there is one
    else early = 1;
}

// R15: the drum's file when it is due: half a second after the last change, at the countdown at once; while lamp
// choices handed to the chain unit wait for its word, not before it (or OWN_WAIT_FRAMES frame calls).
static void drum_frame() {
    if (!drum_dirty) return;
    if (drum_waits && POWER_POPUP == 0) {
        if (++drum_wait_frames < (int)OWN_WAIT_FRAMES) return;
        drum_waits = 0;
    }
    if (POWER_POPUP != 0 || now_ms() - drum_dirty_ms >= (u32)OWN_DELAY_MS) write_drum();
}

// The LFO unit's frame call, first thing: the start's choice once, then the own files' writes when they are due.
extern "C" void op1f_lfo_file_frame() asm("op1f_lfo_file_frame");
extern "C" void op1f_lfo_file_frame() {
    if (!decided) {
        if (chain_there() && &op1f_chain_file_loaded != 0 && !op1f_chain_file_loaded()) return;   // the chain unit has not read its file: its copy is not handed over yet
        decide();
    }
    drum_frame();
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

// From the base's frame hook, which calls the chain unit's frame function when there is one and this otherwise: the
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
        STOCK_LOG(3, LOG_FILE, 3, "lfo alone: the pack fx train is away: the LFOs' fx side, frame call and clock are in");
        op1f_lfopresets_scan();
    }
    ++own_frames;
    op1f_lfo4_frame();
    op1f_lfopresets_frame();
}
