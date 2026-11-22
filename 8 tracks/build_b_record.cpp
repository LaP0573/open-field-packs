// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build B, step B2: recording onto tape tracks 5 and 6 (track indices 4 and 5), armed with the tape key held.
//
// What stock does (OS 1.7.0+0): the record track is the
// GUI's selected track, the int32 at 0x11905a08. The TapeController writes it when a track key (scan codes
// 6-9) is pressed (0x8c0c18ec) and restores it from the tape's JSON; the L1 record function 0x11a07a74 reads
// it every block and writes the recorded audio into slot->data + track * 0x20000, sets the slot's dirty bit
// 1 << track and marks the crossfade range for 1 << track; the clip created at record stop carries it
// (0x8f009b78 -> Clips add 0x8d4eda0c -> validator 0x8d4ed7a6, "track < 4"); the DSP core routes the input
// through the selected track's mixer strip and per-track output buffer (0x11a02b28, 0x11a058b8) and the
// screens show it through the getter 0x8d514348. The tape key is scan code 4; while it is held the
// TapeController keeps the byte 0x84010218 set (0x8c0c341a / 0x8c0c1818), which stock uses to vary the two
// clip-edit keys 0x24/0x25.
//
// This unit (profile group "build-b2"):
//   * op1f_tape_key_filter / op1f_tape_shift_key_filter, called by the entry trampolines in build_b_record.s
//     at the TapeController's two key-down handlers: with the tape key held, track key 1 selects track 5 and
//     track key 2 track 6 (keys 3 and 4 are swallowed: tracks 7-8 do not exist yet); the stock press handler
//     is not run for them, because it writes four-entry tables (0x849a8714, 0x11902e34, 0x84c61d8c). B2 to
//     B3b swallowed the clip keys (10, 11, 12, their shift variants, the shift + blue encoder lift and, by a
//     misreading, the arrows 0x24/0x25) while track 5 or 6 was selected; since B3c every key runs stock, whose
//     tape_c lift, drop and clear helpers the B3c trampolines serve for tracks 4-5 (build_b_edit.cpp). Plain
//     track keys 1-4 go to stock and bring the selection back.
//   * op1f_gui_selected_track replaces the getter the screens use (0x8d514348). B2 mapped tracks 4-5 to 0-1
//     because the TapeScreen indexes four node pointers with the value; since B3a the tape screen's two
//     table accesses go through module stubs (build_b_mix.s) and the other callers (the help screen's compare,
//     the signal-flow screen through the replaced is-unmuted getter) are safe, so it returns the true track.
//   * op1f_clip_valid replaces the Clips record validator (0x8d4ed7a6) with the same checks and track < 6,
//     so the clip of a track-5/6 recording enters the list, is drawn in lane 5/6 and survives the JSON
//     round trip.
//   * op1f_track_output replaces the DSP core's call to the per-track output buffer lookup (0x8f01e3ec at
//     0x11a058b8): tracks 4-5 get a module scratch buffer instead of the bytes past the stock area, which
//     are the cache slot descriptors.
//   * Three register stubs in build_b_record.s bound the track where stock indexes a four-entry table with
//     it: input monitoring gains (0x11a02b60) and the VU peak word (0x11a02d3a), which since B3a point tracks
//     4-5 at the module's own strip and VU words (build_b_mix.cpp), and the JSON "track" field (0x8c0c1bee),
//     clamped to 3 so a tape file never sends stock firmware to a fifth track.
//   * B3a (build_b_mix.cpp): with the tape key held, the blue and green knobs set the selected track's volume
//     and pan (op1f_tape_encoder_filter) and shift + track key 1/2 toggles the mute of track 5/6
//     (op1f_tape_shift_key_filter); tracks 1-4 keep stock's shift + track key mutes.
// The record path itself (L1 record function, B0b's service loop, the version-2 saver) needs no change:
// B0a's slots hold six tracks and B0b writes dirty tracks 4-5 through the extension maps.
// Freestanding like the other units. Logging only from the GUI thread (the key handlers run there).

typedef unsigned int u32;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef i32 (*IntFn)();
typedef void* (*ObjectFn)();
typedef void (*NotifyFn)(void* object);
typedef i32* (*TrackOutputFn)(int track);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_CLIPS_IDLE ((IntFn)0x8d4ed6e4u)          // [0x8401ea04] == 0; stock selects a track only then
#define STOCK_TAPE_MODEL ((ObjectFn)0x8c0c1dd0u)       // the tape model object the screens observe
#define STOCK_NOTIFY ((NotifyFn)0x8fca1ed2u)           // observers redraw
#define STOCK_TRACK_OUTPUT ((TrackOutputFn)0x8f01e3ecu) // 0x84c67c5c + ((track + phase) << 9), tracks 0-3

#define SELECTED_TRACK (*(volatile i32*)0x11905a08u)
#define TAPE_KEY_HELD (*(volatile unsigned char*)0x84010218u)
#define TRANSPORT_MODE (*(volatile i32*)0x11905a04u)   // stock refuses a track change while this is 1

enum {
    STOCK_TRACKS = 4,
    EIGHT_TRACKS = 8,                   // F2: tracks 5-8 (6 from B2 to F1b)
    KEY_TAPE = 4,
    KEY_TRACK_1 = 6,
    KEY_TRACK_4 = 9,
    MAX_FRAME = 0x00f23fa0,   // kMaxTapePos, as the stock validator uses it
    LOG_BUDGET = 8
};
static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_record.cpp";

extern "C" u32 op1f_cache_tracks asm("op1f_cache_tracks");   // 6 with the heap-1 cache, 4 after B0a's fallback
extern "C" int op1f_ext_mix_adjust(int track, int knob, int delta) asm("op1f_ext_mix_adjust");   // B3a, build_b_mix.cpp
extern "C" int op1f_ext_mix_toggle_mute(int track) asm("op1f_ext_mix_toggle_mute");

// Counters for the measurement line (weak references in the first frame hook's measuring unit).
extern "C" u32 op1f_ext_track_selects asm("op1f_ext_track_selects");   // track 5/6 selected through the key combination
extern "C" u32 op1f_ext_keys_blocked asm("op1f_ext_keys_blocked");     // B2-B3b: clip-edit keys swallowed while 5/6 was selected; 0 since B3c
extern "C" u32 op1f_ext_monitor_blocks asm("op1f_ext_monitor_blocks"); // blocks in which the input went to the scratch buffer
u32 op1f_ext_track_selects;
u32 op1f_ext_keys_blocked;
u32 op1f_ext_monitor_blocks;

static u32 select_log_budget = LOG_BUDGET;
static u32 block_log_budget = LOG_BUDGET;
static i32 ext_output_scratch[128];                   // one per-track output block: left at +0, right at +0x100 bytes

// TapeController::on_key_down(this, event). Returns 1 when the module took the key.
extern "C" int op1f_tape_key_filter(void* self, const int* event) asm("op1f_tape_key_filter");
extern "C" int op1f_tape_key_filter(void*, const int* event) {
    int key = event[0];
    if (key >= KEY_TRACK_1 && key <= KEY_TRACK_4 && TAPE_KEY_HELD) {
        u32 track = (u32)STOCK_TRACKS + (u32)(key - KEY_TRACK_1);        // 4..7 for track keys 1..4
        if (track < EIGHT_TRACKS && op1f_cache_tracks >= EIGHT_TRACKS) {
            if (TRANSPORT_MODE != 1 && STOCK_CLIPS_IDLE()) {               // the same two conditions as stock's 0x8c0c18ec
                SELECTED_TRACK = (i32)track;
                STOCK_NOTIFY(STOCK_TAPE_MODEL());
                ++op1f_ext_track_selects;
                if (select_log_budget) {
                    --select_log_budget;
                    STOCK_LOG(3, LOG_FILE, 2, "track %d selected (tape key + track key %d)", track + 1, key - KEY_TRACK_1 + 1);
                }
            }
        } else if (block_log_budget) {
            --block_log_budget;
            STOCK_LOG(2, LOG_FILE, 3, "tape key + track key %d: track %d is not available (%d-track cache)",
                      key - KEY_TRACK_1 + 1, track + 1, op1f_cache_tracks);
        }
        return 1;                                                       // never arm track 1-4 from this combination
    }
    return 0;                                                       // every other key runs stock (since B3c the clip keys too)
}

// TapeController::on_shift_key_down(this, event): shift + track keys toggle the stock mutes of tracks 1-4 and
// stay stock; with the tape key held as well, track keys 1/2 toggle the mutes of tracks 5/6 (B3a) and keys 3/4
// are swallowed. Since B3c the shifted clip keys (lift, or lift-all of tracks 1-4; the mix drop; the join at a
// clip edge) and the arrows run stock for every selected track.
extern "C" int op1f_tape_shift_key_filter(void* self, const int* event) asm("op1f_tape_shift_key_filter");
extern "C" int op1f_tape_shift_key_filter(void*, const int* event) {
    int key = event[0];
    if (key >= KEY_TRACK_1 && key <= KEY_TRACK_4 && TAPE_KEY_HELD) {
        u32 track = (u32)STOCK_TRACKS + (u32)(key - KEY_TRACK_1);
        int done = track < EIGHT_TRACKS && op1f_cache_tracks >= EIGHT_TRACKS && op1f_ext_mix_toggle_mute((int)track);
        if (!done && block_log_budget) {
            --block_log_budget;
            STOCK_LOG(2, LOG_FILE, 5, "tape key + shift + track key %d: track %d is not available (%d-track cache)",
                      key - KEY_TRACK_1 + 1, track + 1, op1f_cache_tracks);
        }
        return 1;                                                       // never toggle a stock mute from this combination
    }
    return 0;
}

// TapeController::on_encoder(this, index, delta) 0x8c0c2e54. B3a: with the tape key held, the blue encoder (0)
// sets the selected track's volume and the green encoder (1) its pan, for all six tracks (build_b_mix.cpp).
// Index 4 (shift + the blue encoder) moves the loop points while the tape runs, but on a stopped tape it
// lifts the clip under the cursor into the clipboard (0x8c0c2608: clip search, then the poster 0x8f00c00c,
// whose assert the B3c trampoline bypasses for tracks 4-5); B2b-B3b swallowed that branch while track 5 or 6
// was selected, since B3c it passes. The other encoders (position, speed, the input and metronome levels) do
// not use the track.
extern "C" int op1f_tape_encoder_filter(void* self, int index, int delta) asm("op1f_tape_encoder_filter");
extern "C" int op1f_tape_encoder_filter(void*, int index, int delta) {
    if (TAPE_KEY_HELD && (index == 0 || index == 1) && op1f_ext_mix_adjust(SELECTED_TRACK, index, delta)) return 1;
    return 0;
}

// Replaces the selected-track getter 0x8d514348 (callers: the tape screen 0x8fc827dc, the help screen
// 0x8fc2e5bc, the signal-flow screen 0x8fc33bc4). B2 returned track - 4 for tracks 4-5; since B3a the true
// track: the tape screen's glyph table is served by build_b_mix.cpp, the help screen only compares, and the
// signal-flow screen's is-unmuted call goes to the replaced getter.
extern "C" int op1f_gui_selected_track() asm("op1f_gui_selected_track");
extern "C" int op1f_gui_selected_track() {
    return SELECTED_TRACK;
}

// Replaces the Clips record validator 0x8d4ed7a6: track in [0, 6), 0 <= start < stop, start < kMaxTapePos,
// stop <= kMaxTapePos. The record is 28 bytes: +0 track, +0x14 start frame, +0x18 stop frame.
extern "C" int op1f_clip_valid(const i32* record) asm("op1f_clip_valid");
extern "C" int op1f_clip_valid(const i32* record) {
    i32 track = record[0];
    i32 start = record[5];
    i32 stop = record[6];
    if (track < 0 || track >= EIGHT_TRACKS) return 0;
    if (start < 0 || stop < 0) return 0;
    if (!(start < stop)) return 0;
    if (!(start < MAX_FRAME)) return 0;
    if (!(stop <= MAX_FRAME)) return 0;
    return 1;
}

// Replaces the DSP core's call of 0x8f01e3ec at 0x11a058b8 (the input added into the selected track's output
// buffer). Runs on the audio thread: no logging.
extern "C" i32* op1f_track_output(int track) asm("op1f_track_output");
extern "C" i32* op1f_track_output(int track) {
    if ((u32)track < STOCK_TRACKS) return STOCK_TRACK_OUTPUT(track);
    ++op1f_ext_monitor_blocks;
    return ext_output_scratch;
}

// The measurement line's track label moved to build_b_mix.cpp with B3a (it shows the strip as well).
