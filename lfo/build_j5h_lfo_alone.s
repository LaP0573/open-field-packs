/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build J packs J5h: the LFOs without the pack `fx train` (OS 1.7.0+0). One hand-written routine; the rest is in
 * J5h's unit for the LFOs without the effect train.
 *
 * The fx side of the three LFOs that are not stock's own (build_i_lfo4.cpp, op1f_lfo4_block) is called once a block
 * of the synth's second stage. With the effect train's units there it is the first thing in the chain's run, which
 * the chain's stub calls after stock's effect call (0x11a052e6, build_h_fx.s). Without them stock's own effect
 * runs, and an LFO's offset must be on the scratch before it does:
 *
 *   0x11a052b4  the load of the word at 0x84a03c84 into R0, in the synth second stage 0x11a04130: the producer's read before the fx_active
 *                                  test, the place of the chain's start stub (op1f_fx_synth_t0). Stock's own LFO
 *                                  has had its call by then (0x11a05220 jumps to its apply at 0x11a054ce, which
 *                                  comes back to 0x11a05222), and stock's effect call follows (0x11a05470, which
 *                                  reads the producer again). Dead there: R1-R3, P0-P2, CC (the code after sets
 *                                  them before use; the vocoder path arrives from a call), no loop register in
 *                                  flight, and the function's RETS is saved in its frame (build_h_fx.s, since H1a).
 *                                  -> op1f_lfo_alone_fx: op1f_lfo4_block(0), then the load.
 *
 * The pack's table marks the entry "only when the pack fx train is away": with that pack there the site is its own.
 * The call is an ordinary C call with its twelve bytes of argument space; the callee keeps R4-R7 and P3-P5, which
 * stock holds live across the site.
 */
.set PRODUCER_SLOT, 0x84a03c84

.text
.align 2

/* Replaces the load of the word at 0x84a03c84 into R0 at 0x11a052b4. */
.global op1f_lfo_alone_fx
.type op1f_lfo_alone_fx, STT_FUNC
op1f_lfo_alone_fx:
    [--SP] = RETS;
    SP += -12;                          /* the callee's argument space */
    R0 = 0;
    CALL op1f_lfo4_block;               /* void (int host): each of the module's LFOs on the scratch, as stock applies its own */
    SP += 12;
    RETS = [SP++];
    P0.H = PRODUCER_SLOT;
    P0.L = PRODUCER_SLOT;
    R0 = [P0];                          /* the displaced load: stock's producer, for stock's test and call */
    RTS;
.size op1f_lfo_alone_fx, .-op1f_lfo_alone_fx
