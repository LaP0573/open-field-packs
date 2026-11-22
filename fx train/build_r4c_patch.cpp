// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4c (base 27), the pack fx train (version 13): fx train 12's patch unit (which stays as it was) with
// one change and nothing else: the slots are asked of the chain in their own order, once each. Until fx train 12 the
// pack lfo's own file took a run of the synth's slots 0 to 7 asked in a row for the sign that the chain's file was being
// written, so this unit asked the synth's slots in descending order, and slot 6 once more where an ask of slot 7 would
// have ended a run of its own. From fx train 13 and lfo 11 on the chain's save says so by a word of its own
// (op1f_lfo_file_chain_saved, fx train 13's chain unit), and no order of asks means anything to lfo.
//
// What follows is the elder's description, which holds but for its paragraph on lfo's run, rewritten.
//
// Step R4b, the pack fx train (version 12): an instrument carries its train and its LFOs in its preset, through level 7
// of the base (module/build_r4b_card.h, OP1F_ASK_KEEPS). A sibling of Build I's patch unit (image I9), which stays
// as it was for the packs before: what goes into a preset and what comes out of one are the same, and the chain unit
// (fx train 11's chain unit) asks this unit the same two things it asked that one (fx train/build_i_patch.h).
//
// What changed. Until base 26 this pack held six places of the firmware (the group build-i9: the three preset reads,
// the sound put on a slot, the two preset saves) and did there itself what the base now does for every pack: base 26
// holds those places, and this pack keeps its member through its card instead. Its ask of kind 6 lists one member,
// `op1f`, with the synth's sounds and the drum's: the slot's integers, an array of whole numbers, exactly as
// user/op1f.json holds them for a slot (the chain, the LFOs' effect wagons, the LFO unit's added LFOs). So a preset
// this pack writes holds the same bytes as one I9's unit wrote for the same slot, and every preset I9's unit wrote is
// read here as there.
//
//   A sound loaded onto a slot from a preset: the base takes the member from the file as read (before the firmware
//   keeps only the members it knows) and tells this pack at its next frame (`changed`, OP1F_KEPT_LOADED). The frame
//   reads the value (op1f_value_get), publishes "a new instrument, with these integers" or "a new instrument, with
//   none" for the chain exactly as I9's assign did (op1f_patch_pending), and has the chain reconcile the slot at once
//   (op1f_chain_instrument), which I9 did only for an instrument the firmware had not applied after half a second: the
//   base tells at the frame, and the firmware's own apply may come before it or after it, so the chain takes the
//   instrument in this frame either way and the apply that comes after finds nothing new.
//   A change of the slot's engine: left alone, as I9 left it (the base keeps the slot's value).
//   A preset saved: the base writes the value it holds for the slot. So this pack sets that value whenever the
//   integers I9's save asked of the chain would differ from it: once a frame, for the synth's and the drum's slot on
//   the screen (the firmware's words of the GUI's slots: a preset saved is the sound on the screen, and the chain
//   changes only the sound on the screen), and for every slot the chain has just reconciled or the base has told of,
//   what op1f_chain_slot_values answers is held against what the base holds. The integers when there are any; with
//   none (a slot as the firmware alone has it), the value "null" when the slot's own document holds the member (a
//   preset of the drum or of the synth sampler read with it: I9 wrote a null over it there, "nothing stale to clear"),
//   and no value at all otherwise (I9 wrote nothing). The slot's document is read inside op1f_mask, as the base reads
//   a slot's record, with one call of the firmware's JSON library that allocates nothing.
//   The chain's answer for a synth slot also hands that slot's LFOs to the pack lfo's own file (its copy of the
//   chain's file), and nothing more: the chain's save tells lfo by a word of its own when the chain's file is written
//   (R4c), so the slots are asked here in their own order, once each. I9's save asked the chain the same, once a save.
//
// The threads. `changed` comes on the display thread outside the pack's frame function: it notes a bit and does
// nothing else. Everything else runs in op1f_patch_frame, which the chain unit calls from its frame work, inside the
// pack's frame function; op1f_patch_pending is asked by the chain unit on the GUI thread, as before, and reads what
// the frame published under the same sequence count I9 used (odd while the writer is in).
//
// Compiled for the PC with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for tests this
// tree does not hold). Nothing is divided (the module links no runtime).

#include "build_r4b_card.h"
#ifdef OP1F_HOST
#include "mock_features_r4b.h"                                   // the PC's stand-ins: the JSON library's "has", the slots' records
#endif
#include "build_i_patch.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef int (*JsonHasFn)(const void* json, const char* key);

#ifndef OP1F_HOST
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)                  // whether an object (or null) has a member of that name
#define RECORDS_OF(host) ((host) ? 0x0802377cu : 0x080235dcu)    // the slots' records, eight of 52 bytes, the document at +8
#define GUI_SLOT(host) (*(volatile u32*)((host) ? 0x11904708u : 0x11904704u))   // the GUI's current sound slot, by host
#endif

// The pack's own card (tools/pack_build.py writes it from the recipe).
extern "C" const Op1fCard op1f_card_fx_train OP1F_NAMED("op1f_card_fx_train");

static const char LOG_FILE[] = "../src/main/op1f_patch.cpp";
static const char KEY[] = "op1f";

enum { HOSTS = 2, SLOTS = 8, INTS = OP1F_PATCH_INTS, RECORD_BYTES = 52, RECORD_DOCUMENT = 8, JSON_KIND_AT = 4, JSON_OBJECT_KIND = 7 };
enum { TEXT_ROOM = OP1F_VALUE_MOST + 1 };
enum { EVENT_LINES = 60, COUNT_LINES = 40, REFUSED_LINES = 8, PRINT_FRAMES = 232 };
// What the base holds for a slot, as this unit last read or set it.
enum { HELD_UNREAD = 0, HELD_NONE = 1, HELD_NULL = 2, HELD_INTS = 3, HELD_OTHER = 4 };

struct Held { u32 how; int count; int values[INTS]; };
static Held held[HOSTS][SLOTS];
static Held want;                                                // the display thread's: what a slot should hold now
static int chain_values[INTS];

// A new instrument on a slot, for the GUI thread. seq is odd while the writer is in and even once it is whole; the
// reader takes a copy only when seq stood still and even around it (I9's form).
struct Pending { volatile u32 seq; int count; int values[INTS]; };
static Pending pending[HOSTS][SLOTS];
static u32 taken_seq[HOSTS][SLOTS];                              // GUI thread: the seq last taken
static volatile u8 seen[HOSTS][SLOTS];                           // the chain has reconciled the slot since the start: it asked here
static volatile u8 check[HOSTS][SLOTS];                          // the slot is to be held to the chain's integers at the next frame

static volatile u32 told[HOSTS][SLOTS];                          // 1 << OP1F_KEPT_..., from `changed`, taken by the frame

static struct {
    u32 loads[HOSTS], with_own, without, engine_changes, starts;
    u32 taken, torn, saves, saves_with, sets, refused;
    u32 printed, event_lines, count_lines, refused_lines, countdown;
} pt;

static char text[TEXT_ROOM];

// ---------------------------------------------------------------- the value's text

static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };

// A whole number written in decimal at `at`, by subtracting powers of ten: nothing divided.
static u32 put_number(char* out, u32 at, int v) {
    u32 m = v < 0 ? 0u - (u32)v : (u32)v;
    if (v < 0) out[at++] = '-';
    int started = 0;
    for (int k = 0; k < 10; ++k) {
        u32 d = 0;
        while (m >= TENS[k]) { m -= TENS[k]; ++d; }
        if (d || started || k == 9) {
            out[at++] = (char)('0' + d);
            started = 1;
        }
    }
    return at;
}

// The integers as the base takes them: [a,b,...]. 160 numbers of eleven characters at the most fit the room.
static void text_of(const int* values, int count) {
    u32 at = 0;
    text[at++] = '[';
    for (int i = 0; i < count; ++i) {
        if (i) text[at++] = ',';
        at = put_number(text, at, values[i]);
    }
    text[at++] = ']';
    text[at] = 0;
}

static const char* skip(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
    return p;
}

// A whole number at p, of 32 bits; 0 when there is none there (with *ok 0).
static const char* number_at(const char* p, int* out, int* ok) {
    int negative = 0;
    if (*p == '-') { negative = 1; ++p; }
    if (*p < '0' || *p > '9') { *ok = 0; return p; }
    u32 m = 0;
    while (*p >= '0' && *p <= '9') {
        u32 d = (u32)(*p - '0');
        if (m > 214748364u || (m == 214748364u && d > (negative ? 8u : 7u))) { *ok = 0; return p; }
        m = m * 10u + d;
        ++p;
    }
    if (*p == '.' || *p == 'e' || *p == 'E') { *ok = 0; return p; }
    *out = negative ? (int)(0u - m) : (int)m;
    return p;
}

// What a value's text says, as I9 read the member from a file: an array of 1 to INTS whole numbers is the slot's
// integers; "null" is the member cleared; anything else (an empty array, another kind, a number of too many) is not
// this pack's and counts as none.
static void read_text(const char* t, Held* h) {
    h->count = 0;
    const char* p = skip(t);
    if (p[0] == 'n' && p[1] == 'u' && p[2] == 'l' && p[3] == 'l' && *skip(p + 4) == 0) { h->how = HELD_NULL; return; }
    h->how = HELD_OTHER;
    if (*p != '[') return;
    p = skip(p + 1);
    if (*p == ']') return;
    for (;;) {
        int ok = 1, v = 0;
        p = number_at(p, &v, &ok);
        if (!ok || h->count >= INTS) { h->count = 0; return; }
        h->values[h->count++] = v;
        p = skip(p);
        if (*p == ',') { p = skip(p + 1); continue; }
        if (*p == ']' && *skip(p + 1) == 0) break;
        h->count = 0;
        return;
    }
    h->how = HELD_INTS;
}

static u32 where_of(int host) {
    return host ? (u32)OP1F_KEEP_DRUM : (u32)OP1F_KEEP_SYNTH;
}

// What the base holds for the slot now, read into held[host][slot].
static void read_held(int host, int slot) {
    Held* h = &held[host][slot];
    int got = op1f_value_get(&op1f_card_fx_train, where_of(host), (u32)slot, KEY, text, sizeof text);
    if (got == OP1F_NO_FILE) { h->how = HELD_NONE; h->count = 0; return; }
    if (got < 0) { h->how = HELD_OTHER; h->count = 0; return; }    // too long for the room: not this pack's
    read_text(text, h);
}

// ---------------------------------------------------------------- the chain's side (the GUI thread)

static void publish(int host, int slot, const Held* h) {
    Pending* p = &pending[host][slot];
    int count = h->how == HELD_INTS ? h->count : 0;
    p->seq = p->seq + 1u;                                        // odd: the writer is in
    p->count = count;
    for (int i = 0; i < count; ++i) p->values[i] = h->values[i];
    p->seq = p->seq + 1u;                                        // even: whole
}

extern "C" int op1f_patch_pending(int host, int slot, int* out) {
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS) return -1;
    seen[host][slot] = 1;                                        // the chain reconciles the slot now: its integers count from here
    check[host][slot] = 1;
    Pending* p = &pending[host][slot];
    u32 seq = p->seq;
    if (seq == taken_seq[host][slot]) return -1;
    if (seq & 1u) { ++pt.torn; return -1; }
    int count = p->count;
    if (count < 0 || count > INTS) count = 0;
    for (int i = 0; i < count; ++i) out[i] = p->values[i];
    if (p->seq != seq) { ++pt.torn; return -1; }
    taken_seq[host][slot] = seq;
    ++pt.taken;
    return count;
}

// ---------------------------------------------------------------- the base's word (the display thread, outside the frame function)

extern "C" void op1f_patch_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_patch_changed");
extern "C" void op1f_patch_changed(unsigned where, unsigned which, unsigned how) {
    if (which >= (unsigned)SLOTS || how > 31u) return;
    if (where == (unsigned)OP1F_KEEP_SYNTH) told[0][which] |= 1u << how;
    else if (where == (unsigned)OP1F_KEEP_DRUM) told[1][which] |= 1u << how;
}

// ---------------------------------------------------------------- once a frame (the pack's frame function)

// Whether the slot's own document holds the member: a preset of the drum or the synth sampler read with it. Inside
// one section of op1f_mask, as the base reads a slot's record; the library's "has" allocates nothing.
static int record_holds_key(int host, int slot) {
    const void* document = (const void*)(RECORDS_OF(host) + (u32)RECORD_BYTES * (u32)slot + (u32)RECORD_DOCUMENT);
    unsigned was = op1f_mask();
    int kind = ((const volatile u8*)document)[JSON_KIND_AT];
    int has = (kind == JSON_OBJECT_KIND || kind == 0) && STOCK_JSON_HAS(document, KEY);
    op1f_unmask(was);
    return has;
}

static int same_held(const Held* a, const Held* b) {
    if (a->how != b->how) return 0;
    if (a->how != (u32)HELD_INTS) return 1;
    if (a->count != b->count) return 0;
    for (int i = 0; i < a->count; ++i) if (a->values[i] != b->values[i]) return 0;
    return 1;
}

// The slot's value made what the chain says, when it is not already.
static void keep_slot(int host, int slot) {
    Held* h = &held[host][slot];
    if (h->how == (u32)HELD_UNREAD) read_held(host, slot);
    int n = op1f_chain_slot_values(host, slot, chain_values);
    if (n < 0 || n > INTS) n = 0;
    want.count = n;
    if (n) {
        want.how = HELD_INTS;
        for (int i = 0; i < n; ++i) want.values[i] = chain_values[i];
    } else {
        want.how = record_holds_key(host, slot) ? (u32)HELD_NULL : (u32)HELD_NONE;   // as stock alone has it: clear what the document carries, add nothing
    }
    if (same_held(h, &want)) return;
    const char* value = 0;
    if (want.how == (u32)HELD_INTS) { text_of(want.values, want.count); value = text; }
    else if (want.how == (u32)HELD_NULL) value = "null";
    int answer = op1f_value_set(&op1f_card_fx_train, where_of(host), (u32)slot, KEY, value);
    if (answer == 0) {
        h->how = want.how;
        h->count = want.count;
        for (int i = 0; i < want.count; ++i) h->values[i] = want.values[i];
        ++pt.sets;
        return;
    }
    ++pt.refused;
    h->how = HELD_UNREAD;                                        // read again at the next frame, and tried again
    if (pt.refused_lines < (u32)REFUSED_LINES) {
        ++pt.refused_lines;
        op1f_log(2, LOG_FILE, 6, "patch: %s s%d: the base did not take its %d integers (answer %d)", host ? "drum" : "synth", slot, n, answer);
    }
}

extern "C" void op1f_patch_frame() {
    for (int h = 0; h < HOSTS; ++h) {
        for (int s = 0; s < SLOTS; ++s) {
            u32 bits = told[h][s];
            if (!bits) continue;
            told[h][s] = 0;
            read_held(h, s);                                     // what the base holds now: a load, a start, a value let go
            check[h][s] = 1;
            if (bits & (1u << OP1F_KEPT_LOADED)) {               // a new instrument on the slot: its integers, or none
                publish(h, s, &held[h][s]);
                ++pt.loads[h];
                if (held[h][s].how == (u32)HELD_INTS) ++pt.with_own; else ++pt.without;
                op1f_chain_instrument(h, s);                     // the chain takes it now, whether the firmware's apply came or not
            }
            if (bits & (1u << OP1F_KEPT_ENGINE)) ++pt.engine_changes;
            if (bits & ((1u << OP1F_KEPT_START) | (1u << OP1F_KEPT_DROPPED))) ++pt.starts;
            if (bits & (1u << OP1F_KEPT_SAVED)) {
                ++pt.saves;
                int n = held[h][s].how == (u32)HELD_INTS ? held[h][s].count : 0;
                if (n) ++pt.saves_with;
                if (pt.event_lines < (u32)EVENT_LINES) {
                    ++pt.event_lines;
                    op1f_log(3, LOG_FILE, 3, "patch: %s s%d saved: %d integers of its train and LFOs added to the file", h ? "drum" : "synth", s, n);
                }
            }
        }
    }
    for (int h = 0; h < HOSTS; ++h) {
        int on_screen = (int)(GUI_SLOT(h) & (u32)(SLOTS - 1));
        for (int s = 0; s < SLOTS; ++s) {                        // R4c: in their own order: lfo's file counts no run
            if (!seen[h][s] || (s != on_screen && !check[h][s])) continue;
            check[h][s] = 0;
            keep_slot(h, s);
        }
    }
    if (++pt.countdown < (u32)PRINT_FRAMES) return;
    pt.countdown = 0;
    u32 sum = pt.loads[0] + pt.loads[1] + pt.engine_changes + pt.starts + pt.taken + pt.torn + pt.saves + pt.sets + pt.refused;
    if (sum != pt.printed && pt.count_lines < (u32)COUNT_LINES) {
        pt.printed = sum;
        ++pt.count_lines;
        op1f_log(3, LOG_FILE, 4, "patch: loads %u synth, %u drum (%u with a train of their own, %u without); %u taken, %u torn",
                 pt.loads[0], pt.loads[1], pt.with_own, pt.without, pt.taken, pt.torn);
        op1f_log(3, LOG_FILE, 5, "patch: %u engine changes left alone, %u slots told at the start; %u saves (%u with their train); %u values set, %u refused",
                 pt.engine_changes, pt.starts, pt.saves, pt.saves_with, pt.sets, pt.refused);
    }
}
