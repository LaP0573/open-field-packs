// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R2: the pack lfo's frame function, which its card names. Called once a frame by the base,
// on the display thread, in the order of the packs' file names (behind fx train's).
//
// Step R15 (lfo 12): the UFO page is the page of lfo's LFO layer "ufo", shown by the
// base on T4, and lfo shows it and its presets itself, with fx train or without. In this order:
//   the copy of the owner's LFO presets into the pack's own folder, one slot a frame until it has ended well
//     (build_r2_move.cpp); the frame it ends in runs the presets' scan that waited for it (build_r2_lfopresets.cpp);
//   what the frame says of lfo's page, for the presets' frame work (build_r15_lfo_input.cpp);
//   only while fx train is away: the LFOs' frame work, the presets' and the scan (build_r4c_lfo_alone.cpp); while fx
//     train runs its frame runs those three, before this one;
//   always: one ride start and the page choice (build_r15_lfo_input.cpp); the UFOs' document, its stand-in and its
//     dressing (build_r4c_train_lfo.cpp); the presets' list and save screen dressed (build_r2_presets_lfo.cpp), then
//     shown over the UFOs or closed (build_r4c_train_lfo.cpp).

#include "build_r1_card.h"
#ifdef OP1F_HOST
#include "mock_page_r2.h"                                   // the PC's stand-ins (for tests this tree does not hold)
#endif

struct Op1fChainView;                                        // build_h_chain.h: only its address is asked here

extern "C" const Op1fCard op1f_card_lfo OP1F_NAMED("op1f_card_lfo");
extern "C" int op1f_r2_move_step(const Op1fCard* card, const char* old_folder, int (*holds_preset)(void* json), const Op1fFrame* now) OP1F_NAMED("op1f_r2_move_step");
extern "C" int op1f_lfo_holds_preset(void* json) OP1F_NAMED("op1f_lfo_holds_preset");
extern "C" void op1f_lfo_presets_owed() OP1F_NAMED("op1f_lfo_presets_owed");
extern "C" void op1f_lfo_alone_frame() OP1F_NAMED("op1f_lfo_alone_frame");
extern "C" void op1f_lfo12_see(const Op1fFrame* now) OP1F_NAMED("op1f_lfo12_see");
extern "C" void op1f_lfo12_rides(const Op1fFrame* now) OP1F_NAMED("op1f_lfo12_rides");
extern "C" void op1f_lfo_ufo_frame(const Op1fFrame* now) OP1F_NAMED("op1f_lfo_ufo_frame");
extern "C" void op1f_presets_frame() OP1F_NAMED("op1f_presets_frame");
extern "C" void op1f_lfo_page_frame(const Op1fFrame* now) OP1F_NAMED("op1f_lfo_page_frame");
extern "C" void __attribute__((weak)) op1f_chain_view(int host, Op1fChainView* out) OP1F_NAMED("op1f_chain_view");   // fx train's: there while it runs

extern "C" void lfo_frame(const Op1fFrame* now) OP1F_NAMED("lfo_frame");
extern "C" void lfo_frame(const Op1fFrame* now) {
    if (op1f_r2_move_step(&op1f_card_lfo, "user/lf/", op1f_lfo_holds_preset, now)) op1f_lfo_presets_owed();
    op1f_lfo12_see(now);
    if (&op1f_chain_view == 0) op1f_lfo_alone_frame();
    op1f_lfo12_rides(now);
    op1f_lfo_ufo_frame(now);
    op1f_presets_frame();
    op1f_lfo_page_frame(now);
}
