// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R2, the packs `fx train` and `lfo`: the presets of before copied into the pack's own folder, once. One unit for
// both packs: each links it and calls it from its own frame function with its own card, its old folder and its own rule
// of a file that holds a preset.
//
// Until R2 the effect presets lay in user/fx/ and the LFO presets in user/lf/, sixty-four files each (01.json to
// 64.json, {} for an empty slot). From R2 each pack keeps its presets in a folder of its own that the base makes, the
// first folder of its card (user/pf/fx/, user/pf/lfo/). This unit brings the presets over, one slot at each call, so
// that every preset the unit held is found in the new place:
//
//   * Slot NN is copied when the old file holds a preset by the pack's own rule (`holds_preset`, the one the pack's
//     reader goes by) and the pack's folder has no file NN.json with something in it yet: an empty file (0 bytes)
//     under the slot's name holds nothing and is no preset, and the copy goes over it. The document is written whole,
//     as the firmware's reader made it, by the firmware's writer, so keys the pack does not know are kept; the new
//     file's size is read back, and more than 0 is a file that landed. An empty slot's {}, a slot with no old file and a file that cannot
//     be read as json are not copied.
//   * The old files are only read: never written, renamed or removed. A file with something in it already in the
//     pack's folder is never written over: it is kept, and counted. The copy takes no file off the disk.
//   * When all sixty-four slots were looked at with no failed write, the mark moved.json is written into the card's
//     second folder, the pack's private one, through the base (op1f_write). At every later start the mark is read
//     first (op1f_read), and then nothing is copied.
//   * A write that does not land stops the copy for this start, with one line and no mark. The next start walks the
//     slots again from the first: a slot whose new file has something in it is kept, so nothing is written twice and
//     nothing is written over; an empty file a failed write left is copied over then.
//   * While the disk mode is open (the frame's `disk`) the computer owns the folders, and the copy waits.
//
// The pack's store stays shut until the copy has ended well, that is the mark was found, or every slot was looked at
// with no failed write: op1f_r2_move_done answers it, from any thread, as a single word. A mark that could not be
// written is said on the log and does not shut the store: the next start looks at every slot again and writes over
// nothing.
//
// The firmware's calls are the ones the presets' own units bind, each as our code meets it: the json reader, handed a
// C path and a json value (it answers 0 for a file that is not there or not json, and the value stays the caller's
// to destroy); the json writer, handed a firmware string and the value (1 when the file was written and put in place,
// 0 otherwise; it keeps nothing of the string, whose text, longer than fifteen characters, lies on the heap and is
// freed here); the size of a file (-1 for no file, and no line on the log); the json value's maker and destructor, the
// string's maker and the free that matches it; the logger. Nothing is divided (the module links no runtime).
// Compiled for the PC as it is, with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for
// tests this tree does not hold).

#include "build_r1_card.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*JsonCtorFn)(void* json, int kind);                    // 0x8c085ba8: a fresh value of that kind; 0 is null
typedef void (*JsonDtorFn)(void* json);                              // 0x8c0844e0
typedef int (*JsonReadFileFn)(const char* path, void* json);         // 0x8c0534d0: the file parsed into the json, non-zero on success
typedef int (*JsonWriteFileFn)(void* path_string, void* json);       // 0x8c053168: the json written to the path, a stock string object; 0 when that fails
typedef void (*StringCtorFn)(void* out, const char* text);           // 0x8c03e434: a stock string object from text
typedef void (*FreeFn)(void* block);                                 // 0x8d4e63cc: the match of the allocator stock's strings use
typedef int (*FileSizeFn)(const char* path);                         // 0x8c051f9c: the file's size, -1 when it is not there

#ifdef OP1F_HOST
#include "mock_move_r2.h"                              // the PC's stand-ins: the logger, the json values, a folder of the PC for the unit's disk
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#endif

static const char LOG_FILE[] = "../src/main/op1f_presets_move.cpp";
static const char MARK[] = "moved.json";                                         // in the card's second folder, the pack's private one

enum { SLOTS = 64, PRESETS_FOLDER = 0, PRIVATE_FOLDER = 1, PATH = 40, NAME = 8, MARK_TEXT = 128, FRAME_WITH_DISK = 36 };
enum { START = 0, WALKING = 1, ENDED = 2, STOPPED = 3 };

struct Json { u32 words[4]; };                                                   // a stock json value is 12 bytes; 16 for alignment

static struct {
    volatile u32 done;                                                           // 1 once the copy has ended well: the store may be used
    u32 state;                                                                   // START, WALKING, ENDED or STOPPED (a failed write: for this start)
    u32 next;                                                                    // the slot the next call looks at, 1 to 64; 65: the mark
    u32 copied, kept, empty, unread;                                             // this start's counts; unread: old files that are no json
} mv;

// NN.json, the slot's number in two digits, by subtraction: nothing is divided.
static void slot_name(u32 slot, char* out) {
    u32 tens = 0, ones = slot;
    while (ones >= 10u) { ones -= 10u; ++tens; }
    out[0] = (char)('0' + tens);
    out[1] = (char)('0' + ones);
    const char* tail = ".json";
    u32 at = 2;
    while (*tail) out[at++] = *tail++;
    out[at] = 0;
}

// `text` behind out[at], cut where the room ends; the place of the end. `fits` is cleared when it was cut.
static u32 put(char* out, u32 at, u32 room, const char* text, int* fits) {
    while (*text) {
        if (at + 1u >= room) { *fits = 0; break; }
        out[at++] = *text++;
    }
    out[at] = 0;
    return at;
}

// A number in decimal behind out[at], by subtraction.
static u32 put_number(char* out, u32 at, u32 room, u32 value, int* fits) {
    static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };
    char digits[12];
    u32 n = 0;
    for (u32 i = 0; i < 10u; ++i) {
        u32 d = 0;
        while (value >= TENS[i]) { value -= TENS[i]; ++d; }
        if (d || n || i == 9u) digits[n++] = (char)('0' + d);
    }
    digits[n] = 0;
    return put(out, at, room, digits, fits);
}

// The old folder as the pack hands it: a short path of plain characters ending in a slash, which goes into the mark's
// text as it is.
static int plain_folder(const char* folder) {
    u32 n = 0;
    for (; folder[n]; ++n) {
        char c = folder[n];
        if ((u8)c < 0x20 || (u8)c > 0x7e || c == '"' || c == '\\') return 0;
        if (n + (u32)NAME >= (u32)PATH) return 0;
    }
    return n > 0 && folder[n - 1] == '/';
}

static void stop() { mv.state = STOPPED; }                                   // for this start: the store stays shut

// Slot `slot` (1 to 64) looked at: 1 when the copy goes on, 0 when a write did not land.
static int copy_slot(const Op1fCard* card, const char* old_folder, int (*holds_preset)(void* json), u32 slot) {
    char name[NAME];
    slot_name(slot, name);
    char fresh[PATH];
    int got = op1f_path(card, PRESETS_FOLDER, name, fresh, PATH);
    if (got <= 0) {
        STOCK_LOG(1, LOG_FILE, 4, "presets move: %s: slot %u: the pack's folder gives no place for it (%d): the copy goes on at the next start", card->name, slot, got);
        return 0;
    }
    if (STOCK_FILE_SIZE(fresh) > 0) { ++mv.kept; return 1; }                  // a file with something in it is there already: never written over; an empty one is no file
    char old[PATH];
    int fits = 1;
    put(old, put(old, 0, PATH, old_folder, &fits), PATH, name, &fits);
    if (!fits || STOCK_FILE_SIZE(old) < 0) { ++mv.empty; return 1; }          // no old file: nothing to copy (its size first: the reader would say so on the log)
    Json doc;
    STOCK_JSON_CTOR(&doc, 0);
    int read = STOCK_JSON_READ_FILE(old, &doc);
    if (!read || !holds_preset(&doc)) {                                       // {}, or no json: not a preset, not copied
        STOCK_JSON_DTOR(&doc);
        ++mv.empty;
        if (!read) ++mv.unread;
        return 1;
    }
    u32 str[12];
    for (int i = 0; i < 12; ++i) str[i] = 0;
    STOCK_STRING_CTOR(str, fresh);
    int written = STOCK_JSON_WRITE_FILE(str, &doc);                           // a temporary file renamed over the new one; 0 when that fails
    if (str[6] >= 16u && str[1]) STOCK_FREE((void*)str[1]);                   // the path's text lives on the heap; the writer keeps nothing of it
    STOCK_JSON_DTOR(&doc);
    int size = STOCK_FILE_SIZE(fresh);
    if (written && size > 0) { ++mv.copied; return 1; }
    STOCK_LOG(1, LOG_FILE, 3, "presets move: %s: slot %u: the write failed (writer %d, size %d): the copy goes on at the next start", card->name, slot, written, size);
    return 0;
}

// All sixty-four looked at with no failed write: the mark, and the store opens whether the mark landed or not.
static void finish(const Op1fCard* card, const char* old_folder) {
    char text[MARK_TEXT];
    int fits = 1;
    u32 at = put(text, 0, MARK_TEXT, "{\"from\": \"", &fits);
    at = put(text, at, MARK_TEXT, old_folder, &fits);
    at = put(text, at, MARK_TEXT, "\", \"copied\": ", &fits);
    at = put_number(text, at, MARK_TEXT, mv.copied, &fits);
    at = put(text, at, MARK_TEXT, ", \"kept\": ", &fits);
    at = put_number(text, at, MARK_TEXT, mv.kept, &fits);
    at = put(text, at, MARK_TEXT, "}", &fits);
    int wrote = fits ? op1f_write(card, PRIVATE_FOLDER, MARK, text, at) : OP1F_TOO_LARGE;
    mv.state = ENDED;
    mv.done = 1;
    if (mv.unread)
        STOCK_LOG(2, LOG_FILE, 5, "presets move: %s: %u file(s) of %s could not be read as json: not copied, and left where they are", card->name, mv.unread, old_folder);
    if (wrote == (int)at)
        STOCK_LOG(3, LOG_FILE, 2, "presets move: %s: %u copied from %s, %u kept as they were, %u empty; the mark written", card->name, mv.copied, old_folder, mv.kept, mv.empty);
    else
        STOCK_LOG(2, LOG_FILE, 6, "presets move: %s: %u copied from %s, %u kept as they were, %u empty; the mark could NOT be written (%d): every slot is looked at again at the next start, and nothing written over",
                  card->name, mv.copied, old_folder, mv.kept, mv.empty, wrote);
}

// From the pack's own frame function, every frame, before anything reads the pack's store: one slot a call. Answers
// 1 once the copy has ended well, 0 while it has not (copying, waiting for the disk mode to close, or stopped for this
// start by a write that did not land).
extern "C" int op1f_r2_move_step(const Op1fCard* card, const char* old_folder, int (*holds_preset)(void* json), const Op1fFrame* now) OP1F_NAMED("op1f_r2_move_step");
extern "C" int op1f_r2_move_step(const Op1fCard* card, const char* old_folder, int (*holds_preset)(void* json), const Op1fFrame* now) {
    if (mv.state == ENDED) return 1;
    if (mv.state == STOPPED || card == 0 || old_folder == 0 || holds_preset == 0 || now == 0) return 0;
    if (now->bytes >= (u32)FRAME_WITH_DISK && now->disk != 0u) return 0;     // the computer owns the folders: the copy waits
    if (mv.state == START) {
        char mark[MARK_TEXT];
        int found = op1f_read(card, PRIVATE_FOLDER, MARK, mark, MARK_TEXT);
        if (found >= 0 || found == OP1F_TOO_LARGE) {                          // the mark: copied at an earlier start
            mv.state = ENDED;
            mv.done = 1;
            return 1;
        }
        if (found == OP1F_NOT_NOW) return 0;                                  // not the pack's frame call: asked again at the next
        if (found == OP1F_NO_FOLDER || !plain_folder(old_folder)) {
            STOCK_LOG(1, LOG_FILE, 1, "presets move: %s: %s: the presets of %s are NOT copied, and the pack's presets are not used at this start",
                      card->name, found == OP1F_NO_FOLDER ? "the base made no folder for the pack" : "the old folder is not a plain path", old_folder);
            stop();
            return 0;
        }
        STOCK_LOG(3, LOG_FILE, 1, "presets move: %s: the presets of %s are copied into the pack's folder, the old files stay", card->name, old_folder);
        mv.state = WALKING;
        mv.next = 1;
        mv.copied = mv.kept = mv.empty = mv.unread = 0;
    }
    if (mv.next <= (u32)SLOTS) {
        if (!copy_slot(card, old_folder, holds_preset, mv.next)) { stop(); return 0; }
        ++mv.next;
        return 0;
    }
    finish(card, old_folder);
    return 1;
}

// The same answer as a single word, from any thread, doing nothing: the store's scan, save, rename and delete ask it.
extern "C" int op1f_r2_move_done(void) OP1F_NAMED("op1f_r2_move_done");
extern "C" int op1f_r2_move_done(void) { return mv.done != 0u; }
