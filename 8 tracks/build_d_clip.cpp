// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build D, step D1: lift-all and drop-all over six tracks (OS 1.7.0+0).
//
// What stock does (audio_clipboard.cpp / tape_c.cpp in record 307): shift + lift with the byte 0x11905a4d set
// runs the TapeController's lift-all 0x8c0c1c98, a loop over tracks 0-3 that puts every track with a clip at
// the cursor (0x8d4ed7b4) into a vector and posts command 4 with it, the cursor and the end 0x8f01ebc4()
// through 0x8f00aea8 (no per-track check there). The tape thread's handler 0x8f00b24c: take_snapshot_ex(0, -1);
// per track the split 0x8f00b1d0 at the start and at the end, the clip after the start (0x8d4edfa0) and its
// 28-byte record copied by 0x8d4eda26 into the clipboard state's slot of that track (state + 0x30 + 28 x track:
// four slots, a fifth would overwrite the length and the content type); the lift region 0x8fc809fc(-1, start,
// end, 1); the four effective volumes (volume x mute gain, Q31) into state + 0x10 and the pans into + 0x20; the
// vector assigned into the state (0x8f0099a4), the content type 1, the length, the patch string cleared
// (0x8c052744); per track the lift worker 0x8f00a62c(track, start, end), whose helper 0x8f0209f0 is hooked
// since B3c; then 0x8fc809cc() and the clipboard JSON 0x8f009e94(). The drop key with a multi-track clipboard
// posts command 1 (0x8f00b068) with the state's vector and the position; the handler 0x8f00ac78 bounds the
// position and the end (a level-3 line and return), asserts the content type, takes a snapshot of the chunks
// the drop will touch, runs the drop worker 0x8f00a870(record of the track, track, track, pos, end, 0) per
// track (the drop copy 0x8f021038 inside is hooked since B3c), clears the region and, with the tape stopped
// ([0x11905a04] == 0), moves the position to the end (0x8f01ff94).
//
// This unit (profile group "build-d"; the trampolines in build_d_clip.s):
//   * The loop bound of 0x8c0c1c98 becomes 6 (op1f_lift_all_bound), so a clip at the cursor on track 5 or 6
//     joins the lift.
//   * op1f_lift_all_ext / op1f_drop_all_ext: stock's two handlers for a vector that holds a track of 4 or
//     more (a vector of stock tracks only continues in stock): the same calls in the same order, with the
//     records of tracks 5-6 in op1f_ext_clip_records and their effective volumes and pans in
//     op1f_ext_clip_vol / op1f_ext_clip_pan (build_b_edit.cpp, which reads them for a mix drop and for the
//     sampler render). The audio goes where build_b_edit.cpp's rule puts it: stock's partition for a
//     clipboard of one track, a file for a track 5-6 of a clipboard of several.
// Freestanding like the other units. Everything here runs on the tape thread (the command handlers), so
// logging is allowed; the lines are budgeted. Counters for the measurement line: op1f_ext_lift_alls,
// op1f_ext_drop_alls.

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*SplitFn)(u32 track, i32 position);                                   // the clip at a position is split there
typedef i32 (*ClipAtFn)(u32 track, i32 position);                                   // the id of the clip at a position, or -1
typedef void* (*ClipsFn)();                                                         // the Clips store instance (asserts it exists)
typedef void* (*ClipRecordFn)(void* clips, i32 id, void* record);                   // copies the 28-byte record; 0 when the id is bad
typedef void (*LiftRegionFn)(i32 track, i32 start, i32 end, i32 flag);              // the tape screen's lift region (-1: every track)
typedef void (*VoidFn)();
typedef void (*VectorAssignFn)(void* destination, const void* source);              // the vector at +4/+8 of source into destination
typedef void (*StringClearFn)(void* string);                                        // a std::string emptied
typedef void (*LiftWorkerFn)(u32 track, i32 start, i32 end);
typedef void (*DropWorkerFn)(const void* record, u32 source, u32 destination, i32 position, i32 end, i32 mix);
typedef void (*SetPositionFn)(i32 position);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_SPLIT ((SplitFn)0x8f00b1d0u)
#define STOCK_CLIP_AT ((ClipAtFn)0x8d4edfa0u)
#define STOCK_CLIPS ((ClipsFn)0x8d4ed944u)
#define STOCK_CLIP_RECORD ((ClipRecordFn)0x8d4eda26u)
#define STOCK_LIFT_REGION ((LiftRegionFn)0x8fc809fcu)
#define STOCK_CLEAR_REGION ((VoidFn)0x8fc809ccu)
#define STOCK_WRITE_CLIPBOARD_JSON ((VoidFn)0x8f009e94u)
#define STOCK_VECTOR_ASSIGN ((VectorAssignFn)0x8f0099a4u)
#define STOCK_STRING_CLEAR ((StringClearFn)0x8c052744u)
#define STOCK_LIFT_WORKER ((LiftWorkerFn)0x8f00a62cu)
#define STOCK_DROP_WORKER ((DropWorkerFn)0x8f00a870u)
#define STOCK_SET_POSITION ((SetPositionFn)0x8f01ff94u)
#define STOCK_VOLUMES ((const volatile i32*)0x11b01eb0u)   // tracks 0-3: volumes, pans at +4 words, mute gains at +8 words
#define TRANSPORT_MODE (*(volatile i32*)0x11905a04u)        // 0: stopped

#define STATE ((u8*)0x84bdd944u)                              // the clipboard state (audio_clipboard.cpp)
#define STATE_VOLUMES ((volatile i32*)(STATE + 0x10))
#define STATE_PANS ((volatile i32*)(STATE + 0x20))
#define STATE_RECORDS (STATE + 0x30)                          // four 28-byte clip records, one per stock track
#define STATE_LENGTH (*(volatile i32*)(STATE + 0xa0))
#define STATE_CONTENT (*(volatile i32*)(STATE + 0xa4))        // 1: tape
#define STATE_PATCH ((void*)(STATE + 0xa8))

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                   // F2: tracks 5-8 (6 from B2 to F1b)
    EXT_TRACKS = 4,                     // the stride of op1f_ext_params: volumes 0-3, pans 4-7, mute gains 8-11
    RECORD_BYTES = 28,
    CHUNK_SHIFT = 14,
    MAX_FRAME = 0x00f23fa0,             // kMaxTapePos
    LOG_BUDGET = 32
};
static const char LOG_FILE[] = "../src/dsp/op1f_tape_clip_all.cpp";

extern "C" void op1f_take_snapshot_ex(i32 min_free, i32 tag) asm("op1f_take_snapshot_ex");   // build_b_storage.cpp (Build C)
extern "C" i32 op1f_ext_params[3 * EXT_TRACKS] asm("op1f_ext_params");                      // build_b_mix.cpp
extern "C" i32 op1f_ext_clip_vol[EXT_TRACKS] asm("op1f_ext_clip_vol");                      // build_b_edit.cpp (F2: four)
extern "C" i32 op1f_ext_clip_pan[EXT_TRACKS] asm("op1f_ext_clip_pan");
extern "C" i32 op1f_ext_clip_records[EXT_TRACKS][RECORD_BYTES / 4] asm("op1f_ext_clip_records");

extern "C" u32 op1f_ext_lift_alls asm("op1f_ext_lift_alls");
extern "C" u32 op1f_ext_drop_alls asm("op1f_ext_drop_alls");
u32 op1f_ext_lift_alls;
u32 op1f_ext_drop_alls;

static u32 log_budget = LOG_BUDGET;

static inline i32 mult_t(i32 a, i32 b) {                            // R = Ra * Rb (T), as stock's effective volume
    i32 r;
    asm("%0 = %1 * %2 (T);" : "=d"(r) : "d"(a), "d"(b));
    return r;
}

static void say(int level, int line, const char* what, i32 a, i32 b, u32 count) {
    if (!log_budget) return;
    --log_budget;
    STOCK_LOG(level, LOG_FILE, line, "%s: frames %d..%d, %u track(s)", what, a, b, count);
}

static void* record_of(u32 track) {
    return track < STOCK_TRACKS ? (void*)(STATE_RECORDS + track * RECORD_BYTES) : (void*)op1f_ext_clip_records[track - STOCK_TRACKS];
}

// Stock 0x8f00b24c(command) for a vector that holds a track of 4 or more, reached through op1f_lift_all. The
// command: the vector's begin at +8 and end at +0xc, the start at +0x14, the end at +0x18.
extern "C" void op1f_lift_all_ext(const i32* command) asm("op1f_lift_all_ext");
extern "C" void op1f_lift_all_ext(const i32* command) {
    const i32* begin = (const i32*)command[2];
    const i32* end = (const i32*)command[3];
    i32 start = command[5];
    i32 stop = command[6];
    u32 count = (begin && end >= begin) ? (u32)(end - begin) : 0;
    if (!count || start < 0 || !(start < stop) || stop > MAX_FRAME) {
        say(2, 1, "lift all refused", start, stop, count);
        return;
    }
    op1f_take_snapshot_ex(0, -1);
    for (const i32* p = begin; p != end; ++p) {
        u32 track = (u32)*p;
        if (track >= EIGHT_TRACKS) continue;
        STOCK_SPLIT(track, start);
        STOCK_SPLIT(track, stop);
        i32 id = STOCK_CLIP_AT(track, start + 1);
        if ((id == -1 || !STOCK_CLIP_RECORD(STOCK_CLIPS(), id, record_of(track))) && log_budget) {
            --log_budget;                                           // as stock's own handler: the track's record stays what it was (0x8f00a054)
            STOCK_LOG(3, LOG_FILE, 2, "lift all: track %u has no clip at the start of frames %d..%d: its record in the clipboard is left as it was", track + 1, start, stop);
        }
    }
    STOCK_LIFT_REGION(-1, start, stop, 1);
    for (u32 track = 0; track < STOCK_TRACKS; ++track) {          // stock: the effective volume and the pan of each track
        STATE_VOLUMES[track] = mult_t(STOCK_VOLUMES[2 * STOCK_TRACKS + track], STOCK_VOLUMES[track]);
        STATE_PANS[track] = STOCK_VOLUMES[STOCK_TRACKS + track];
    }
    for (u32 j = 0; j < EXT_TRACKS; ++j) {                          // the same for tracks 5-8, from the module's strip
        op1f_ext_clip_vol[j] = mult_t(op1f_ext_params[2 * EXT_TRACKS + j], op1f_ext_params[j]);
        op1f_ext_clip_pan[j] = op1f_ext_params[EXT_TRACKS + j];
    }
    STOCK_VECTOR_ASSIGN(STATE, (const u8*)command + 4);
    STATE_CONTENT = 1;
    STATE_LENGTH = stop - start;
    STOCK_STRING_CLEAR(STATE_PATCH);
    for (const i32* p = begin; p != end; ++p) {                    // the state's vector is set: build_b_edit.cpp's storage rule applies
        u32 track = (u32)*p;
        if (track < EIGHT_TRACKS) STOCK_LIFT_WORKER(track, start, stop);
    }
    STOCK_CLEAR_REGION();
    STOCK_WRITE_CLIPBOARD_JSON();
    ++op1f_ext_lift_alls;
    say(3, 3, "lift all", start, stop, count);
}

// Stock 0x8f00ac78(command) for a vector that holds a track of 4 or more, reached through op1f_drop_all. The
// command: the vector's begin at +8 and end at +0xc, the position at +0x14; the length is the state's.
extern "C" void op1f_drop_all_ext(const i32* command) asm("op1f_drop_all_ext");
extern "C" void op1f_drop_all_ext(const i32* command) {
    const i32* begin = (const i32*)command[2];
    const i32* end = (const i32*)command[3];
    i32 position = command[5];
    i32 length = STATE_LENGTH;
    i32 stop = position + length;
    u32 count = (begin && end >= begin) ? (u32)(end - begin) : 0;
    if (!count || position < 0 || length <= 0 || position > MAX_FRAME || stop > MAX_FRAME) {   // stock: a level-3 line and return
        say(2, 4, "drop all refused (out of range)", position, stop, count);
        return;
    }
    if (STATE_CONTENT == 0) {                                       // stock asserts here
        say(2, 5, "drop all refused (no content)", position, stop, count);
        return;
    }
    i32 chunks = (((stop - 1) >> CHUNK_SHIFT) - (position >> CHUNK_SHIFT) + 1) * (i32)count;
    op1f_take_snapshot_ex(chunks, -1);
    for (const i32* p = begin; p != end; ++p) {
        u32 track = (u32)*p;
        if (track < EIGHT_TRACKS) STOCK_DROP_WORKER(record_of(track), track, track, position, stop, 0);
    }
    STOCK_CLEAR_REGION();
    if (TRANSPORT_MODE == 0) STOCK_SET_POSITION(stop);
    ++op1f_ext_drop_alls;
    say(3, 6, "drop all", position, stop, count);
}
