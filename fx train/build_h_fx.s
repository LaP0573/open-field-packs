/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* 2026-11-27: fx train 19 [C]: two more stubs, for the synth's AMP path, and nothing else changes here.
 * On a synth sound of TE's amp engine (the live input played as a sound) the synth's audio stage never reaches the two
 * synth places below: it hands the whole block to the AMP path (0x8d504a80), which plays the engine on the input,
 * copies the block onto the same two buses and runs the slot's effect there itself, with the same object (the word
 * at 0x84a03c84), the same on byte and the same eight words (0x11904754). So up to 18 only the wagon under the lamp
 * played on an amp sound. The two stubs do there what the synth's two do in the voice path:
 *
 *   0x8d504c7c  the load of the word at 0x84a03c84 into R7, the first instruction behind the loop that puts the block
 *               onto the buses; the AMP path tests R7 for zero and skips its own effect call on zero. LIVE there: R0
 *               to R3, P1 and P3 (the loop's last samples are stored by the instructions after it). Dead: R7 (the load
 *               writes it), P0 and P2 (set again or never read before the function's end), CC.
 *               -> op1f_fx_amp_t0: CYCLES into op1f_fx_t0[0], as op1f_fx_synth_t0 stamps it, and a zero in R7; it
 *               writes R7 and P0 alone.
 *   0x8d504cba  the load of the left bus's address into R2, the first instruction behind the effect's call, reached
 *               after the call, after its skip (the effect switched off) and when there is no object. Dead there: R0
 *               to R3, P0 to P2, CC. LIVE: P4 and P5 (the two buses, read by the function's end), which a C call
 *               keeps. -> op1f_fx_amp_t1: op1f_fx_chain_run(0), then the load.
 *
 * op1f_fx_chain_run(0) is the synth's run, unchanged: the same buses, the same object, the same words, the built slot
 * read where the AMP path reads it, and the lfo pack's call first (op1f_lfo4_block). A sound that rides no train gets
 * the slot's effect through the object exactly as the AMP path would have called it, so an amp sound on one effect
 * sounds as before. RETS is free at both places: the function saved it with its LINK. The end stub gives the run its
 * twelve bytes of argument space, as the other two do (I5).
 *
 * What fx train 18's unit said follows and holds.
 *
 * Build H images H1a and H1: the four stubs around stock's instrument effect call. Profile group "build-h1a". Each
 * replaces one 8-byte instruction inside a LINKed L1 function with
 * a CALL.a (the function's RETS is saved in its frame, so the CALL's RETS write is harmless).
 *
 * H1a timed stock's call: the start stubs stamped CYCLES and performed the displaced load, the end stubs accounted.
 * H1 owns the chain: the start stubs stamp CYCLES and hand stock a null producer (R0 = 0 for the synth, R1 = 0 for
 * the drum), so stock's test of that register for zero skips its own effect call and falls to the end site, where the end stubs
 * call op1f_fx_chain_run(host) (Build H's chain unit), which runs the coupled wagons in order on the two buses, the
 * mirrored wagon through stock's own producer object with stock's ctx, the others through the module's instances.
 *
 *   0x11a052b4  the load of the word at 0x84a03c84 into R0, in the synth second stage 0x11a04130: the producer's
 *               read before the fx_active test. Dead there: R1-R3, P0-P2, CC (the code after sets them before
 *               use; the vocoder path arrives from a call). -> op1f_fx_synth_t0: CYCLES into op1f_fx_t0[0], and
 *               a zero in R0.
 *   0x11a052e6  the load of 0x84a01554 into P0, the first instruction after the effect call's return
 *               (0x11a0548a jumps here) or after its skip. Dead there: R0-R3, P0-P2, CC. -> op1f_fx_synth_t1:
 *               op1f_fx_chain_run(0), then the load.
 *   0x11a03e34  the load of the word at 0x84a03c84 into R1, in the drum second stage 0x11a03670: the producer's
 *               read. R0 and P0 are LIVE there (the next instruction stores the last bus sample through P0):
 *               the stub touches only R1, R2 and P1, which the code after sets before use.
 *               -> op1f_fx_drum_t0: CYCLES into op1f_fx_t0[1], and a zero in R1.
 *   0x11a03e68  the load of 0x84a01564 into P0, after the drum's effect call (0x11a0401a jumps here) or its skip.
 *               Dead there: R0-R3, P0-P2, CC. -> op1f_fx_drum_t1: op1f_fx_chain_run(1), then the load.
 *
 * The chain call is an ordinary C call, as the stubs of build_b_record.s make them: R0-R3, P0-P2 and RETS are
 * scratch in the ABI, the callee keeps R4-R7 and P3-P5, which stock holds live across the sites (the bus pointers
 * P3, P4, P5). No loop registers are in flight at the sites (every LSETUP there is after them), and the accumulators
 * and DAG registers are not live. The buses the chain processes are the ones stock's effect processed: the left at
 * 0x11902ba0 and the right at 0x11902ca0, 64 words each, in place.
 *
 * I5: the two end stubs give the chain's run the twelve bytes of argument space the calling convention asks of a
 * caller. A compiled function may store its register arguments there, and without the space that store would land
 * on the stub's saved RETS. The run has kept its argument in a register in every image so far (read in each
 * listing since I5); it changes with nearly every image, so the space is given rather than relied on.
 */
.text
.align 2

.set SYNTH_SLOT_BASE, 0x84a01554
.set DRUM_SLOT_BASE, 0x84a01564

/* Replaces the load of the word at 0x84a03c84 into R0 at 0x11a052b4. */
.global op1f_fx_synth_t0
.type op1f_fx_synth_t0, STT_FUNC
op1f_fx_synth_t0:
    R1 = CYCLES;
    P0.H = op1f_fx_t0;
    P0.L = op1f_fx_t0;
    [P0] = R1;
    R0 = 0;                             /* no producer for stock: the module runs the chain at the end site */
    RTS;
.size op1f_fx_synth_t0, .-op1f_fx_synth_t0

/* Replaces the load of 0x84a01554 into P0 at 0x11a052e6. */
.global op1f_fx_synth_t1
.type op1f_fx_synth_t1, STT_FUNC
op1f_fx_synth_t1:
    [--SP] = RETS;
    SP += -12;                          /* the callee's argument space (I5) */
    R0 = 0;
    CALL op1f_fx_chain_run;             /* void (int host): the chain on the buses, the section's cycles counted */
    SP += 12;
    RETS = [SP++];
    P0.H = SYNTH_SLOT_BASE;
    P0.L = SYNTH_SLOT_BASE;
    RTS;
.size op1f_fx_synth_t1, .-op1f_fx_synth_t1

/* Replaces the load of the word at 0x84a03c84 into R1 at 0x11a03e34. R0 and P0 stay untouched. */
.global op1f_fx_drum_t0
.type op1f_fx_drum_t0, STT_FUNC
op1f_fx_drum_t0:
    R2 = CYCLES;
    P1.H = op1f_fx_t0;
    P1.L = op1f_fx_t0;
    [P1 + 4] = R2;
    R1 = 0;
    RTS;
.size op1f_fx_drum_t0, .-op1f_fx_drum_t0

/* Replaces the load of 0x84a01564 into P0 at 0x11a03e68. */
.global op1f_fx_drum_t1
.type op1f_fx_drum_t1, STT_FUNC
op1f_fx_drum_t1:
    [--SP] = RETS;
    SP += -12;
    R0 = 1;
    CALL op1f_fx_chain_run;
    SP += 12;
    RETS = [SP++];
    P0.H = DRUM_SLOT_BASE;
    P0.L = DRUM_SLOT_BASE;
    RTS;
.size op1f_fx_drum_t1, .-op1f_fx_drum_t1

/* 19: replaces the load of the word at 0x84a03c84 into R7 at 0x8d504c7c (the AMP path). R0 to R3, P1, P3 stay. */
.global op1f_fx_amp_t0
.type op1f_fx_amp_t0, STT_FUNC
op1f_fx_amp_t0:
    R7 = CYCLES;
    P0.H = op1f_fx_t0;
    P0.L = op1f_fx_t0;
    [P0] = R7;
    R7 = 0;                             /* no object for the AMP path: the module runs the chain at the next place */
    RTS;
.size op1f_fx_amp_t0, .-op1f_fx_amp_t0

/* 19: replaces the load of 0x11902ba0 into R2 at 0x8d504cba (the AMP path, behind the effect's call). */
.global op1f_fx_amp_t1
.type op1f_fx_amp_t1, STT_FUNC
op1f_fx_amp_t1:
    [--SP] = RETS;
    SP += -12;                          /* the callee's argument space (I5) */
    R0 = 0;                             /* the synth's chain */
    CALL op1f_fx_chain_run;
    SP += 12;
    RETS = [SP++];
    R2.H = 0x1190;
    R2.L = 0x2ba0;
    RTS;
.size op1f_fx_amp_t1, .-op1f_fx_amp_t1
