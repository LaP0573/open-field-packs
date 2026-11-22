/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build B, step B3a: register-preserving stubs for the tape screen's glyph table (OS 1.7.0+0). Logic in build_b_mix.cpp.
 *
 * TapeScreen's update (record 309) shows the selected track's number by indexing four svg node pointers at
 * this + 0x34 with the track: at 0x8fc8428c it hides the one shown before, at 0x8fc842a0 it remembers the new
 * track at this + 0xd0 and addresses its node pointer. Both 8-byte spans become CALL.a stubs (profile group
 * "build-b3a"):
 *
 *   at 0x8fc8428c  the table's entry for the index in P1 is fetched and that node's display byte is cleared;
 *                  P1 holds the index shown before, P4 the screen and R7 zero.
 *   at 0x8fc842a0  the track to show, which is in R0, is stored at this + 0xd0 and P1 is made the address of
 *                  its entry in the table; P4 holds the screen.
 *
 * R0-R3, P0-P2 and CC are dead at both sites; R6, R7, P4 and P5 are
 * callee-saved and survive the C call; the function is LINKed, so RETS only needs saving around the call.
 */
.section .text
.align 2

.global op1f_tape_glyph_off
.type op1f_tape_glyph_off, STT_FUNC
op1f_tape_glyph_off:
    [--SP] = RETS;
    R0 = P4;                            /* the screen */
    R1 = P1;                            /* the index shown before */
    CALL op1f_tape_glyph_hide;          /* void (void* screen, int index) */
    RETS = [SP++];
    RTS;
.size op1f_tape_glyph_off, .-op1f_tape_glyph_off

.global op1f_tape_glyph_on
.type op1f_tape_glyph_on, STT_FUNC
op1f_tape_glyph_on:
    [--SP] = RETS;
    R1 = R0;                            /* the track to show */
    R0 = P4;                            /* the screen */
    CALL op1f_tape_glyph_slot;          /* void** (void* screen, int track): stores this + 0xd0, returns the slot */
    P1 = R0;
    RETS = [SP++];
    RTS;
.size op1f_tape_glyph_on, .-op1f_tape_glyph_on
