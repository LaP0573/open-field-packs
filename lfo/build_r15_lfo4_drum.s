/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R15, the pack `lfo` 12: the three LFOs that are not stock's own, on the drum (OS 1.7.0 addresses, read again
 * 2026-11-24). Five CALL.a stubs, each in place of one 8-byte instruction in the drum's stage 0x11a03670, whose
 * frame holds its return address, so the stub's call writes over nothing the stage needs. The drum's reset
 * 0x8d50441a(voice) needs no assembly: its two calls 0x8f0075fc and 0x8f008338 (CALL.a, the voice in R0) are
 * retargeted to op1f_lfo4_reset_drum in build_i_lfo4.cpp.
 *
 * What stock does there. The stage's loop over the eight voices runs 0x11a036f8 to 0x11a03976 with the voice in R7,
 * a loop of its own counting that no hardware loop spans. A silent voice (its byte from 0x11902de4 clear) jumps from
 * 0x11a036fe to the loop's end; every sounding voice passes S1, S2 and S3:
 *
 *   0x11a03790  kind 0 falls to S1; kinds 1 to 5 go to 0x11a04122: 0x8d503d5c(R7), stock's class 3 on the scratch,
 *               then back to S1 (0x11a0412c)
 *   S1 0x11a037c8  the load of the scratch's first word, at 0x11904754, into R1, for the voice's pitch (0x8d5033b4
 *               after). Dead: R0-R3, P0-P2 (each set before it is read: 0x11a037a0 to 0x11a037be), CC (0x11a037f8).
 *               Live: R6 (1, for a compare at 0x11a037e0), R7, P3 to P5, FP.          -> op1f_lfo4_drum_s1_stub
 *   0x11a03806, 0x11a0381e  no LFO object or the slot's LFO off fall to S2; else 0x11a04110: the velocity method,
 *               its answer in R2, back to S2 (0x11a04120)
 *   S2 0x11a03858  the load of the pitch buffer's address, 0x11903c04, into P1; the buffer is filled from R5
 *               (0x11a03834), then the level buffer 0x11904404 from the level in R2 (0x11a03836). Live: R2 (the
 *               level, its low half; the high half junk from the call before), R5, R6, R7, P3 to P5. Dead: R0, R1,
 *               R3, P0, P2, CC.                                                      -> op1f_lfo4_drum_s2_stub
 *   S3 0x11a0387a  the load of the kind table's address, 0x119035c0, into P0, for stock's tremolo test (0x11a03858;
 *               tremolo: 0x11a04104, 0x8d503d5c(R7) on the filled buffers). Live: P1 (the pointer 0x84a01528, read
 *               from the frame at 0x11a03852), R6, R7, P3 to P5. Dead: R0-R3, P2, CC.  -> op1f_lfo4_drum_s3_stub
 *
 * After the loop, once a block:
 *
 *   0x11a039fe, 0x11a03a0e  no fx-side object [0x84a01574] or the LFO off fall to C4; else 0x11a04062: stock's
 *               class 4 on the fx-side object (the scratch filled from 0x119038e0 + 32 x slot), the page copy
 *               (slot, 0, 4) on a non-zero answer, back to C4 (0x11a04078, 0x11a04098)
 *   C4 0x11a03a48  the load of the stage's cycle stamps' address, 0x0801b2c4, into P0. Live: P5 only of the
 *               registers; R0-R3, P0-P2, CC and R4, R6, R7 are set again before they are read.
 *                                                                                   -> op1f_lfo4_drum_c4_stub
 *   0x11a03dd0  stock's class 5 at 0x11a0401c (the fx words 0x119039e0 + 32 x slot), back to 0x11a03dd2
 *   F5 0x11a03e34  the load of the effect's producer, the word at 0x84a03c84, into R1, right after the hardware loop
 *               that ends at 0x11a03dfc (no loop in flight): fx train's site op1f_fx_drum_t0. R0 and P0 are LIVE (the
 *               next instruction stores the last bus sample through P0): the stub keeps both. Dead: R1-R3, P1, P2,
 *               CC. The table places it only when fx train is away; with fx train there its chain's run calls
 *               op1f_lfo4_block(1) at its own end site 0x11a03e68.                     -> op1f_lfo4_drum_f5_stub
 *
 * Each stub tests op1f_lfo4_drum_live (build_i_lfo4.cpp: how many drum LFOs of the module's are published) first and
 * leaves after that one test while it is 0. The C functions are ordinary calls with the caller's twelve bytes of
 * argument space; the callee keeps R4-R7 and P3-P5, which the stage holds live across the sites. Plain numbers in
 * the compares (build card).
 */

.text
.align 2

/* Replaces the load of the word at 0x11904754 into R1 at 0x11a037c8. */
.global op1f_lfo4_drum_s1_stub
.type op1f_lfo4_drum_s1_stub, STT_FUNC
op1f_lfo4_drum_s1_stub:
    P1.H = op1f_lfo4_drum_live;
    P1.L = op1f_lfo4_drum_live;
    R1 = [P1];
    CC = R1 == 0;
    IF CC JUMP .Ls1_load;               /* no drum LFO of the module's */
    [--SP] = RETS;
    SP += -12;                          /* the callee's argument space */
    R0 = R7;                            /* the voice */
    CALL op1f_lfo4_drum_s1;             /* int (int voice): class 3 of the extras of kinds 1 to 5, on the scratch */
    SP += 12;
    RETS = [SP++];
.Ls1_load:
    P1.H = 0x1190;                      /* the displaced load, after the extras moved the scratch */
    P1.L = 0x4754;
    R1 = [P1];
    RTS;
.size op1f_lfo4_drum_s1_stub, .-op1f_lfo4_drum_s1_stub

/* Replaces the load of 0x11903c04 into P1 at 0x11a03858. */
.global op1f_lfo4_drum_s2_stub
.type op1f_lfo4_drum_s2_stub, STT_FUNC
op1f_lfo4_drum_s2_stub:
    P1.H = op1f_lfo4_drum_live;
    P1.L = op1f_lfo4_drum_live;
    R1 = [P1];
    CC = R1 == 0;
    IF CC JUMP .Ls2_load;
    [--SP] = RETS;
    [--SP] = R2;                        /* the level: its high half goes back as it was */
    SP += -12;
    R0 = R7;                            /* the voice */
    R1 = R2;                            /* the level, in the low half */
    CALL op1f_lfo4_drum_s2;             /* int (int voice, int level): the level, in the low half */
    SP += 12;
    R2 = [SP++];
    R2 = PACK (R2.H, R0.L);             /* the extras' velocity methods' answer into the low half */
    RETS = [SP++];
.Ls2_load:
    P1.H = 0x1190;                      /* the displaced load */
    P1.L = 0x3c04;
    RTS;
.size op1f_lfo4_drum_s2_stub, .-op1f_lfo4_drum_s2_stub

/* Replaces the load of 0x119035c0 into P0 at 0x11a0387a. */
.global op1f_lfo4_drum_s3_stub
.type op1f_lfo4_drum_s3_stub, STT_FUNC
op1f_lfo4_drum_s3_stub:
    P0.H = op1f_lfo4_drum_live;
    P0.L = op1f_lfo4_drum_live;
    R0 = [P0];
    CC = R0 == 0;
    IF CC JUMP .Ls3_load;
    [--SP] = RETS;
    [--SP] = P1;                        /* [FP + 0x10], read by the next instruction */
    SP += -12;
    R0 = R7;                            /* the voice */
    CALL op1f_lfo4_drum_s3;             /* void (int voice): class 3 of the extra tremolos, on the filled buffers */
    SP += 12;
    P1 = [SP++];
    RETS = [SP++];
.Ls3_load:
    P0.H = 0x1190;                      /* the displaced load */
    P0.L = 0x35c0;
    RTS;
.size op1f_lfo4_drum_s3_stub, .-op1f_lfo4_drum_s3_stub

/* Replaces the load of 0x0801b2c4 into P0 at 0x11a03a48. */
.global op1f_lfo4_drum_c4_stub
.type op1f_lfo4_drum_c4_stub, STT_FUNC
op1f_lfo4_drum_c4_stub:
    P0.H = op1f_lfo4_drum_live;
    P0.L = op1f_lfo4_drum_live;
    R0 = [P0];
    CC = R0 == 0;
    IF CC JUMP .Lc4_load;
    [--SP] = RETS;
    SP += -12;
    CALL op1f_lfo4_drum_c4;             /* void (): class 4 of the extras with an fx-side object, on the scratch */
    SP += 12;
    RETS = [SP++];
.Lc4_load:
    P0.H = 0x0801;                      /* the displaced load */
    P0.L = 0xb2c4;
    RTS;
.size op1f_lfo4_drum_c4_stub, .-op1f_lfo4_drum_c4_stub

/* Replaces the load of the word at 0x84a03c84 into R1 at 0x11a03e34. R0 and P0 come back as they were. */
.global op1f_lfo4_drum_f5_stub
.type op1f_lfo4_drum_f5_stub, STT_FUNC
op1f_lfo4_drum_f5_stub:
    P1.H = op1f_lfo4_drum_live;
    P1.L = op1f_lfo4_drum_live;
    R1 = [P1];
    CC = R1 == 0;
    IF CC JUMP .Lf5_load;
    [--SP] = RETS;
    [--SP] = R0;                        /* the last bus sample */
    [--SP] = P0;                        /* where it goes */
    SP += -12;
    R0 = 1;                             /* the drum */
    CALL op1f_lfo4_block;               /* void (int host): class 5 of the extras, on the scratch, before stock's effect */
    SP += 12;
    P0 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
.Lf5_load:
    P1.H = 0x84a0;                      /* the displaced load: stock's producer, for stock's test and call */
    P1.L = 0x3c84;
    R1 = [P1];
    RTS;
.size op1f_lfo4_drum_f5_stub, .-op1f_lfo4_drum_f5_stub
