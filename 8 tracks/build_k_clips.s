/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build K, pack K4: the clips of tracks 5 to 8 in a list of their own.
 * Profile group build-k4. Four CALL.a stubs, each in place of the 8-byte load of 0x8401ebe8 into R1 (the text
 * "clips") by which one of Clips.cpp's four functions names the tape document's list to the db (db2.cpp). Each
 * stub leaves in R1 the key for the call that follows: stock's own text, or the module's "op1f_clips" for a clip
 * of tracks 5 to 8. The C functions are in build_k_clips.cpp.
 *
 * The four sites, all inside LINKed functions (the function's RETS is in its frame, so the CALL.a's write of
 * RETS is harmless):
 *
 *   the list cleared (the function 0x8d4ed7d4): the site is 0x8d4ee340, the load of the key before the db's
 *               call that clears a list (db, key).                                     <- op1f_clips_key_clear
 *               Nothing is live at the site. The stub has the db clear the list of ours first.
 *
 *   a clip added (the function 0x8d4ed7ec, which builds the json object in its frame and keeps its address in
 *               R7): the site is 0x8d4ee506, the load of the key before the db's call that adds to a list
 *               (db, key, &object).                                                    <- op1f_clips_key_push
 *               R2 and R7 hold the object. The stub asks by the object and puts R2 back from R7.
 *
 *   a clip changed (the function 0x8d4edbfc, the same object in R7): the site is 0x8d4ee91c, the load of the
 *               key before the db's call that changes an entry (db, key, id, &object). <- op1f_clips_key_update
 *               R2 is the id and stock's fourth argument already stands on the stack, twelve bytes up. The stub
 *               keeps R2 on the stack and gives its callee twelve bytes of its own below that, so stock's word
 *               is not touched.
 *
 *   a clip removed (the function 0x8d4ee37e, which has the id in R2): the site is 0x8d4eeeec, the load of the
 *               key before the db's call that takes an entry out (db, key, id).        <- op1f_clips_key_erase
 *               R2 is the id. The stub asks by the id and keeps R2.
 *
 * At each site R0 is loaded right after the stub returns, R1 is the stub's result, and R3, P0 to P2, the loop
 * registers and the flags are dead: a call follows at once. R4 to R7, P3 to P5 and FP are kept by the C functions
 * (the calling convention). Each stub makes the twelve bytes of argument space for its callee after its pushes
 * (the J3 lesson; tools/stub_calls.py holds it).
 */

.section .text
.align 2

/* Replaces the load of 0x8401ebe8 into R1 at 0x8d4ee340. */
.global op1f_clips_key_clear
.type op1f_clips_key_clear, @function
op1f_clips_key_clear:
    [--SP] = RETS;
    SP += -12;
    CALL op1f_clips_clear_ours;             /* the list of ours emptied by the db, as stock's own is next */
    SP += 12;
    RETS = [SP++];
    R1.H = 0x8401;                          /* the displaced load: stock's own text, 0x8401ebe8 */
    R1.L = 0xebe8;
    RTS;
.size op1f_clips_key_clear, .-op1f_clips_key_clear

/* Replaces the load of 0x8401ebe8 into R1 at 0x8d4ee506. */
.global op1f_clips_key_push
.type op1f_clips_key_push, @function
op1f_clips_key_push:
    [--SP] = RETS;
    SP += -12;
    R0 = R7;                                /* the clip's json object, built by stock just above */
    CALL op1f_clips_key_of_object;          /* stock's text or ours, by the object's "ch" */
    SP += 12;
    R1 = R0;
    R2 = R7;                                /* as stock had set it at 0x8d4ed99c */
    RETS = [SP++];
    RTS;
.size op1f_clips_key_push, .-op1f_clips_key_push

/* Replaces the load of 0x8401ebe8 into R1 at 0x8d4ee91c. */
.global op1f_clips_key_update
.type op1f_clips_key_update, @function
op1f_clips_key_update:
    [--SP] = RETS;
    [--SP] = R2;                            /* the id */
    SP += -12;
    R0 = R7;
    CALL op1f_clips_key_of_object;
    SP += 12;
    R1 = R0;
    R2 = [SP++];
    RETS = [SP++];
    RTS;
.size op1f_clips_key_update, .-op1f_clips_key_update

/* Replaces the load of 0x8401ebe8 into R1 at 0x8d4eeeec. */
.global op1f_clips_key_erase
.type op1f_clips_key_erase, @function
op1f_clips_key_erase:
    [--SP] = RETS;
    [--SP] = R2;                            /* the id */
    SP += -12;
    R0 = R2;
    CALL op1f_clips_key_of_id;              /* stock's text or ours, by the track of the store's record */
    SP += 12;
    R1 = R0;
    R2 = [SP++];
    RETS = [SP++];
    RTS;
.size op1f_clips_key_erase, .-op1f_clips_key_erase
