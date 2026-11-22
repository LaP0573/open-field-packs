// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Packs L3: Build K's block map unit (which is kept) with its log lines in this project's own
// words. Until L3 its lines for a block map file that cannot be read or written were the lines stock's own loader
// and saver print in the same cases: six format strings were stock's, whole or in their first words, and a seventh
// was made after one of them. They say the same things here, with the same arguments. Nothing else differs from
// the elder: no statement, no number, no other line.
//
// Build K: a tape's block map in two files (packs K1 and K1b), and the tape's identity (pack K2).
//
// Until K1 a tape with anything on tracks 5 to 8 had a block map file of this project's own, version 2: eight
// tracks in user/tapeNN_map.bin. Stock firmware accepts version 1 only. Given a version-2 file it assumes the layout
// of a fresh tape, plays the wrong chunks on tracks 1 to 4, and saves that layout over the real one at the first
// recording. So a unit could not go back to stock firmware without its tapes being prepared.
//
// From K1 the file stock reads is always stock's own: magic, version 1, the four tables of tracks 1 to 4, byte for
// byte what stock writes. The tables of tracks 5 to 8 stand in a file of the module's beside it,
// user/tapeNN_ext.bin. Stock never opens that file; to stock the tape is a four-track tape and the chunks of
// tracks 5 to 8 are free.
//
// The extension file, little-endian. Its first 7,768 bytes are K1's file and stay so, version 1, so that K1's
// pack reads the file of a later pack and a later pack the file of K1's; what later packs add stands in sections
// after the tables, each with a magic and its length:
//
//   0   2   the magic "XM" (0x4d58)
//   2   2   the version, 1
//   4   2   the tracks, 4
//   6   2   the chunks a track, 969
//   8   4   the CRC-32 of stock's file as it was written with this one (its 7,756 bytes)
//   12  4   the CRC-32 of the tables
//   16      4 x 969 physical chunk numbers, 0xFFFF for none
//
//   the identity section (K2), sixteen bytes after the tables:
//   0   2   the magic "XI" (0x4958)
//   2   2   the section's words, 8
//   4   4   the CRC-32 of stock's file as it stood on the disk before that save
//   8   4   the id of this file, 0 for none
//   12  2   flags: bit 0, bound: the tape's json has been seen on the disk with the same id
//   14  2   a check: the other seven words exclusive-ored, inverted
//
// Whatever else holds, an extension entry is dropped when its chunk is out of range, is held by one of tracks 1 to
// 4 in stock's file, or was already taken by an earlier extension entry: stock's file is the one that is right.
// A chunk stock took and gave back (a take undone under stock) is not seen by this; that needs a sum for every
// chunk and is left for a later pack.
//
// Writing: the extension file first, then stock's. The extension file is written when an extension track holds a
// chunk or when the file is already there and ours to rewrite; a tape that never used tracks 5 to 8 never gets
// one. A file of a later version is left as it is (tracks 5 to 8 taken as empty, the file replaced only when they
// are recorded under this pack): a pack may run as the spare of a later one. And K1b's pack, run as K2's spare or
// as the way back from it, reads K2's files as its own: it sees tracks 5 to 8 of every tape, and a file it saves
// loses the identity section, which K2 gives it again (a new id) at the tape's next load.
//
// K2, whose file it is. An extension file can be left behind in a tape's slot and met by another tape: stock's
// duplicate of a tape (0x8d4fb908) copies the tape's json, its map file and the chunks of tracks 1 to 4 itself and
// formats nothing; a deleted tape's files stay until the slot is used again; and a tape formatted under stock
// firmware keeps the old file (under ours a format goes through this unit's save and empties it). The one place
// in stock's files for a mark that goes with the tape is user/tapeNN.json: stock reads that file whole into a
// document (0x8d4ef84c), sets its own keys in it one by one (0x8d4f24dc) and writes the document whole
// (0x8d4ef760, about a second after a change), so a key of ours is kept through stock's saves, copied by a
// duplicate and gone after a format, which writes the file anew from a template. So:
//
//   * an extension file with entries carries an id, a number made when the file is first written for the tape;
//     the same number is put into the tape's json as "op1f_tape_id" (from the frame hook, on the GUI thread, and
//     at every write of the json by the TapeController);
//   * once the json on the disk has been seen to carry it, the file is bound (the flag). A bound file is the
//     tape's when the json's number is its own, and then stays the tape's whatever stock's file has become: what
//     another firmware changed on tracks 1 to 4 is met by the filter. With another number in the json, or none,
//     it is not this tape's: tracks 5 to 8 are taken as empty;
//   * a file not yet bound (its first seconds, or a file of K1's, which has no identity section) is trusted only
//     while stock's file is the one it was written with, or the one before it (a start that fell between the two
//     writes);
//   * a file with no entries has no id, and a save that finds tracks 5 to 8 empty takes the id away.
//
// A version-2 map file of the first builds is still read (tracks 5 to 8 from the one file), and op1f_xmap_convert
// rewrites it as the two files; build_b_storage.cpp calls that for every tape once a start.
//
// Stock functions used: disk_io's fopen 0x8fc22180, fread 0x8fc2214c, fwrite 0x8fc220b8, fclose 0x8fc220a0 and
// rename 0x8fc222f4, as build_b_storage.cpp has used them since B0b for the same files; the file size 0x8c051f9c;
// the logger; and since K2 the json functions as Build B's mixer unit has bound them since B3d (a json file read
// 0x8c0534d0, has 0x8c085068, get 0x8c084f74, as int 0x8c084840, from int 0x8c083b1c, a null value 0x8c085ba8,
// destroy 0x8c0844e0), the tape db's set 0x8d4f24dc(2, key, &value) and the mounted tape's index 0x8fc219a8.
//
// The open's answer is a descriptor, an integer: 0 or more when the file is open, negative when it is not (read in
// stock for K1b: the wrapper returns what the open underneath returns, and -1 for a mode it does not know). It
// writes an error line for every open that fails. K1 tested the answer against zero, as the storage unit had
// since B0b, and so took a tape without an extension file for a tape with an unusable one (the K1 device result).
// A file that may be absent is therefore asked for with the size function first, which answers -1 and logs nothing.
// Freestanding: no library, no division.

#include "build_k_maps.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

typedef i32 (*FopenFn)(const char* path, const char* mode);      // a descriptor; negative when the open fails
typedef i32 (*FreadFn)(void* destination, i32 size, i32 count, i32 file);
typedef i32 (*FwriteFn)(const void* source, i32 size, i32 count, i32 file);
typedef void (*FcloseFn)(i32 file);
typedef i32 (*FileSizeFn)(const char* path);                     // the file's bytes, -1 when there is none
typedef void (*RenameFn)(const char* old_path, const char* new_path);
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*JsonCtorFn)(void* json, int kind);                // 0: a null value
typedef void (*JsonDtorFn)(void* json);
typedef int (*JsonReadFileFn)(const char* path, void* json);     // the file parsed into the json, non-zero on success
typedef int (*JsonHasFn)(void* json, const char* key);
typedef void* (*JsonGetFn)(void* json, const char* key);         // the member (a reference)
typedef int (*JsonIntFn)(void* json);
typedef void (*JsonFromIntFn)(void* json, int value);
typedef void (*DbSetFn)(int db, const char* key, void* value);   // sets the key in the db's document and marks it for saving
typedef int (*MountedTapeFn)();                                  // the mounted tape's index, -1 for none
typedef u32 (*FileMtimeFn)(const char* path);                    // the file's modification time, 0 when it is not there; later is larger

#define STOCK_FOPEN ((FopenFn)0x8fc22180u)
#define STOCK_FREAD ((FreadFn)0x8fc2214cu)
#define STOCK_FWRITE ((FwriteFn)0x8fc220b8u)
#define STOCK_FCLOSE ((FcloseFn)0x8fc220a0u)
#define STOCK_RENAME ((RenameFn)0x8fc222f4u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_DB_SET ((DbSetFn)0x8d4f24dcu)
#define STOCK_MOUNTED_TAPE ((MountedTapeFn)0x8fc219a8u)
#define STOCK_FILE_MTIME ((FileMtimeFn)0x8fcbc350u)               // K5: as the preset lists have used it since H6
#define STOCK_TAPE_DB_DIRTY (*(volatile u8*)0x0802327eu)
#define STOCK_MAP_DIRTY (*(volatile u8*)0x848cff9cu)

#ifdef OP1F_HOST
u32 op1f_host_cycles();                          // the test's clock
static u32 cycles_now() { return op1f_host_cycles(); }
#else
static u32 cycles_now() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}
#endif

enum {
    TAPES = 8,
    STOCK_TRACKS = 4,
    EXT_TRACKS = 4,
    CHUNKS_PER_TRACK = 969,
    TABLE_ENTRIES = 4 * CHUNKS_PER_TRACK,        // 3,876: four tracks' tables
    CHUNKS_PER_PARTITION = 8192,
    UNALLOCATED = 0xffff,
    MAP_MAGIC = 0x424d,                          // "MB"
    EXT_MAGIC = 0x4d58,                          // "XM"
    EXT_VERSION = 1,                             // K1's, and it stays: what K2 adds is a section after the tables
    EXT_HEADER_WORDS = 8,
    ID_MAGIC = 0x4958,                           // "XI": the identity section
    ID_WORDS = 8,
    FORM_K1 = 1,                                 // a file without the identity section
    FORM_K2 = 2,                                 // with it
    EXT_FLAG_BOUND = 1,
    SUMS_MAGIC = 0x5358,                         // "XS": K5's section, the sums of the chunks of tracks 5 to 8
    SUMS_WORDS = 8,                              // its header, the identity section's size
    SUMS_VERSION = 1,
    MAX_FILE_TRACKS = 16,
    TAPE_DB = 2,
    JSON_OBJECT = 7,                             // a json value's kind, its byte at +4
    SETTLE_FRAMES = 60,                          // a second with the same tape mounted before the json is touched
    PUBLISH_TRIES = 4,
    LOG_BUDGET = 24,
};

struct Json { u32 words[4]; };                   // a stock json value is 12 bytes; 16 for alignment
struct ExtHead { u32 version, form, stock_crc, body_crc, prev_crc, id, flags, sums; };   // version: the file's; form: FORM_K1 or FORM_K2; sums: K5's section read

static const char LOG_FILE[] = "../src/dsp/op1f_tape_xmap.cpp";
static const char ID_KEY[] = "op1f_tape_id";

static u32 crc_table[256];
static u32 crc_table_made;
static u8 taken[CHUNKS_PER_PARTITION / 8];       // the filter's set of chunks already held
static u16 scratch_stock[TABLE_ENTRIES];         // op1f_xmap_convert's tables
static u16 scratch_ext[TABLE_ENTRIES];

// K5: the sums of the chunks of tracks 5 to 8, entry for entry (build_k_maps.h). A third section of the extension
// file, after the identity (or after the tables in a file that has no identity, a copy's): eight half-words (the
// magic "XS", the header's length, the version, the number of entries, the CRC-32 of what follows, a spare, a
// check word as the identity's), then a 32-bit sum an entry, then a flag byte an entry. K2's to K4's packs read
// a file up to its identity and leave the rest alone; what they save has no such section, and a chunk then gets
// its sum again at its next whole read.
//
// Two pairs of arrays, because two threads use them: what a load read (the loaded tape's load, and the load the
// copy of a tape makes of its original, both from the GUI's side, one at a time) and what a save writes (the
// service loop's thread, which may save the loaded tape's map while a copy is being made).
u32 op1f_xmap_entry_sums[TABLE_ENTRIES];
u8 op1f_xmap_entry_flags[TABLE_ENTRIES];
u32 op1f_xmap_save_entry_sums[TABLE_ENTRIES];
u8 op1f_xmap_save_entry_flags[TABLE_ENTRIES];
int op1f_xmap_save_sums;
extern "C" u32 op1f_xmap_sum_counts[4] asm("op1f_xmap_sum_counts");
u32 op1f_xmap_sum_counts[4];                     // sections read, sections written, files written a second time for their time, loads that found another firmware's save

// What is known of each tape's extension file. Set at every load and save of the tape; read by the frame hook.
static u32 tape_id[TAPES];                       // its id, 0 for none
static u8 tape_bound[TAPES];                     // the tape's json on the disk carries it
static u8 tape_hands_off[TAPES];                 // a later pack's file: its id is not ours to touch
static u32 last_crc[TAPES];                      // stock's file as last read or written
static u8 last_crc_known[TAPES];
static u8 publish_tries[TAPES];
static u32 id_counter;
static u32 log_budget = LOG_BUDGET;
// the frame hook's
static int frame_tape = -1;
static u32 frame_stable;
static int publish_waiting;
static u32 publish_id;

// CRC-32 as zlib computes it, continued: `crc` is the register (start with 0xffffffff, invert at the end).
static u32 crc_feed(u32 crc, const void* data, u32 bytes) {
    if (!crc_table_made) {
        for (u32 i = 0; i < 256; ++i) {
            u32 c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? ((c >> 1) ^ 0xedb88320u) : (c >> 1);
            crc_table[i] = c;
        }
        crc_table_made = 1;
    }
    const u8* p = (const u8*)data;
    for (u32 i = 0; i < bytes; ++i) crc = crc_table[(crc ^ p[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

// The CRC-32 of stock's file for these tables: the magic, the version 1, the four tables.
static u32 stock_file_crc(const u16* stock) {
    u16 header[2];
    header[0] = MAP_MAGIC;
    header[1] = 1;
    u32 crc = crc_feed(0xffffffffu, header, 4);
    crc = crc_feed(crc, stock, TABLE_ENTRIES * 2);
    return crc ^ 0xffffffffu;
}

static void path_of(char* path, int tape, const char* suffix) {    // "user/tapeNN" + suffix, NN = tape + 1
    const char* prefix = "user/tape";
    while (*prefix) *path++ = *prefix++;
    int number = tape + 1;
    *path++ = (char)('0' + (number >= 10 ? 1 : 0));
    *path++ = (char)('0' + (number >= 10 ? number - 10 : number));
    while (*suffix) *path++ = *suffix++;
    *path = 0;
}

static void fill_unallocated(u16* table, u32 entries) {
    for (u32 i = 0; i < entries; ++i) table[i] = UNALLOCATED;
}

static int no_entry(const u16* table) {
    for (u32 i = 0; i < TABLE_ENTRIES; ++i)
        if (table[i] != UNALLOCATED) return 0;
    return 1;
}

static void forget(int tape) {                   // the tape has no extension file of its own
    tape_id[tape] = 0;
    tape_bound[tape] = 0;
    publish_tries[tape] = 0;
}

// A number for a new extension file: 1 to 0x3fffffff, so that it is a positive integer in the json.
static u32 new_id(int tape, u32 stock_crc) {
    u32 x = cycles_now() ^ (stock_crc << 7) ^ (stock_crc >> 11) ^ ((u32)tape << 27) ^ (++id_counter * 0x9e3779b1u);
    x ^= x >> 15;
    x *= 0x2c1b3c6du;
    x ^= x >> 12;
    x &= 0x3fffffffu;
    return x ? x : 1;
}

// The id in the tape's json on the disk, 0 for none. Read with stock's own reader, as stock reads another tape's
// json (0x8d4f1ad0); the file is asked for first, and "has" is asked of an object only.
static u32 json_id(int tape) {
    char path[24];
    path_of(path, tape, ".json");
    if (STOCK_FILE_SIZE(path) < 0) return 0;
    Json doc;
    u32 id = 0;
    STOCK_JSON_CTOR(&doc, 0);
    if (STOCK_JSON_READ_FILE(path, &doc) && ((const u8*)&doc)[4] == JSON_OBJECT && STOCK_JSON_HAS(&doc, ID_KEY))
        id = (u32)STOCK_JSON_INT(STOCK_JSON_GET(&doc, ID_KEY));
    STOCK_JSON_DTOR(&doc);
    return id;
}

static u16 section_check(const u16* s) {
    u16 x = 0;
    for (int i = 0; i < ID_WORDS - 1; ++i) x ^= s[i];
    return (u16)~x;
}

static void clear_sums() {
    for (u32 i = 0; i < TABLE_ENTRIES; ++i) {
        op1f_xmap_entry_sums[i] = 0;
        op1f_xmap_entry_flags[i] = 0;
    }
}

static u32 sums_crc(const u32* sums, const u8* flags) {
    return crc_feed(crc_feed(0xffffffffu, sums, TABLE_ENTRIES * 4), flags, TABLE_ENTRIES) ^ 0xffffffffu;
}

// K5's section written from a pair of arrays. 1 when every item went out.
static int put_sums(i32 file, const u32* sums, const u8* flags) {
    u32 crc = sums_crc(sums, flags);
    u16 head[SUMS_WORDS];
    head[0] = SUMS_MAGIC;
    head[1] = SUMS_WORDS;
    head[2] = SUMS_VERSION;
    head[3] = TABLE_ENTRIES;
    head[4] = (u16)crc;
    head[5] = (u16)(crc >> 16);
    head[6] = 0;
    head[7] = section_check(head);
    ++op1f_xmap_sum_counts[1];
    return STOCK_FWRITE(head, 2, SUMS_WORDS, file) == SUMS_WORDS && STOCK_FWRITE(sums, 4, TABLE_ENTRIES, file) == TABLE_ENTRIES &&
           STOCK_FWRITE(flags, 1, TABLE_ENTRIES, file) == TABLE_ENTRIES;
}

// Reads an extension file that is there. 0: whole; 1: not usable; 2: of a later version (nothing read).
static int read_ext_file(const char* path, u16* ext, ExtHead* h) {
    u16 w[EXT_HEADER_WORDS];
    u16 s[ID_WORDS];
    for (int i = 0; i < EXT_HEADER_WORDS; ++i) w[i] = 0;
    for (int i = 0; i < ID_WORDS; ++i) s[i] = 0;
    h->version = 0;
    h->form = FORM_K1;
    i32 file = STOCK_FOPEN(path, "rb");
    if (file < 0) return 1;
    int got = STOCK_FREAD(w, 2, EXT_HEADER_WORDS, file);
    h->version = w[1];
    if (got >= 2 && w[0] == EXT_MAGIC && w[1] > EXT_VERSION) {
        STOCK_FCLOSE(file);
        return 2;
    }
    int whole = got == EXT_HEADER_WORDS && w[0] == EXT_MAGIC && w[1] == EXT_VERSION && w[2] == EXT_TRACKS && w[3] == CHUNKS_PER_TRACK &&
                STOCK_FREAD(ext, 2, TABLE_ENTRIES, file) == TABLE_ENTRIES;
    // the identity section, when the file has one: a file of K1's ends with the tables
    int after = whole ? STOCK_FREAD(s, 2, ID_WORDS, file) : 0;
    int identity = after == ID_WORDS && s[0] == ID_MAGIC && s[1] == ID_WORDS && s[7] == section_check(s);
    // K5: the sums' section, after the identity, or right after the tables in a file that has no identity
    u16 q[SUMS_WORDS];
    const u16* sums_head = 0;
    if (identity) {
        if (STOCK_FREAD(q, 2, SUMS_WORDS, file) == SUMS_WORDS) sums_head = q;
    } else if (after == ID_WORDS) {
        sums_head = s;
    }
    h->sums = 0;
    if (sums_head && sums_head[0] == SUMS_MAGIC && sums_head[1] == SUMS_WORDS && sums_head[2] == SUMS_VERSION && sums_head[3] == TABLE_ENTRIES &&
        sums_head[7] == section_check(sums_head) && STOCK_FREAD(op1f_xmap_entry_sums, 4, TABLE_ENTRIES, file) == TABLE_ENTRIES &&
        STOCK_FREAD(op1f_xmap_entry_flags, 1, TABLE_ENTRIES, file) == TABLE_ENTRIES)
        h->sums = sums_crc(op1f_xmap_entry_sums, op1f_xmap_entry_flags) == ((u32)sums_head[4] | ((u32)sums_head[5] << 16));
    STOCK_FCLOSE(file);
    if (!h->sums) clear_sums();                  // none, or not whole: no chunk has a sum
    h->stock_crc = (u32)w[4] | ((u32)w[5] << 16);
    h->body_crc = (u32)w[6] | ((u32)w[7] << 16);
    if (identity) {
        h->form = FORM_K2;
        h->prev_crc = (u32)s[2] | ((u32)s[3] << 16);
        h->id = (u32)s[4] | ((u32)s[5] << 16);
        h->flags = s[6];
    } else {                                     // K1's file: no id, and one stock sum
        h->prev_crc = h->stock_crc;
        h->id = 0;
        h->flags = 0;
    }
    if (!whole || (crc_feed(0xffffffffu, ext, TABLE_ENTRIES * 2) ^ 0xffffffffu) != h->body_crc) return 1;
    return 0;
}

// The extension file beside a version-1 stock file. Leaves `ext` empty unless the file is whole and the tape's.
static void read_ext(int tape, u16* ext, XmapLoad* out) {
    char path[24];
    path_of(path, tape, "_ext.bin");
    forget(tape);
    tape_hands_off[tape] = 0;
    if (STOCK_FILE_SIZE(path) < 0) {                 // none: asked without an open, which would log an error
        out->ext = XEXT_NONE;
        return;
    }
    ExtHead h;
    int state = read_ext_file(path, ext, &h);
    out->ext_version = state == 2 ? h.version : h.form;   // a later pack's version, or which of the two forms this pack knows
    if (state == 2) {                                // a later pack's file: left alone, its id with it
        STOCK_LOG(2, LOG_FILE, 10, "ext map: %s is version %d, newer than this pack reads: tracks 5-8 are taken as empty, the file is kept", path, h.version);
        tape_hands_off[tape] = 1;
        out->ext = XEXT_NEWER;
        return;
    }
    if (state == 1) {
        STOCK_LOG(1, LOG_FILE, 1, "ext map: %s is not usable, tracks 5-8 are taken as empty", path);
        fill_unallocated(ext, TABLE_ENTRIES);
        out->ext = XEXT_BAD;
        return;
    }
    if (no_entry(ext)) {                             // nothing in it: nothing to ask whose it is
        out->ext = XEXT_SAME;
        return;
    }
    out->sums = (int)h.sums;                         // K5: they count when the file is taken (op1f_xmap_load drops them when it is not)
    int same_stock = out->stock_crc == h.stock_crc || out->stock_crc == h.prev_crc;
    out->id = h.id;
    out->json_id = h.id ? json_id(tape) : 0;
    if (h.id && out->json_id == h.id) {              // the tape's json names this file: it is the tape's
        tape_id[tape] = h.id;
        tape_bound[tape] = 1;
        out->bound = 1;
        out->ext = same_stock ? XEXT_SAME : XEXT_CHANGED;
        out->rewrite = !(h.flags & EXT_FLAG_BOUND) || h.form != FORM_K2;
        return;
    }
    if (!(h.flags & EXT_FLAG_BOUND) && same_stock) { // not bound yet: stock's file is still the one it was written with
        tape_id[tape] = h.id;
        out->ext = XEXT_SAME;
        out->rewrite = h.form != FORM_K2;
        return;
    }
    if (h.id)
        STOCK_LOG(2, LOG_FILE, 12, "ext map: %s is not this tape's (id %08x, %s; the json has %08x; stock's file %s): tracks 5-8 taken as empty",
                  path, h.id, (h.flags & EXT_FLAG_BOUND) ? "bound" : "not bound", out->json_id, same_stock ? "the same" : "another");
    else                                             // K4: a file with no id is held to stock's file alone; the json was not asked (the K2 result's finding)
        STOCK_LOG(2, LOG_FILE, 12, "ext map: %s is not this tape's (it has no id, so the json was not asked; stock's file %s): tracks 5-8 taken as empty",
                  path, same_stock ? "the same" : "another");
    fill_unallocated(ext, TABLE_ENTRIES);
    out->ext = XEXT_STALE;
}

// Stock's file is the one that is right: an extension entry goes when its chunk is out of range, is held by one of
// tracks 1 to 4, or was taken by an earlier extension entry.
static void filter(const u16* stock, u16* ext, XmapLoad* out) {
    for (u32 i = 0; i < CHUNKS_PER_PARTITION / 8; ++i) taken[i] = 0;
    for (u32 i = 0; i < TABLE_ENTRIES; ++i) {
        u32 chunk = stock[i];
        if (chunk < CHUNKS_PER_PARTITION) taken[chunk >> 3] |= (u8)(1u << (chunk & 7));
    }
    for (u32 t = 0; t < EXT_TRACKS; ++t) out->allocated[t] = 0;
    u32 t = 0, in_track = 0;
    for (u32 i = 0; i < TABLE_ENTRIES; ++i) {
        u32 chunk = ext[i];
        if (chunk != UNALLOCATED) {
            if (chunk >= CHUNKS_PER_PARTITION || (taken[chunk >> 3] & (1u << (chunk & 7)))) {
                ext[i] = UNALLOCATED;
                ++out->dropped;
            } else {
                taken[chunk >> 3] |= (u8)(1u << (chunk & 7));
                ++out->allocated[t];
            }
        }
        if (++in_track == CHUNKS_PER_TRACK) {       // the track of an entry, without a division
            in_track = 0;
            ++t;
        }
    }
}

int op1f_xmap_load(int tape, u16* stock, u16* ext, XmapLoad* out) {
    out->status = XMAP_NO_FILE;
    out->version = 0;
    out->tracks_in_file = STOCK_TRACKS;
    out->stock_crc = 0;
    out->ext = XEXT_NONE;
    out->dropped = 0;
    for (u32 t = 0; t < EXT_TRACKS; ++t) out->allocated[t] = 0;
    out->ext_version = 0;
    out->id = 0;
    out->json_id = 0;
    out->bound = 0;
    out->rewrite = 0;
    out->sums = 0;
    out->foreign = 0;
    clear_sums();
    fill_unallocated(ext, TABLE_ENTRIES);
    forget(tape);
    tape_hands_off[tape] = 0;
    last_crc_known[tape] = 0;

    char path[24];
    path_of(path, tape, "_map.bin");
    if (STOCK_FILE_SIZE(path) < 0) return 0;         // no tape in this slot: asked without an open
    i32 file = STOCK_FOPEN(path, "rb");
    if (file < 0) return 0;
    u16 magic = 0, version = 0, tracks_in_file = STOCK_TRACKS;
    if (STOCK_FREAD(&magic, 2, 1, file) != 1 || magic != MAP_MAGIC) {
        STOCK_LOG(1, LOG_FILE, 2, "block map: %s does not begin as a block map file", path);
        STOCK_FCLOSE(file);
        out->status = XMAP_BAD_MAGIC;
        return 0;
    }
    if (STOCK_FREAD(&version, 2, 1, file) != 1 || (version != 1 && version != 2)) {
        STOCK_LOG(1, LOG_FILE, 3, "block map: version %d is not one this pack reads", version);
        STOCK_FCLOSE(file);
        out->status = XMAP_BAD_VERSION;
        return 0;
    }
    if (version == 2 && (STOCK_FREAD(&tracks_in_file, 2, 1, file) != 1 || tracks_in_file < 1 || tracks_in_file > MAX_FILE_TRACKS)) {
        STOCK_LOG(1, LOG_FILE, 4, "block map: a version-2 file of %d tracks is not one this pack reads", tracks_in_file);
        STOCK_FCLOSE(file);
        out->status = XMAP_BAD_VERSION;
        return 0;
    }
    out->version = version;
    out->tracks_in_file = tracks_in_file;
    for (u32 track = 0; track < STOCK_TRACKS; ++track) {
        u16* table = stock + track * CHUNKS_PER_TRACK;
        if (track >= tracks_in_file) {
            fill_unallocated(table, CHUNKS_PER_TRACK);
        } else if (STOCK_FREAD(table, 2, CHUNKS_PER_TRACK, file) != CHUNKS_PER_TRACK) {
            STOCK_LOG(1, LOG_FILE, 5, "block map: the table of track %d could not be read from the file", track);
            STOCK_FCLOSE(file);
            out->status = XMAP_SHORT;
            return 0;
        }
    }
    if (version == 2) {                              // the old file: tracks 5 to 8 follow in it; any beyond are left
        for (u32 track = STOCK_TRACKS; track < tracks_in_file && track < STOCK_TRACKS + EXT_TRACKS; ++track) {
            if (STOCK_FREAD(ext + (track - STOCK_TRACKS) * CHUNKS_PER_TRACK, 2, CHUNKS_PER_TRACK, file) != CHUNKS_PER_TRACK) {
                STOCK_LOG(1, LOG_FILE, 6, "block map: the table of track %d could not be read from the file", track);
                STOCK_FCLOSE(file);
                fill_unallocated(ext, TABLE_ENTRIES);
                out->status = XMAP_SHORT;
                return 0;
            }
        }
    }
    STOCK_FCLOSE(file);

    if (version == 1) {
        out->stock_crc = stock_file_crc(stock);
        last_crc[tape] = out->stock_crc;
        last_crc_known[tape] = 1;
        read_ext(tape, ext, out);
        out->status = XMAP_V1;
        if (out->ext == XEXT_SAME || out->ext == XEXT_CHANGED) {
            // K5: has another firmware saved the map since this pack wrote the file of tracks 5 to 8? This pack
            // never leaves that file the older of the two (op1f_xmap_save), and K1's sign counts as well.
            char ext_path[24];
            path_of(ext_path, tape, "_ext.bin");
            out->foreign = (out->ext == XEXT_CHANGED || STOCK_FILE_MTIME(path) > STOCK_FILE_MTIME(ext_path)) ? 1 : 0;
            if (out->sums) ++op1f_xmap_sum_counts[0];
            else clear_sums();
            if (out->foreign) ++op1f_xmap_sum_counts[3];
        } else {                                     // the file was not taken: neither are its sums
            out->sums = 0;
            clear_sums();
        }
    } else {
        out->ext = XEXT_LEGACY;
        out->status = XMAP_LEGACY;
    }
    filter(stock, ext, out);
    return 1;
}

// What stands at the extension file's place: 0 nothing, 1 a file this pack may rewrite, 2 a later version's file.
static int ext_file_there(const char* path) {
    if (STOCK_FILE_SIZE(path) < 0) return 0;
    i32 file = STOCK_FOPEN(path, "rb");
    if (file < 0) return 1;                          // there, and not to be opened: ours to replace
    u16 head[2];
    head[0] = 0;
    head[1] = 0;
    int got = STOCK_FREAD(head, 2, 2, file);
    STOCK_FCLOSE(file);
    return (got == 2 && head[0] == EXT_MAGIC && head[1] > EXT_VERSION) ? 2 : 1;
}

// The file of tracks 5 to 8, through a temporary and a rename: K1's header and tables as K1 wrote them, K2's
// identity after them, and K5's sums after that when the caller has them. A temporary that cannot be made is
// said and the file is left as it was.
static void write_ext(int tape, const u16* ext, u32 stock_crc, u32 prev_crc, u32 id, int bound, int with_sums) {
    char temp_path[24], final_path[24];
    path_of(temp_path, tape, "_ext.tmp");
    path_of(final_path, tape, "_ext.bin");
    i32 file = STOCK_FOPEN(temp_path, "w+b");
    if (file < 0) {
        STOCK_LOG(1, LOG_FILE, 7, "ext map: %s could not be opened for writing", temp_path);
        return;
    }
    u32 body_crc = crc_feed(0xffffffffu, ext, TABLE_ENTRIES * 2) ^ 0xffffffffu;
    u16 header[EXT_HEADER_WORDS];
    header[0] = EXT_MAGIC;
    header[1] = EXT_VERSION;
    header[2] = EXT_TRACKS;
    header[3] = CHUNKS_PER_TRACK;
    header[4] = (u16)stock_crc;
    header[5] = (u16)(stock_crc >> 16);
    header[6] = (u16)body_crc;
    header[7] = (u16)(body_crc >> 16);
    u16 section[ID_WORDS];
    section[0] = ID_MAGIC;
    section[1] = ID_WORDS;
    section[2] = (u16)prev_crc;
    section[3] = (u16)(prev_crc >> 16);
    section[4] = (u16)id;
    section[5] = (u16)(id >> 16);
    section[6] = bound ? EXT_FLAG_BOUND : 0;
    section[7] = section_check(section);
    STOCK_FWRITE(header, 2, EXT_HEADER_WORDS, file);     // K1's file, as K1 wrote it
    STOCK_FWRITE(ext, 2, TABLE_ENTRIES, file);
    STOCK_FWRITE(section, 2, ID_WORDS, file);            // and the identity after it
    if (with_sums) put_sums(file, op1f_xmap_save_entry_sums, op1f_xmap_save_entry_flags);   // K5: and the sums of the chunks, as the caller collected them
    STOCK_FCLOSE(file);
    STOCK_RENAME(temp_path, final_path);
}

int op1f_xmap_save(int tape, const u16* stock, const u16* ext) {
    char temp_path[24], final_path[24];
    int any = !no_entry(ext);
    u32 stock_crc = stock_file_crc(stock);
    path_of(final_path, tape, "_ext.bin");
    int there = ext_file_there(final_path);
    if (any && there == 2)
        STOCK_LOG(2, LOG_FILE, 11, "ext map: %s of a newer version is replaced: tracks 5-8 were recorded under this pack", final_path);
    if (any) {
        tape_hands_off[tape] = 0;
        if (!tape_id[tape]) {                        // the file is new for this tape: a number of its own
            tape_id[tape] = new_id(tape, stock_crc);
            tape_bound[tape] = 0;
            publish_tries[tape] = 0;
            if (log_budget) {
                --log_budget;
                STOCK_LOG(3, LOG_FILE, 13, "tape %d: the ext map takes the id %08x", tape + 1, tape_id[tape]);
            }
        }
    } else if (there != 2) {
        forget(tape);                                // tracks 5 to 8 are empty: no file of the tape's, or an empty one
    }
    int with_sums = any && op1f_xmap_save_sums;      // K5: the caller has just filled the save's two arrays for this tape
    op1f_xmap_save_sums = 0;
    int ext_written = 0;
    u32 prev_crc = last_crc_known[tape] ? last_crc[tape] : stock_crc;
    u32 id = any ? tape_id[tape] : 0;
    int bound = any && tape_bound[tape];
    if (any || there == 1) {                         // the extension file first: it names the stock file that follows
        write_ext(tape, ext, stock_crc, prev_crc, id, bound, with_sums);
        ext_written = 1;
    }
    path_of(temp_path, tape, "_map.tmp");
    path_of(final_path, tape, "_map.bin");
    i32 file = STOCK_FOPEN(temp_path, "w+b");
    if (file < 0) {
        STOCK_LOG(1, LOG_FILE, 8, "block map: %s could not be opened for writing", temp_path);
        return 0;
    }
    u16 header[2];
    header[0] = MAP_MAGIC;
    header[1] = 1;                                   // always stock's own version
    STOCK_FWRITE(&header[0], 2, 1, file);
    STOCK_FWRITE(&header[1], 2, 1, file);
    for (u32 track = 0; track < STOCK_TRACKS; ++track)
        STOCK_FWRITE(stock + track * CHUNKS_PER_TRACK, 2, CHUNKS_PER_TRACK, file);
    STOCK_FCLOSE(file);
    STOCK_RENAME(temp_path, final_path);
    last_crc[tape] = stock_crc;
    last_crc_known[tape] = 1;
    if (ext_written && any) {
        // K5: the file of tracks 5 to 8 is never left the older of the two. A map file that is newer than it means
        // another firmware saved the map (op1f_xmap_load), so when the clock moved between the two writes above,
        // the same file is written once more.
        char ext_path[24];
        path_of(ext_path, tape, "_ext.bin");
        if (STOCK_FILE_MTIME(final_path) > STOCK_FILE_MTIME(ext_path)) {
            write_ext(tape, ext, stock_crc, prev_crc, id, bound, with_sums);
            ++op1f_xmap_sum_counts[2];
        }
    }
    return 1;
}

// The version of a slot's map file, 0 when there is none or it is no map file. Four bytes read.
static u32 map_version(int tape) {
    char path[24];
    path_of(path, tape, "_map.bin");
    if (STOCK_FILE_SIZE(path) < 0) return 0;
    i32 file = STOCK_FOPEN(path, "rb");
    if (file < 0) return 0;
    u16 head[2];
    head[0] = 0;
    head[1] = 0;
    int got = STOCK_FREAD(head, 2, 2, file);
    STOCK_FCLOSE(file);
    return (got == 2 && head[0] == MAP_MAGIC) ? head[1] : 0;
}

int op1f_xmap_convert(int tape) {
    if (tape < 0 || tape >= TAPES) return 0;
    if (map_version(tape) != 2) return 0;            // nothing of the first builds in this slot
    XmapLoad load;
    if (!op1f_xmap_load(tape, scratch_stock, scratch_ext, &load) || load.status != XMAP_LEGACY) return 0;
    int saved = op1f_xmap_save(tape, scratch_stock, scratch_ext);
    STOCK_LOG(saved ? 3 : 1, LOG_FILE, 9, "tape %d: version-2 block map %s: tracks 5-8 hold %d/%d/%d/%d chunk(s), %d dropped",
              tape + 1, saved ? "rewritten as stock's file and an ext map" : "could NOT be rewritten",
              load.allocated[0], load.allocated[1], load.allocated[2], load.allocated[3], load.dropped);
    return saved ? 1 : -1;
}

// ---------------------------------------------------------------------------------------------------------
// K4: what the copy of a tape needs of this unit (build_k_copy.cpp). Stock's clone 0x8d4fb908 copies the tape's
// json, its map file and the chunks of tracks 1 to 4; the module copies the chunks of tracks 5 to 8 after it and
// then gives the copy a file of tracks 5 to 8 of its own.

// The tracks 5 to 8 of a tape that is not mounted, as a load of it takes them (the identity rules, the filter), in
// this unit's own tables, which hold until the next call here or the next conversion. 0 when the slot has no
// version-1 map file.
const u16* op1f_xmap_source(int tape, XmapLoad* out) {
    if (tape < 0 || tape >= TAPES) return 0;
    if (!op1f_xmap_load(tape, scratch_stock, scratch_ext, out) || out->status != XMAP_V1) return 0;
    return scratch_ext;
}

// The CRC-32 of a slot's map file as it stands on the disk. 0 when it is not a whole version-1 file. It reads into
// the table of tracks 1 to 4 that op1f_xmap_source filled; the table of tracks 5 to 8 is left as it is.
int op1f_xmap_file_crc(int tape, u32* crc) {
    char path[24];
    if (tape < 0 || tape >= TAPES) return 0;
    path_of(path, tape, "_map.bin");
    if (STOCK_FILE_SIZE(path) != 4 + TABLE_ENTRIES * 2) return 0;
    i32 file = STOCK_FOPEN(path, "rb");
    if (file < 0) return 0;
    u16 head[2];
    head[0] = 0;
    head[1] = 0;
    int whole = STOCK_FREAD(head, 2, 2, file) == 2 && head[0] == MAP_MAGIC && head[1] == 1 &&
                STOCK_FREAD(scratch_stock, 2, TABLE_ENTRIES, file) == TABLE_ENTRIES;
    STOCK_FCLOSE(file);
    if (!whole) return 0;
    *crc = stock_file_crc(scratch_stock);
    return 1;
}

// A file of tracks 5 to 8 for a slot whose tape was just made by a copy: these tables, the sum of the stock file
// they go with, and no identity section. That is K1's form, on purpose: the load that follows takes such a file
// by stock's sum alone (the copy's json still carries the original's id, which names no file in this slot), marks
// it to be written anew, and the save after it gives the copy an id of its own, puts it into the copy's json and
// binds the file, as the K2 test showed for three files of this form. With `only_if_there` nothing is written
// for a slot that holds no such file (a copy of a four-track tape gets none; a file an earlier tape left in the
// slot is replaced by the copy's empty one). With `with_sums` (K5) the sums of the last load, the original's, go
// after the tables: the copy's chunks are the original's bytes. 1 when the file was written.
int op1f_xmap_write_plain(int tape, const u16* ext, u32 stock_crc, int only_if_there, int with_sums) {
    char temp_path[24], final_path[24];
    if (tape < 0 || tape >= TAPES) return 0;
    path_of(temp_path, tape, "_ext.tmp");
    path_of(final_path, tape, "_ext.bin");
    if (only_if_there && STOCK_FILE_SIZE(final_path) < 0) return 0;
    i32 file = STOCK_FOPEN(temp_path, "w+b");
    if (file < 0) return 0;                          // stock has logged the open
    u32 body_crc = crc_feed(0xffffffffu, ext, TABLE_ENTRIES * 2) ^ 0xffffffffu;
    u16 header[EXT_HEADER_WORDS];
    header[0] = EXT_MAGIC;
    header[1] = EXT_VERSION;
    header[2] = EXT_TRACKS;
    header[3] = CHUNKS_PER_TRACK;
    header[4] = (u16)stock_crc;
    header[5] = (u16)(stock_crc >> 16);
    header[6] = (u16)body_crc;
    header[7] = (u16)(body_crc >> 16);
    int put = STOCK_FWRITE(header, 2, EXT_HEADER_WORDS, file) == EXT_HEADER_WORDS && STOCK_FWRITE(ext, 2, TABLE_ENTRIES, file) == TABLE_ENTRIES;
    if (put && with_sums) put = put_sums(file, op1f_xmap_entry_sums, op1f_xmap_entry_flags);   // K5: the sums the source's load read; the chunks are copies
    STOCK_FCLOSE(file);
    if (!put) return 0;                              // the temporary is left; the slot's file is as it was
    STOCK_RENAME(temp_path, final_path);
    forget(tape);                                    // nothing is known of the slot's file until its tape is loaded
    tape_hands_off[tape] = 0;
    last_crc_known[tape] = 0;
    return 1;
}

// ---------------------------------------------------------------------------------------------------------
// The id in the tape's json (K2). Both functions run on the GUI thread.

static void set_json_id(u32 id) {
    Json value;
    STOCK_JSON_FROM_INT(&value, (int)id);
    STOCK_DB_SET(TAPE_DB, ID_KEY, &value);
    STOCK_JSON_DTOR(&value);
}

// From the TapeController's json writer (Build B's mixer unit), at every write of the mounted tape's json: the id
// goes with the tape's other keys. Nothing is written for a tape without one: a later pack's number is not ours
// to change, and a number that came with a duplicate's json names no file in this slot. That number stays in
// the copy's json, which leaves one case open (the working note's 5.10): the original deleted, and the copy
// duplicated into the original's old slot, meets the original's file with the same number. It goes with the
// stage that handles a duplicate as a whole.
void op1f_xmap_json_write() {
    int m = STOCK_MOUNTED_TAPE();
    if (m < 0 || m >= TAPES || !tape_id[m] || tape_hands_off[m]) return;
    set_json_id(tape_id[m]);
}

// From the frame hook. An extension file that is not bound yet gets its id into the mounted tape's json; when
// stock has written the json (its dirty byte is clear again) the file on the disk is read, and only if it carries
// the id is the file bound and the map marked for saving, so that the flag reaches the disk. The id in the
// extension file never runs ahead of the json: a bound file whose json lacks the id is not the tape's.
void op1f_xmap_frame() {
    int m = STOCK_MOUNTED_TAPE();
    if (m != frame_tape) {                           // another tape, or none: start over
        frame_tape = m;
        frame_stable = 0;
        publish_waiting = 0;
        return;
    }
    if (m < 0 || m >= TAPES) return;
    if (frame_stable < SETTLE_FRAMES) {
        ++frame_stable;
        return;
    }
    u32 id = tape_id[m];
    if (publish_waiting) {
        if (id != publish_id) {                      // the file changed hands meanwhile
            publish_waiting = 0;
            return;
        }
        if (STOCK_TAPE_DB_DIRTY) return;             // stock has not written the document yet
        publish_waiting = 0;
        if (json_id(m) == id) {
            tape_bound[m] = 1;
            STOCK_MAP_DIRTY = 1;
            if (log_budget) {
                --log_budget;
                STOCK_LOG(3, LOG_FILE, 15, "tape %d: the tape's json on the disk carries the id %08x: the ext map is bound", m + 1, id);
            }
        } else if (publish_tries[m] >= PUBLISH_TRIES && log_budget) {
            --log_budget;
            STOCK_LOG(2, LOG_FILE, 16, "tape %d: the tape's json did not take the id %08x in %d tries: the ext map stays unbound", m + 1, id, PUBLISH_TRIES);
        }
        return;
    }
    if (id && !tape_bound[m] && !tape_hands_off[m] && publish_tries[m] < PUBLISH_TRIES) {
        ++publish_tries[m];
        set_json_id(id);
        publish_id = id;
        publish_waiting = 1;
        if (log_budget) {
            --log_budget;
            STOCK_LOG(3, LOG_FILE, 14, "tape %d: the id %08x is set in the tape's json (try %d)", m + 1, id, publish_tries[m]);
        }
    }
}
