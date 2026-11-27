// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// 2026-11-27: fx train 19 [D]: the train page lost under its own feet, healed. What changes in this unit,
// and nothing else. When the first wagon's effect changes (a turn of its knob, a preset), the unit asks for the new
// effect's screen; where it asks before the base has its answer for that effect ready, the unit's own effect page comes
// up although the page choice is the train's, and nothing asks again: the page stays the unit's, every turn of a knob
// after it goes to that page, and T3 there switches the effect on or off (seen in the emulator under fast turns of
// several knobs). So: while a riding sound's own effect page is on the screen (its effect's screen, no pack's page) with
// the choice the train's for LOST_FRAMES frames, the frame asks the unit for its screen again (the firmware's request,
// masked, as fx train 17 made it from its frame); where that does not bring it back, the next try sets the page choice
// to the unit's for one frame and back to the train's, the way a tap and T3 swap the two pages; at most HEAL_TRIES
// tries a loss. T3 in that state is taken (it would switch the effect) and tries at once.
//
// Step R14, the pack `fx train` 18: its input function, its layer's `shown`, the page choice and the rides' start.
// The train is the effect layer "train", ask 2 of the
// card (module/build_r6e_card.h, "Base 35: layers"); the store, the rides' bits and the grammar of the page are the
// chain unit's (fx train/build_r6c_chain.cpp, the op1f_train_ names below). The shape is lfo 12's
// (lfo/build_r15_lfo_input.cpp), with T3 in the place of T4.
//
//   fx_train_input, on the key thread (every key and knob is offered to it before the synth's or the drum's
//     controller, and before lfo's input: the files' names' order):
//     outside the synth and the drum mode, or where the place on the screen does not ride "train": 0 for every event;
//     shift + T3: 0, the unit's list of effects; the presets' list, when it shows, closed first (op1f_train_key), and
//       its page at once;
//     while the train's page, or the presets' list or save screen over it, is up (`mine`): T3 and the knobs' clicks
//       and presses taken and handed to op1f_train_key for the place on the screen (the chain hands the list its keys
//       first while it shows); T1, T2 and T4 handed on and let go (the list closes, and its page at once); every turn
//       taken (in steps of 1024, rounded away from 0) and handed to op1f_train_turn;
//     on the stock page of a riding place's effect after the page choice went to the unit's (a tap; the controller's
//       page word reads 2): T3 down taken, and its up, and the train page wanted again;
//     the up of every other down taken, in any mode, goes to op1f_train_key with the place of its down;
//     the keys 13 to 53 (they may come on the audio thread): 0 at once.
//   fx_train_shown, the layer's `shown` (the display thread): the place whose train page is up.
//   op1f_train14_see, in fx_train_frame before the chain's frame work: the place whose train page, or the presets
//     over it, is up (op1f_train14_up), -1 for none.
//   op1f_train14_place: the place the page unit dresses this frame, or -1.
//   op1f_train14_rides, in fx_train_frame after the chain's frame work: one ride start a frame (op1f_train_ride_due);
//     a ride of "train" at the master ended (the master keeps its own effect; the train there is a later piece); the
//     page up told to the chain (op1f_train_up); the page choice applied (op1f_layer_page): the train page at the
//     first frame and whenever a ride begins; the unit's page when the chain asks it (op1f_train_take_page 2, a tap);
//     the train page again on take_page 1, or once neither page has been on the screen for three frames, so T3 from
//     T1, T2 or T4 always shows the train.
// The stock effect page is known by the controller's page word (the synth's controller 0x08018804, the drum's
// 0x080188f4, a static object each, the word at +0x28 reading 2 on T3's page, as lfo/build_i_lfo4.cpp reads it for
// T4's 3): the effect pages' screens are one per effect. Nothing is divided, nothing is logged from the key thread or
// the audio thread.

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned char u8;

enum {
    FRAME_LEVEL_4 = 40, INPUT_LEVEL_2 = 36, INPUT_LEVEL_4 = 40,
    ASK_TRAIN = 2, PLACES = 16, PLACE_DRUM = OP1F_PLACE_DRUM, PLACE_MASTER = OP1F_PLACE_MASTER, MODE_SYNTH = 0, MODE_DRUM = 1,
    KEY_T1 = 6, KEY_T3 = 8, KEY_T4 = 9, KEY_PLAY_FIRST = 13, KEY_PLAY_LAST = 53, KEY_CLICK0 = 54, KEY_PRESS0 = 61, KNOBS = 4, KEYS = 72,
    CONTROLLER_PAGE = 0x28, PAGE_EFFECT = 2,
    OFF_FRAMES = 3, SOON_FRAMES = 8, RIDE_LINES = 64, PAGE_LINES = 64, MASTER_LINES = 16,
};

static const u32 SYNTH_CONTROLLER = 0x08018804u;               // the synth's controller, a static object (its page word at +0x28)
static const u32 DRUM_CONTROLLER = 0x080188f4u;                // the drum's
// 19: the heal (above). The screen of each of the unit's effects, the synth's and the drum's type words a slot, and the
// firmware's own screen request (fx train 17's).
typedef u32 (*ScreenOfFn)(u32 type);
#define STOCK_SCREEN_OF ((ScreenOfFn)0x8fc273f0u)
typedef void (*RequestScreenFn)();
#define STOCK_REQUEST_SCREEN ((RequestScreenFn)0x8c0b2554u)
static const u32 TYPE_WORDS[2] = { 0x11904794u, 0x119047b4u };   // the synth's and the drum's effect type, a word a slot
enum { FX_TYPES = 11, LOST_FRAMES = 4, HEAL_GAP_FRAMES = 30, HEAL_TRIES = 3, HEAL_LINES = 24 };

static const char LOG_FILE[] = "op1f/train_layer.cpp";         // the log shows "train_layer"

extern "C" const Op1fCard op1f_card_fx_train OP1F_NAMED("op1f_card_fx_train");

// The chain unit's (build_r6c_chain.cpp).
extern "C" volatile unsigned op1f_train_rides OP1F_NAMED("op1f_train_rides");
extern "C" int op1f_train_place() OP1F_NAMED("op1f_train_place");
extern "C" int op1f_train_key(int place, int key, int what) OP1F_NAMED("op1f_train_key");
extern "C" int op1f_train_turn(int place, int knob, int steps, int shifted) OP1F_NAMED("op1f_train_turn");
extern "C" int op1f_train_take_page() OP1F_NAMED("op1f_train_take_page");
extern "C" void op1f_train_up(int place) OP1F_NAMED("op1f_train_up");
extern "C" int op1f_train_ride_due() OP1F_NAMED("op1f_train_ride_due");
extern "C" void op1f_train_ride_answer(int place, int answer) OP1F_NAMED("op1f_train_ride_answer");
// The presets' screen the chain wants for a host: the save screen's id, the list's, or 0 (build_h_chain.h).
extern "C" int op1f_chain_screen_wanted(int host) OP1F_NAMED("op1f_chain_screen_wanted");

// The key thread's words, written there only.
static volatile u8 taken_at[KEYS];                  // the place + 1 of each key whose down was taken, 0 for none
static volatile u32 back_t3;                        // T3's down taken on the stock effect page: its up is taken too
static volatile u32 soon;                           // T3 went down for a riding place: its train page may come up
// Both threads': the page choice wanted (OP1F_LAYER_PAGE_OWN or _UNIT), each write whole.
static volatile u32 page_want = OP1F_LAYER_PAGE_OWN;
// The display thread's words.
static volatile int page_now = -1;                  // the choice the base took last, -1 before the first
static volatile u32 shown_where, shown_up;          // the layer's `shown`: the place whose train page is up
static volatile int up_place = -1;                  // the place whose train page, or the presets over it, is up (op1f_train14_see)
static u32 rides_was, off_frames, soon_frames, ride_lines, page_lines, page_refused, master_lines;
static int master_said = 1;                         // the last answer of the master's end that was said, 1 for none yet
static volatile u32 heal_now;                       // 19: the key thread met T3 on a lost train page: try at the next frame
static u32 fx_screens[FX_TYPES], fx_screens_read;   // 19: the display thread's
static u32 lost_frames, heal_wait, heal_tries, heals, heal_lines;
static u32 flip_back;                               // 19: the heal set the choice to the unit's page: back to the train's at the next frame

static int riding(int place) { return place >= 0 && place < PLACES && ((op1f_train_rides >> place) & 1u) != 0; }
static int instrument(u32 mode) { return mode == (u32)MODE_SYNTH || mode == (u32)MODE_DRUM; }
static int host_of(int place) { return place >= PLACE_DRUM ? 1 : 0; }
static u32 page_word(int host) { return *(volatile const u32*)((host ? DRUM_CONTROLLER : SYNTH_CONTROLLER) + (u32)CONTROLLER_PAGE); }
static int presets_up(int host) { return op1f_chain_screen_wanted(host) != 0; }

static int turn(const Op1fInput* e) {
    if (!e->mine || !instrument(e->mode) || e->key >= (u32)KNOBS) return 0;
    int place = op1f_train_place();
    if (!riding(place)) return 0;
    int t = e->turn;
    int steps = t > 0 ? (t + 1023) >> 10 : -((1023 - t) >> 10);       // 1024 a step; a smaller turn is a step
    if (t == 0) return 1;
    op1f_train_turn(place, (int)e->key, steps, e->shift != 0);         // the presets' list's turn while it shows, else the wagon's
    return 1;                                                           // the page's knobs are its own
}

extern "C" int fx_train_input(const Op1fInput* e) OP1F_NAMED("fx_train_input");
extern "C" int fx_train_input(const Op1fInput* e) {
    if (!e || e->bytes < (u32)INPUT_LEVEL_2) return 0;
    u32 what = e->what, key = e->key;
    if (what == (u32)OP1F_TURN) return turn(e);
    if (key >= (u32)KEY_PLAY_FIRST && key <= (u32)KEY_PLAY_LAST) return 0;   // maybe the audio thread: nothing here
    if (e->bytes >= (u32)INPUT_LEVEL_4 && e->thread != (u32)OP1F_KEY_THREAD) return 0;
    if (key >= (u32)KEYS) return 0;
    if (what == (u32)OP1F_UP) {                                         // the up of a down this pack took
        if (key == (u32)KEY_T3 && back_t3) { back_t3 = 0; return 1; }
        u32 at = taken_at[key];
        if (at == 0) return 0;
        taken_at[key] = 0;
        op1f_train_key((int)at - 1, (int)key, OP1F_UP);
        return 1;
    }
    if (what != (u32)OP1F_DOWN && what != (u32)OP1F_DOWN_SHIFT) return 0;
    if (!instrument(e->mode)) return 0;
    int place = op1f_train_place();
    if (!riding(place)) return 0;
    int host = host_of(place);
    if (key == (u32)KEY_T3 && what == (u32)OP1F_DOWN_SHIFT) {           // the unit's list of effects; the presets' list closed first
        if (e->mine) {
            int had = presets_up(host);
            op1f_train_key(place, KEY_T3, OP1F_DOWN_SHIFT);             // answers 0 after closing the list
            if (had && !presets_up(host)) op1f_page_close(&op1f_card_fx_train);
        }
        soon = 1;                                                       // a pick of "train" there brings its page
        return 0;
    }
    if (!e->mine) {
        if (key != (u32)KEY_T3 || what != (u32)OP1F_DOWN) return 0;
        if (page_now == (int)OP1F_LAYER_PAGE_UNIT && page_word(host) == (u32)PAGE_EFFECT) {   // the stock page after a tap: back to the train
            page_want = OP1F_LAYER_PAGE_OWN;
            back_t3 = 1;
            return 1;
        }
        if (page_now == (int)OP1F_LAYER_PAGE_OWN && page_word(host) == (u32)PAGE_EFFECT) {    // 19: the train page lost under its own feet: no switch, a heal now
            heal_now = 1;
            back_t3 = 1;
            return 1;
        }
        soon = 1;                                                       // the page may come: dressed from the next frame
        return 0;
    }
    int knob_key = (key >= (u32)KEY_CLICK0 && key < (u32)(KEY_CLICK0 + KNOBS)) || (key >= (u32)KEY_PRESS0 && key < (u32)(KEY_PRESS0 + KNOBS));
    if (!knob_key && (key < (u32)KEY_T1 || key > (u32)KEY_T4)) return 0;
    int had = presets_up(host);
    int taken = op1f_train_key(place, (int)key, (int)what) != 0;        // the list's keys first while it shows, else the page's
    if (key == (u32)KEY_T3 || knob_key) taken = 1;                      // the train page's own keys, whatever the chain did with them
    if (had && !presets_up(host)) op1f_page_close(&op1f_card_fx_train);  // the list closed by this key: its page goes now
    if (taken) taken_at[key] = (u8)(place + 1);
    return taken ? 1 : 0;
}

extern "C" void fx_train_shown(unsigned where, unsigned how) OP1F_NAMED("fx_train_shown");
extern "C" void fx_train_shown(unsigned where, unsigned how) {
    if (how) shown_where = where;
    shown_up = how != 0 && where < (unsigned)PLACES;
}

// The train's page up: the frame names fx train's card (the train page, or the presets over it) or the base says the
// train page is up, in the synth or the drum mode, for a place that rides.
extern "C" void op1f_train14_see(const Op1fFrame* now) OP1F_NAMED("op1f_train14_see");
extern "C" void op1f_train14_see(const Op1fFrame* now) {
    int mine = now->bytes >= (u32)FRAME_LEVEL_4 && now->page == &op1f_card_fx_train;
    int place = op1f_train_place();
    up_place = (mine || shown_up) && instrument(now->mode) && riding(place) ? place : -1;
}

extern "C" int op1f_train14_up() OP1F_NAMED("op1f_train14_up");
extern "C" int op1f_train14_up() { return up_place; }

// The place whose train the page unit dresses this frame, or -1: the place the base says is up; else the place on the
// screen while it rides and its train page is up or may come up (T3's page word: the stock effect page, or the list of
// effects over it; T3 or shift + T3 just down).
extern "C" int op1f_train14_place(const Op1fFrame* now) OP1F_NAMED("op1f_train14_place");
extern "C" int op1f_train14_place(const Op1fFrame* now) {
    if (shown_up) return (int)shown_where;
    if (!instrument(now->mode)) return -1;
    int place = op1f_train_place();
    if (!riding(place)) return -1;
    if (up_place >= 0 || page_word(host_of(place)) == (u32)PAGE_EFFECT || soon_frames != 0) return place;
    return -1;
}

static const char* page_name(u32 which) { return which == (u32)OP1F_LAYER_PAGE_OWN ? "the train" : "the unit's effect page"; }

static void choose(u32 which, const char* why, const Op1fFrame* now) {
    int taken = op1f_layer_page(&op1f_card_fx_train, ASK_TRAIN, which);
    if (taken) page_now = (int)which;
    else ++page_refused;
    if (page_lines < (u32)PAGE_LINES && (taken || page_refused == 1u)) {
        ++page_lines;
        op1f_log(3, LOG_FILE, 1, "train layer: page on T3: %s (%s), answer %d, screen %u at %u ms", page_name(which), why, taken, now->screen, now->ms);
    }
}

extern "C" void op1f_train14_rides(const Op1fFrame* now) OP1F_NAMED("op1f_train14_rides");
extern "C" void op1f_train14_rides(const Op1fFrame* now) {
    int due = op1f_train_ride_due();
    if (due >= 0 && due < PLACES) {                                     // an old train loaded: its ride, one a frame
        int answer = op1f_layer_start(&op1f_card_fx_train, ASK_TRAIN, (unsigned)due);
        op1f_train_ride_answer(due, answer);
        if (ride_lines < (u32)RIDE_LINES && answer != OP1F_NOT_NOW) {
            ++ride_lines;
            op1f_log(3, LOG_FILE, 2, "train layer: ride started at place %d (0-7 synth, 8-15 drum), answer %d, riding 0x%05x at %u ms", due, answer, op1f_train_rides, now->ms);
        }
    }
    u32 rides = op1f_train_rides;
    if ((rides >> PLACE_MASTER) & 1u) {                                 // "train" picked on the master: ended, the master keeps its effect
        int answer = op1f_layer_end(&op1f_card_fx_train, ASK_TRAIN, (unsigned)PLACE_MASTER);
        if (master_lines < (u32)MASTER_LINES && (answer == 0 || answer != master_said)) {
            ++master_lines;
            master_said = answer;
            op1f_log(3, LOG_FILE, 3, "train layer: the master's pick of train ended (the master keeps its own effect), answer %d at %u ms", answer, now->ms);
        }
    }
    op1f_train_up(up_place);
    if (soon) { soon = 0; soon_frames = (u32)SOON_FRAMES; }
    else if (soon_frames) --soon_frames;
    const char* why = 0;
    if (flip_back) { flip_back = 0; page_want = OP1F_LAYER_PAGE_OWN; why = "the heal, back to the train"; }   // 19
    u32 places = rides & ((1u << PLACES) - 1u);
    if (page_now < 0) why = "the first frame";
    if (places & ~rides_was) { page_want = OP1F_LAYER_PAGE_OWN; why = "a ride began"; }
    rides_was = places;
    int asked = op1f_train_take_page();
    if (asked == 1) { page_want = OP1F_LAYER_PAGE_OWN; why = "the train's"; }
    else if (asked == 2) { page_want = OP1F_LAYER_PAGE_UNIT; why = "a tap"; }
    int host = now->mode == (u32)MODE_DRUM ? 1 : 0;
    int on = instrument(now->mode) && (up_place >= 0 || page_word(host) == (u32)PAGE_EFFECT);
    if (page_now == (int)OP1F_LAYER_PAGE_UNIT && page_want == (u32)OP1F_LAYER_PAGE_UNIT && !on) {
        if (++off_frames >= (u32)OFF_FRAMES) { page_want = OP1F_LAYER_PAGE_OWN; why = "neither train page up"; }
    } else {
        off_frames = 0;
    }
    u32 want = page_want;
    if ((int)want != page_now) choose(want, why ? why : "T3 on the unit's page", now);
    // 19: the heal (above), for the place on the screen.
    if (!fx_screens_read) {
        for (u32 t = 0; t < (u32)FX_TYPES; ++t) fx_screens[t] = STOCK_SCREEN_OF(t);
        fx_screens_read = 1;
    }
    int place = op1f_train_place();
    int lost = 0;
    if (page_now == (int)OP1F_LAYER_PAGE_OWN && instrument(now->mode) && riding(place) && !shown_up && now->page == 0) {
        u32 type = *(volatile const u32*)(TYPE_WORDS[host_of(place)] + 4u * ((u32)place & 7u));
        lost = type < (u32)FX_TYPES && fx_screens[type] != 0 && now->screen == fx_screens[type];
    }
    if (!lost && !flip_back) { lost_frames = 0; heal_tries = 0; heal_now = 0; }
    else ++lost_frames;
    if (heal_wait) --heal_wait;
    if (lost && (heal_now || (heal_tries < (u32)HEAL_TRIES && lost_frames >= (u32)LOST_FRAMES && heal_wait == 0))) {   // T3 always tries; the frame HEAL_TRIES times a loss
        heal_now = 0;
        ++heal_tries;
        ++heals;
        heal_wait = (u32)HEAL_GAP_FRAMES;
        int flip = heal_tries > 1;                                      // the first try asks for the screen; the later ones swap the choice
        if (flip) {
            page_want = OP1F_LAYER_PAGE_UNIT;
            choose(OP1F_LAYER_PAGE_UNIT, "the heal, through the unit's page", now);
            flip_back = 1;
        } else {
            unsigned was = op1f_mask();                                 // the key thread cannot be inside the request's queue now
            STOCK_REQUEST_SCREEN();
            op1f_unmask(was);
        }
        if (heal_lines < (u32)HEAL_LINES) {
            ++heal_lines;
            op1f_log(3, LOG_FILE, 4, "train layer: the train page was lost under the unit's effect page (screen %u, place %d, %u frames): %s, %u heals in all", now->screen, place, lost_frames, flip ? "the choice swapped" : "the screen asked again", heals);
        }
    }
}
