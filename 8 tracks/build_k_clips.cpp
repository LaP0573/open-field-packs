// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K, pack K4: the clips of tracks 5 to 8 in a list of their own.
//
// A tape's clips are kept twice by stock: in the clip store of the current snapshot record (Clips.cpp: 512 records
// of 28 bytes, a has-clip bitmap, a count of free records; the record is the track, four paddings, the start, the
// stop, and its index is the clip's id), and in the tape's document, the json of user/tapeNN.json, as the array
// "clips" of objects { id, ch, start, stop, padding }. The store is the one the firmware works on. The document
// follows it entry by entry, through four small functions of Clips.cpp that each name the array by its key and
// call the db (db2.cpp, the tape db is 2):
//
//   0x8d4ed7d4  the list cleared                  -> 0x8d4f23f8(2, "clips")             (from the relink 0x8d4ee6f8)
//   0x8d4ed7ec  a clip added                      -> 0x8d4f15a8(2, "clips", &object)    (from the add and the relink)
//   0x8d4edbfc  a clip changed                    -> 0x8d4f16ac(2, "clips", id, &object)
//   0x8d4ee37e  a clip removed                    -> 0x8d4f188c(2, "clips", id)
//
// and the loader 0x8d4eea84(store) fills the store from the document's "clips" when a tape is loaded (its two
// calls: the start-up at 0x8c0b4600 and the tape load 0x8c0d611c at 0x8c0d6cd0, each right after the document
// was read from the tape's file by 0x8d4ef84c, which has no other caller).
//
// Until K4 the clips of tracks 5 to 8 stood in "clips" with the others (ch 4 to 7). Stock firmware's loader holds
// every record against a validator that knows four tracks: it drops such a clip with a warning, and the entry is
// gone from the file at stock's next change of the list. So a unit that went to stock firmware and came back had
// lost the clips of tracks 5 to 8 of every tape it had loaded there.
//
// Since K4 they stand in a second array of the same document, "op1f_clips", with the same objects. The tape
// document is read whole, set key by key and written whole (0x8d4ef84c, 0x8d4f24dc, 0x8d4ef760: read for K2), so a
// key stock does not know stays through stock's saves, is copied with the file by stock's copy of a tape, and is
// gone after a format, which writes the file anew from a template: what a tape's clips should do. "clips" then
// names tracks 1 to 4 only, and stock firmware finds nothing in the file it has to drop.
//
// Eight hooks, the profile's group build-k4 (four of them through the stubs of build_k_clips.s):
//
//   * the four 8-byte loads of 0x8401ebe8 into R1 (the text "clips") in the four functions above, at 0x8d4ee340,
//     0x8d4ee506, 0x8d4ee91c and 0x8d4eeeec, each replaced by a CALL.a of a stub that leaves in R1 stock's text or
//     ours: by the "ch" of the object stock has just built (add, change), by the track of the store's record
//     (remove); the clear's stub clears the list of ours and then hands stock its own key;
//   * the two calls of stock's loader, retargeted to op1f_clips_load: stock's loader, then the list of ours;
//   * the tape list's call of 0x8d4ef8ec(tape), which reads a tape's json for the end of its last clip (the fill
//     shown for the highlighted tape), retargeted to op1f_tape_seconds, which counts both lists;
//   * the tape list's call of stock's clone, in build_k_copy.cpp.
//
// The store in memory is as it was: it holds the clips of all eight tracks, and undo and redo move between the
// snapshots' stores as before; the relink that follows them clears both lists and adds every clip to its own.
//
// At a load, after stock's loader:
//
//   * a clip of tracks 5 to 8 that stock's loader took from "clips" (a file of before K4, or one an older pack
//     wrote while it ran as the spare) stays in the store and is moved to the list of ours by the rebuild below;
//   * every entry of "op1f_clips" is held to the store's own rule (a track of 5 to 8, 0 <= start < stop, both
//     within the tape) and goes into the store under its id, or under a free one when the id is taken (stock
//     firmware gives its new clips the free ids it sees, and it does not see ours); an entry that overlaps a clip
//     already on its track is left out: what stock's loader brought is the later recording;
//   * a clip of tracks 5 to 8 none of whose chunks is mapped is taken out: it has nothing under it (the copy of a
//     tape made under stock firmware has the original's list and none of its audio of tracks 5 to 8). Not when
//     the tape's file of tracks 5 to 8 is a later pack's, which this pack leaves alone;
//   * when any of this changed something, stock's own relink writes both lists anew from the store.
//
// The list of ours is made in the document at every load that finds none: the db's functions say "database group
// not found" and do nothing for a key the document does not have.
//
// Once a start, after the first load, every other tape's file is looked at: a tape that has a file of tracks 5 to 8
// and still names a track of 5 to 8 in "clips" has those entries moved to "op1f_clips" in its file, so that a unit
// which has run this pack holds only tapes whose list stock firmware reads whole (as K1 did for the block maps
// and K3 for the clipboard). The json is read with stock's reader, the two arrays are built as stock's own remove
// builds its array (a new array, each kept entry pushed, the array assigned to the member: 0x8d4f188c), and the
// file is written with stock's writer, a temporary and a rename. The mounted tape is not touched there: its list
// is the document's, moved by the load above. A tape with no file of tracks 5 to 8 is left as it is: clips of
// tracks 5 to 8 in its list have nothing under them.
//
// Threads: the loader runs on the main thread, at the start-up and from the tape list, as stock's does, reading the
// document as stock's loader reads it. The four stubs run wherever stock's four functions run (the tape thread's
// handlers, the GUI thread's undo); they read the object on the caller's stack and the store, never the document,
// which the db's functions lock.
// Freestanding: no library, no division.

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*ClipsLoadFn)(void* store);                              // 0x8d4eea84: the store filled from the document's "clips"
typedef void (*ClipFillFn)(i32* record, i32 track, i32 start, i32 stop, const i32* paddings);   // 0x8d4ed7e8: paddings 0 for four times -1
typedef void (*ClipSetFn)(void* store, i32 id, const i32* record, i32 to_document);   // 0x8d4ee98c: the record at the id, the bit and the count
typedef void (*ClipRemoveFn)(void* store, i32 id);                     // 0x8d4eef00: the bit cleared, the entry removed from the document
typedef void (*ClipsRelinkFn)(void* store);                            // 0x8d4ee6f8: the document's list cleared and every clip added
typedef void* (*TapeDocFn)();                                          // 0x8d4f2b80: the tape db's document
typedef void (*DbSetFn)(int db, const char* key, void* value);         // 0x8d4f24dc: the member made or found, assigned, the db marked
typedef int (*DbClearFn)(int db, const char* key);                     // 0x8d4f23f8: the member, an array, made empty
typedef int (*MountedTapeFn)();                                        // 0x8fc219a8: the mounted tape's index, -1 for none
typedef void (*JsonCtorFn)(void* json, int kind);                      // 0x8c085ba8: 0 a null, 6 an array
typedef void (*JsonDtorFn)(void* json);                                // 0x8c0844e0
typedef int (*JsonHasFn)(void* json, const char* key);                 // 0x8c085068, of an object
typedef void* (*JsonGetFn)(void* json, const char* key);               // 0x8c084f74: the member
typedef void* (*JsonAtFn)(void* json, int index);                      // 0x8c084ee0: the array's element
typedef int (*JsonSizeFn)(void* json);                                 // 0x8c084c22, of an array
typedef int (*JsonIntFn)(void* json);                                  // 0x8c084840: the three number kinds
typedef int (*JsonReadFileFn)(const char* path, void* json);           // 0x8c0534d0: non-zero when the file was read and parsed
typedef void* (*JsonLookupFn)(void* json, const char* key);            // 0x8c085e84: the member, made when it is not there
typedef void (*JsonAssignFn)(void* target, void* value);               // 0x8c0845b8
typedef void (*JsonPushFn)(void* array, void* value);                  // 0x8c085d54: a copy of the value at the array's end
typedef int (*JsonWriteFileFn)(void* path_string, void* json);         // 0x8c053168: a temporary, then a rename; 0 when that fails
typedef void (*StringCtorFn)(void* out, const char* text);             // 0x8c03e434: a stock string object from text
typedef void (*FreeFn)(void* block);                                   // 0x8d4e63cc: the match of the allocator stock's strings use
typedef i32 (*FileSizeFn)(const char* path);                           // 0x8c051f9c: the bytes, -1 when there is no such file
typedef u32 (*FloatFromIntFn)(i32 value);                              // ROM 0x04021854: the float of an int, as bits

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_CLIPS_LOAD ((ClipsLoadFn)0x8d4eea84u)
#define STOCK_CLIP_FILL ((ClipFillFn)0x8d4ed7e8u)
#define STOCK_CLIP_SET ((ClipSetFn)0x8d4ee98cu)
#define STOCK_CLIP_REMOVE ((ClipRemoveFn)0x8d4eef00u)
#define STOCK_CLIPS_RELINK ((ClipsRelinkFn)0x8d4ee6f8u)
#define STOCK_TAPE_DOC ((TapeDocFn)0x8d4f2b80u)
#define STOCK_DB_SET ((DbSetFn)0x8d4f24dcu)
#define STOCK_DB_CLEAR ((DbClearFn)0x8d4f23f8u)
#define STOCK_MOUNTED_TAPE ((MountedTapeFn)0x8fc219a8u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FLOAT_FROM_INT ((FloatFromIntFn)0x04021854u)
#define STOCK_CLIPS_KEY ((const char*)0x8401ebe8u)                     // stock's own text "clips", the pointer its four functions pass
#define STOCK_STORE (*(u8* volatile*)0x8401ebe4u)                      // s_clips_instance: the current snapshot's store, 0 before the first

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,
    TAPES = 8,
    MAX_CLIPS = 512,
    RECORD_BYTES = 28,                           // +0 the track, +4 four paddings, +0x14 the start, +0x18 the stop
    RECORD_WORDS = 7,
    BITMAP_OFFSET = 0x3800,                      // 512 bits as 32 half-words: bit id & 15 of word id >> 4
    CHUNK_SHIFT = 14,                            // 16,384 frames a chunk
    FRAMES_A_SECOND = 44100,
    MAX_FRAME = 0x00f23fa0,                      // kMaxTapePos, as the store's validator has it
    TAPE_DB = 2,
    JSON_ARRAY = 6,                              // a json value's kind, its byte at +4
    JSON_OBJECT = 7,
    XEXT_NEWER = 5,                              // build_k_maps.h: a later pack's file of tracks 5 to 8, left alone
    LOG_BUDGET = 32
};
static const u32 MINUS_ONE_FLOAT = 0xbf800000u;  // what 0x8d4ef8ec answers for a tape whose json cannot be read

static const char LOG_FILE[] = "../src/db2/op1f_clips.cpp";
static const char OURS[] = "op1f_clips";
static const char KEY_ID[] = "id";
static const char KEY_CH[] = "ch";
static const char KEY_START[] = "start";
static const char KEY_STOP[] = "stop";
static const char KEY_PADDING[] = "padding";

struct Json { u32 words[4]; };                   // a stock json value is 12 bytes; 16 for alignment

extern "C" int op1f_ext_chunk_mapped(u32 track, u32 logical) asm("op1f_ext_chunk_mapped");   // build_b_storage.cpp
extern "C" int op1f_ext_load_tape asm("op1f_ext_load_tape");
extern "C" int op1f_ext_load_state asm("op1f_ext_load_state");

extern "C" const char* op1f_clips_key_of_object(void* object) asm("op1f_clips_key_of_object");
extern "C" const char* op1f_clips_key_of_id(i32 id) asm("op1f_clips_key_of_id");
extern "C" void op1f_clips_clear_ours() asm("op1f_clips_clear_ours");
extern "C" void op1f_clips_load(void* store) asm("op1f_clips_load");
extern "C" u32 op1f_tape_seconds(int tape) asm("op1f_tape_seconds");
extern "C" u32 op1f_clips_counts[14] asm("op1f_clips_counts");
// loads, lists made, from ours, from stock's list, overlapping, not usable, with nothing under them, renumbered,
// rebuilds, entries sent to the list of ours, clears, tape lengths asked, files rewritten by the sweep, its writes
// that failed
u32 op1f_clips_counts[14];

static u32 log_budget = LOG_BUDGET;
static u32 swept;

static void tape_path(char* path, int tape, const char* suffix) {      // "user/tapeNN" + suffix, NN = tape + 1 (1 to 8)
    const char* prefix = "user/tape";
    while (*prefix) *path++ = *prefix++;
    *path++ = '0';
    *path++ = (char)('1' + tape);
    while (*suffix) *path++ = *suffix++;
    *path = 0;
}

static int kind_of(const void* json) { return ((const u8*)json)[4]; }

static int extension_track(i32 track) { return track >= STOCK_TRACKS && track < EIGHT_TRACKS; }

static const i32* record_at(const u8* store, i32 id) { return (const i32*)(store + id * RECORD_BYTES); }

static int has_clip(const u8* store, i32 id) {
    const u16* bitmap = (const u16*)(store + BITMAP_OFFSET);
    return (bitmap[id >> 4] >> (id & 15)) & 1;
}

// ---------------------------------------------------------------------------------------------- the four stubs

// For stock's add and change: the object stock has just built on its own stack. Its "ch" decides the list.
const char* op1f_clips_key_of_object(void* object) {
    if (object && kind_of(object) == JSON_OBJECT && STOCK_JSON_HAS(object, KEY_CH)) {
        i32 ch = STOCK_JSON_INT(STOCK_JSON_GET(object, KEY_CH));
        if (extension_track(ch)) {
            ++op1f_clips_counts[9];
            return OURS;
        }
    }
    return STOCK_CLIPS_KEY;
}

// For stock's remove, which has the id alone: the store's record still names the track (the remove 0x8d4eef00
// clears the clip's bit and leaves its record).
const char* op1f_clips_key_of_id(i32 id) {
    const u8* store = STOCK_STORE;
    if (store && id >= 0 && id < MAX_CLIPS && extension_track(record_at(store, id)[0])) {
        ++op1f_clips_counts[9];
        return OURS;
    }
    return STOCK_CLIPS_KEY;
}

// For stock's clear, before it clears its own list.
void op1f_clips_clear_ours() {
    ++op1f_clips_counts[10];
    STOCK_DB_CLEAR(TAPE_DB, OURS);
}

// ---------------------------------------------------------------------------------------------- the loader

struct Entry { i32 id, track, start, stop; i32 padding[4]; };

static int int_member(void* object, const char* key, i32* out) {
    if (!STOCK_JSON_HAS(object, key)) return 0;
    *out = STOCK_JSON_INT(STOCK_JSON_GET(object, key));
    return 1;
}

// One entry of the list of ours, held to the store's rule for a record. The paddings as stock's loader reads
// them: the array's first four numbers, -1 where there is none.
static int read_entry(void* object, Entry* e) {
    if (!object || kind_of(object) != JSON_OBJECT) return 0;
    if (!int_member(object, KEY_CH, &e->track) || !int_member(object, KEY_START, &e->start) || !int_member(object, KEY_STOP, &e->stop)) return 0;
    if (!int_member(object, KEY_ID, &e->id)) e->id = -1;
    for (int k = 0; k < 4; ++k) e->padding[k] = -1;
    if (STOCK_JSON_HAS(object, KEY_PADDING)) {
        void* list = STOCK_JSON_GET(object, KEY_PADDING);
        if (kind_of(list) == JSON_ARRAY) {
            int n = STOCK_JSON_SIZE(list);
            for (int k = 0; k < 4 && k < n; ++k) e->padding[k] = STOCK_JSON_INT(STOCK_JSON_AT(list, k));
        }
    }
    if (!extension_track(e->track)) return 0;
    if (e->start < 0 || e->stop < 0 || !(e->start < e->stop) || !(e->start < MAX_FRAME) || !(e->stop <= MAX_FRAME)) return 0;
    return 1;
}

static int overlaps(const u8* store, const Entry* e) {
    for (i32 id = 0; id < MAX_CLIPS; ++id) {
        if (!has_clip(store, id)) continue;
        const i32* r = record_at(store, id);
        if (r[0] == e->track && e->start < r[6] && r[5] < e->stop) return 1;
    }
    return 0;
}

static i32 free_id(const u8* store) {
    for (i32 id = 0; id < MAX_CLIPS; ++id)
        if (!has_clip(store, id)) return id;
    return -1;
}

static int nothing_under(const i32* record) {
    u32 first = (u32)record[5] >> CHUNK_SHIFT;
    u32 last = (u32)(record[6] - 1) >> CHUNK_SHIFT;
    for (u32 chunk = first; chunk <= last; ++chunk)
        if (op1f_ext_chunk_mapped((u32)record[0], chunk)) return 0;
    return 1;
}

// ---------------------------------------------------------------------------------------------- the other tapes

static int entry_of_ours(void* entry) {              // an object whose "ch" is a track of 5 to 8
    i32 ch;
    return entry && kind_of(entry) == JSON_OBJECT && int_member(entry, KEY_CH, &ch) && extension_track(ch);
}

// A tape that is not mounted: the entries of tracks 5 to 8 in its file's "clips" moved to "op1f_clips" in the same
// file. The entries moved; 0 when there was nothing to do; -1 when the file could not be written.
static int sweep_tape(int tape) {
    char path[24];
    tape_path(path, tape, "_ext.bin");
    if (STOCK_FILE_SIZE(path) < 0) return 0;         // no tracks 5 to 8: nothing in its list that has audio
    tape_path(path, tape, ".json");
    if (STOCK_FILE_SIZE(path) < 0) return 0;
    Json doc;
    int moved = 0, written = 1;
    STOCK_JSON_CTOR(&doc, 0);
    if (STOCK_JSON_READ_FILE(path, &doc) && kind_of(&doc) == JSON_OBJECT && STOCK_JSON_HAS(&doc, STOCK_CLIPS_KEY)) {
        void* clips = STOCK_JSON_GET(&doc, STOCK_CLIPS_KEY);
        int n = kind_of(clips) == JSON_ARRAY ? STOCK_JSON_SIZE(clips) : 0;
        for (int i = 0; i < n; ++i)
            if (entry_of_ours(STOCK_JSON_AT(clips, i))) ++moved;
        if (moved) {
            Json keep, ours;
            STOCK_JSON_CTOR(&keep, JSON_ARRAY);
            STOCK_JSON_CTOR(&ours, JSON_ARRAY);
            if (STOCK_JSON_HAS(&doc, OURS)) {        // what the list of ours already holds comes first
                void* old = STOCK_JSON_GET(&doc, OURS);
                int m = kind_of(old) == JSON_ARRAY ? STOCK_JSON_SIZE(old) : 0;
                for (int i = 0; i < m; ++i) STOCK_JSON_PUSH(&ours, STOCK_JSON_AT(old, i));
            }
            for (int i = 0; i < n; ++i) {
                void* entry = STOCK_JSON_AT(clips, i);
                STOCK_JSON_PUSH(entry_of_ours(entry) ? &ours : &keep, entry);
            }
            STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(&doc, STOCK_CLIPS_KEY), &keep);
            STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(&doc, OURS), &ours);
            STOCK_JSON_DTOR(&keep);
            STOCK_JSON_DTOR(&ours);
            u32 name[8];                             // a stock string object: +4 the text or its pointer, +0x18 the capacity
            for (int i = 0; i < 8; ++i) name[i] = 0;
            STOCK_STRING_CTOR(name, path);
            written = STOCK_JSON_WRITE_FILE(name, &doc);
            if (name[6] >= 16 && name[1]) STOCK_FREE((void*)name[1]);   // sixteen characters: the text is on the heap
        }
    }
    STOCK_JSON_DTOR(&doc);
    return written ? moved : -1;
}

static void sweep(int mounted) {
    if (swept) return;
    swept = 1;
    for (int tape = 0; tape < TAPES; ++tape) {
        if (tape == mounted) continue;
        int moved = sweep_tape(tape);
        if (!moved) continue;
        if (moved > 0) ++op1f_clips_counts[12];
        else ++op1f_clips_counts[13];
        if (moved > 0)
            STOCK_LOG(3, LOG_FILE, 2, "clips: tape %d, not loaded: %d clip(s) of tracks 5-8 moved from stock's list to the list of ours in its file", tape + 1, moved);
        else
            STOCK_LOG(1, LOG_FILE, 3, "clips: tape %d, not loaded: its file could not be written; its clips of tracks 5-8 stay in stock's list", tape + 1);
    }
}

// In place of the two calls of stock's loader.
void op1f_clips_load(void* store_pointer) {
    STOCK_CLIPS_LOAD(store_pointer);
    ++op1f_clips_counts[0];
    const u8* store = (const u8*)store_pointer;
    void* doc = STOCK_TAPE_DOC();
    if (!store || !doc || kind_of(doc) != JSON_OBJECT) return;     // stock's loader has made the document an object by asking it for "clips"
    int tape = STOCK_MOUNTED_TAPE();

    u32 from_stock = 0, from_ours = 0, overlapping = 0, unusable = 0, renumbered = 0, empty = 0;
    for (i32 id = 0; id < MAX_CLIPS; ++id)
        if (has_clip(store, id) && extension_track(record_at(store, id)[0])) ++from_stock;

    int listed = 0;
    if (STOCK_JSON_HAS(doc, OURS)) {
        void* list = STOCK_JSON_GET(doc, OURS);
        if (kind_of(list) == JSON_ARRAY) {
            listed = 1;
            int n = STOCK_JSON_SIZE(list);
            for (int i = 0; i < n; ++i) {
                Entry e;
                if (!read_entry(STOCK_JSON_AT(list, i), &e)) {
                    ++unusable;
                    continue;
                }
                if (overlaps(store, &e)) {
                    ++overlapping;
                    continue;
                }
                i32 id = e.id;
                if (id < 0 || id >= MAX_CLIPS || has_clip(store, id)) {
                    id = free_id(store);
                    if (id < 0) {                                  // the store is full
                        ++unusable;
                        continue;
                    }
                    ++renumbered;
                }
                i32 record[RECORD_WORDS];
                STOCK_CLIP_FILL(record, e.track, e.start, e.stop, e.padding);
                STOCK_CLIP_SET(store_pointer, id, record, 0);      // as stock's loader sets its own: the store alone
                ++from_ours;
            }
        }
    }
    if (!listed) {                                                 // the list of ours, for the db's functions to find
        Json none;
        STOCK_JSON_CTOR(&none, JSON_ARRAY);
        STOCK_DB_SET(TAPE_DB, OURS, &none);
        STOCK_JSON_DTOR(&none);
        ++op1f_clips_counts[1];
    }
    // a clip of tracks 5 to 8 with nothing under it; the map of this tape was loaded just before, at its mount
    if (tape >= 0 && tape == op1f_ext_load_tape && op1f_ext_load_state >= 0 && op1f_ext_load_state != XEXT_NEWER) {
        for (i32 id = 0; id < MAX_CLIPS; ++id) {
            if (!has_clip(store, id)) continue;
            const i32* r = record_at(store, id);
            if (extension_track(r[0]) && nothing_under(r)) {
                STOCK_CLIP_REMOVE(store_pointer, id);
                ++empty;
            }
        }
    }
    int rebuild = from_stock || overlapping || unusable || renumbered || empty;
    if (rebuild) {
        STOCK_CLIPS_RELINK(store_pointer);                         // stock's own: both lists cleared, every clip added to its own
        ++op1f_clips_counts[8];
    }
    op1f_clips_counts[2] += from_ours;
    op1f_clips_counts[3] += from_stock;
    op1f_clips_counts[4] += overlapping;
    op1f_clips_counts[5] += unusable;
    op1f_clips_counts[6] += empty;
    op1f_clips_counts[7] += renumbered;
    if (log_budget) {
        --log_budget;                                              // a warning when a clip was left out, else a plain line
        STOCK_LOG((overlapping || unusable || empty) ? 2 : 3, LOG_FILE, 1, "clips: tape %d, tracks 5-8: %d from the list of ours, %d from stock's list; left out: %d overlapping, %d not usable, %d with nothing under them; %d renumbered; %s",
                  tape + 1, from_ours, from_stock, overlapping, unusable, empty, renumbered,
                  rebuild ? "both lists written anew" : (listed ? "the lists as they were" : "the list of ours made"));
    }
    sweep(tape);                                                   // once a start: the other tapes' files
}

// ---------------------------------------------------------------------------------------------- the tape list

static i32 last_stop(void* doc, const char* key, i32 longest) {
    if (!STOCK_JSON_HAS(doc, key)) return longest;
    void* list = STOCK_JSON_GET(doc, key);
    if (kind_of(list) != JSON_ARRAY) return longest;
    int n = STOCK_JSON_SIZE(list);
    for (int i = 0; i < n; ++i) {
        void* object = STOCK_JSON_AT(list, i);
        i32 stop;
        if (object && kind_of(object) == JSON_OBJECT && int_member(object, KEY_STOP, &stop) && stop > longest) longest = stop;
    }
    return longest;
}

// In place of the tape list's call of 0x8d4ef8ec(tape): the end of the tape's last clip, in whole seconds, as a
// float; -1.0 when the tape's json cannot be read. Stock's reads the file and walks "clips"; this one walks both
// lists, so that a tape whose last clip is on tracks 5 to 8 is not shown shorter than it is.
u32 op1f_tape_seconds(int tape) {
    ++op1f_clips_counts[11];
    if (tape < 0 || tape >= TAPES) return MINUS_ONE_FLOAT;
    char path[24];
    tape_path(path, tape, ".json");
    if (STOCK_FILE_SIZE(path) < 0) return MINUS_ONE_FLOAT;         // asked first: no error line for a slot with no tape
    Json doc;
    STOCK_JSON_CTOR(&doc, 0);
    int read = STOCK_JSON_READ_FILE(path, &doc);
    i32 longest = 0;
    if (read && kind_of(&doc) == JSON_OBJECT) {
        longest = last_stop(&doc, STOCK_CLIPS_KEY, longest);
        longest = last_stop(&doc, OURS, longest);
    }
    STOCK_JSON_DTOR(&doc);
    if (!read) return MINUS_ONE_FLOAT;
    if (longest > MAX_FRAME) longest = MAX_FRAME;
    i32 seconds = 0;
    while (longest >= FRAMES_A_SECOND) {                           // at most 360 turns; no division in the module
        longest -= FRAMES_A_SECOND;
        ++seconds;
    }
    return STOCK_FLOAT_FROM_INT(seconds);
}
