// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4b, the pack 8 tracks (version 9): 8 tracks 8's frame unit with one call more, and nothing else
// changed: at the head of each frame, op1f_tape_keeps_frame (8 tracks 9's values unit), which reads tracks 5 to
// 8's strip and the selected track from a tape the base has just told of, and keeps the base's values of the tape
// what they are. It is called inside the pack's frame function, where op1f_value_get and op1f_value_set are answered.
// What follows is the elder's description, which holds.
//
// Step R2: the pack `8 tracks` on a card of level 1. Its frame function does, through the pack's own card, what the
// base's frame hook did for this pack until base 19 (M1's measuring unit): the mixer page's flag, the two
// frame calls of the tape's units, the meters of tracks 5 to 8 read and cleared once a second, and the pack's seven
// lines on the log. And the pack's choice among its pictures (`pick`), so that the signal flow screen's picture is
// the sidechain's where that pack runs.
//
// The calls, in this order, once a frame:
//   - the mixer page's flag cleared when the mode is not the mixer: the key's release went elsewhere (the same word
//     the base read, which the frame's `mode` is made from);
//   - op1f_sums_frame: the sums of the chunks of tracks 5 to 8 (8 tracks/build_k_sums.cpp);
//   - op1f_xmap_frame: the id of the tape's extension file into the tape's json (8 tracks/build_l3_maps.cpp);
//   - once every 1,000 ms of the base's clock, the meters of tracks 5 to 8 read and cleared, as the mixer page and the
//     signal flow screen clear them (Build B's mixer unit, 8 tracks/build_e_screens.cpp);
//   - once every sixteen of those seconds, a period of the gate (module/build_q_quiet.h): the seven lines are printed
//     for the first three periods of a start, for a period in which a count they state changed and the two after it,
//     when the power popup comes up, and otherwise once in 75 periods (twenty minutes). Their words are the base's,
//     byte for byte, under the same file and the same tags, so that a read-out shows `op1f_cfw[9]: ext mixer: ...`
//     as before. The rates of the `ext io:` line are since the last line printed, as its words say.
//
// What it needs: op1f_level_1 (the card), the counters of the pack's own units, the firmware's logger at the address
// the pack's other units call it at, and two names it asks for weakly: op1f_svg_served of the base (the pictures the
// base served, 0 where the base does not offer it) and op1f_duck_screen of the pack `sidechain` (which runs or not).
//
// Runs on the display thread; the pick on whatever thread asks for a picture. Nothing is divided by a number that is
// not a power of two but through udiv (the module links no runtime). Compiled for the PC with OP1F_HOST as it is.

#include "build_r1_card.h"
#include "build_q_quiet.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);

#ifdef OP1F_HOST
extern "C" void mock_log(int level, const char* file, int line, const char* format, ...);
extern int mock_svg_served_there;                   // the PC run's stand-ins for what the linker answers on the unit
extern int mock_sidechain_there;
#define STOCK_LOG mock_log
#define OP1F_WEAK
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)             // log(level, file, line, fmt, ...), level 3 = I
#define OP1F_WEAK __attribute__((weak))
#endif

// The pack's own units: what they count, read here.
extern "C" u32 op1f_ext_play_blocks OP1F_NAMED("op1f_ext_play_blocks");
extern "C" u32 op1f_ext_play_active OP1F_NAMED("op1f_ext_play_active");
extern "C" int op1f_ext_play_w0 OP1F_NAMED("op1f_ext_play_w0");
extern "C" u32 op1f_ext_play_skipped OP1F_NAMED("op1f_ext_play_skipped");
extern "C" u32 op1f_ext_play_tracks OP1F_NAMED("op1f_ext_play_tracks");
extern "C" u32 op1f_ext_track_selects OP1F_NAMED("op1f_ext_track_selects");
extern "C" u32 op1f_ext_keys_blocked OP1F_NAMED("op1f_ext_keys_blocked");
extern "C" u32 op1f_ext_monitor_blocks OP1F_NAMED("op1f_ext_monitor_blocks");
extern "C" const char* op1f_record_track_label() OP1F_NAMED("op1f_record_track_label");
extern "C" unsigned short op1f_ext_vu[4] OP1F_NAMED("op1f_ext_vu");              // the meters of tracks 5 to 8, read and cleared here once a second
extern "C" u32 op1f_ext_mix_changes OP1F_NAMED("op1f_ext_mix_changes");
extern "C" u32 op1f_ext_xfade_computes OP1F_NAMED("op1f_ext_xfade_computes");
extern "C" u32 op1f_ext_xfade_fades OP1F_NAMED("op1f_ext_xfade_fades");
extern "C" u32 op1f_ext_xfade_waits OP1F_NAMED("op1f_ext_xfade_waits");
extern "C" u32 op1f_ext_loop_blocks OP1F_NAMED("op1f_ext_loop_blocks");
extern "C" u32 op1f_ext_edge_fades OP1F_NAMED("op1f_ext_edge_fades");
extern "C" u32 op1f_ext_lifts OP1F_NAMED("op1f_ext_lifts");
extern "C" u32 op1f_ext_drops OP1F_NAMED("op1f_ext_drops");
extern "C" u32 op1f_ext_clears OP1F_NAMED("op1f_ext_clears");
extern "C" u32 op1f_ext_edit_refusals OP1F_NAMED("op1f_ext_edit_refusals");
extern "C" u32 op1f_snapshots_taken OP1F_NAMED("op1f_snapshots_taken");
extern "C" u32 op1f_snapshots_dropped OP1F_NAMED("op1f_snapshots_dropped");
extern "C" u32 op1f_ext_lift_alls OP1F_NAMED("op1f_ext_lift_alls");
extern "C" u32 op1f_ext_drop_alls OP1F_NAMED("op1f_ext_drop_alls");
extern "C" u32 op1f_ext_renders OP1F_NAMED("op1f_ext_renders");
extern "C" u32 op1f_midi_ext_changes OP1F_NAMED("op1f_midi_ext_changes");
extern "C" u32 op1f_mtp_extra_lookups OP1F_NAMED("op1f_mtp_extra_lookups");
extern "C" volatile unsigned char op1f_mixer_page OP1F_NAMED("op1f_mixer_page");   // set while the mixer key is held on the levels page
extern "C" u32 op1f_ext_page_holds OP1F_NAMED("op1f_ext_page_holds");
extern "C" u32 op1f_ext_page_knob_moves OP1F_NAMED("op1f_ext_page_knob_moves");
extern "C" u32 op1f_ext_params_saves OP1F_NAMED("op1f_ext_params_saves");
extern "C" u32 op1f_ext_params_loads OP1F_NAMED("op1f_ext_params_loads");
extern "C" u32 op1f_ext_track_restores OP1F_NAMED("op1f_ext_track_restores");
extern "C" u32 op1f_help_ext_frames OP1F_NAMED("op1f_help_ext_frames");
extern "C" u32 op1f_signalflow_ext_reads OP1F_NAMED("op1f_signalflow_ext_reads");
extern "C" u32 op1f_signalflow_meter_frames OP1F_NAMED("op1f_signalflow_meter_frames");
extern "C" u32 op1f_io_reloads OP1F_NAMED("op1f_io_reloads");
extern "C" u32 op1f_io_writebacks OP1F_NAMED("op1f_io_writebacks");
extern "C" u32 op1f_io_read_chunks OP1F_NAMED("op1f_io_read_chunks");
extern "C" u32 op1f_io_read_cycles OP1F_NAMED("op1f_io_read_cycles");
extern "C" u32 op1f_io_reload_cycles OP1F_NAMED("op1f_io_reload_cycles");
extern "C" u32 op1f_tape_free_chunks OP1F_NAMED("op1f_tape_free_chunks");
extern "C" void op1f_sums_frame() OP1F_NAMED("op1f_sums_frame");
extern "C" void op1f_xmap_frame() OP1F_NAMED("op1f_xmap_frame");
extern "C" void op1f_tape_keeps_frame() OP1F_NAMED("op1f_tape_keeps_frame");   // R4b: the tape's values through level 7

// Asked weakly: the base's count of the pictures it served, and a function of the pack `sidechain`, which says that
// pack runs.
#ifndef OP1F_HOST
extern "C" u32 OP1F_WEAK op1f_svg_served OP1F_NAMED("op1f_svg_served");
extern "C" void OP1F_WEAK op1f_duck_screen(unsigned char* screen, const unsigned char* vector, const u32* ext_db) OP1F_NAMED("op1f_duck_screen");
#else
extern "C" u32 op1f_svg_served;
#endif

static const char LOG_FILE[] = "../src/main/op1f_cfw.cpp";     // the logger shows `op1f_cfw` and the tag, as before
static const u32 CORE_CLOCK_DEFAULT = 400000000u;             // the core's cycles a second, which the eMMC counters count in
enum { MODE_MIXER = 3, FRAME_LEVEL_1_BYTES = 28, WINDOW_MS = 1000, WINDOWS_A_PERIOD = 16 };

static struct {
    u32 started;
    u32 window_from;                  // the clock at the start of this one-second window
    u32 windows_left;                 // windows still to come in this period: a countdown, nothing divided
    u32 vu5, vu6, vu7, vu8;          // the last second's peaks of tracks 5 to 8, 16-bit like the firmware's meter words
    u32 io_chunks_at_log, io_cycles_at_log, io_reloads_at_log, io_reload_cycles_at_log;   // the eMMC counters at the last line printed
    QuietGate quiet;
} state;

// Restoring shift-subtract division, so no runtime routine is needed. d == 0 yields 0.
static u32 udiv(u32 n, u32 d) {
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static u32 svg_served() {
#ifdef OP1F_HOST
    return mock_svg_served_there ? op1f_svg_served : 0u;
#else
    return &op1f_svg_served != 0 ? op1f_svg_served : 0u;
#endif
}

static int sidechain_runs() {
#ifdef OP1F_HOST
    return mock_sidechain_there;
#else
    return &op1f_duck_screen != 0;
#endif
}

// What the lines state, as one word: every count they print and the selected track's strip, never a measure (the
// eMMC's cycles and the rates made from them, the meters' peaks, the play window's frame).
static QUIET_OUT_OF_LINE u32 signature() {
    u32 s = quiet_start();
    s = quiet_mix(s, op1f_ext_play_blocks); s = quiet_mix(s, op1f_ext_play_active); s = quiet_mix(s, op1f_ext_play_skipped);
    s = quiet_mix(s, op1f_ext_play_tracks);
    s = quiet_mix(s, op1f_io_read_chunks); s = quiet_mix(s, op1f_io_reloads); s = quiet_mix(s, op1f_io_writebacks);
    s = quiet_mix(s, op1f_tape_free_chunks);
    s = quiet_mix(s, op1f_ext_track_selects); s = quiet_mix(s, op1f_ext_keys_blocked); s = quiet_mix(s, op1f_ext_monitor_blocks);
    for (const char* c = op1f_record_track_label(); *c; ++c) s = quiet_mix(s, (u32)(unsigned char)*c);
    s = quiet_mix(s, op1f_ext_mix_changes);
    s = quiet_mix(s, op1f_ext_xfade_computes); s = quiet_mix(s, op1f_ext_xfade_fades); s = quiet_mix(s, op1f_ext_xfade_waits);
    s = quiet_mix(s, op1f_ext_loop_blocks); s = quiet_mix(s, op1f_ext_edge_fades);
    s = quiet_mix(s, op1f_ext_lifts); s = quiet_mix(s, op1f_ext_drops); s = quiet_mix(s, op1f_ext_clears); s = quiet_mix(s, op1f_ext_edit_refusals);
    s = quiet_mix(s, op1f_snapshots_taken); s = quiet_mix(s, op1f_snapshots_dropped); s = quiet_mix(s, op1f_ext_lift_alls);
    s = quiet_mix(s, op1f_ext_drop_alls); s = quiet_mix(s, op1f_ext_renders); s = quiet_mix(s, op1f_midi_ext_changes);
    s = quiet_mix(s, op1f_mtp_extra_lookups);
    s = quiet_mix(s, op1f_ext_page_holds); s = quiet_mix(s, op1f_ext_page_knob_moves); s = quiet_mix(s, op1f_ext_params_saves);
    s = quiet_mix(s, op1f_ext_params_loads); s = quiet_mix(s, op1f_ext_track_restores); s = quiet_mix(s, svg_served());
    s = quiet_mix(s, op1f_help_ext_frames); s = quiet_mix(s, op1f_signalflow_ext_reads); s = quiet_mix(s, op1f_signalflow_meter_frames);
    return s;
}

// The gate's reason as a word: its floor is 75 periods, twenty minutes at sixteen seconds a period.
static const char* why_word(int why) {
    return why == QUIET_WHY_FLOOR ? "twenty minutes" : quiet_why(why);
}

// The seven lines, in the base's words and order, and the gate's line when lines were held before these.
static QUIET_OUT_OF_LINE void say(int why) {
    if (op1f_ext_play_blocks != 0)
        STOCK_LOG(3, LOG_FILE, 4, "ext play: %u blocks, %u with signal, %u skipped as silent, %u track-blocks filtered, window from frame %d",
                  op1f_ext_play_blocks, op1f_ext_play_active, op1f_ext_play_skipped, op1f_ext_play_tracks, op1f_ext_play_w0);
    {                                                                // the eMMC rate since the last line, and the free tape
        u32 chunks = op1f_io_read_chunks - state.io_chunks_at_log;
        u32 cyc = op1f_io_read_cycles - state.io_cycles_at_log;
        u32 reloads = op1f_io_reloads - state.io_reloads_at_log;
        u32 reload_cyc = op1f_io_reload_cycles - state.io_reload_cycles_at_log;
        state.io_chunks_at_log = op1f_io_read_chunks;
        state.io_cycles_at_log = op1f_io_read_cycles;
        state.io_reloads_at_log = op1f_io_reloads;
        state.io_reload_cycles_at_log = op1f_io_reload_cycles;
        u32 read_ms = udiv(cyc, CORE_CLOCK_DEFAULT / 1000);
        u32 kib_per_s = read_ms ? udiv(chunks * 128u * 1000u, read_ms) : 0u;
        u32 us_per_chunk = chunks ? udiv(udiv(cyc, chunks), CORE_CLOCK_DEFAULT / 1000000) : 0u;
        u32 us_per_reload = reloads ? udiv(udiv(reload_cyc, reloads), CORE_CLOCK_DEFAULT / 1000000) : 0u;
        u32 free_seconds = udiv(op1f_tape_free_chunks * 16384u, 44100u);
        STOCK_LOG(3, LOG_FILE, 10, "ext io: %u chunks read in %u ms since the last line (%u KiB/s, %u us per chunk), %u slot reloads (%u us each), %u write-backs; tape free %u chunks = %u:%02u",
                  chunks, read_ms, kib_per_s, us_per_chunk, reloads, us_per_reload, op1f_io_writebacks,
                  op1f_tape_free_chunks, udiv(free_seconds, 60u), free_seconds - udiv(free_seconds, 60u) * 60u);
    }
    STOCK_LOG(3, LOG_FILE, 5, "ext rec: %u track 5-8 selections, %u clip keys blocked, %u monitor blocks diverted%s",
              op1f_ext_track_selects, op1f_ext_keys_blocked, op1f_ext_monitor_blocks, op1f_record_track_label());
    STOCK_LOG(3, LOG_FILE, 6, "ext mix: %u knob or mute changes on tracks 5-8, last second's peaks %u/%u/%u/%u",
              op1f_ext_mix_changes, state.vu5, state.vu6, state.vu7, state.vu8);
    STOCK_LOG(3, LOG_FILE, 7, "ext xfade: %u loop computes, %u junction fades, %u waits for a chunk, %u blocks from the loop buffers, %u clip edge fades",
              op1f_ext_xfade_computes, op1f_ext_xfade_fades, op1f_ext_xfade_waits, op1f_ext_loop_blocks, op1f_ext_edge_fades);
    STOCK_LOG(3, LOG_FILE, 8, "ext edit: %u lifts, %u drops, %u clears on tracks 5-8, %u refusals, %u snapshots taken, %u dropped, %u lift-alls, %u drop-alls, %u renders, %u midi, %u disk files",
              op1f_ext_lifts, op1f_ext_drops, op1f_ext_clears, op1f_ext_edit_refusals, op1f_snapshots_taken, op1f_snapshots_dropped,
              op1f_ext_lift_alls, op1f_ext_drop_alls, op1f_ext_renders, op1f_midi_ext_changes, op1f_mtp_extra_lookups);
    STOCK_LOG(3, LOG_FILE, 9, "ext mixer: %u page holds, %u knob moves on the page, %u saves, %u loads of tracks 5-8's strip, %u track restores, %u artworks from the module, %u help frames on 5/6, %u signal-flow reads, %u meter frames",
              op1f_ext_page_holds, op1f_ext_page_knob_moves, op1f_ext_params_saves, op1f_ext_params_loads,
              op1f_ext_track_restores, svg_served(), op1f_help_ext_frames, op1f_signalflow_ext_reads, op1f_signalflow_meter_frames);
    if (state.quiet.silent_before)
        STOCK_LOG(3, LOG_FILE, 14, "ext quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  state.quiet.silent_before, why_word(why), state.quiet.spoken, state.quiet.skipped);
}

// The meters' last second: read and cleared, as the base's window did.
static void read_meters() {
    state.vu5 = op1f_ext_vu[0];
    state.vu6 = op1f_ext_vu[1];
    state.vu7 = op1f_ext_vu[2];
    state.vu8 = op1f_ext_vu[3];
    op1f_ext_vu[0] = 0;
    op1f_ext_vu[1] = 0;
    op1f_ext_vu[2] = 0;
    op1f_ext_vu[3] = 0;
}

extern "C" void eight_tracks_frame(const Op1fFrame* now) OP1F_NAMED("eight_tracks_frame");
extern "C" void eight_tracks_frame(const Op1fFrame* now) {
    int framed = now != 0 && now->bytes >= (unsigned)FRAME_LEVEL_1_BYTES;    // a frame of level 1 at the least: its mode, clock and popup
    if (framed && op1f_mixer_page && now->mode != (unsigned)MODE_MIXER) op1f_mixer_page = 0;   // the key's release went elsewhere
    op1f_tape_keeps_frame();                                          // R4b: a tape told of, and the base's values of the tape
    op1f_sums_frame();                                                // sums learnt by reading on their way to the file, the sums' log lines
    op1f_xmap_frame();                                                // binds the mounted tape's extension file to its json
    if (!framed) return;
    if (!state.started) {
        state.started = 1;
        state.window_from = now->ms;
        state.windows_left = WINDOWS_A_PERIOD;
    }
    int why = quiet_popup(&state.quiet, now->popup != 0);             // the lines once more before the unit is off
    if (now->ms - state.window_from >= (u32)WINDOW_MS) {
        state.window_from = now->ms;
        read_meters();
        if (--state.windows_left == 0) {                              // sixteen seconds: a period of the gate
            state.windows_left = WINDOWS_A_PERIOD;
            why |= quiet_period(&state.quiet, signature(), 0);
        }
    }
    if (why) say(why);
}

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// The pack's choice among its pictures, asked before its directory is looked into: nothing of this pack's for the
// signal flow screen while the pack `sidechain` runs, whose picture of that screen carries its page; the name itself
// otherwise. It reads one word the linker wrote and takes no time.
extern "C" const char* op1f_art_pick_8_tracks(const char* name) OP1F_NAMED("op1f_art_pick_8_tracks");
extern "C" const char* op1f_art_pick_8_tracks(const char* name) {
    if (name != 0 && sidechain_runs() && same(name, "signalflow.svg")) return 0;
    return name;
}
