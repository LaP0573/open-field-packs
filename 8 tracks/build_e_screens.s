/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build E: the two cosmetic screens for tracks 5 and 6 (OS 1.7.0+0).
 * Two 8-byte CALL.a retargets in record 309; the logic is in build_e_screens.cpp.
 *
 *      help screen update   0x8fc2e5bc:  the call of 0x8d514348 (the selected-track getter) -> CALL.a op1f_help_track.
 *                           The hardware loop that follows colours the four legend boxes t1tape..t4tape: the
 *                           highlight word at this + 0xa4 for the box whose index equals the returned value,
 *                           the normal word at this + 0xa8 for the others (P4 = this at the site, untouched by
 *                           the call). For tracks 4-5 the module returns track - 4 with its own colour word at
 *                           this + 0xa4; for 0-3 stock's word is back and the track returned as the getter would.
 *      signal-flow update   0x8fc3574e:  the call of 0x8d516884 (ReadInterface is-unmuted) -> CALL.a op1f_signalflow_unmuted.
 *                           R0 holds the selected track, R7 the eight-float vector of 0x8d5168f4, and the stack word at SP + 0x34 the
 *                           track again, which stock then uses as the index into the vector for its "level low"
 *                           hint. Tracks 0-3 go on to the module's six-track getter as before; for 4-5 the
 *                           module answers the mute from its strip and turns the index into one that lands on
 *                           the module's float of that track's volume.
 */
.section .text
.align 2

.global op1f_help_track
.type op1f_help_track, STT_FUNC
op1f_help_track:
    R0 = P4;                            /* the HelpScreen object */
    P2.H = op1f_help_track_ext;
    P2.L = op1f_help_track_ext;
    JUMP (P2);                          /* returns R0 = the track to highlight, to stock's RETS */
.size op1f_help_track, .-op1f_help_track

.global op1f_signalflow_unmuted
.type op1f_signalflow_unmuted, STT_FUNC
op1f_signalflow_unmuted:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lsignalflow_ext;
    P2.H = op1f_track_unmuted;          /* tracks 0-3: the six-track getter, as the stock entry dispatches since B3a */
    P2.L = op1f_track_unmuted;
    JUMP (P2);
.Lsignalflow_ext:
    R1 = R7;                            /* the float vector */
    R2 = SP;
    R2 += 0x34;                         /* the caller's slot holding the track, read back as the index */
    P2.H = op1f_signalflow_level_ext;
    P2.L = op1f_signalflow_level_ext;
    JUMP (P2);                          /* returns R0 = unmuted, to stock's RETS */
.size op1f_signalflow_unmuted, .-op1f_signalflow_unmuted

/* Build E4: two more tape meters on the signal-flow screen.
 *      signal-flow update   0x8fc35738:  the call of 0x8d516c90 (ReadInterface's four-float VU vector) -> CALL.a op1f_signalflow_vu.
 *                           P4 holds the screen at the site (loaded from the frame at 0x8fc33b8a, untouched by the calls
 *                           before). The C function returns stock's vector and reshapes the lines tapevu5 and tapevu6
 *                           of the module's signalflow.svg from the module's peaks, with stock's helpers.
 *      op1f_matrix_call     stock's 0x8fc2facc builds a meter's transform from (anchor x, anchor y, value) and
 *                           returns it through a struct pointer in P0; C cannot pass P0, so this stub takes the
 *                           pointer as a fourth argument (on the stack at SP + 0xc) and tail-calls.
 */
.set METER_MATRIX, 0x8fc2facc

.global op1f_signalflow_vu
.type op1f_signalflow_vu, STT_FUNC
op1f_signalflow_vu:
    R0 = P4;                            /* the SignalFlowScreen object */
    P2.H = op1f_signalflow_vu_ext;
    P2.L = op1f_signalflow_vu_ext;
    JUMP (P2);                          /* returns R0 = stock's VU vector, to stock's RETS */
.size op1f_signalflow_vu, .-op1f_signalflow_vu

.global op1f_matrix_call
.type op1f_matrix_call, STT_FUNC
op1f_matrix_call:
    P0 = [SP + 0xc];                    /* the fourth argument: the matrix the method fills */
    P2.H = METER_MATRIX;
    P2.L = METER_MATRIX;
    JUMP (P2);
.size op1f_matrix_call, .-op1f_matrix_call
