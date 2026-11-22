// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build H, images H0, H1a and H1: the effect stage measured.
//
// H0 (section 7): no hook of its own. The measurement unit's frame hook (the first frame hook's measuring unit) calls op1f_fx_frame() once
// per frame through a weak reference; this unit reads stock's own CYCLES stage profiler and the effect hosts' state
// and logs them every four seconds. The one write is the clearing of each record's running maximum after it is read.
//
// H1a (section 8) timed stock's effect call through the four stubs of build_h_fx.s and filled the cost table; it also
// corrected H0's two findings (the record's maximum is at +16, every line under 200 characters). Since H1 (section 9)
// the stubs belong to the chain: the start stubs stamp CYCLES into op1f_fx_t0[host] and hand stock a null producer,
// the end stubs call op1f_fx_chain_run (Build H's chain unit); this unit keeps the fx: line and the stage lines.
//
// What stock does, as read for H0 (the study, section 6):
//   * The audio block function 0x11a05620 (L1) stamps CYCLES around its stages into a table of twelve records at
//     0x0801b240 (L2). A record is 44 bytes: start lo/hi, delta lo/hi, max lo/hi, min lo/hi, sum lo/hi, count; the
//     function writes the end stamp over the delta words, subtracts, keeps max and min, adds to the sum and counts.
//     Record 0 is the whole block (the end stamp at +8); 2 the stage on the main buses (0x8f0041e0); 3 the instrument
//     stage (the synth second stage 0x11a04130 or the drum second stage 0x11a03670); 6 the record stage (0x11a07a74);
//     7 the tape block driver (0x11a07fe4); 9 and 10 stages of the tape mix (the L1 code around 0x11a02ec0). The synth
//     second stage keeps four more records at 0x0801b450: 0 the voice loop, 2 the engine render (the engine object's
//     method +8), 3 the stage after it. The effect itself has no record of its own: it runs inside record 3, after the
//     engine render and before the fade and the sum to the main buses, and is skipped while the instrument is silent.
//   * The instrument's effect is one producer object per host, built by the producer factory 0x8f011068(type, index)
//     into a fixed L1 storage (index 0, the synth's and the drum's: 0x11906918; index 1, the master's: 0x11906af8),
//     its pointer in the slot 0x84a03c84 (the master's 0x84a03c88). The second stages call producer->vtable[2](this,
//     L bus 0x11902ba0, R bus 0x11902ca0, ctx 0x11904754 = the slot's eight parameters) on the buses in place, only
//     while the slot's fx_active byte is set (synth B[0x84a0154c + slot], drum B[0x84a0155c + slot]); the master's
//     producer runs on the stereo sums 0x119027a0 / 0x119028a0 while B[0x84a0156c] is set.
//   * The type word per slot: 0x11904794[8] (synth), 0x119047b4[8] (drum), in the firmware's own enumeration 0 delay,
//     1 grid, 2 punch, 3 phone, 4 spring, 5 mother, 6 nitro, 7 cwo, 8 vocoder, 9 terminal, 10 fazer (the name table at
//     0x8490e288, eight bytes per entry: the name's pointer, the value). The GUI's slot words 0x11904704 (synth) and
//     0x11904708 (drum); the audio thread's built slots 0x84a01524 / 0x84a01528, the ones the second stages read.
//   * The fade around a rebuild: the level 0x11902df4 (Q31, applied and stepped per sample by the sum to the main
//     buses 0x8d503c54), the step 0x11902df8, the step's magnitude 0x11902e00. A slot or mode change fades the level
//     to zero, rebuilds the engine, the producer and the LFOs once (0x8f0112e8, 0x8d505390, 0x8d50426c), fades up.
//
// The log, every four seconds (about 232 frames): `fx:` the hosts' state, `fx stage a:` the twelve records of the
// first table as average/maximum cycles per execution over the interval, `fx stage b:` the synth stage's four. (H1a's
// `fx cost` lines are gone with the accounting; the chain unit logs its own `chain` lines.)
//
// Q1: the three lines are printed through the gate of build_q_quiet.h. The records
// are read and their maxima cleared at every period as before; the lines go to the log when the state the `fx:`
// line reports changed, while synth voices sound, for two periods after either, at the power popup, and otherwise
// once in five minutes. "Voices sound" is stock's own profiler: the synth stage's four records (table B) count an
// execution only in a block in which a voice was rendered (the device's logs: `fx stage b: 0:- 1:- 2:- 3:-` in every
// period without a note, numbers in every period with one). That count is handed to the chain unit as
// op1f_fx_voices, so that both units speak while the synth is played. A fourth line, `fx quiet:`, follows the three
// when periods went unprinted before them and says how many and why these were printed.

typedef unsigned int u32;
typedef unsigned char u8;

#include "build_q_quiet.h"

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
#define STOCK_LOG ((LogFn)0x8c053a20u)
static const char LOG_FILE[] = "../src/main/op1f_fx.cpp";

#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))

enum { TABLE_A = 0x0801b240u, TABLE_A_RECORDS = 12, TABLE_B = 0x0801b450u, TABLE_B_RECORDS = 4, RECORD_BYTES = 44 };
enum { FX_TYPES = 11, HOSTS = 2, PRINT_EVERY_FRAMES = 232 };

static const u32 FX_NAME_TABLE = 0x8490e288u;          // {const char* name; u32 value} x 11, the value its index
static const u32 SYNTH_SLOT = 0x11904704u, DRUM_SLOT = 0x11904708u;
static const u32 SYNTH_FX_TYPE = 0x11904794u, DRUM_FX_TYPE = 0x119047b4u;
static const u32 SYNTH_FX_ACTIVE = 0x84a0154cu, DRUM_FX_ACTIVE = 0x84a0155cu, MASTER_FX_ACTIVE = 0x84a0156cu;
static const u32 FX_PRODUCER = 0x84a03c84u, MASTER_PRODUCER = 0x84a03c88u;
static const u32 FADE_LEVEL = 0x11902df4u, FADE_STEP = 0x11902df8u, FADE_MAGNITUDE = 0x11902e00u;
static const u32 MODE_GUI = 0x84d21088u, MODE_AUDIO = 0x11902dfcu - 0x58u, BUILT_SYNTH_SLOT = 0x84a01524u, BUILT_DRUM_SLOT = 0x84a01528u;
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;      // Q1: the power popup's byte, 1 while the countdown popup is on the screen (the chain unit reads it too)

struct Seen { u32 sum_lo, sum_hi, count; };

// H1a: the start stamps the stubs write (0 synth, 1 drum); since H1 the end stubs call the chain unit (Build H's chain unit).
extern "C" u32 op1f_fx_t0[HOSTS] asm("op1f_fx_t0");
u32 op1f_fx_t0[HOSTS];

static struct {
    u32 frames;
    int countdown;                    // frames until the next print; 0 at the start so the first frame prints
    u32 prints;
    u32 last_producer;
    u32 producer_changes;
    Seen a[TABLE_A_RECORDS];
    Seen b[TABLE_B_RECORDS];
    char line[304];                   // Q1: the first table's line at its widest is 301 characters (twelve records of two ten-digit numbers); it had 200 until now, 167 the longest ever written
    char line_b[128];                 // Q1: both tables' lines are made before it is known whether they are printed; the second's widest is 107
    QuietGate quiet;                  // Q1: the gate of the three lines
} state;

extern "C" u32 op1f_fx_prints asm("op1f_fx_prints");
u32 op1f_fx_prints;

// Q1: the executions the synth stage's four records counted in the last period: 0 while no synth voice sounds.
extern "C" u32 op1f_fx_voices asm("op1f_fx_voices");
u32 op1f_fx_voices;

// Restoring shift-subtract division (no libgcc routine). d == 0 yields 0.
static u32 udiv(u32 n, u32 d) {
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

static char* put_u32(char* p, u32 v) {
    char digits[10];
    int n = 0;
    do {
        u32 q = udiv(v, 10);
        digits[n++] = (char)('0' + (v - q * 10));
        v = q;
    } while (v);
    while (n) *p++ = digits[--n];
    return p;
}

static const char* fx_name(u32 type) {
    if (type >= FX_TYPES) return "?";
    return *(const char* const*)(FX_NAME_TABLE + 8u * type);
}

// One record of stock's profiler: the average over the interval from the sum and count deltas (64-bit sum, the
// interval's part fits 32 bits), the interval's maximum at +16 (then cleared, the one write of H0's part).
static char* put_record(char* p, u32 record, Seen* seen, u32* executions) {
    u32 sum_lo = RD32(record + 32), sum_hi = RD32(record + 36), count = RD32(record + 40);
    u32 max_lo = RD32(record + 16), max_hi = RD32(record + 20);
    u32 dcount = count - seen->count;
    *executions += dcount;
    u32 dsum_lo = sum_lo - seen->sum_lo;
    u32 dsum_hi = sum_hi - seen->sum_hi - (sum_lo < seen->sum_lo ? 1u : 0u);
    seen->sum_lo = sum_lo;
    seen->sum_hi = sum_hi;
    seen->count = count;
    *(volatile u32*)(record + 16) = 0;
    *(volatile u32*)(record + 20) = 0;
    if (dcount == 0) return put_text(p, "-");
    if (dsum_hi != 0) return put_text(p, "big");
    p = put_u32(p, udiv(dsum_lo, dcount));
    *p++ = '/';
    p = put_u32(p, max_hi ? 0xffffffffu : max_lo);
    return p;
}

// One table's line made in `text` (Q1: made at every period, printed when the gate says so); the answer is the
// executions its records counted over the interval.
static u32 make_table(char* text, const char* label, u32 table, Seen* seen, int records) {
    u32 executions = 0;
    char* p = put_text(text, label);
    for (int k = 0; k < records; ++k) {
        *p++ = ' ';
        p = put_u32(p, (u32)k);
        *p++ = ':';
        p = put_record(p, table + (u32)k * RECORD_BYTES, &seen[k], &executions);
    }
    *p = 0;
    return executions;
}

extern "C" void op1f_fx_frame() asm("op1f_fx_frame");
extern "C" void op1f_fx_frame() {
    u32 producer = RD32(FX_PRODUCER);
    if (state.frames && producer != state.last_producer) ++state.producer_changes;
    state.last_producer = producer;
    ++state.frames;
    int why = quiet_popup(&state.quiet, RD8(POWER_POPUP_SHOWN) != 0);      // Q1: the lines once more before the unit is off
    int period = --state.countdown <= 0;                     // a countdown, not a modulo: no division routine
    if (!period && !why) return;
    if (period) state.countdown = PRINT_EVERY_FRAMES;
    u32 synth_slot = RD32(SYNTH_SLOT) & 7u, drum_slot = RD32(DRUM_SLOT) & 7u;
    u32 synth_type = RD32(SYNTH_FX_TYPE + 4u * synth_slot), drum_type = RD32(DRUM_FX_TYPE + 4u * drum_slot);
    u32 vtable = producer ? RD32(producer) : 0u;
    u32 synth_on = RD8(SYNTH_FX_ACTIVE + synth_slot) ? 1u : 0u, drum_on = RD8(DRUM_FX_ACTIVE + drum_slot) ? 1u : 0u, master_on = RD8(MASTER_FX_ACTIVE) ? 1u : 0u;
    u32 master = RD32(MASTER_PRODUCER), changes = state.producer_changes;
    u32 fade_level = RD32(FADE_LEVEL), fade_step = RD32(FADE_STEP), fade_magnitude = RD32(FADE_MAGNITUDE);
    u32 mode_gui = RD32(MODE_GUI), mode_audio = RD32(MODE_AUDIO), built_synth = RD32(BUILT_SYNTH_SLOT), built_drum = RD32(BUILT_DRUM_SLOT);
    state.producer_changes = 0;
    make_table(state.line, "fx stage a:", TABLE_A, state.a, TABLE_A_RECORDS);       // the records read and their maxima cleared at every period, printed or not
    u32 voices = make_table(state.line_b, "fx stage b:", TABLE_B, state.b, TABLE_B_RECORDS);
    if (period) {
        op1f_fx_voices = voices;
        u32 s = quiet_start();                               // what the `fx:` line states; the stage lines are measures
        s = quiet_mix(s, synth_slot); s = quiet_mix(s, synth_type); s = quiet_mix(s, synth_on);
        s = quiet_mix(s, drum_slot); s = quiet_mix(s, drum_type); s = quiet_mix(s, drum_on);
        s = quiet_mix(s, master_on); s = quiet_mix(s, producer); s = quiet_mix(s, vtable); s = quiet_mix(s, changes); s = quiet_mix(s, master);
        s = quiet_mix(s, fade_level); s = quiet_mix(s, fade_step); s = quiet_mix(s, fade_magnitude);
        s = quiet_mix(s, mode_gui); s = quiet_mix(s, mode_audio); s = quiet_mix(s, built_synth); s = quiet_mix(s, built_drum);
        why |= quiet_period(&state.quiet, s, voices != 0);
    }
    if (!why) return;
    STOCK_LOG(3, LOG_FILE, 1, "fx: synth s%u t%u %s %s, drum s%u t%u %s %s, master %s; producer %08x vt %08x %u chg, master %08x; fade %d/%d/%d; mode %u/%u slots %u/%u",
              synth_slot, synth_type, fx_name(synth_type), synth_on ? "on" : "off",
              drum_slot, drum_type, fx_name(drum_type), drum_on ? "on" : "off",
              master_on ? "on" : "off", producer, vtable, changes, master,
              (int)fade_level, (int)fade_step, (int)fade_magnitude,
              mode_gui, mode_audio, built_synth, built_drum);
    STOCK_LOG(3, LOG_FILE, 2, "%s", state.line);
    STOCK_LOG(3, LOG_FILE, 3, "%s", state.line_b);
    if (state.quiet.silent_before)
        STOCK_LOG(3, LOG_FILE, 4, "fx quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  state.quiet.silent_before, quiet_why(why), state.quiet.spoken, state.quiet.skipped);
    op1f_fx_prints = ++state.prints;
}
