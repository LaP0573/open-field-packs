// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R15, the pack `lfo` 12: its input function, its layer's `shown`, the page choice and the rides' start.
// The four LFOs are the LFO layer "ufo", ask 0 of the card
// (module/build_r6e_card.h, "Base 35: layers"); the store, the rides' bits and the grammar of the page are the LFO
// unit's (lfo/build_i_lfo4.cpp, the op1f_lfo4_ names below).
//
//   lfo_input, on the key thread (every key and knob is offered to it before the synth's or the drum's controller):
//     outside the synth and the drum mode, or where the place on the screen does not ride "ufo": 0 for every event;
//     shift + T4: 0, the unit's list of LFO kinds; the presets' list, when it shows, closed first;
//     while lfo's page is up (`mine`): T4, the knobs' clicks and presses and every turn taken (a turn in steps of
//       1024, rounded away from 0) and handed, on the UFO page, to op1f_lfo4_key and op1f_lfo4_turn for the place on
//       the screen; while the presets' list or save screen lies over it, to the presets unit itself (the base tells
//       the UFO page's `shown` 0 while a page covers it, and the LFO unit's page keys wait for it); T1 to T3 handed on
//       and let go (the list closes, and its page at once);
//     on the stock LFO page of a riding place after the page choice went to the unit's (a tap): T4 down taken, and
//       its up, and the UFO page wanted again; where the screen does not say so, the LFO unit's own sign is asked
//       (op1f_lfo4_key);
//     the up of every other down taken, in any mode, goes to op1f_lfo4_key with the place of its down;
//     the keys 13 to 53 (they may come on the audio thread): 0 at once.
//   lfo_ufo_shown, the layer's `shown` (the display thread): the place whose UFO page is up, for the page's dressing,
//     passed on to op1f_lfo4_shown.
//   op1f_lfo12_see, first in lfo_frame: whether lfo's page is up, for the presets' frame work.
//   op1f_lfo12_rides, in lfo_frame after the LFOs' frame work: one ride start a frame (op1f_lfo4_ride_due), and the
//     page choice applied (op1f_layer_page): the UFO page at the first frame and whenever a ride begins; the unit's
//     page when the LFO unit asks it (op1f_lfo4_take_page); the UFO page again once neither LFO page has been on the
//     screen for three frames, so T4 from T1 to T3 always shows the UFOs.
// The screens: 77 the base's page (the UFO page, the presets over it), 31 to 36 the stock LFO pages, and the screen
// the unit's page came on after it was last chosen, learnt in case
// 1.7.3 numbers them otherwise; 45 the list of LFO kinds (the base's number). Nothing is divided, nothing is logged
// from the key thread or the audio thread.

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned char u8;

enum {
    FRAME_LEVEL_4 = 40, INPUT_LEVEL_2 = 36, INPUT_LEVEL_4 = 40,
    ASK_UFO = 0, PLACES = 16, MODE_SYNTH = 0, MODE_DRUM = 1,
    KEY_T1 = 6, KEY_T3 = 8, KEY_T4 = 9, KEY_PLAY_FIRST = 13, KEY_PLAY_LAST = 53, KEY_CLICK0 = 54, KEY_PRESS0 = 61, KNOBS = 4, KEYS = 72,
    PAGE_SCREEN = 77, LFO_SCREEN_FIRST = 31, LFO_SCREEN_LAST = 36, LFO_LIST_SCREEN = 45,
    OFF_FRAMES = 3, SOON_FRAMES = 8, RIDE_LINES = 64, PAGE_LINES = 64,
};

static const char LOG_FILE[] = "op1f/lfo_ufo.cpp";              // the log shows "lfo_ufo"

extern "C" const Op1fCard op1f_card_lfo OP1F_NAMED("op1f_card_lfo");

// The LFO unit's (build_i_lfo4.cpp, design section 9).
extern "C" volatile unsigned op1f_lfo4_rides OP1F_NAMED("op1f_lfo4_rides");
extern "C" int op1f_lfo4_place() OP1F_NAMED("op1f_lfo4_place");
extern "C" int op1f_lfo4_key(int place, int key, int what) OP1F_NAMED("op1f_lfo4_key");
extern "C" int op1f_lfo4_turn(int place, int knob, int delta, int shifted) OP1F_NAMED("op1f_lfo4_turn");
extern "C" int op1f_lfo4_take_page() OP1F_NAMED("op1f_lfo4_take_page");
extern "C" void op1f_lfo4_shown(int place, int up) OP1F_NAMED("op1f_lfo4_shown");
extern "C" int op1f_lfo4_ride_due() OP1F_NAMED("op1f_lfo4_ride_due");
extern "C" void op1f_lfo4_ride_answer(int place, int answer) OP1F_NAMED("op1f_lfo4_ride_answer");
// The presets' (build_r2_lfopresets.cpp): their keys and knobs while they show, and the screen they want.
extern "C" int op1f_lfopresets_key_down(int key, int shifted) OP1F_NAMED("op1f_lfopresets_key_down");
extern "C" int op1f_lfopresets_turn(int knob, int delta) OP1F_NAMED("op1f_lfopresets_turn");
extern "C" int op1f_lfo12_presets_screen() OP1F_NAMED("op1f_lfo12_presets_screen");

// The key thread's words, written there only.
static volatile u8 taken_at[KEYS];                  // the place + 1 of each key whose down was taken, 0 for none
static volatile u32 back_t4;                        // T4's down taken on the stock LFO page: its up is taken too
static volatile u32 soon;                           // T4 went down for a riding place: its UFO page may come up
// Both threads': the page choice wanted (OP1F_LAYER_PAGE_OWN or _UNIT), each write whole.
static volatile u32 page_want = OP1F_LAYER_PAGE_OWN;
// The display thread's words.
static volatile int page_now = -1;                  // the choice the base took last, -1 before the first
static volatile u32 shown_where, shown_up;          // the layer's `shown`: the place whose UFO page is up
static volatile u32 page_up;                        // lfo's page is up for a riding place (op1f_lfo12_see)
static volatile u32 unit_screen;                    // the screen the unit's LFO page came on after the last choice of it, 0 not seen
static u32 rides_was, off_frames, soon_frames, unit_watch, ride_lines, page_lines, page_refused;

static int riding(int place) { return place >= 0 && place < PLACES && ((op1f_lfo4_rides >> place) & 1u) != 0; }
static int instrument(u32 mode) { return mode == (u32)MODE_SYNTH || mode == (u32)MODE_DRUM; }
static int lfo_screen(u32 screen) { return (screen >= (u32)LFO_SCREEN_FIRST && screen <= (u32)LFO_SCREEN_LAST) || (unit_screen != 0 && screen == unit_screen); }

static int turn(const Op1fInput* e) {
    if (!e->mine || !instrument(e->mode) || e->key >= (u32)KNOBS) return 0;
    int place = op1f_lfo4_place();
    if (!riding(place)) return 0;
    int t = e->turn;
    int steps = t > 0 ? (t + 1023) >> 10 : -((1023 - t) >> 10);       // 1024 a step; a smaller turn is a step
    if (t == 0) return 1;
    if (op1f_lfo12_presets_screen() != 0) op1f_lfopresets_turn((int)e->key, steps);   // the presets over the UFO page: theirs
    else op1f_lfo4_turn(place, (int)e->key, steps, e->shift != 0);
    return 1;                                                           // the page's knobs are its own
}

extern "C" int lfo_input(const Op1fInput* e) OP1F_NAMED("lfo_input");
extern "C" int lfo_input(const Op1fInput* e) {
    if (!e || e->bytes < (u32)INPUT_LEVEL_2) return 0;
    u32 what = e->what, key = e->key;
    if (what == (u32)OP1F_TURN) return turn(e);
    if (key >= (u32)KEY_PLAY_FIRST && key <= (u32)KEY_PLAY_LAST) return 0;   // maybe the audio thread: nothing here
    if (e->bytes >= (u32)INPUT_LEVEL_4 && e->thread != (u32)OP1F_KEY_THREAD) return 0;
    if (key >= (u32)KEYS) return 0;
    if (what == (u32)OP1F_UP) {                                         // the up of a down this pack took
        if (key == (u32)KEY_T4 && back_t4) { back_t4 = 0; return 1; }
        u32 at = taken_at[key];
        if (at == 0) return 0;
        taken_at[key] = 0;
        op1f_lfo4_key((int)at - 1, (int)key, OP1F_UP);
        return 1;
    }
    if (what != (u32)OP1F_DOWN && what != (u32)OP1F_DOWN_SHIFT) return 0;
    if (!instrument(e->mode)) return 0;
    int place = op1f_lfo4_place();
    if (!riding(place)) return 0;
    if (key == (u32)KEY_T4 && what == (u32)OP1F_DOWN_SHIFT) {           // the unit's list of LFO kinds; the presets' list closed first
        if (e->mine && op1f_lfo12_presets_screen() != 0) {
            op1f_lfopresets_key_down(KEY_T4, 1);
            op1f_page_close(&op1f_card_lfo);
        }
        return 0;
    }
    if (!e->mine) {
        if (key != (u32)KEY_T4 || what != (u32)OP1F_DOWN) return 0;
        if (page_now == (int)OP1F_LAYER_PAGE_UNIT && lfo_screen(e->screen)) {   // the stock page after a tap: back to the UFOs
            page_want = OP1F_LAYER_PAGE_OWN;
            back_t4 = 1;
            return 1;
        }
        if (op1f_lfo4_key(place, KEY_T4, OP1F_DOWN)) {                  // the LFO unit's own sign of that page (its controller's page word)
            taken_at[KEY_T4] = (u8)(place + 1);
            return 1;
        }
        soon = 1;                                                       // the page may come: dressed from the next frame
        return 0;
    }
    int knob_key = (key >= (u32)KEY_CLICK0 && key < (u32)(KEY_CLICK0 + KNOBS)) || (key >= (u32)KEY_PRESS0 && key < (u32)(KEY_PRESS0 + KNOBS));
    if (key != (u32)KEY_T4 && !knob_key && (key < (u32)KEY_T1 || key > (u32)KEY_T3)) return 0;
    int taken;
    if (op1f_lfo12_presets_screen() != 0) {                             // the presets over the UFO page (its `shown` is 0 meanwhile): their keys
        taken = op1f_lfopresets_key_down((int)key, what == (u32)OP1F_DOWN_SHIFT) > 0;
        if (op1f_lfo12_presets_screen() == 0) op1f_page_close(&op1f_card_lfo);   // the list closed by this key: its page goes now
    } else {
        taken = op1f_lfo4_key(place, (int)key, (int)what);
        if (key == (u32)KEY_T4 || knob_key) taken = 1;                  // the UFO page's own keys, whatever the unit did with them
    }
    if (taken) taken_at[key] = (u8)(place + 1);
    return taken ? 1 : 0;
}

extern "C" void lfo_ufo_shown(unsigned where, unsigned how) OP1F_NAMED("lfo_ufo_shown");
extern "C" void lfo_ufo_shown(unsigned where, unsigned how) {
    if (how) shown_where = where;
    shown_up = how != 0 && where < (unsigned)PLACES;
    op1f_lfo4_shown((int)where, how != 0);
}

// lfo's page up: the frame names lfo's card (the UFO page, or the presets over it) or the base says the UFO page is
// up, in the synth or the drum mode, for a place that rides.
extern "C" void op1f_lfo12_see(const Op1fFrame* now) OP1F_NAMED("op1f_lfo12_see");
extern "C" void op1f_lfo12_see(const Op1fFrame* now) {
    int mine = now->bytes >= (u32)FRAME_LEVEL_4 && now->page == &op1f_card_lfo;
    page_up = (mine || shown_up) && instrument(now->mode) && riding(op1f_lfo4_place()) ? 1u : 0u;
}

extern "C" int op1f_lfo12_page_up() OP1F_NAMED("op1f_lfo12_page_up");
extern "C" int op1f_lfo12_page_up() { return (int)page_up; }

// The place whose UFOs the page copy dresses this frame, or -1: the place the base says is up; else the place on the
// screen while it rides and its UFO page is up or may come up (a stock LFO page, the list of kinds, T4 just down).
extern "C" int op1f_lfo12_ufo_place(const Op1fFrame* now) OP1F_NAMED("op1f_lfo12_ufo_place");
extern "C" int op1f_lfo12_ufo_place(const Op1fFrame* now) {
    if (shown_up) return (int)shown_where;
    if (!instrument(now->mode)) return -1;
    int place = op1f_lfo4_place();
    if (!riding(place)) return -1;
    u32 s = now->screen;
    if (s == (u32)PAGE_SCREEN || lfo_screen(s) || s == (u32)LFO_LIST_SCREEN || soon_frames != 0) return place;
    return -1;
}

static const char* page_word(u32 which) { return which == (u32)OP1F_LAYER_PAGE_OWN ? "the ufos" : "the unit's lfo page"; }

static void choose(u32 which, const char* why, const Op1fFrame* now) {
    int taken = op1f_layer_page(&op1f_card_lfo, ASK_UFO, which);
    if (taken) page_now = (int)which;
    else ++page_refused;
    if (taken && which == (u32)OP1F_LAYER_PAGE_UNIT) { unit_screen = 0; unit_watch = (u32)SOON_FRAMES; }   // the screen it comes on is learnt
    if (page_lines < (u32)PAGE_LINES && (taken || page_refused == 1u)) {
        ++page_lines;
        op1f_log(3, LOG_FILE, 1, "lfo ufo: page on T4: %s (%s), answer %d, screen %u at %u ms", page_word(which), why, taken, now->screen, now->ms);
    }
}

extern "C" void op1f_lfo12_rides(const Op1fFrame* now) OP1F_NAMED("op1f_lfo12_rides");
extern "C" void op1f_lfo12_rides(const Op1fFrame* now) {
    int due = op1f_lfo4_ride_due();
    if (due >= 0 && due < PLACES) {                                     // a sound kept on "ufo" loaded: its ride, one a frame
        int answer = op1f_layer_start(&op1f_card_lfo, ASK_UFO, (unsigned)due);
        op1f_lfo4_ride_answer(due, answer);
        if (ride_lines < (u32)RIDE_LINES && answer != OP1F_NOT_NOW) {
            ++ride_lines;
            op1f_log(3, LOG_FILE, 2, "lfo ufo: ride started at place %d (0-7 synth, 8-15 drum), answer %d, riding 0x%04x at %u ms", due, answer, op1f_lfo4_rides, now->ms);
        }
    }
    if (soon) { soon = 0; soon_frames = (u32)SOON_FRAMES; }
    else if (soon_frames) --soon_frames;
    const char* why = 0;
    u32 rides = op1f_lfo4_rides;
    if (page_now < 0) why = "the first frame";
    if (rides & ~rides_was) { page_want = OP1F_LAYER_PAGE_OWN; why = "a ride began"; }
    rides_was = rides;
    int asked = op1f_lfo4_take_page();
    if (asked == 1) { page_want = OP1F_LAYER_PAGE_OWN; why = "the lfo unit's"; }
    else if (asked == 2) { page_want = OP1F_LAYER_PAGE_UNIT; why = "a tap"; }
    if (unit_watch) {                                                   // the first screen after 77 once the unit's page was chosen
        --unit_watch;
        if (now->screen != (u32)PAGE_SCREEN && instrument(now->mode)) { unit_screen = now->screen; unit_watch = 0; }
    }
    int on = instrument(now->mode) && (now->screen == (u32)PAGE_SCREEN || lfo_screen(now->screen));
    if (page_now == (int)OP1F_LAYER_PAGE_UNIT && page_want == (u32)OP1F_LAYER_PAGE_UNIT && !on) {
        if (++off_frames >= (u32)OFF_FRAMES) { page_want = OP1F_LAYER_PAGE_OWN; why = "neither lfo page up"; }
    } else {
        off_frames = 0;
    }
    u32 want = page_want;
    if ((int)want != page_now) choose(want, why ? why : "T4 on the unit's page", now);
}
