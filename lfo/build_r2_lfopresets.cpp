// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R2, the pack `lfo`: the LFO presets unit as the pack carries it from R2 on. A sibling of
// Build I's LFO presets unit, which stays as it was for the packs built before. What changed, and nothing else, as
// in the effect presets' sibling (fx train 11's chain unit):
//
//   * The LFO presets lie in the pack's own folder, the first folder of its card (user/pf/lfo/), no longer in
//     user/lf/. A slot's path is asked of the base (op1f_path) into a buffer of forty characters: user/pf/lfo/NN.json
//     has nineteen, past the fifteen a stock string keeps in its own buffer, so the text the string puts on the heap
//     is freed after each write, as json_text frees a long text. The writer keeps nothing of the string.
//   * The folder is the base's to make: the scan makes no folder and no file. A slot whose file is not there is an
//     empty slot. A preset deleted on the unit (the red knob held) has its file taken off the disk through the base
//     (op1f_remove) at the pack's next frame call; a slot whose file is still to be taken off is not free for a save.
//     When the base does not take it at once (the disk mode is open) the delete is dropped, never tried later: the
//     file stays, and the preset is in the list again.
//   * The presets of user/lf/ are copied into the pack's folder once, by shared/build_r2_move.cpp, from the pack's
//     frame call, before anything reads the store. Until that copy has ended well the store is shut: a scan is owed,
//     and runs when the copy ends; a save, a rename and a delete write nothing and go the way of a write that failed,
//     and so until the first scan since the copy has run, which runs in the pack's frame call right after the copy
//     ends, asked or not (op1f_lfo_presets_owed); the frame work says how many. op1f_lfo_presets_owed runs in the
//     frame call of this unit's own pack, which is where the base takes a file off: the list's own frame work may run
//     in the frame call of `fx train`. op1f_lfo_holds_preset is this unit's own rule of a file that holds a preset, which the copy asks
//     of each file.
//   * After the release of shift + T4 the train's page step of `fx train` runs where that pack runs
//     (op1f_train_page_now, a weak name), so that the page the pack asks the base for follows the key.
//
// The rest is the elder's, unchanged; its description follows and holds, but where it says the presets lie in
// user/lf/ and the scan makes the folder and the missing files.
//
// Build I image I8: presets for the LFOs.
//
// The wish after I7: LFO presets that work exactly as the effect presets do, so that an effect preset and
// an LFO preset made on one sound can be shared and loaded as a pair. This unit is the effect presets' twin
// (Build H's chain unit) for the four LFOs of a synth sound, written as
// a unit of its own so the effect presets' proven lines stand as they are:
//
//   * The store: sixty-four slots, one file each in the module's folder user/lf/ (01.json .. 64.json), shown by the
//     disk mode as lfo/NN.json (Build H's disk unit). The folder's name on the eMMC is one letter shorter than the
//     name the disk mode shows, so user/lf/NN.json keeps to the fifteen characters a stock string holds in its own
//     buffer, as user/fx/NN.json does; user/lfo/NN.json has sixteen. A used slot's file is a JSON object:
//     op1f_lfo_preset (the format, 1), name (the font's lowercase, twelve characters at most), order (the knob at
//     each place on the page), lfos (four objects, one a knob: lfo, stock's own name of the kind or "none"; on;
//     params, the eight integers as stock keeps them; fx_knob, the knob of the effect wagon its effect destination
//     turns, -1 for the wagon under the effect train's lamp; for the midi kind midi_fx_knobs, the same for its four
//     destinations). An empty slot's file is {}. Written by stock's json file writer, read by its reader; the folder
//     is made with its trailing slash and every write's result is checked and logged (the H6 lessons); the folder is
//     scanned at the first file load and at every list open (a file whose time or size changed is read again, a
//     missing one is made), so a file the computer wrote through the disk mode is a preset at the next list open.
//   * A kind's name is compared with stock's six names here (op1f_lfo4_kind_name, the names the LFO unit writes into
//     stock's document), never handed to a lookup of stock's (the H6i lesson: stock's lookups answer a name they do
//     not know with a valid kind). "none" and any other text is an empty UFO.
//   * The keys, on the LFO page (build_i_lfo4.cpp passes them here first): shift + T4, a tap opens the list and a
//     hold saves the sound's four LFOs into the lowest free slot as lfo NN (the save screen from 200 ms, the save at
//     700 ms, saved! for a second); on the list the blue knob scrolls and its tap or T4 loads the highlighted preset
//     into the sound on the screen, the ochre knob's tap opens the rename editor (blue the caret, ochre the letter,
//     grey or T4 keeps, red cancels), the grey knob's tap saves, the red knob held 400 ms deletes after a deleted
//     flash; T1 to T3 close the list and the page. The effect presets' grammar, with T4 where they have T3.
//   * The screens are the effect presets' own two (the ids 37 and 38, Build H's presets unit): no id is free below the
//     power popup's, and nothing in them says effect. This unit fills the same view (Op1fPresetsView): the names,
//     and in the left column the highlighted preset's four kinds in the knob colours where the effect list shows
//     the four effects. The synth controller's screen-id virtual asks op1f_lfopresets_screen_wanted
//     (Build H's train page unit).
//   * The lamp of the list marks the slot the four LFOs were loaded from or saved as, while they still stand as
//     they stood then (compared when the list opens; the wagon under the page's lamp is stock's own and changes on
//     stock's page without a word to this unit, so there is nothing to mark dirty).
//
// What a preset holds, how the four are read off a sound and put onto one: build_i_lfo4.cpp (op1f_lfo4_preset_capture,
// op1f_lfo4_preset_apply). GUI thread only (the scan at the first file load runs where the effect presets' runs).

#include "build_h_chain.h"
#include "build_i_lfo4.h"
#include "build_r1_card.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*JsonToStringFn)(void* out_string, void* json);     // 0x8c084782: the json's text into a std::string
typedef int (*JsonBoolFn)(void* json);                              // 0x8c084b94
typedef int (*JsonSizeFn)(void* json);                              // 0x8c084c22
typedef void* (*JsonLookupFn)(void* json, const char* key);          // 0x8c085e84: the member, inserted when missing
typedef int (*JsonHasFn)(void* json, const char* key);               // 0x8c085068
typedef void* (*JsonGetFn)(void* json, const char* key);             // 0x8c084f74
typedef void* (*JsonAtFn)(void* json, int index);                   // 0x8c084ee0
typedef int (*JsonIntFn)(void* json);                                // 0x8c084840: converts the three number kinds; 0 and a log line for anything else
typedef void (*JsonCtorFn)(void* json, int kind);                    // 0x8c085ba8: a fresh value of that kind; 0 is null, which the first push turns into an array
typedef void (*JsonFromIntFn)(void* json, int value);                // 0x8c083b1c
typedef void (*JsonFromCstrFn)(void* json, const char* text);        // 0x8c084430: a string copied from the text (the kind 4)
typedef void (*JsonFromBoolFn)(void* json, int value);               // 0x8c083aec: a bool (the kind 5)
typedef void (*JsonPushFn)(void* array, void* value);                // 0x8c085d54
typedef void (*JsonAssignFn)(void* target, void* value);             // 0x8c0845b8
typedef void (*JsonDtorFn)(void* json);                              // 0x8c0844e0
typedef int (*JsonReadFileFn)(const char* path, void* json);         // 0x8c0534d0: the file parsed into the json, non-zero on success
typedef int (*JsonWriteFileFn)(void* path_string, void* json);       // 0x8c053168: the json written to the path, a stock string object; 0 when the rename over the target fails
typedef void (*StringCtorFn)(void* out, const char* text);           // 0x8c03e434: a stock string object from text
typedef void (*FreeFn)(void* block);                                 // 0x8d4e63cc: the match of the allocator stock's strings use
typedef int (*FileSizeFn)(const char* path);                         // 0x8c051f9c: the file's size, -1 when it is not there
typedef u32 (*FileMtimeFn)(const char* path);                        // 0x8fcbc350: the file's modification time, 0 when it is not there
typedef void (*RequestScreenFn)();                                   // 0x8c0b2554: the GUI asks the current controller's screen-id virtual and queues the screen

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_JSON_TO_STRING ((JsonToStringFn)0x8c084782u)
#define STOCK_JSON_BOOL ((JsonBoolFn)0x8c084b94u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_FROM_BOOL ((JsonFromBoolFn)0x8c083aecu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_MTIME ((FileMtimeFn)0x8fcbc350u)
#define STOCK_REQUEST_SCREEN ((RequestScreenFn)0x8c0b2554u)
// J5i: the request's queue takes one writer at a time and stops the OS when a second one enters its push; stock
// makes the request on the key thread. From the frame call, on the display thread, it is made with interrupts
// masked (Build H's chain unit has the note).
#define STOCK_IRQ_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define STOCK_IRQ_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")

static const char LOG_FILE[] = "../src/main/op1f_lfopresets.cpp";

enum { PRESETS = 64, NAME_CHARS = OP1F_PRESET_NAME, LIST_ROWS = OP1F_PRESET_ROWS, WAGONS = OP1F_LFO_WAGONS, KINDS = OP1F_LFO_KINDS, KIND_NONE = -1, KIND_MIDI = 4 };
enum { PRESET_INTS = OP1F_LFO_PRESET_INTS, WAGON_INTS = OP1F_LFO_PRESET_WAGON_INTS, MIDI_DESTS = OP1F_LFO_PRESET_MIDI, PARAMS = OP1F_LFO_PRESET_PARAMS };
enum { PRESET_FORMAT = 1, JSON_ARRAY_KIND = 6, JSON_OBJECT_KIND = 7, JSON_STRING_KIND = 4, JSON_BOOL_KIND = 5 };
enum { KEY_T1 = 6, KEY_T4 = 9, KEY_CLICK0 = 54, KEY_PRESS0 = 61 };
enum { KNOB_BLUE = 0, KNOB_OCHRE = 1, KNOB_GREY = 2, KNOB_RED = 3 };
enum { HOLD_MS = 400, SWALLOW_MS = 300, FLASH_MS = 1000, SAVE_SHOW_MS = 200, SAVE_HOLD_MS = 700, SAVED_MS = 1000 };   // the effect presets' own times (Build H's chain unit)
enum { FLASH_NONE = 0, FLASH_SAVED = 1, FLASH_DELETED = 2, FLASH_FULL = 3 };
enum { SAVE_NONE = OP1F_SAVE_NONE, SAVE_SAVING = OP1F_SAVE_SAVING, SAVE_SAVED = OP1F_SAVE_SAVED };
enum { PRINT_FRAMES = 232, FAILURE_LINES = 4, EVENT_LINES = 200, COUNT_LINES = 120 };
static const char KEY_FORMAT[] = "op1f_lfo_preset", KEY_NAME[] = "name", KEY_ORDER[] = "order", KEY_LFOS[] = "lfos", KEY_LFO[] = "lfo", KEY_ON[] = "on", KEY_PARAMS[] = "params", KEY_FX_KNOB[] = "fx_knob", KEY_MIDI_KNOBS[] = "midi_fx_knobs";
static const char ALPHABET[] = " abcdefghijklmnopqrstuvwxyz0123456789-.";          // the rename editor's letters, in the font's lowercase (the effect presets' own)
enum { ALPHABET_COUNT = 39 };

// Build H's chain unit: the millisecond clock of the frame call
extern "C" u32 __attribute__((weak)) op1f_chain_ms() asm("op1f_chain_ms");            // J5h: optional, as in build_i_lfo4.cpp: the chain unit's clock when it is there
extern "C" u32 __attribute__((weak)) op1f_lfo_alone_ms() asm("op1f_lfo_alone_ms");    // J5h's unit for the LFOs without the effect train: the LFOs' own when it is not
// Build H's presets unit: the two preset screens exist
extern "C" int __attribute__((weak)) op1f_presets_available() asm("op1f_presets_available");
// R2: the pack's card (written by tools/pack_build.py from the recipe), whose first folder holds the presets; the copy
// of the presets of user/lf/ into it (build_r2_move.cpp), which says whether the store may be used; the train's page
// step of `fx train`, where that pack runs.
extern "C" const Op1fCard op1f_card_lfo asm("op1f_card_lfo");
extern "C" int op1f_r2_move_done(void) asm("op1f_r2_move_done");
extern "C" void __attribute__((weak)) op1f_train_page_now(int host) asm("op1f_train_page_now");
enum { PRESETS_FOLDER = 0, PATH_ROOM = 40, SHUT_LINES = 16, REMOVE_LINES = 16 };
static struct {                                                // R2: the store's own words beside the elder's
    volatile u32 owed;                                         // a scan asked while the store was shut: 1, or 2 for a forced one
    volatile u32 removing[2];                                  // the slots whose file is to be taken off the disk, a bit each
    u32 dropped;                                               // removals dropped because the base answered not now (the disk mode open)
    volatile u32 shut_writes, shut_deletes;                    // saves and renames, and deletes, that wrote nothing while the store was shut
    u32 said_writes, said_deletes, shut_lines;                 // what the frame work has said of them
    u32 removed, remove_failures, remove_lines;
} r2;
static int removal_owed(int slot) { return (int)((r2.removing[slot >> 5] >> (slot & 31)) & 1u); }

struct Json { u32 words[4]; };                                 // a stock json value is 12 bytes; 16 for alignment

static struct {                                                // the store, one slot per file user/lf/NN.json
    u8 used[PRESETS];                                          // the slot's file holds a preset
    char name[PRESETS][NAME_CHARS + 1];                        // per slot, in the font's lowercase
    int values[PRESETS][PRESET_INTS];                          // per slot: the four LFOs as build_i_lfo4.h lays them out
    u32 mtime[PRESETS];                                        // the file's time at the last read
    int size[PRESETS];                                         // and its size (a changed file is read again at the next scan)
    int order[PRESETS];                                        // the used slots in slot order: the list's rows
    int count;                                                 // used slots
    int scanned;                                               // the folder was scanned once (the first file load)
} presets;
static struct {                                                // the list's state (the synth's alone: the drum has one LFO)
    int started;
    int open, cursor, first, editing, caret;
    int loaded_slot;                                           // the slot the four LFOs were loaded from or saved as, or -1
    int loaded_values[PRESET_INTS];                            // the four as they stood right after that load or save
    char buffer[NAME_CHARS + 1];                               // the rename editor's text
    int flash_kind, flash_slot;                                // one row's word for a second: saved, deleted, list full
    u32 flash_until_ms;
    int pending_delete_slot;                                   // the slot emptied when its deleted flash ends, or -1
    int t4_pressed, t4_acted;                                  // shift + T4 held: the save after SAVE_HOLD_MS, else the tap at the release
    u32 t4_ms;
    int pressed[WAGONS], acted[WAGONS], held_at_open[WAGONS];  // the knobs on the list (the red hold deletes); a knob that was down on the page when the list opened
    u32 press_ms[WAGONS], swallow_until[WAGONS];
    int save_state, save_slot;                                 // the save screen: SAVING from 200 ms of the hold, SAVED for a second after a save; the slot shown as its number
    u32 save_ms;
    char row_text[LIST_ROWS][NAME_CHARS + 1];
    char wagon_text[WAGONS][NAME_CHARS + 6];
    char cell_text[NAME_CHARS][2];
    u32 opens, saves, loads, deletes, renames, scans, rereads, writes, write_failures, folder_made, held;
    u32 event_lines, count_lines, counted, print_countdown;
} ls;

static u32 now_ms() { return &op1f_chain_ms != 0 ? op1f_chain_ms() : (&op1f_lfo_alone_ms != 0 ? op1f_lfo_alone_ms() : 0u); }

static void start() {
    if (ls.started) return;
    ls.started = 1;
    ls.loaded_slot = -1;
    ls.pending_delete_slot = -1;
    ls.flash_slot = -1;
    ls.save_slot = -1;
}

static u32 udiv(u32 n, u32 d) {                                  // restoring shift-subtract division (the module links no runtime)
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static char* put_text(char* p, const char* s) {
    while (*s) *p++ = *s++;
    return p;
}

static int alphabet_index(char c) {
    for (int i = 0; i < ALPHABET_COUNT; ++i) if (ALPHABET[i] == c) return i;
    return 0;
}

static void name_copy(char* dst, const char* src) {                          // at most NAME_CHARS characters, NUL-terminated
    int i = 0;
    for (; i < NAME_CHARS && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

static void name_trim(char* name) {                                          // the trailing spaces off
    int n = 0;
    while (name[n]) ++n;
    while (n > 0 && name[n - 1] == ' ') name[--n] = 0;
}

static char lower_of(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }   // the font's letters are the lowercase codes (H5b)

static void default_name(char* out, int slot) {                             // lfo NN, the slot's number (the file's)
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    char* p = put_text(out, "lfo ");
    *p++ = (char)('0' + tens);
    *p++ = (char)('0' + n);
    *p = 0;
}

static void slot_name(int slot, char* out) {                               // NN.json, the slot's number (R2)
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    out[0] = (char)('0' + tens);
    out[1] = (char)('0' + n);
    *put_text(out + 2, ".json") = 0;
}

// R2: user/pf/lfo/NN.json, asked of the base: the path's characters, or 0 and an empty path where the base has no
// folder for the pack.
static int lfo_path(int slot, char* out) {
    char name[8];
    slot_name(slot, name);
    int got = op1f_path(&op1f_card_lfo, PRESETS_FOLDER, name, out, PATH_ROOM);
    if (got <= 0) out[0] = 0;
    return got > 0 ? got : 0;
}

static int free_slot() {                                                    // R2: a slot whose file is still to be taken off is not free
    for (int s = 0; s < PRESETS; ++s) if (!presets.used[s] && !removal_owed(s)) return s;
    return -1;
}

static void rebuild_order() {                                               // the used slots in slot order
    int n = 0;
    for (int s = 0; s < PRESETS; ++s) if (presets.used[s]) presets.order[n++] = s;
    presets.count = n;
}

static int row_of_slot(int slot) {                                          // the list row of a slot, or -1
    for (int i = 0; i < presets.count; ++i) if (presets.order[i] == slot) return i;
    return -1;
}

// ---------------------------------------------------------------- the files

static int is_array(void* j) { return j != 0 && *((u8*)j + 4) == JSON_ARRAY_KIND; }

// An integer of a file, or the fallback for a value that is no number (the kinds 1, 2 and 3 are the numbers; stock's
// getter answers anything else with 0 and an error line).
static int json_number(void* entry, int fallback) {
    if (!entry) return fallback;
    u8 kind = *((u8*)entry + 4);
    return (kind >= 1 && kind <= 3) ? STOCK_JSON_INT(entry) : fallback;
}

// a json string's characters: a stock string object has its data inline at +4 up to fifteen characters, else a
// pointer there; the length at +0x14, the capacity at +0x18 (0x8c03e434)
static void json_text(void* entry, char* out, int max) {
    u32 str[8];
    for (int i = 0; i < 8; ++i) str[i] = 0;
    STOCK_JSON_TO_STRING(str, entry);
    u32 len = str[5], cap = str[6];
    const char* data = cap >= 16 ? (const char*)str[1] : (const char*)&str[1];
    int n = 0;
    if (data) for (; n < max && (u32)n < len && data[n]; ++n) out[n] = data[n];
    out[n] = 0;
    if (cap >= 16 && str[1]) STOCK_FREE((void*)str[1]);                      // a long text lives on the heap
}

static int text_equal(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// "tremolo" .. "velocity" by stock's own names; "none" and anything else none. The name is compared here: stock's
// lookup 0x8fca618c answers a name it does not know with a kind (6) and an error line, and the effect's lookup with
// the delay, which cost H6 to H6i every empty wagon of a file.
static int json_lfo_kind(void* entry) {
    if (!entry || *((u8*)entry + 4) != JSON_STRING_KIND) return KIND_NONE;
    char name[16];
    json_text(entry, name, 15);
    for (int kind = 0; kind < KINDS; ++kind) if (text_equal(name, op1f_lfo4_kind_name(kind))) return kind;
    return KIND_NONE;
}

static int json_truth(void* entry) {                                        // true/false, or a number
    if (!entry) return 0;
    if (*((u8*)entry + 4) == JSON_BOOL_KIND) return STOCK_JSON_BOOL(entry) ? 1 : 0;
    return json_number(entry, 0) != 0;
}

// A file's knob (0 blue .. 3 red) as the choice the LFO unit keeps: 1..4, and 0, the wagon under the effect train's
// lamp, for -1 and for anything that is no knob.
static int pick_of_knob(int knob) { return (knob >= 0 && knob < WAGONS) ? knob + 1 : 0; }

static void set_int(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_FROM_INT(&item, value);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_text(void* obj, const char* key, const char* text) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_CSTR(&item, text);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_bool(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_BOOL(&item, value ? 1 : 0);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_ints(void* obj, const char* key, const int* values, int count) {
    Json arr, item;
    STOCK_JSON_CTOR(&arr, 0);
    for (int i = 0; i < count; ++i) {
        STOCK_JSON_FROM_INT(&item, values[i]);
        STOCK_JSON_PUSH(&arr, &item);
        STOCK_JSON_DTOR(&item);
    }
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &arr);
    STOCK_JSON_DTOR(&arr);
}

// The slot's file: {} for an empty slot, else the object above. The time and size are noted so the next scan does
// not read the module's own write back. 1 when the file landed (the writer's result and the file's size checked).
static int write_slot(int slot) {
    if (!op1f_r2_move_done() || !presets.scanned) {                            // R2: the store is shut until the presets are copied and read once since: nothing is written, as when a write fails
        ++r2.shut_writes;
        return 0;
    }
    Json root;
    STOCK_JSON_CTOR(&root, JSON_OBJECT_KIND);
    if (presets.used[slot]) {
        const int* v = presets.values[slot];
        set_int(&root, KEY_FORMAT, PRESET_FORMAT);
        set_text(&root, KEY_NAME, presets.name[slot]);
        int order[WAGONS];
        for (int p = 0; p < WAGONS; ++p) order[p] = v[p] & 3;
        set_ints(&root, KEY_ORDER, order, WAGONS);
        Json lfos, w;
        STOCK_JSON_CTOR(&lfos, 0);
        for (int k = 0; k < WAGONS; ++k) {
            const int* wv = v + WAGONS + k * WAGON_INTS;
            int kind = (wv[0] >= 0 && wv[0] < KINDS) ? wv[0] : KIND_NONE;
            STOCK_JSON_CTOR(&w, JSON_OBJECT_KIND);
            set_text(&w, KEY_LFO, op1f_lfo4_kind_name(kind));
            set_bool(&w, KEY_ON, kind >= 0 && wv[1]);
            set_ints(&w, KEY_PARAMS, wv + 3 + MIDI_DESTS, PARAMS);
            if (kind >= 0) set_int(&w, KEY_FX_KNOB, wv[2] - 1);                   // the knob 0..3, or -1 for the wagon under the effect train's lamp
            if (kind == KIND_MIDI) {
                int knobs[MIDI_DESTS];
                for (int i = 0; i < MIDI_DESTS; ++i) knobs[i] = wv[3 + i] - 1;
                set_ints(&w, KEY_MIDI_KNOBS, knobs, MIDI_DESTS);
            }
            STOCK_JSON_PUSH(&lfos, &w);
            STOCK_JSON_DTOR(&w);
        }
        STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(&root, KEY_LFOS), &lfos);
        STOCK_JSON_DTOR(&lfos);
    }
    char path[PATH_ROOM];
    int written = 0;
    if (lfo_path(slot, path)) {
        u32 str[12];
        for (int i = 0; i < 12; ++i) str[i] = 0;
        STOCK_STRING_CTOR(str, path);
        written = STOCK_JSON_WRITE_FILE(str, &root);                             // the writer writes tmp/NN.json and renames it over the slot's file; 0 when that fails
        if (str[6] >= 16 && str[1]) STOCK_FREE((void*)str[1]);                  // R2: the path's text lives on the heap; the writer keeps nothing of it
    }
    STOCK_JSON_DTOR(&root);
    presets.mtime[slot] = path[0] ? STOCK_FILE_MTIME(path) : 0;
    presets.size[slot] = path[0] ? STOCK_FILE_SIZE(path) : -1;
    ++ls.writes;
    if (!written || presets.size[slot] < 0) {
        ++ls.write_failures;
        if (ls.write_failures <= (u32)FAILURE_LINES) STOCK_LOG(1, LOG_FILE, 20, "lfo presets: the write of %s failed (writer %d, size %d): the folder is missing or the disk is full", path, written, presets.size[slot]);
        return 0;
    }
    return 1;
}

// The slot's file read: a preset when it carries four lfos, else an empty slot. A kind stock does not know is an
// empty UFO; a missing parameter is the catalog's default for the kind; a missing name is lfo NN.
static void read_slot(int slot) {
    char path[PATH_ROOM];
    if (!lfo_path(slot, path)) { presets.used[slot] = 0; return; }            // R2: the base has no folder for the pack
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(path, &root);
    presets.used[slot] = 0;
    if (read && *((u8*)&root + 4) == JSON_OBJECT_KIND && STOCK_JSON_HAS(&root, KEY_LFOS)) {
        void* lfos = STOCK_JSON_GET(&root, KEY_LFOS);
        if (is_array(lfos) && STOCK_JSON_SIZE(lfos) >= WAGONS) {
            int* v = presets.values[slot];
            for (int i = 0; i < PRESET_INTS; ++i) v[i] = 0;
            for (int p = 0; p < WAGONS; ++p) v[p] = p;
            if (STOCK_JSON_HAS(&root, KEY_ORDER)) {
                void* o = STOCK_JSON_GET(&root, KEY_ORDER);
                if (is_array(o) && STOCK_JSON_SIZE(o) >= WAGONS) for (int p = 0; p < WAGONS; ++p) v[p] = json_number(STOCK_JSON_AT(o, p), p) & 3;
            }
            int seen = 0;
            for (int p = 0; p < WAGONS; ++p) seen |= 1 << v[p];
            if (seen != 0xf) for (int p = 0; p < WAGONS; ++p) v[p] = p;
            for (int k = 0; k < WAGONS; ++k) {
                int* wv = v + WAGONS + k * WAGON_INTS;
                void* w = STOCK_JSON_AT(lfos, k);
                int kind = KIND_NONE, on = 1;
                int is_object = w && *((u8*)w + 4) == JSON_OBJECT_KIND;
                if (is_object) {
                    if (STOCK_JSON_HAS(w, KEY_LFO)) kind = json_lfo_kind(STOCK_JSON_GET(w, KEY_LFO));
                    if (STOCK_JSON_HAS(w, KEY_ON)) on = json_truth(STOCK_JSON_GET(w, KEY_ON));
                }
                wv[0] = kind;
                if (kind < 0) continue;                                        // an empty UFO: the rest stays 0
                wv[1] = on ? 1 : 0;
                if (STOCK_JSON_HAS(w, KEY_FX_KNOB)) wv[2] = pick_of_knob(json_number(STOCK_JSON_GET(w, KEY_FX_KNOB), -1));
                if (kind == KIND_MIDI && STOCK_JSON_HAS(w, KEY_MIDI_KNOBS)) {
                    void* knobs = STOCK_JSON_GET(w, KEY_MIDI_KNOBS);
                    if (is_array(knobs)) {
                        int n = STOCK_JSON_SIZE(knobs);
                        for (int i = 0; i < MIDI_DESTS && i < n; ++i) wv[3 + i] = pick_of_knob(json_number(STOCK_JSON_AT(knobs, i), -1));
                    }
                }
                for (int i = 0; i < PARAMS; ++i) wv[3 + MIDI_DESTS + i] = op1f_lfo4_default(kind, i);
                if (STOCK_JSON_HAS(w, KEY_PARAMS)) {
                    void* params = STOCK_JSON_GET(w, KEY_PARAMS);
                    if (is_array(params)) {
                        int n = STOCK_JSON_SIZE(params);
                        for (int i = 0; i < PARAMS && i < n; ++i) wv[3 + MIDI_DESTS + i] = json_number(STOCK_JSON_AT(params, i), wv[3 + MIDI_DESTS + i]);
                    }
                }
            }
            char name[NAME_CHARS + 1];
            name[0] = 0;
            if (STOCK_JSON_HAS(&root, KEY_NAME)) {
                void* e = STOCK_JSON_GET(&root, KEY_NAME);
                if (e && *((u8*)e + 4) == JSON_STRING_KIND) json_text(e, name, NAME_CHARS);
            }
            for (int c = 0; name[c]; ++c) name[c] = (name[c] >= 32 && name[c] < 127) ? lower_of(name[c]) : ' ';
            name_trim(name);
            if (!name[0]) default_name(name, slot);
            name_copy(presets.name[slot], name);
            presets.used[slot] = 1;
        }
    }
    STOCK_JSON_DTOR(&root);
}

// The folder's sixty-four files. R2: the folder is the base's to make, and the scan makes no file: a slot whose file is
// not there, or whose file is still to be taken off the disk, is an empty slot. A file whose time or size changed
// since the last read (or every file when forced) is read again. While the presets of user/lf/ are being copied into
// the folder the store is shut: the scan is owed, and runs when the copy has ended (op1f_lfo_presets_owed).
static void lfo_scan(int force) {
    start();
    if (!op1f_r2_move_done()) {
        u32 want = force ? 2u : 1u;
        if (r2.owed < want) r2.owed = want;
        return;
    }
    char folder[PATH_ROOM];
    if (op1f_path(&op1f_card_lfo, PRESETS_FOLDER, 0, folder, PATH_ROOM) <= 0) folder[0] = 0;
    int reread = 0, made = 0;
    for (int s = 0; s < PRESETS; ++s) {
        char path[PATH_ROOM];
        int size = lfo_path(s, path) ? STOCK_FILE_SIZE(path) : -1;
        if (size < 0 || removal_owed(s)) {                                     // no file, or one to be taken off: an empty slot, and no file made
            presets.used[s] = 0;
            presets.mtime[s] = 0;
            presets.size[s] = -1;
            continue;
        }
        u32 mtime = STOCK_FILE_MTIME(path);
        if (force || !presets.scanned || mtime != presets.mtime[s] || size != presets.size[s]) {
            read_slot(s);
            presets.mtime[s] = mtime;
            presets.size[s] = size;
            ++reread;
        }
    }
    presets.scanned = 1;
    ++ls.scans;
    ls.rereads += (u32)reread;
    rebuild_order();
    if (ls.loaded_slot >= 0 && !presets.used[ls.loaded_slot]) ls.loaded_slot = -1;   // the computer emptied the slot the lamp stood on
    STOCK_LOG(3, LOG_FILE, 19, "lfo presets: scan %u of %s: %d files read, %d made, %d slots used, %u writes failed in all", ls.scans, folder, reread, made, presets.count, ls.write_failures);
}

// The chain unit's first file load (the boot's first apply): the folder and its files, where the effect presets'
// first scan runs, so the disk mode always finds sixty-four files.
extern "C" void op1f_lfopresets_scan() asm("op1f_lfopresets_scan");
extern "C" void op1f_lfopresets_scan() { lfo_scan(1); }

// R2: this unit's own rule of a file that holds a preset, the one read_slot reads by: an object whose lfos are an
// array of four or more. The copy of the presets of user/lf/ asks it of each file (build_r2_move.cpp).
extern "C" int op1f_lfo_holds_preset(void* json) asm("op1f_lfo_holds_preset");
extern "C" int op1f_lfo_holds_preset(void* json) {
    if (!json || *((u8*)json + 4) != JSON_OBJECT_KIND || !STOCK_JSON_HAS(json, KEY_LFOS)) return 0;
    void* lfos = STOCK_JSON_GET(json, KEY_LFOS);
    return is_array(lfos) && STOCK_JSON_SIZE(lfos) >= WAGONS;
}

// R2: from this pack's frame call, once the presets are copied: the files of the presets deleted on the unit taken off
// the disk through the base, then the scan asked while the store was shut, and the first scan in any case: from the
// frame call the copy ends (or the first, where its mark is found) the store has been read, so that a save, a rename
// and a delete are never refused for want of a scan. The base takes a file off only in the pack's own frame call and
// not while the disk mode is open. When it answers not now, the computer has the folder: every removal still owed is
// dropped, never tried later (the file under that name may by then be one the computer brought), the files stay, and
// a forced scan puts the presets back in the list: the deletes did not happen.
extern "C" void op1f_lfo_presets_owed(void) asm("op1f_lfo_presets_owed");
extern "C" void op1f_lfo_presets_owed(void) {
    if (r2.removing[0] | r2.removing[1]) {
        for (int s = 0; s < PRESETS; ++s) {
            if (!removal_owed(s)) continue;
            char name[8];
            slot_name(s, name);
            int answer = op1f_remove(&op1f_card_lfo, PRESETS_FOLDER, name);
            if (answer == OP1F_NOT_NOW) {
                u32 dropped = 0;
                for (int d = 0; d < PRESETS; ++d) if (removal_owed(d)) ++dropped;
                r2.removing[0] = r2.removing[1] = 0;
                r2.owed = 2u;
                r2.dropped += dropped;
                if (r2.remove_lines < (u32)REMOVE_LINES) {
                    ++r2.remove_lines;
                    STOCK_LOG(2, LOG_FILE, 38, "%s: the disk mode was open: %u delete(s) did not happen, their files stay and the presets are in the list again", "lfo presets", dropped);
                }
                break;
            }
            r2.removing[s >> 5] &= ~(1u << (s & 31));
            if (answer == 0 || answer == OP1F_NO_FILE) { ++r2.removed; continue; }
            ++r2.remove_failures;
            if (r2.remove_lines < (u32)REMOVE_LINES) {
                ++r2.remove_lines;
                STOCK_LOG(1, LOG_FILE, 37, "lfo presets: the file of deleted slot %d could NOT be taken off the disk (%d): the preset is back at the next start", s + 1, answer);
            }
        }
    }
    u32 owed = r2.owed;
    if (owed || !presets.scanned) {                                            // the store read as soon as the copy has ended well, asked or not
        r2.owed = 0;
        lfo_scan(owed == 2u);
    }
}

// R2: what a shut store did not write, said from the frame work (a save or a rename may come on the key thread).
static void r2_lines() {
    u32 writes = r2.shut_writes, deletes = r2.shut_deletes;
    if ((writes != r2.said_writes || deletes != r2.said_deletes) && r2.shut_lines < (u32)SHUT_LINES) {
        ++r2.shut_lines;
        STOCK_LOG(2, LOG_FILE, 36, "lfo presets: the store is shut until the presets are copied into the pack's folder: %u saves or renames and %u deletes wrote nothing", writes, deletes);
    }
    r2.said_writes = writes;
    r2.said_deletes = deletes;
}

// ---------------------------------------------------------------- the list

static void flash(int kind, int slot) {
    ls.flash_kind = kind;
    ls.flash_slot = slot;
    ls.flash_until_ms = now_ms() + (u32)FLASH_MS;
}

static void list_clamp() {                                                  // the cursor inside the list, the window around the cursor
    int n = presets.count;
    if (ls.cursor >= n) ls.cursor = n - 1;
    if (ls.cursor < 0) ls.cursor = 0;
    if (ls.editing) { ls.first = ls.cursor; return; }                       // the edited preset on the first row
    int max_first = n > LIST_ROWS ? n - LIST_ROWS : 0;
    if (ls.first > max_first) ls.first = max_first;
    if (ls.first < 0) ls.first = 0;
    if (ls.cursor < ls.first) ls.first = ls.cursor;
    if (ls.cursor >= ls.first + LIST_ROWS) ls.first = ls.cursor - LIST_ROWS + 1;
}

// The lamp stays on the slot the four LFOs came from only while they still stand as they stood then.
static void check_loaded() {
    if (ls.loaded_slot < 0) return;
    int standing[PRESET_INTS];
    op1f_lfo4_preset_capture(standing);
    for (int i = 0; i < PRESET_INTS; ++i) if (standing[i] != ls.loaded_values[i]) { ls.loaded_slot = -1; return; }
}

static void list_open() {
    if (&op1f_presets_available == 0 || !op1f_presets_available()) return;  // no list screen: shift + T4 does nothing
    lfo_scan(0);                                                             // a file the computer changed through the disk mode is a preset now
    check_loaded();
    ls.open = 1;
    ls.editing = 0;
    int row = row_of_slot(ls.loaded_slot);
    if (row >= 0) ls.cursor = row;
    list_clamp();
    u32 t = now_ms();
    int down = op1f_lfotrain_release_knobs();                                // a knob held on the page fires no hold under the list, and the click of its release is not a tap on the list
    for (int k = 0; k < WAGONS; ++k) { ls.pressed[k] = 0; ls.acted[k] = 0; ls.swallow_until[k] = t; ls.held_at_open[k] = (down >> k) & 1; }
    ++ls.opens;
}

static void list_close() {
    ls.open = 0;
    ls.editing = 0;
}

static void event_line(const char* what, int s) {
    if (ls.event_lines >= (u32)EVENT_LINES) return;
    ++ls.event_lines;
    Op1fLfoTrainView v;
    op1f_lfotrain_view(&v);
    STOCK_LOG(3, LOG_FILE, 14, "lfo presets: %s slot %d (%s), synth s%d: %s, %s, %s, %s; on %d%d%d%d; the searchlight on knob %d; %u values held to their range in all",
              what, s + 1, (s >= 0 && s < PRESETS) ? presets.name[s] : "", v.slot,
              op1f_lfo4_kind_name(v.kind[0]), op1f_lfo4_kind_name(v.kind[1]), op1f_lfo4_kind_name(v.kind[2]), op1f_lfo4_kind_name(v.kind[3]),
              v.on[0], v.on[1], v.on[2], v.on[3], v.lamp, ls.held);
}

// The sound's four LFOs into the lowest free slot, as lfo NN. 0 when the list is full, or when the file did not land
// (the disk is full): the list says list full either way, and shows no preset the next boot would not find.
static int preset_save() {
    int s = free_slot();
    if (s < 0) { flash(FLASH_FULL, presets.count ? presets.order[ls.cursor] : -1); return 0; }
    default_name(presets.name[s], s);
    op1f_lfo4_preset_capture(presets.values[s]);
    presets.used[s] = 1;
    if (!write_slot(s)) {
        presets.used[s] = 0;
        flash(FLASH_FULL, presets.count ? presets.order[ls.cursor] : -1);
        return 0;
    }
    rebuild_order();
    ls.cursor = row_of_slot(s);
    ls.editing = 0;
    list_clamp();
    ls.loaded_slot = s;
    for (int i = 0; i < PRESET_INTS; ++i) ls.loaded_values[i] = presets.values[s][i];
    flash(FLASH_SAVED, s);
    ls.save_state = SAVE_SAVED;                                                // "saved!" with the check for a second (the save screen), then the list
    ls.save_slot = s;
    ls.save_ms = now_ms();
    ++ls.saves;
    event_line("saved", s);
    return 1;
}

static void preset_load(int row) {
    if (row < 0 || row >= presets.count) return;
    int s = presets.order[row];
    ls.held += (u32)op1f_lfo4_preset_apply(presets.values[s]);                  // the lamp of the page stays on its knob; the instances follow at the next frames
    ls.loaded_slot = s;
    op1f_lfo4_preset_capture(ls.loaded_values);                                // as the sound stands now (a value held to its range, a choice of the lamp's wagon written as its knob)
    ++ls.loads;
    event_line("loaded", s);
}

static void preset_delete_now(int s) {
    if (s < 0 || s >= PRESETS || !presets.used[s]) return;
    event_line("deleted", s);
    presets.used[s] = 0;
    if (op1f_r2_move_done() && presets.scanned) r2.removing[s >> 5] |= 1u << (s & 31);   // R2: the file is taken off the disk at the pack's next frame call (op1f_lfo_presets_owed)
    else ++r2.shut_deletes;                                                    // the store is shut: nothing is written
    rebuild_order();
    if (ls.loaded_slot == s) ls.loaded_slot = -1;
    list_clamp();
    ++ls.deletes;
}

static void delete_begin() {
    if (!presets.count || ls.pending_delete_slot >= 0 || ls.editing) return;
    int s = presets.order[ls.cursor];
    ls.pending_delete_slot = s;
    flash(FLASH_DELETED, s);                                                   // the row reads deleted for a second, then the list closes up
}

static void rename_begin() {
    if (!presets.count) return;
    ls.editing = 1;
    ls.caret = 0;
    const char* name = presets.name[presets.order[ls.cursor]];
    int c = 0;
    for (; c < NAME_CHARS && name[c]; ++c) ls.buffer[c] = name[c];
    for (; c < NAME_CHARS; ++c) ls.buffer[c] = ' ';
    ls.buffer[NAME_CHARS] = 0;
    list_clamp();
}

static void rename_keep() {
    if (!ls.editing) return;
    ls.editing = 0;
    if (ls.cursor < 0 || ls.cursor >= presets.count) { list_clamp(); return; }
    int s = presets.order[ls.cursor];
    char name[NAME_CHARS + 1];
    name_copy(name, ls.buffer);
    name_trim(name);
    if (name[0] == 0) default_name(name, s);                                   // all spaces: the slot's name
    name_copy(presets.name[s], name);
    write_slot(s);
    list_clamp();
    ++ls.renames;
    event_line("renamed", s);
}

static void rename_cancel() {
    ls.editing = 0;
    list_clamp();
}

static void t4_press() {
    ls.t4_pressed = 1;
    ls.t4_ms = now_ms();
    ls.t4_acted = 0;
}

static void t4_release() {
    ls.t4_pressed = 0;
    if (!ls.t4_acted) {                                                        // the tap (any release before the save): the list opens or closes
        if (ls.save_state == SAVE_SAVING) ls.save_state = SAVE_NONE;           // "saving" was showing: cancelled
        if (ls.open) list_close(); else if (op1f_lfotrain_open()) list_open();
    }
    STOCK_REQUEST_SCREEN();                                                    // the GUI asks the screen-id virtual now (a release alone may not)
    if (&op1f_train_page_now) op1f_train_page_now(0);                          // R2: the page follows the key (fx train's page step, where that pack runs)
}

static void list_press(int knob) {
    ls.pressed[knob] = 1;
    ls.press_ms[knob] = now_ms();
    ls.acted[knob] = 0;
    ls.held_at_open[knob] = 0;
}

static void list_release(int knob) {
    u32 t = now_ms();
    if (ls.held_at_open[knob]) {                                               // down since before the list opened: the click stock makes of this release is passed over
        ls.held_at_open[knob] = 0;
        ls.swallow_until[knob] = t + (u32)SWALLOW_MS;
    }
    if (ls.pressed[knob] && ls.acted[knob]) ls.swallow_until[knob] = t + (u32)SWALLOW_MS;   // the click that follows the red hold is not a tap
    ls.pressed[knob] = 0;
}

static void list_click(int knob, int shifted) {
    u32 t = now_ms();
    if ((i32)(ls.swallow_until[knob] - t) > 0) {                             // the click after the red hold
        ls.swallow_until[knob] = t;
        return;
    }
    if (shifted) return;
    if (ls.editing) {
        if (knob == KNOB_GREY) rename_keep();
        else if (knob == KNOB_RED) rename_cancel();
        return;
    }
    if (knob == KNOB_BLUE) { preset_load(ls.cursor); list_close(); }
    else if (knob == KNOB_OCHRE) rename_begin();
    else if (knob == KNOB_GREY) preset_save();
}

// A knob turned on the LFO page: 1 when the list took it (the scroll, the caret, the letter).
extern "C" int op1f_lfopresets_turn(int knob, int delta) asm("op1f_lfopresets_turn");
extern "C" int op1f_lfopresets_turn(int knob, int delta) {
    if (!ls.open) return 0;
    int step = delta > 0 ? 1 : -1;
    if (ls.editing) {
        if (knob == KNOB_BLUE) {
            int c = ls.caret + step;
            if (c >= 0 && c < NAME_CHARS) ls.caret = c;
        } else if (knob == KNOB_OCHRE) {
            char* ch = &ls.buffer[ls.caret];
            int i = alphabet_index(*ch) + step;
            if (i < 0) i = ALPHABET_COUNT - 1;
            if (i >= ALPHABET_COUNT) i = 0;
            *ch = ALPHABET[i];
        }
        return 1;
    }
    if (knob == KNOB_BLUE && presets.count) {
        ls.cursor += step;
        list_clamp();
    }
    return 1;
}

// A key-down on the LFO page (shifted: with shift held), before the page's own handling. -1: not the list's key.
extern "C" int op1f_lfopresets_key_down(int key, int shifted) asm("op1f_lfopresets_key_down");
extern "C" int op1f_lfopresets_key_down(int key, int shifted) {
    start();
    if (!ls.open) {
        if (key == KEY_T4 && shifted) { t4_press(); return 1; }               // shift + T4 on the page: a tap opens the list at the release, a hold saves
        return -1;
    }
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { list_press(key - KEY_PRESS0); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) { list_click(key - KEY_CLICK0, shifted); return 1; }
    if (key == KEY_T4) {
        if (shifted) { t4_press(); return 1; }                                // the tap closes the list at the release, the hold saves
        if (ls.editing) rename_keep(); else preset_load(ls.cursor);
        list_close();                                                         // back to the page (the GUI re-asks the screen after this key)
        return 1;
    }
    if (key >= KEY_T1 && key < KEY_T4) { list_close(); return -1; }           // another page: the list closes, the page's own handling closes the page and stock goes on
    return shifted ? 1 : -1;                                                  // shift + another key on the list: nothing; any other key is stock's
}

// Every key-up of the synth: the list's knobs let go, and T4 let go (the tap of shift + T4; the hold has acted already).
extern "C" void op1f_lfopresets_key_up(int key) asm("op1f_lfopresets_key_up");
extern "C" void op1f_lfopresets_key_up(int key) {
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) list_release(key - KEY_PRESS0);
    if (key == KEY_T4 && ls.t4_pressed) t4_release();
}

// The save screen over everything, the list over the page; 0 for the page or stock.
extern "C" int op1f_lfopresets_screen_wanted() asm("op1f_lfopresets_screen_wanted");
extern "C" int op1f_lfopresets_screen_wanted() {
    if (ls.save_state != SAVE_NONE) return OP1F_SAVE_SCREEN_ID;
    if (ls.open) return OP1F_PRESETS_SCREEN_ID;
    return 0;
}

// The list's view: Build H's presets unit dresses the two screens from it while the effect presets show nothing.
extern "C" void op1f_lfopresets_view(Op1fPresetsView* out) asm("op1f_lfopresets_view");
extern "C" void op1f_lfopresets_view(Op1fPresetsView* out) {
    out->open = ls.open;
    out->count = presets.count;
    out->first = ls.first;
    out->cursor = ls.cursor;
    out->loaded = row_of_slot(ls.loaded_slot);
    out->editing = ls.editing;
    out->caret = ls.caret;
    for (int r = 0; r < LIST_ROWS; ++r) {
        int i = ls.first + r;
        int s = i < presets.count ? presets.order[i] : -1;
        const char* text = s >= 0 ? presets.name[s] : "";
        if (ls.flash_kind != FLASH_NONE && s >= 0 && ls.flash_slot == s) text = ls.flash_kind == FLASH_SAVED ? "saved" : (ls.flash_kind == FLASH_DELETED ? "deleted" : "list full");
        if (ls.flash_kind == FLASH_FULL && ls.flash_slot < 0 && r == 0) text = "list full";
        if (presets.count == 0 && r == 0 && ls.flash_kind == FLASH_NONE) text = "no presets";
        if (ls.editing && r == 0) text = "";                                   // the cells show the name
        name_copy(ls.row_text[r], text);
        out->row[r] = ls.row_text[r];
    }
    const int* v = presets.count ? presets.values[presets.order[ls.cursor < presets.count ? ls.cursor : 0]] : 0;
    for (int k = 0; k < WAGONS; ++k) {                                         // the highlighted preset's four LFOs, one row a knob in its colour
        int kind = v ? v[WAGONS + k * WAGON_INTS] : KIND_NONE;
        int on = v ? v[WAGONS + k * WAGON_INTS + 1] : 1;
        char* p = ls.wagon_text[k];
        if (kind >= 0 && kind < KINDS) {
            p = put_text(p, op1f_lfo4_kind_name(kind));                        // stock's own lowercase names
            if (!on) p = put_text(p, " off");
        } else {
            *p++ = '-';
        }
        *p = 0;
        out->wagon[k] = ls.wagon_text[k];
    }
    for (int c = 0; c < NAME_CHARS; ++c) {
        ls.cell_text[c][0] = ls.buffer[c] ? ls.buffer[c] : ' ';
        ls.cell_text[c][1] = 0;
        out->cell[c] = ls.cell_text[c];
    }
    out->save_state = ls.save_state;
    out->save_host = 0;                                                        // the synth's box on the save screen
    out->save_number = ls.save_slot + 1;
    u32 held = now_ms() - ls.save_ms;
    int spokes = ls.save_state == SAVE_SAVING ? (int)udiv(held * (u32)OP1F_SAVE_SPOKES, (u32)SAVE_HOLD_MS) : OP1F_SAVE_SPOKES;
    out->save_progress = spokes > OP1F_SAVE_SPOKES ? OP1F_SAVE_SPOKES : spokes;
}

// J5i: the screen request from the frame call (the display thread), behind the mask the binding's note asks for.
// The request at the key's release (t4_release) runs on the key thread, as stock's own, and stays plain.
static void request_screen_from_frame() {
    u32 mask;
    STOCK_IRQ_OFF(mask);
    STOCK_REQUEST_SCREEN();
    STOCK_IRQ_ON(mask);
}

// Every frame, after the LFO unit's: the hold of shift + T4, the red hold, the flash, and the counts on the log.
extern "C" void op1f_lfopresets_frame() asm("op1f_lfopresets_frame");
extern "C" void op1f_lfopresets_frame() {
    start();
    u32 t = now_ms();
    if (!op1f_lfotrain_open()) {                                               // the page left the screen (another page's key, another mode): the list, the hold and the save screen go with it
        ls.open = 0;
        ls.editing = 0;
        ls.t4_pressed = 0;
        ls.save_state = SAVE_NONE;
    }
    if (ls.t4_pressed && !ls.t4_acted) {
        u32 held = t - ls.t4_ms;
        if (held >= (u32)SAVE_HOLD_MS) {                                       // the hold: the save (its screen says saved!)
            ls.t4_acted = 1;
            if (!preset_save()) ls.save_state = SAVE_NONE;                     // the list is full: no saved!, the list says so
            if (!ls.open) list_open();                                         // the list shows the new preset after the confirmation
            request_screen_from_frame();
        } else if (held >= (u32)SAVE_SHOW_MS && ls.save_state == SAVE_NONE) {  // "saving" with the clock filling towards the save
            ls.save_state = SAVE_SAVING;
            ls.save_slot = free_slot();                                        // the number the save would take (-1 when the list is full)
            ls.save_ms = ls.t4_ms;
            request_screen_from_frame();
        }
    }
    if (ls.save_state == SAVE_SAVED && t - ls.save_ms >= (u32)SAVED_MS) {      // the confirmation ends, the list is back
        ls.save_state = SAVE_NONE;
        request_screen_from_frame();
    }
    if (ls.open && ls.pressed[KNOB_RED] && !ls.acted[KNOB_RED] && t - ls.press_ms[KNOB_RED] >= (u32)HOLD_MS) {
        ls.acted[KNOB_RED] = 1;
        delete_begin();
    }
    if (ls.flash_kind != FLASH_NONE && (i32)(t - ls.flash_until_ms) >= 0) {
        if (ls.flash_kind == FLASH_DELETED && ls.pending_delete_slot >= 0) preset_delete_now(ls.pending_delete_slot);
        ls.pending_delete_slot = -1;
        ls.flash_kind = FLASH_NONE;
    }
    r2_lines();                                                                // R2: what a shut store did not write
    if (++ls.print_countdown >= (u32)PRINT_FRAMES) {                           // the counts, when something happened since the last line
        ls.print_countdown = 0;
        u32 counted = ls.opens + ls.saves + ls.loads + ls.deletes + ls.renames + ls.scans + ls.writes;
        if (counted != ls.counted && ls.count_lines < (u32)COUNT_LINES) {
            ls.counted = counted;
            ls.count_lines += 1;                                                   // R2: one line now: the disk's went
            STOCK_LOG(3, LOG_FILE, 18, "lfo presets: %d slots used, %u opens, %u saves, %u loads, %u deletes, %u renames, %u scans, %u files read, %u written, %u writes failed; list %s cursor %d loaded slot %d",
                      presets.count, ls.opens, ls.saves, ls.loads, ls.deletes, ls.renames, ls.scans, ls.rereads, ls.writes, ls.write_failures, ls.open ? "open" : "closed", ls.cursor, ls.loaded_slot);
        }
    }
}
