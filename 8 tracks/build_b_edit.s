/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build B, step B3c: entry trampolines for the lift, drop and clear paths of tape_c.cpp / audio_clipboard.cpp
 * and for the sampler import (OS 1.7.0+0). Logic in build_b_edit.cpp.
 *
 * A JUMP.a at the first 8 bytes of each stock function lands here with the arguments and RETS as the caller
 * left them. A call that stock serves (every track below 4) goes on through the stub's room and continues at
 * the instruction after the ones the hook displaced; the others tail-jump to the module's version with the
 * registers and the stack untouched. R1-R3, P0-P2 and CC are scratch at a function entry (the functions take at
 * most one argument register beyond the ones the tests read), which the vector walks use.
 *
 *      post_lift     0x8f00c00c  -> its room, then 0x8f00ad14
 *                    for a track of 4 to 7: the frame and the moves the range test leaves standing, in this
 *                    file's own order, then 0x8f00c036, past the assert (see op1f_post_lift)
 *      lift helper   0x8f0209f0  -> its room, then 0x8f01ef08
 *      drop copy     0x8f021038  -> its room, then 0x8f01f550
 *      drop mix      0x8f020d90  -> its room, then 0x8f01f2a8
 *      range clear   0x8f01f73c  -> its room, then 0x8f01dc54
 *      to sample     0x8f004204  -> its room, then 0x8f003050 (the instructions under the hook end twelve
 *                    bytes in: the second is eight bytes long and begins inside the hook's eight)
 *
 * The rooms (packs L3): until L3 each stub ran the instructions its hook
 * displaced itself, as a copy or said in other words. Now it holds none of them on its way into stock and ends in
 * a jump to its room: zeroed data of the pack into which the base copies those instructions from the running
 * firmware, with a jump back behind them, before it writes the hook (module/build_m4_loader.cpp, make_trampoline;
 * the table entry names the room and how many bytes the instructions take). P2, the register a stub jumps
 * through, is the one it jumped through before.
 */
.set POST_LIFT_CHECKED,       0x8f00c036
.set CLIPBOARD_VECTOR,        0x84bdd948   /* the clipboard state's vector: begin, then end at +4 */

.section .text
.align 2

/* The lift poster. A track below 4 is stock's from its first instruction on. A track of 4 to 7 cannot go that
 * way: right behind the hook stock tests the track against 4 and stops the OS. So this path does not use the
 * room. It joins stock behind that test, at 0x8f00c036, and has to bring what stock's code from there on works
 * with: the frame (its size, the four registers its epilogue takes back, the room for arguments), the track in the
 * caller's argument space, where stock reads it again at 0x8f00ad82 for the command's vector, and start, end and a
 * zero in the registers stock keeps them in. The steps are in this file's own order, the track kept first; none of
 * them can be left out. Until L3 this path began with the hook's displaced instructions as they stand in stock. */
.global op1f_post_lift
.type op1f_post_lift, STT_FUNC
op1f_post_lift:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lpost_lift_ext;
    P2.H = op1f_post_lift_room;         /* the instructions the hook displaced, then 0x8f00ad14 */
    P2.L = op1f_post_lift_room;
    JUMP (P2);
.Lpost_lift_ext:
    LINK 0x54;                          /* the frame stock's poster works in */
    [FP + 0xc] = R0;                    /* the track */
    [--SP] = (R7:4);                    /* what its epilogue takes back */
    R6 = R2;                            /* end */
    R5 = R1;                            /* start */
    R7 = 0;
    SP += -0x18;                        /* its room for arguments */
    P2.H = POST_LIFT_CHECKED;
    P2.L = POST_LIFT_CHECKED;
    JUMP (P2);
.size op1f_post_lift, .-op1f_post_lift

.global op1f_lift
.type op1f_lift, STT_FUNC
op1f_lift:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Llift_ext;
    P2.H = op1f_lift_room;              /* the instructions the hook displaced, then 0x8f01ef08 */
    P2.L = op1f_lift_room;
    JUMP (P2);
.Llift_ext:
    P2.H = op1f_lift_ext;
    P2.L = op1f_lift_ext;
    JUMP (P2);
.size op1f_lift, .-op1f_lift

.global op1f_drop_copy
.type op1f_drop_copy, STT_FUNC
op1f_drop_copy:
    CC = R0 < 4 (IU);                   /* src */
    IF !CC JUMP .Ldrop_copy_ext;
    CC = R1 < 4 (IU);                   /* dst */
    IF !CC JUMP .Ldrop_copy_ext;
    P2.H = op1f_drop_copy_room;         /* the instructions the hook displaced, then 0x8f01f550 */
    P2.L = op1f_drop_copy_room;
    JUMP (P2);
.Ldrop_copy_ext:
    P2.H = op1f_drop_copy_ext;
    P2.L = op1f_drop_copy_ext;
    JUMP (P2);
.size op1f_drop_copy, .-op1f_drop_copy

.global op1f_drop_mix
.type op1f_drop_mix, STT_FUNC
op1f_drop_mix:
    P2 = R0;                            /* params */
    R1 = [P2 + 0x4];                    /* dst */
    CC = R1 < 4 (IU);
    IF !CC JUMP .Ldrop_mix_ext;
    P1 = [P2];                          /* the clipboard state: its vector begins at +4, ends at +8 */
    P0 = [P1 + 0x4];
    P1 = [P1 + 0x8];
.Ldrop_mix_next:
    CC = P0 == P1;
    IF CC JUMP .Ldrop_mix_stock;
    R1 = [P0++];
    CC = R1 < 4 (IU);
    IF !CC JUMP .Ldrop_mix_ext;
    JUMP .Ldrop_mix_next;
.Ldrop_mix_stock:
    P2.H = op1f_drop_mix_room;          /* the instructions the hook displaced, then 0x8f01f2a8 */
    P2.L = op1f_drop_mix_room;
    JUMP (P2);
.Ldrop_mix_ext:
    P2.H = op1f_drop_mix_ext;
    P2.L = op1f_drop_mix_ext;
    JUMP (P2);
.size op1f_drop_mix, .-op1f_drop_mix

.global op1f_clear_range
.type op1f_clear_range, STT_FUNC
op1f_clear_range:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lclear_range_ext;
    P2.H = op1f_clear_range_room;       /* the instructions the hook displaced, then 0x8f01dc54 */
    P2.L = op1f_clear_range_room;
    JUMP (P2);
.Lclear_range_ext:
    P2.H = op1f_clear_range_ext;
    P2.L = op1f_clear_range_ext;
    JUMP (P2);
.size op1f_clear_range, .-op1f_clear_range

.global op1f_clip_to_sample
.type op1f_clip_to_sample, STT_FUNC
op1f_clip_to_sample:
    P2.H = CLIPBOARD_VECTOR;
    P2.L = CLIPBOARD_VECTOR;
    P0 = [P2];
    P1 = [P2 + 0x4];
.Lclip_to_sample_next:
    CC = P0 == P1;
    IF CC JUMP .Lclip_to_sample_stock;
    R1 = [P0++];
    CC = R1 < 4 (IU);
    IF !CC JUMP .Lclip_to_sample_refuse;
    JUMP .Lclip_to_sample_next;
.Lclip_to_sample_stock:
    P2.H = op1f_clip_to_sample_room;    /* the instructions the hook displaced, twelve bytes, then 0x8f003050 */
    P2.L = op1f_clip_to_sample_room;
    JUMP (P2);
.Lclip_to_sample_refuse:                /* Build D: the module's render, no longer a refusal */
    P2.H = op1f_clip_to_sample_ext;
    P2.L = op1f_clip_to_sample_ext;
    JUMP (P2);
.size op1f_clip_to_sample, .-op1f_clip_to_sample

/* The rooms: zeroed data, which the base clears when it loads the pack and fills when it applies the table. Each is
 * the bytes its trampoline copies (eight; twelve for the sampler import) and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_post_lift_room
.type op1f_post_lift_room, STT_OBJECT
op1f_post_lift_room:
    .space 16
.size op1f_post_lift_room, .-op1f_post_lift_room
.global op1f_lift_room
.type op1f_lift_room, STT_OBJECT
op1f_lift_room:
    .space 16
.size op1f_lift_room, .-op1f_lift_room
.global op1f_drop_copy_room
.type op1f_drop_copy_room, STT_OBJECT
op1f_drop_copy_room:
    .space 16
.size op1f_drop_copy_room, .-op1f_drop_copy_room
.global op1f_drop_mix_room
.type op1f_drop_mix_room, STT_OBJECT
op1f_drop_mix_room:
    .space 16
.size op1f_drop_mix_room, .-op1f_drop_mix_room
.global op1f_clear_range_room
.type op1f_clear_range_room, STT_OBJECT
op1f_clear_range_room:
    .space 16
.size op1f_clear_range_room, .-op1f_clear_range_room
.global op1f_clip_to_sample_room
.type op1f_clip_to_sample_room, STT_OBJECT
op1f_clip_to_sample_room:
    .space 20
.size op1f_clip_to_sample_room, .-op1f_clip_to_sample_room
