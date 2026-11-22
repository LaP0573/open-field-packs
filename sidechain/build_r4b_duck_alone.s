/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R4b, the pack sidechain (version 9): J5g's stubs for the sidechain without 8 tracks with the tape load's trampoline and its room
 * taken out, and nothing else changed. Base 26 holds the load's entry, 0x8c0c348c; the ducks go with the tape through
 * level 7 (sidechain 9's values unit). What follows is the elder's description, which holds but for the
 * trampoline's paragraph and its room's.
 *
 * Build J packs J5g: the sidechain without the pack `8 tracks` (OS 1.7.0+0). The duck is called from four places of
 * stock that the tape's units have made theirs since G1 and G2
 * (the tape driver's call of read_block, the signal flow screen's call for its VU vector, the tape json's writer and
 * its loader). Where those units are away, the pack's table puts the sidechain's own routines at the same four
 * sites: its entries there are marked "only when the pack 8 tracks is away", and the base passes them over while
 * that pack runs. The logic is in J5g's unit for the sidechain without 8 tracks; three of the routines need a hand-written part, each
 * the tape units' routine of the same place with the sidechain's function behind it:
 *
 *   op1f_duck_vu           0x8fc35738: the call of 0x8d516c90 (ReadInterface's four-float VU vector) -> CALL.a op1f_duck_vu.
 *                          P4 = the screen at the site, untouched by the calls before (build_e_screens.s,
 *                          op1f_signalflow_vu, since E4). The C function answers stock's vector and draws the page.
 *   op1f_duck_matrix_call  stock's 0x8fc2facc builds a meter's transform from (anchor x, anchor y, value) and returns
 *                          it through a struct pointer in P0; C cannot pass P0, so the pointer comes as a fourth
 *                          argument (on the stack at SP + 0xc). The two instructions of op1f_matrix_call
 *                          (build_e_screens.s, since E4), which the duck uses when the tape's units are there.
 *   op1f_duck_json_load    a JUMP.a at the first 8 bytes of TapeController's load(this, flag) 0x8c0c348c lands here.
 *                          As op1f_tape_json_load (Build B's mixer stubs, since B3d): the caller's RETS and the flag kept,
 *                          RETS pointed at the continuation, and stock entered through the routine's room; stock's
 *                          own return comes back to the continuation with the stack as it
 *                          was at entry, where the C function reads the duck's settings. The stock function takes
 *                          no stack argument, and its result is kept. Here the call of the C function is given
 *                          its twelve bytes of argument space (the rule since J3b).
 *
 *      load  0x8c0c348c  -> its room, then 0x8c0c2ae4
 *
 * The room (packs L3): until L3 op1f_duck_json_load ran the instructions
 * its hook displaced itself, as a copy. Now it holds none of them and ends in a jump to its room: sixteen bytes of
 * the pack's zeroed data into which the base copies those instructions from the running firmware, with a jump back
 * behind them, before it writes the hook (module/build_m4_loader.cpp, make_trampoline; the table entry names the
 * room). The entry is one of the four the base makes only when the pack 8 tracks is away, and the room is filled
 * only then: with that pack there, nothing jumps to this routine and its room stays zero.
 *
 * The two other sites are plain retargets of a call of a C function and need no routine of their own
 * (op1f_duck_read_block, op1f_duck_json_set).
 */
.set METER_MATRIX,       0x8fc2facc

.section .text
.align 2

.global op1f_duck_vu
.type op1f_duck_vu, STT_FUNC
op1f_duck_vu:
    R0 = P4;                            /* the SignalFlowScreen object */
    P2.H = op1f_duck_vu_ext;
    P2.L = op1f_duck_vu_ext;
    JUMP (P2);                          /* returns R0 = stock's VU vector, to stock's RETS */
.size op1f_duck_vu, .-op1f_duck_vu

.global op1f_duck_matrix_call
.type op1f_duck_matrix_call, STT_FUNC
op1f_duck_matrix_call:
    P0 = [SP + 0xc];                    /* the fourth argument: the matrix the method fills */
    P2.H = METER_MATRIX;
    P2.L = METER_MATRIX;
    JUMP (P2);
.size op1f_duck_matrix_call, .-op1f_duck_matrix_call
