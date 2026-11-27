// Made for OS 1.7.3; every address the code uses is 1.7.3's.
// 2026-11-27: fx train 19 [A]: the train on the master. What the card asks of the base for it
// (recipe.json): ask 2, the effect layer "train", a row of the master's list of effects as of every synth and drum
// sound's, now with a `process`; ask 3, a stage before the master's effect. This unit is the card's side of the master:
// its part of the input function, the layer's `process` and the stage's, and a frame step. The train itself (its
// wagons, their instances, the keys' work, the file) is the chain unit's (fx train/build_r6c_chain.cpp, the
// op1f_train_master_ names below), the page the train unit's (fx train/build_r6b_train.cpp).
//
// The sound. The master that rides "train" plays the train around its own effect, which is the first wagon: the stage
// (fx_train_master_before) runs the wagons placed before it, the layer's process (fx_train_master_after) the wagons
// after it. At a sound's place the process does nothing: a sound's train runs at fx train's places in the instrument's
// stage, as in 18.
//
// The page. The master's train is the pack's own page shown over the mixer (the train unit's page step, op1f_page_show
// with OP1F_PAGE_STAYS, as fx train 17 showed the train over a sound), not the layer's page standing in for the master
// effect's: a page that stands in for an effect's page went away on the master when the first wagon's type changed
// under it (another effect on the first wagon: the unit asked for the new effect's screen and its own page came up).
// While the mixer is up the layer's page choice is the unit's (build_r14_train_input.cpp), so the master effect's own
// page is always the unit's. The mixer's keys come through the card's input function:
//   T3 in the mixer, the train's page not up, the master on "train": the train opens. From another page of the mixer
//     the key goes on to the firmware as well, which turns the mixer to its effect page under the train; on that effect
//     page it is taken (the firmware's T3 there would switch the master's effect on or off);
//   on the train (this pack's page up), every key and knob is the chain unit's (op1f_train_master_key and _turn), as on
//     a sound's train: turn a knob to pick a wagon's effect, hold to couple, shift + turn to move, tap for the wagon's
//     page, T3 held for the presets, its tap the first wagon's page; T1, T2, T4 go on to the mixer's pages and close the
//     train;
//   the first wagon's page (the master's own effect page, after a tap or T3 on the train): T3 opens the train again.
//
// The threads. The input function runs on the key thread (keys 13 to 53 may come on the audio thread: 0 at once); the
// frame step in the card's frame function; the two processes on the audio thread, which do the chain unit's run
// alone. Single words between them. Nothing is divided; nothing is logged but from the frame.

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned char u8;

// The chain unit's (build_r6c_chain.cpp).
extern "C" int op1f_train_master_key(int key, int up, int shifted) OP1F_NAMED("op1f_train_master_key");
extern "C" int op1f_train_master_turn(int index, int delta) OP1F_NAMED("op1f_train_master_turn");
extern "C" void op1f_train_master_open(int open) OP1F_NAMED("op1f_train_master_open");
extern "C" int op1f_train_master_rides() OP1F_NAMED("op1f_train_master_rides");
extern "C" void op1f_train_master_run(int part, int* left, int* right) OP1F_NAMED("op1f_train_master_run");
extern "C" int op1f_chain_train_open(int host) OP1F_NAMED("op1f_chain_train_open");
// The train unit's (build_r6b_train.cpp): the master's page step, after a key.
extern "C" void op1f_train_page_now(int host) OP1F_NAMED("op1f_train_page_now");

typedef u32 (*ScreenOfFn)(u32 type);
#define STOCK_SCREEN_OF ((ScreenOfFn)0x8fc273f0u)
#define RD32(a) (*(volatile u32*)(a))

enum {
    INPUT_LEVEL_2 = 36, INPUT_LEVEL_4 = 40,
    MASTER = OP1F_PLACE_MASTER, MODE_MIXER = 3, MODE_LIST = 8, MASTER_HOST = 2, TYPES = 11,
    KEY_T1 = 6, KEY_T3 = 8, KEY_T4 = 9, KEY_PLAY_FIRST = 13, KEY_PLAY_LAST = 53, KEYS = 72,
};
static const u32 MASTER_TYPE = 0x11902e24u;                       // the master's effect's type word

static const char LOG_FILE[] = "op1f/train_master.cpp";          // the log shows "train_master"

// The key thread's words, written there only.
static volatile u8 taken_at[KEYS];                  // 1 for each key whose down was taken: its up is taken too
// Both threads': whole words.
static volatile u32 unit_up;                        // the master's own effect page is up after a tap or T3 on the train
// The display thread's: the effect screen of each of the unit's effects, read at the first frame (0 unknown).
static volatile u32 fx_screen[TYPES];
static volatile u32 screens_read;
static volatile u32 opens, unit_pages;
static u32 lines;

// From the chain unit, on the key thread (a tap, or T3's tap on the train): the first wagon's page, the unit's. The
// chain unit has closed the train, and the master effect's page under it shows.
extern "C" void op1f_train_master_unit_page() OP1F_NAMED("op1f_train_master_unit_page");
extern "C" void op1f_train_master_unit_page() {
    unit_up = 1;
    unit_pages = unit_pages + 1;
}

// The mixer shows the master effect's page now (its screen is the effect screen of the master's type).
static int effect_page_up(u32 screen) {
    u32 t = RD32(MASTER_TYPE);
    return screens_read && t < (u32)TYPES && fx_screen[t] != 0 && screen == fx_screen[t];
}

static int turn_steps(int t) { return t > 0 ? (t + 1023) >> 10 : -((1023 - t) >> 10); }   // 1024 a detent; a smaller turn is a detent

// The master's part of the card's input function (build_r14_train_input.cpp's fx_train_input asks it first): 1 for an
// event it took, 0 for any other, which goes on to a sound's part and then to the firmware.
extern "C" int op1f_train_master_input(const Op1fInput* e) OP1F_NAMED("op1f_train_master_input");
extern "C" int op1f_train_master_input(const Op1fInput* e) {
    if (!e || e->bytes < (u32)INPUT_LEVEL_2) return 0;
    u32 what = e->what, key = e->key;
    if (what == (u32)OP1F_TURN) {
        if (!e->mine || e->mode != (u32)MODE_MIXER || key >= 4u || !op1f_train_master_rides() || !op1f_chain_train_open(MASTER_HOST)) return 0;
        int steps = turn_steps(e->turn);
        if (e->turn == 0) return 1;
        return op1f_train_master_turn((int)key + (e->shift ? 4 : 0), steps) ? 1 : 0;   // the train's page's knobs are its own
    }
    if (key >= (u32)KEY_PLAY_FIRST && key <= (u32)KEY_PLAY_LAST) return 0;   // maybe the audio thread: nothing here
    if (e->bytes >= (u32)INPUT_LEVEL_4 && e->thread != (u32)OP1F_KEY_THREAD) return 0;
    if (key >= (u32)KEYS) return 0;
    if (what == (u32)OP1F_UP) {                                         // the up of a down this unit took, in any mode
        if (!taken_at[key]) return 0;
        taken_at[key] = 0;
        op1f_train_master_key((int)key, 1, e->shift != 0);
        op1f_train_page_now(MASTER_HOST);
        return 1;
    }
    if (what != (u32)OP1F_DOWN && what != (u32)OP1F_DOWN_SHIFT) return 0;
    if (e->mode != (u32)MODE_MIXER || !op1f_train_master_rides()) return 0;
    int shifted = what == (u32)OP1F_DOWN_SHIFT;
    if (!e->mine || !op1f_chain_train_open(MASTER_HOST)) {
        if (key != (u32)KEY_T3 || shifted) {
            if (key >= (u32)KEY_T1 && key <= (u32)KEY_T4) unit_up = 0;  // another page of the mixer
            return 0;                                                   // shift + T3: the unit's list of effects, as always
        }
        // T3: the train opens over the mixer. On the master effect's page (the first wagon's, after a tap, or reached
        // before the train was picked) the key is taken; from another page the firmware has it too, and turns the mixer
        // to its effect page under the train.
        int on_effect = unit_up || effect_page_up(e->screen);
        unit_up = 0;
        op1f_train_master_open(1);
        op1f_train_page_now(MASTER_HOST);
        opens = opens + 1;
        if (!on_effect) return 0;
        taken_at[key] = 1;
        return 1;
    }
    int taken = op1f_train_master_key((int)key, 0, shifted);         // T1, T2, T4: 0, the train closed, the mixer's page
    op1f_train_page_now(MASTER_HOST);
    if (taken) taken_at[key] = 1;
    return taken ? 1 : 0;
}

// The layer's process, after the unit's effect at a place that rides it (or where it would have played, when it is
// switched off): the master's wagons after the first one. Nothing at a sound's place.
extern "C" void fx_train_master_after(const Op1fEffectBlock* b) OP1F_NAMED("fx_train_master_after");
extern "C" void fx_train_master_after(const Op1fEffectBlock* b) {
    if (b == 0 || b->place != (unsigned)MASTER) return;
    op1f_train_master_run(1, b->left, b->right);
}

// The stage before the master's effect: the master's wagons before the first one.
extern "C" void fx_train_master_before(const Op1fBlock* b) OP1F_NAMED("fx_train_master_before");
extern "C" void fx_train_master_before(const Op1fBlock* b) {
    if (b == 0) return;
    op1f_train_master_run(0, b->left, b->right);
}

// The frame step, from the card's frame function: the effect screens read once; the first wagon's page left with the
// mixer; a line when the counts moved.
extern "C" void op1f_train_master_frame(const Op1fFrame* now) OP1F_NAMED("op1f_train_master_frame");
extern "C" void op1f_train_master_frame(const Op1fFrame* now) {
    if (!screens_read) {
        for (u32 t = 0; t < (u32)TYPES; ++t) fx_screen[t] = STOCK_SCREEN_OF(t);
        screens_read = 1;
    }
    if (now->mode != (u32)MODE_MIXER && now->mode != (u32)MODE_LIST) unit_up = 0;
    static u32 said_opens, said_pages;
    if ((opens != said_opens || unit_pages != said_pages) && lines < 64u) {
        ++lines;
        said_opens = opens;
        said_pages = unit_pages;
        op1f_log(3, LOG_FILE, 2, "train master: the train opened %u times on the mixer's T3, the first wagon's page %u times; screen %u at %u ms", opens, unit_pages, now->screen, now->ms);
    }
}
