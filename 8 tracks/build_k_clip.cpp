// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K, pack K3: the clipboard's file is always one that stock
// firmware reads.
//
// Stock describes the clipboard in user/clipboard.json: the lifted tracks ("channels"), the content's name, the
// length in frames, four volumes and four pans, a patch's data. Its loader 0x8f00c5e0 puts the channels back into
// the clipboard's state as they are, and its drop takes each of them for a track of 0 to 3: a channel above that
// runs into an assert, which is fatal (tape_c.cpp:1746, Sampler.cpp:74). Until K3 a lift of tracks 5 to 8 put
// their numbers, 4 to 7, into that file, written by stock's own writer 0x8f009e94 from the state's list.
//
// Since K3:
//
//   * user/clipboard.json names tracks 1 to 4 only. A clipboard that holds nothing of them is written as stock's
//     empty clipboard: no channel, the content "", the length 0. That is what a new unit has, and what stock's drop
//     tests before it posts anything (0x8f00af54: the content, then the length).
//   * What the clipboard holds of tracks 5 to 8 stands in user/op1f_clip.bin: the whole list of tracks in its
//     order, the true content and length, and the volumes and pans of tracks 5 to 8 (which no file kept until now).
//   * The two files are tied by an id: an integer "op1f_clip_id" in stock's json, the same number in ours. Stock's
//     own writer builds its json anew at every lift, so under stock firmware the key is gone with the first lift,
//     and our file is then no longer the clipboard's. Stock's loader looks its keys up by name and passes over one
//     it does not know.
//   * Only a tape lift names tracks. Stock's four lifts from the samplers (0x8f009c78, 0x8f009d24, 0x8f009dd0,
//     0x8f009e6c) put a list of one element, 0, into the state with another content; for any content but the tape
//     the json is stock's own, with no id and no file of ours.
//   * A clipboard of tracks 1 to 4 alone is written exactly as stock writes it: the same calls in the same order,
//     no key of ours, nothing written to our file (a file of ours left from before is not the clipboard's: stock's
//     json has no id).
//
// Stock's file is written first, ours second. A unit that goes off between the two has a stock file with a new id
// and a file of ours with an old one: the clipboard is then what stock's file says, tracks 1 to 4.
//
// The file of ours, little-endian, 68 bytes:
//
//   0   2   the magic "XC" (0x4358)
//   2   2   the version, 1
//   4   4   the id, 1 to 0x3fffffff
//   8   2   the number of tracks in the clipboard, 1 to 8
//   10  2   the content (stock's index: 1 tape)
//   12  4   the length in frames
//   16  16  the tracks in the clipboard's order, 0 to 7 each, 0xFFFF for none
//   32  16  the effective volumes of tracks 5 to 8 (Q31)
//   48  16  their pans
//   64  4   the CRC-32 of the 64 bytes before
//
// Two hooks (the profile's group build-k3):
//
//   * 0x8f009e94, stock's writer, replaced by a JUMP.a to
//     op1f_clipboard_write. Its callers: the lift handler (0x8f00961c), the four lifts from the samplers
//     (0x8f009d16, 0x8f009dc2, 0x8f009e5e, 0x8f009efa), the lift-all handler (0x8f00a120), and the module's own
//     lift-all (build_d_clip.cpp). None takes an answer.
//   * 0x8c0b4a04, the start-up's CALL.a of stock's loader, right after "[init callbacks (17)]", retargeted to
//     op1f_clipboard_load: stock's loader, then the file of ours when it is the clipboard's. A json of before K3
//     that names a track above four is written anew there, as the two files.
//
// The clipboard's state in memory is as it was: the list holds every lifted track, 0 to 7, and the module's
// editing code (build_b_edit.cpp, build_d_clip.cpp) reads it as before.
//
// Read in stock for it (the sweeps of 2026-11-11): the writer whole (the json built member by member through
// 0x8c085e84, which makes a member that is not there; an int by 0x8c083b1c, a float by 0x8c08437c from the ROM's
// int-to-float less 31 in the exponent, zero kept zero; the content's name from the table 0x848c5bc0; the patch
// string by 0x8c08448c; the path a stock string object, freed when it is not held in the object itself); the
// loader whole (the file read; "channels" walked and pushed; "mixer_volume" asked for by name, the live mixer's
// words when it is absent; "framecount", "patch_data", "content" by name; nothing else looked at); the drop's
// poster 0x8f00af54; the list's assign 0x8f0099a4, which reads its source's two pointers and nothing more.

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*VoidFn)();
typedef void* (*VectorAssignFn)(void* target, const void* source);    // 0x8f0099a4: the list at +4/+8 of the source into the target's
typedef void (*JsonCtorFn)(void* json, int kind);                      // 0x8c085ba8: 0 a null
typedef void (*JsonDtorFn)(void* json);                                // 0x8c0844e0
typedef void* (*JsonLookupFn)(void* json, const char* key);            // 0x8c085e84: the member, made when it is not there
typedef int (*JsonHasFn)(void* json, const char* key);                 // 0x8c085068, of an object
typedef void* (*JsonGetFn)(void* json, const char* key);               // 0x8c084f74
typedef int (*JsonIntFn)(void* json);                                  // 0x8c084840: the three number kinds
typedef void (*JsonFromIntFn)(void* json, int value);                  // 0x8c083b1c
typedef void (*JsonFromFloatFn)(void* json, u32 float_bits);           // 0x8c08437c: the kind 3 from a float's bits
typedef void (*JsonFromCstrFn)(void* json, const char* text);          // 0x8c084430: a string copied from the text
typedef void (*JsonFromStringFn)(void* json, const void* string);      // 0x8c08448c: a string copied from a stock string object
typedef void (*JsonPushFn)(void* array, void* value);                  // 0x8c085d54: a null becomes an array at its first push
typedef void (*JsonAssignFn)(void* target, void* value);               // 0x8c0845b8
typedef int (*JsonReadFileFn)(const char* path, void* json);           // 0x8c0534d0: non-zero when the file was read and parsed
typedef int (*JsonWriteFileFn)(void* path_string, void* json);         // 0x8c053168: a temporary, then a rename; 0 when that fails
typedef void (*StringCtorFn)(void* out, const char* text);             // 0x8c03e434: a stock string object from text
typedef void (*FreeFn)(void* block);                                   // 0x8d4e63cc: the match of the allocator stock's strings use
typedef u32 (*FloatFromIntFn)(i32 value);                              // ROM 0x04021854: the float of an int, as bits
typedef i32 (*FopenFn)(const char* path, const char* mode);            // 0x8fc22180: a descriptor, negative when the open fails
typedef i32 (*FwriteFn)(const void* source, i32 size, i32 count, i32 file);
typedef i32 (*FreadFn)(void* destination, i32 size, i32 count, i32 file);
typedef void (*FcloseFn)(i32 file);
typedef void (*RenameFn)(const char* old_path, const char* new_path);
typedef i32 (*FileSizeFn)(const char* path);                           // 0x8c051f9c: the bytes, -1 when there is no such file; no log line

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_CLIPBOARD_LOAD ((VoidFn)0x8f00c5e0u)
#define STOCK_VECTOR_ASSIGN ((VectorAssignFn)0x8f0099a4u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_FROM_FLOAT ((JsonFromFloatFn)0x8c08437cu)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_FROM_STRING ((JsonFromStringFn)0x8c08448cu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
#define STOCK_FLOAT_FROM_INT ((FloatFromIntFn)0x04021854u)
#define STOCK_FOPEN ((FopenFn)0x8fc22180u)
#define STOCK_FWRITE ((FwriteFn)0x8fc220b8u)
#define STOCK_FREAD ((FreadFn)0x8fc2214cu)
#define STOCK_FCLOSE ((FcloseFn)0x8fc220a0u)
#define STOCK_RENAME ((RenameFn)0x8fc222f4u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_CLIP_STATE ((u8*)0x84bdd944u)                            // the clipboard's state (audio_clipboard.cpp)
#define STOCK_CONTENT_NAMES ((const char* const*)0x848c5bc0u)         // "", tape, drum, sampler, synth, synth_drum

// the state: +4/+8 the list of lifted tracks, +0x10 four volumes, +0x20 four pans, +0xa0 the length in frames,
// +0xa4 the content, +0xa8 the patch's data, a stock string object
#define VECTOR_BEGIN (*(i32* volatile*)(STOCK_CLIP_STATE + 0x04))
#define VECTOR_END (*(i32* volatile*)(STOCK_CLIP_STATE + 0x08))
#define STATE_VOLUMES ((volatile i32*)(STOCK_CLIP_STATE + 0x10))
#define STATE_PANS ((volatile i32*)(STOCK_CLIP_STATE + 0x20))
#define STATE_LENGTH (*(volatile i32*)(STOCK_CLIP_STATE + 0xa0))
#define STATE_CONTENT (*(volatile i32*)(STOCK_CLIP_STATE + 0xa4))
#define STATE_PATCH ((void*)(STOCK_CLIP_STATE + 0xa8))

#ifdef OP1F_HOST
unsigned int op1f_host_cycles();
static u32 cycles() { return op1f_host_cycles(); }
#else
static u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}
#endif

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,
    EXT_TRACKS = EIGHT_TRACKS - STOCK_TRACKS,
    CONTENTS = 6,
    CONTENT_NONE = 0,
    CONTENT_TAPE = 1,                            // the one content that names tracks: stock's four other lifts put track 0 in the list
    NO_TRACK = 0xffff,
    EXT_MAGIC = 0x4358,                          // "XC"
    EXT_VERSION = 1,
    EXT_WORDS = 34,                              // 68 bytes
    EXT_SUMMED_WORDS = 32,
    EXT_BYTES = EXT_WORDS * 2,
    JSON_OBJECT = 7,
    LOG_BUDGET = 24
};

static const char LOG_FILE[] = "../src/dsp/op1f_clipboard.cpp";
static const char CLIPBOARD_PATH[] = "user/clipboard.json";
static const char EXT_PATH[] = "user/op1f_clip.bin";
static const char EXT_TEMP[] = "user/op1f_clip.tmp";
static const char KEY_CHANNELS[] = "channels";       // stock's six, as its writer and loader name them
static const char KEY_VOLUME[] = "mixer_volume";
static const char KEY_PAN[] = "mixer_pan";
static const char KEY_CONTENT[] = "content";
static const char KEY_PATCH[] = "patch_data";
static const char KEY_FRAMES[] = "framecount";
static const char KEY_ID[] = "op1f_clip_id";         // ours

struct Json { u32 words[4]; };                       // a stock json value is 12 bytes; 16 for alignment

extern "C" i32 op1f_ext_clip_vol[EXT_TRACKS] asm("op1f_ext_clip_vol");   // build_b_edit.cpp: the effective volumes and pans of
extern "C" i32 op1f_ext_clip_pan[EXT_TRACKS] asm("op1f_ext_clip_pan");   // tracks 5 to 8 as the last lift-all found them

extern "C" void op1f_clipboard_write() asm("op1f_clipboard_write");
extern "C" void op1f_clipboard_load() asm("op1f_clipboard_load");
extern "C" u32 op1f_clipboard_counts[8] asm("op1f_clipboard_counts");
u32 op1f_clipboard_counts[8];                        // writes, with tracks 5-8, write failures, loads, merges, stale, conversions, unusable

static u32 log_budget = LOG_BUDGET;
static u32 id_counter;

// What the clipboard holds, as its state's list says, or as the file of ours does.
struct Clip {
    u16 tracks[EIGHT_TRACKS];                        // every track of 0 to 7 in the list's order
    u32 count;                                       // how many of them
    u32 stock;                                       // of tracks 1 to 4
    u32 ext;                                         // of tracks 5 to 8
    u32 content;
    u32 length;
    u32 id;
    i32 vol[EXT_TRACKS];
    i32 pan[EXT_TRACKS];
};

static int extension_track(u32 track) { return track >= STOCK_TRACKS && track < EIGHT_TRACKS; }

static void count_tracks(Clip* c) {
    c->stock = 0;
    c->ext = 0;
    for (u32 i = 0; i < c->count; ++i) {
        if (c->tracks[i] < STOCK_TRACKS) ++c->stock;
        else ++c->ext;
    }
}

static void read_state(Clip* c) {
    const i32* begin = VECTOR_BEGIN;
    const i32* end = VECTOR_END;
    c->count = 0;
    if (begin && end > begin) {
        for (const i32* p = begin; p != end && c->count < EIGHT_TRACKS; ++p) {
            u32 track = (u32)*p;
            if (track < EIGHT_TRACKS) c->tracks[c->count++] = (u16)track;
        }
    }
    for (u32 i = c->count; i < EIGHT_TRACKS; ++i) c->tracks[i] = NO_TRACK;
    count_tracks(c);
    c->content = (u32)STATE_CONTENT;
    c->length = (u32)STATE_LENGTH;
    c->id = 0;
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        c->vol[j] = op1f_ext_clip_vol[j];
        c->pan[j] = op1f_ext_clip_pan[j];
    }
}

// "1 2 5" for the tracks of [low, high) in the list; "none" for none. At most eight tracks: 16 characters.
static void tracks_text(char* out, const Clip* c, u32 low, u32 high) {
    u32 n = 0;
    for (u32 i = 0; i < c->count; ++i) {
        u32 track = c->tracks[i];
        if (track < low || track >= high) continue;
        if (n) out[n++] = ' ';
        out[n++] = (char)('1' + track);
    }
    if (!n) {
        out[0] = 'n'; out[1] = 'o'; out[2] = 'n'; out[3] = 'e';
        n = 4;
    }
    out[n] = 0;
}

static u32 crc32(const u8* bytes, u32 length) {
    u32 crc = 0xffffffffu;
    for (u32 i = 0; i < length; ++i) {
        crc ^= bytes[i];
        for (u32 bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

static u32 new_id() {
    u32 c = cycles();
    u32 id = (c ^ (c >> 13) ^ (++id_counter * 0x9e3779b1u)) & 0x3fffffffu;
    return id ? id : 1;
}

// A Q31 word as the float stock writes: the ROM's float of the int, less 31 in the exponent; zero stays zero.
static u32 q31_float(i32 value) {
    return value ? STOCK_FLOAT_FROM_INT(value) - 0x0f800000u : 0;
}

static void push_float(Json* root, const char* key, i32 value) {
    void* member = STOCK_JSON_LOOKUP(root, key);
    Json item;
    STOCK_JSON_FROM_FLOAT(&item, q31_float(value));
    STOCK_JSON_PUSH(member, &item);
    STOCK_JSON_DTOR(&item);
}

static void set_int(Json* root, const char* key, int value) {
    void* member = STOCK_JSON_LOOKUP(root, key);
    Json item;
    STOCK_JSON_FROM_INT(&item, value);
    STOCK_JSON_ASSIGN(member, &item);
    STOCK_JSON_DTOR(&item);
}

// Stock's json, call for call as stock's writer 0x8f009e94 builds it, with two differences: a track of 5 to 8 is
// not named, and when the clipboard holds one, the id goes in, and a clipboard with nothing of tracks 1 to 4 is
// written as an empty one.
static int write_stock_json(const Clip* c) {
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    const i32* begin = VECTOR_BEGIN;
    const i32* end = VECTOR_END;
    if (begin) {
        for (const i32* p = begin; p != end; ++p) {
            if (extension_track((u32)*p)) continue;
            void* member = STOCK_JSON_LOOKUP(&root, KEY_CHANNELS);
            Json item;
            STOCK_JSON_FROM_INT(&item, *p);
            STOCK_JSON_PUSH(member, &item);
            STOCK_JSON_DTOR(&item);
        }
    }
    for (u32 i = 0; i < STOCK_TRACKS; ++i) {
        push_float(&root, KEY_VOLUME, STATE_VOLUMES[i]);
        push_float(&root, KEY_PAN, STATE_PANS[i]);
    }
    int empty_for_stock = c->id && !c->stock;        // a clipboard of ours with nothing of tracks 1 to 4 in it
    u32 content = empty_for_stock ? (u32)CONTENT_NONE : c->content;
    {
        void* member = STOCK_JSON_LOOKUP(&root, KEY_CONTENT);
        Json item;
        STOCK_JSON_FROM_CSTR(&item, STOCK_CONTENT_NAMES[content < CONTENTS ? content : CONTENT_NONE]);
        STOCK_JSON_ASSIGN(member, &item);
        STOCK_JSON_DTOR(&item);
    }
    {
        void* member = STOCK_JSON_LOOKUP(&root, KEY_PATCH);
        Json item;
        STOCK_JSON_FROM_STRING(&item, STATE_PATCH);
        STOCK_JSON_ASSIGN(member, &item);
        STOCK_JSON_DTOR(&item);
    }
    set_int(&root, KEY_FRAMES, empty_for_stock ? 0 : (int)c->length);
    if (c->id) set_int(&root, KEY_ID, (int)c->id);
    u32 path[8];                                     // a stock string object: +4 the text or its pointer, +0x14 the length, +0x18 the capacity
    for (int i = 0; i < 8; ++i) path[i] = 0;
    STOCK_STRING_CTOR(path, CLIPBOARD_PATH);
    int written = STOCK_JSON_WRITE_FILE(path, &root);
    if (path[6] >= 16 && path[1]) STOCK_FREE((void*)path[1]);   // nineteen characters: the text is on the heap
    STOCK_JSON_DTOR(&root);
    return written;
}

static void put32(u16* words, u32 value) {
    words[0] = (u16)value;
    words[1] = (u16)(value >> 16);
}

static u32 get32(const u16* words) { return (u32)words[0] | ((u32)words[1] << 16); }

static int write_ext_file(const Clip* c) {
    u16 w[EXT_WORDS];
    w[0] = EXT_MAGIC;
    w[1] = EXT_VERSION;
    put32(&w[2], c->id);
    w[4] = (u16)c->count;
    w[5] = (u16)c->content;
    put32(&w[6], c->length);
    for (u32 i = 0; i < EIGHT_TRACKS; ++i) w[8 + i] = i < c->count ? c->tracks[i] : (u16)NO_TRACK;
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        put32(&w[16 + 2 * j], (u32)c->vol[j]);
        put32(&w[24 + 2 * j], (u32)c->pan[j]);
    }
    put32(&w[EXT_SUMMED_WORDS], crc32((const u8*)w, EXT_SUMMED_WORDS * 2));
    i32 file = STOCK_FOPEN(EXT_TEMP, "w+b");
    if (file < 0) return 0;                          // stock has logged the open
    i32 put = STOCK_FWRITE(w, 2, EXT_WORDS, file);
    STOCK_FCLOSE(file);
    if (put != EXT_WORDS) return 0;
    STOCK_RENAME(EXT_TEMP, EXT_PATH);
    return 1;
}

// The file of ours. 0: read whole; 1: there is none; 2: not usable; 3: of a later version (left alone).
static int read_ext_file(Clip* c) {
    i32 size = STOCK_FILE_SIZE(EXT_PATH);            // asked without an open, which would log an error for a missing file
    if (size < 0) return 1;
    u16 w[EXT_WORDS];
    for (int i = 0; i < EXT_WORDS; ++i) w[i] = 0;
    i32 file = STOCK_FOPEN(EXT_PATH, "rb");
    if (file < 0) return 2;
    i32 got = STOCK_FREAD(w, 2, EXT_WORDS, file);
    STOCK_FCLOSE(file);
    if (got >= 2 && w[0] == EXT_MAGIC && w[1] > EXT_VERSION) return 3;
    if (got != EXT_WORDS || w[0] != EXT_MAGIC || w[1] != EXT_VERSION) return 2;
    if (get32(&w[EXT_SUMMED_WORDS]) != crc32((const u8*)w, EXT_SUMMED_WORDS * 2)) return 2;
    c->id = get32(&w[2]);
    c->count = w[4];
    c->content = w[5];
    c->length = get32(&w[6]);
    if (!c->id || c->count < 1 || c->count > EIGHT_TRACKS || c->content != CONTENT_TAPE) return 2;
    for (u32 i = 0; i < EIGHT_TRACKS; ++i) {
        c->tracks[i] = w[8 + i];
        if (i < c->count ? c->tracks[i] >= EIGHT_TRACKS : c->tracks[i] != NO_TRACK) return 2;
    }
    count_tracks(c);
    if (!c->ext) return 2;                           // a file of ours is written for tracks 5 to 8 only
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        c->vol[j] = (i32)get32(&w[16 + 2 * j]);
        c->pan[j] = (i32)get32(&w[24 + 2 * j]);
    }
    return 0;
}

// The id stock's json carries, 0 for none. The file is there: stock's loader has just read it.
static u32 stock_json_id() {
    Json json;
    u32 id = 0;
    STOCK_JSON_CTOR(&json, 0);
    if (STOCK_JSON_READ_FILE(CLIPBOARD_PATH, &json) && *((const u8*)&json + 4) == JSON_OBJECT && STOCK_JSON_HAS(&json, KEY_ID))
        id = (u32)STOCK_JSON_INT(STOCK_JSON_GET(&json, KEY_ID));
    STOCK_JSON_DTOR(&json);
    return id;
}

// Every lift ends here, in place of stock's writer.
void op1f_clipboard_write() {
    Clip c;
    read_state(&c);
    int ours_too = c.ext && c.content == CONTENT_TAPE;   // tracks 5 to 8 of a tape lift: the id, and the file of ours
    c.id = ours_too ? new_id() : 0;
    int stock_written = write_stock_json(&c);
    int ext_written = ours_too ? write_ext_file(&c) : 1;
    ++op1f_clipboard_counts[0];
    if (ours_too) ++op1f_clipboard_counts[1];
    if (!stock_written || !ext_written) {
        ++op1f_clipboard_counts[2];
        STOCK_LOG(1, LOG_FILE, 1, "clipboard: the write of %s failed: after a restart the clipboard is what the files on the disk say",
                  stock_written ? EXT_PATH : CLIPBOARD_PATH);
        return;
    }
    if (ours_too && log_budget) {
        char stock_text[20], ext_text[20];
        tracks_text(stock_text, &c, 0, STOCK_TRACKS);
        tracks_text(ext_text, &c, STOCK_TRACKS, EIGHT_TRACKS);
        --log_budget;
        STOCK_LOG(3, LOG_FILE, 2, "clipboard written: stock's file names track(s) %s, ours adds %s, id %08x, %d frames", stock_text, ext_text, c.id, c.length);
    }
}

// The start-up's load, in place of the call of stock's loader.
void op1f_clipboard_load() {
    STOCK_CLIPBOARD_LOAD();
    ++op1f_clipboard_counts[3];
    Clip now;
    read_state(&now);
    if (now.ext && now.content == CONTENT_TAPE) {    // a json of before K3: it names a track above four
        ++op1f_clipboard_counts[6];
        char text[20];
        tracks_text(text, &now, 0, EIGHT_TRACKS);
        STOCK_LOG(3, LOG_FILE, 3, "clipboard: stock's file named tracks above four (%s): written anew as stock's file and ours", text);
        op1f_clipboard_write();
        return;
    }
    if (STOCK_FILE_SIZE(CLIPBOARD_PATH) < 0) return;     // no clipboard yet
    Clip ours;
    int state = read_ext_file(&ours);
    if (state == 1) return;                          // nothing of ours
    if (state == 3) {
        STOCK_LOG(2, LOG_FILE, 4, "clipboard: %s is of a newer version than this pack reads: tracks 5-8 are not in the clipboard", EXT_PATH);
        return;
    }
    if (state == 2) {
        ++op1f_clipboard_counts[7];
        STOCK_LOG(1, LOG_FILE, 5, "clipboard: %s is not usable: tracks 5-8 are not in the clipboard", EXT_PATH);
        return;
    }
    u32 json_id = stock_json_id();
    // ours is the clipboard's when stock's json carries its id and names, in the same order, the tracks 1 to 4 ours knows
    int same = json_id == ours.id && now.stock == ours.stock;
    if (same) {
        u32 at = 0;
        for (u32 i = 0; i < ours.count; ++i) {
            if (ours.tracks[i] >= STOCK_TRACKS) continue;
            if (now.tracks[at++] != ours.tracks[i]) same = 0;
        }
        if (now.stock && (now.length != ours.length || now.content != ours.content)) same = 0;
    }
    if (!same) {
        ++op1f_clipboard_counts[5];
        STOCK_LOG(3, LOG_FILE, 6, "clipboard: %s (id %08x) is not this clipboard's (stock's file has %08x): tracks 5-8 are not in the clipboard", EXT_PATH, ours.id, json_id);
        return;
    }
    i32 list[EIGHT_TRACKS];
    for (u32 i = 0; i < ours.count; ++i) list[i] = ours.tracks[i];
    struct { u32 unused; i32* begin; i32* end; i32* capacity; } holder = { 0, list, list + ours.count, list + ours.count };
    STOCK_VECTOR_ASSIGN(STOCK_CLIP_STATE, &holder);
    STATE_CONTENT = (i32)ours.content;
    STATE_LENGTH = (i32)ours.length;
    for (u32 j = 0; j < EXT_TRACKS; ++j) {
        op1f_ext_clip_vol[j] = ours.vol[j];
        op1f_ext_clip_pan[j] = ours.pan[j];
    }
    ++op1f_clipboard_counts[4];
    char stock_text[20], ext_text[20];
    tracks_text(stock_text, &ours, 0, STOCK_TRACKS);
    tracks_text(ext_text, &ours, STOCK_TRACKS, EIGHT_TRACKS);
    STOCK_LOG(3, LOG_FILE, 7, "clipboard: stock's file names track(s) %s, ours adds %s, id %08x: %d track(s), %d frames", stock_text, ext_text, ours.id, ours.count, ours.length);
}
