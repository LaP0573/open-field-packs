/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build I images I5 and I6: the LFOs that are not stock's own, on the synth. Profile group build-i5: two CALL.a
 * stubs in place of 8-byte loads in the synth's first stage
 * 0x11a04130 (L1), inside its loop over the six voices, at the two places every sounding voice passes
 * whether or not stock's own LFO runs. (The group's three other hooks are CALL.a retargets of the reset 0x8d504496
 * to the C wrapper op1f_lfo4_reset_synth in build_i_lfo4.cpp; they need no assembly.)
 *
 * What stock does there, for each sounding voice:
 *
 *   the envelope's part
 *     0x11a0427a ..       the envelope's eight words of the slot into the scratch 0x11904754; R5 gets the voice's
 *                         velocity; with stock's LFO object there and the slot's LFO on, 0x11a04c84:
 *                         apply(object, block, 1), then velocity(object, block, the velocity) into R5, back to 0x11a04300
 *     0x11a04300 ..       the three arguments of the envelope's call are loaded: the scratch into R2, the voice's
 *                         record into R0 and, at 0x11a04334, the envelope's buffer 0x11904404 into R1
 *                                                                                 <- op1f_lfo4_env_stub
 *     0x11a0430c          the call of 0x8d5009ea       the envelope, rendered with the scratch
 *
 *   the engine's part
 *     0x11a04450 ..       the engine's eight words into the scratch, the buffers 0x11903c04 and 0x11903e04 filled;
 *                         with the object there and the LFO on, 0x11a04c4e: apply(object, block, 0), back to 0x11a045c0
 *     0x11a045c0          the load of 0x11902dd0 into P0                          <- op1f_lfo4_eng_stub
 *     0x11a04598          from here on the envelope's buffer is multiplied by the voice's level
 *
 * What is live at the sites. Both are reached two ways: straight on, and from the out-of-line path that has just
 * called stock's LFO (0x11a04cc6 jumps to 0x11a04300, 0x11a04c64 and 0x11a04c82 to 0x11a045c0). Whatever is live
 * there has therefore come through a call, so it is in the registers a callee keeps (R4-R7, P3-P5, FP) or is set
 * after the site; no loop register, accumulator or DAG register is in flight (the next LSETUP is after 0x11a045c0),
 * CC is set again before its next use (0x11a04316, 0x11a045e8), and RETS is the function's own in its frame.
 *   0x11a04334: R0 and R2 are set by the two instructions before it, for stock's call that follows: the stub keeps
 *               them. R5 is the voice's level, used at 0x11a0459c: the stub hands it to the C function and takes
 *               the answer back, the place where an LFO's velocity method changes the level. R1 is the displaced
 *               load's; P1 is dead (stock's call follows at once).
 *   0x11a045c0: only the registers a callee keeps (R5 the level, P5 the envelope's buffer). R0-R3, P0-P2 are set
 *               by the code after it before they are read.
 *
 * I6: a sound with no LFO but stock's own leaves each stub after one test of the word op1f_lfo4_live (the number of
 * the module's LFOs that are built and switched on; build_i_lfo4.cpp), five instructions before the displaced load.
 * I5's result (the study's 16.11): the path around a call is what costs, not the call.
 *
 * The C functions are ordinary calls with the caller's twelve bytes of argument space; they take stock's frame
 * pointer, where the loop keeps the voice's number plus one (FP - 0x30).
 */

.text
.align 2

/* Replaces the load of 0x11904404 into R1 at 0x11a04334. */
.global op1f_lfo4_env_stub
.type op1f_lfo4_env_stub, STT_FUNC
op1f_lfo4_env_stub:
    P1.H = op1f_lfo4_live;
    P1.L = op1f_lfo4_live;
    R1 = [P1];
    CC = R1 == 0;
    IF CC JUMP .Lenv_load;              /* no LFO of the module's for this sound */
    [--SP] = RETS;
    [--SP] = R0;                        /* the voice's record: the first argument of stock's call that follows */
    [--SP] = R2;                        /* the scratch: its third */
    SP += -12;
    R0 = FP;                            /* stock's frame */
    R1 = R5;                            /* the voice's level */
    CALL op1f_lfo4_env;                 /* int (u32* frame, int level): the level */
    SP += 12;
    R5 = R0;
    R2 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
.Lenv_load:
    R1.H = 0x1190;                      /* the displaced load */
    R1.L = 0x4404;
    RTS;
.size op1f_lfo4_env_stub, .-op1f_lfo4_env_stub

/* Replaces the load of 0x11902dd0 into P0 at 0x11a045c0. */
.global op1f_lfo4_eng_stub
.type op1f_lfo4_eng_stub, STT_FUNC
op1f_lfo4_eng_stub:
    P1.H = op1f_lfo4_live;
    P1.L = op1f_lfo4_live;
    R1 = [P1];
    CC = R1 == 0;
    IF CC JUMP .Leng_load;
    [--SP] = RETS;
    SP += -12;
    R0 = FP;                            /* stock's frame */
    CALL op1f_lfo4_eng;                 /* void (u32* frame) */
    SP += 12;
    RETS = [SP++];
.Leng_load:
    P0.H = 0x1190;                      /* the displaced load */
    P0.L = 0x2dd0;
    RTS;
.size op1f_lfo4_eng_stub, .-op1f_lfo4_eng_stub
