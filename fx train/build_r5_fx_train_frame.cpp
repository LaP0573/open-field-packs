// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// 2026-11-27: fx train 19 [A]: one call more, last: the master's frame step (op1f_train_master_frame,
// build_r16_master.cpp: the effect screens read once, the first wagon's page left with the mixer). op1f_train14_rides
// no longer ends the master's ride, and op1f_train_page_frame now runs the master's page step first.
//
// Step R14, fx train 18: the train page is the page of the pack's
// effect layer "train", shown by the base on T3, and the pack's frame function runs, edited in place, in this order:
//   the names of the packs' effects on the frame's picture (op1f_train_paint, which paints nothing since R6b);
//   the copy of the owner's effect presets into the pack's own folder, one slot a frame until it has ended well
//     (build_r2_move.cpp); the frame it ends in runs the presets' scan that waited for it;
//   the effect stage's measure (build_h_fx.cpp);
//   what the frame says of the train's page (op1f_train14_see, build_r14_train_input.cpp);
//   the chain's frame work, which reads the rides and calls the LFOs' (build_r6c_chain.cpp; handed no frame buffer);
//   one ride start, the master's ride ended, the page up told to the chain and the page choice (op1f_train14_rides,
//     build_r14_train_input.cpp);
//   the train's document, its stand-in and its dressing (op1f_train_frame, build_r6b_train.cpp), after the chain,
//     whose view it reads in the same frame;
//   the presets' list and save screen dressed (op1f_presets_frame, build_r2_presets.cpp), then shown over the train
//     page or closed (op1f_train_page_frame, build_r6b_train.cpp).
// fx train 17's page step (asked or closed by the frame and by the chain's keys) is gone. What follows is the elders'
// description, which held for fx train 14 to 17.
//
// Step R5 (base 28): the pack fx train's frame function in its version 14. A sibling of fx train 11's frame unit
// (fx train 11 to 13's, which stays as it was) with one call more, first: the names of the packs' effects on the wagons
// laid on the frame's picture (fx train 14's train unit, op1f_train_paint), over what the last dressing of the train
// drew, before anything is dressed again. Everything else in the order the elder has it, which holds:
//
// Step R2: the pack fx train's frame function, which its card names. Called once a frame by
// the base, on the display thread, in the order of the packs' file names. It does the pack's frame work in the order
// the base did it before this step, and nothing else:
//
//   the copy of the owner's effect presets into the pack's own folder, one slot a frame until it has ended well
//     (build_r2_move.cpp); the frame it ends in runs the presets' scan that waited for it (fx train 11's chain unit);
//   the effect stage's measure (build_h_fx.cpp);
//   the chain's frame work, which also calls the LFOs' (fx train 11's chain unit; it paints nothing, so it is handed no
//     frame buffer);
//   the train's and the UFOs' page dressed, then the presets' list and save screen (fx train 11's train unit,
//     build_r2_presets.cpp): after the chain and the LFOs, whose views they read in the same frame;
//   the page asked of the base or closed when what is wanted changed (fx train 11's train unit).

#include "build_r1_card.h"

typedef unsigned short u16;

extern "C" const Op1fCard op1f_card_fx_train OP1F_NAMED("op1f_card_fx_train");
extern "C" int op1f_r2_move_step(const Op1fCard* card, const char* old_folder, int (*holds_preset)(void* json), const Op1fFrame* now) OP1F_NAMED("op1f_r2_move_step");
extern "C" int op1f_fx_holds_preset(void* json) OP1F_NAMED("op1f_fx_holds_preset");
extern "C" void op1f_fx_presets_owed() OP1F_NAMED("op1f_fx_presets_owed");
extern "C" void op1f_fx_frame() OP1F_NAMED("op1f_fx_frame");
extern "C" void op1f_chain_frame(u16* fb) OP1F_NAMED("op1f_chain_frame");
extern "C" void op1f_train_frame(const Op1fFrame* now) OP1F_NAMED("op1f_train_frame");
extern "C" void op1f_presets_frame() OP1F_NAMED("op1f_presets_frame");
extern "C" void op1f_train_page_frame(const Op1fFrame* now) OP1F_NAMED("op1f_train_page_frame");
extern "C" void op1f_train_paint(const Op1fFrame* now) OP1F_NAMED("op1f_train_paint");     // R5
extern "C" void op1f_train14_see(const Op1fFrame* now) OP1F_NAMED("op1f_train14_see");     // R14
extern "C" void op1f_train14_rides(const Op1fFrame* now) OP1F_NAMED("op1f_train14_rides"); // R14
extern "C" void op1f_train_master_frame(const Op1fFrame* now) OP1F_NAMED("op1f_train_master_frame");   // 19

extern "C" void fx_train_frame(const Op1fFrame* now) OP1F_NAMED("fx_train_frame");
extern "C" void fx_train_frame(const Op1fFrame* now) {
    op1f_train_paint(now);                                                  // R5: the names of the packs' effects on the frame's picture
    if (op1f_r2_move_step(&op1f_card_fx_train, "user/fx/", op1f_fx_holds_preset, now)) op1f_fx_presets_owed();
    op1f_fx_frame();
    op1f_train14_see(now);                                                  // R14: the train's page up, before the chain reads it
    op1f_chain_frame(0);
    op1f_train14_rides(now);                                                // R14: a ride start, the master's end, the page up, the page choice
    op1f_train_frame(now);
    op1f_presets_frame();
    op1f_train_page_frame(now);
    op1f_train_master_frame(now);                                           // 19: the master's frame step
}
