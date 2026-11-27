/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* 2026-11-27: fx train 19 [C]: a third stub, for the synth's AMP path (build_h_fx.s says why), and
 * nothing else changes here. On a sound of TE's amp engine the AMP path (0x8d504a80) applies the sound's LFO to its
 * effect itself: at 0x8d504d90 one instruction adds the slot's offset (R2) to the address of the synth's LFO blocks
 * (R7) into R1 and fetches the object's vtable into P1 (P0 and R0 are the object); then R2 gets the class 2 and the
 * vtable's third entry is called, as in the synth stage. op1f_lfofx_pre_amp does the sum and puts the module's table
 * into P1 in place of the vtable, so the call goes through the synth's wrapper with the same arguments: the LFO's
 * choice of wagon holds after the amp too. Only R1 and P1 are written, the two the instruction writes.
 *
 * What fx train 18's unit said follows and holds.
 *
 * Build I image I2: the stock LFO's effect destination on any wagon of the train. Profile group build-i2: two CALL.a
 * stubs in place of 8-byte loads in the L1 second stages.
 *
 * What stock does there, once a block, when the slot's fx-side LFO object is there and the slot's LFO is on:
 *
 *   synth, from 0x11a054ce on: the object's vtable is fetched into P1 (R0 and P0 are the object); then, at
 *          0x11a05500, the address of the synth's LFO blocks 0x119037e0 is loaded into R1     <- the stub
 *          and the slot's offset is added to it; R2 gets the class, 2 for the synth's effect; then the vtable's
 *          third entry, the apply, is fetched into P1 and called.
 *   drum,  from 0x11a0401c on: the same steps; the site is 0x11a0404e, the load of the address of the drum's
 *          LFO blocks 0x11903ae0 into R1,                                                      <- the stub
 *          and the class is 5.
 *
 * The stub does the displaced load and puts into P1, in place of the object's vtable, a table of the module's
 * whose word at +8 is the wrapper (op1f_lfofx_table_synth / _drum in Build H's chain unit), so stock's own next four
 * instructions call the wrapper with stock's arguments: R0 the object, R1 the slot's LFO block, R2 the class. The
 * wrapper calls the object's real apply itself. Only R1 and P1 are written, both of which stock sets here anyway;
 * RETS is free at the sites (the CALL that follows sets it).
 */
.text
.align 2

/* Replaces the load of 0x119037e0 into R1 at 0x11a05500. */
.global op1f_lfofx_pre_synth
.type op1f_lfofx_pre_synth, STT_FUNC
op1f_lfofx_pre_synth:
    P1.H = op1f_lfofx_table_synth;
    P1.L = op1f_lfofx_table_synth;
    R1.H = 0x1190;
    R1.L = 0x37e0;
    RTS;
.size op1f_lfofx_pre_synth, .-op1f_lfofx_pre_synth

/* Replaces the load of 0x11903ae0 into R1 at 0x11a0404e. */
.global op1f_lfofx_pre_drum
.type op1f_lfofx_pre_drum, STT_FUNC
op1f_lfofx_pre_drum:
    P1.H = op1f_lfofx_table_drum;
    P1.L = op1f_lfofx_table_drum;
    R1.H = 0x1190;
    R1.L = 0x3ae0;
    RTS;
.size op1f_lfofx_pre_drum, .-op1f_lfofx_pre_drum

/* 19: replaces the sum into R1 and the load of the vtable into P1 at 0x8d504d90 (the AMP path). */
.global op1f_lfofx_pre_amp
.type op1f_lfofx_pre_amp, STT_FUNC
op1f_lfofx_pre_amp:
    R1 = R7 + R2 (NS);
    P1.H = op1f_lfofx_table_synth;
    P1.L = op1f_lfofx_table_synth;
    RTS;
.size op1f_lfofx_pre_amp, .-op1f_lfofx_pre_amp
