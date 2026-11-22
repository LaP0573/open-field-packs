/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build B, step B3b: entry trampolines for the two clip-edge fade writers of tape_c.cpp (OS 1.7.0+0).
 * Logic in build_b_xfade.cpp.
 *
 * A JUMP.a at the first 8 bytes of each stock function lands here with R0 = track, R1 = from (frames),
 * R2 = to (frames) and RETS = the caller's return. A track below 4 goes on through the stub's room and
 * continues in stock; tracks 4-5 (and anything else) tail-jump to the module's version with the registers
 * and the stack untouched. Stock's bodies assert track < 4 a little further on (0x8f01dd00, 0x8f01de0e,
 * fatal), which the module versions never reach; they refuse a bad range with a log line instead.
 *
 *      fade out  0x8f01f7d0  -> its room, then 0x8f01dce8
 *      fade in   0x8f01f8e0  -> its room, then 0x8f01ddf8
 *
 * The rooms (packs L3): until L3 each stub ran the instructions its hook
 * displaced itself, as a copy. Now it holds none of them and ends in a jump to its room, sixteen bytes of the
 * pack's zeroed data into which the base copies those instructions from the running firmware, with a jump back
 * behind them, before it writes the hook (module/build_m4_loader.cpp, make_trampoline; the table entry names the
 * room). P2, the register a stub jumps through, is the one it jumped through before.
 */
.section .text
.align 2

.global op1f_fade_out
.type op1f_fade_out, STT_FUNC
op1f_fade_out:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lfade_out_ext;
    P2.H = op1f_fade_out_room;          /* the instructions the hook displaced, then 0x8f01dce8 */
    P2.L = op1f_fade_out_room;
    JUMP (P2);
.Lfade_out_ext:
    P2.H = op1f_fade_out_ext;
    P2.L = op1f_fade_out_ext;
    JUMP (P2);
.size op1f_fade_out, .-op1f_fade_out

.global op1f_fade_in
.type op1f_fade_in, STT_FUNC
op1f_fade_in:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lfade_in_ext;
    P2.H = op1f_fade_in_room;           /* the instructions the hook displaced, then 0x8f01ddf8 */
    P2.L = op1f_fade_in_room;
    JUMP (P2);
.Lfade_in_ext:
    P2.H = op1f_fade_in_ext;
    P2.L = op1f_fade_in_ext;
    JUMP (P2);
.size op1f_fade_in, .-op1f_fade_in

/* The rooms: zeroed data, which the base clears when it loads the pack and fills when it applies the table. Each is
 * the eight bytes its trampoline copies and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_fade_out_room
.type op1f_fade_out_room, STT_OBJECT
op1f_fade_out_room:
    .space 16
.size op1f_fade_out_room, .-op1f_fade_out_room
.global op1f_fade_in_room
.type op1f_fade_in_room, STT_OBJECT
op1f_fade_in_room:
    .space 16
.size op1f_fade_in_room, .-op1f_fade_in_room
