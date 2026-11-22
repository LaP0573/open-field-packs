/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build D, step D1: the TapeController's lift-all loop bound, the lift-all and drop-all command handlers of
 * audio_clipboard.cpp / tape_c.cpp, and the channel bound of the MIDI mixer handler (OS 1.7.0+0). Logic in
 * build_d_clip.cpp (lift-all, drop-all) and build_b_mix.cpp (op1f_midi_mixer).
 *
 * A JUMP.a over 8 bytes of stock lands here with the registers as stock left them. The two handler
 * trampolines walk the command's vector of tracks (begin at +8, end at +0xc): tracks all below 4 go on through
 * the stub's room and continue in stock; any other tail-jumps to the module's version of the whole
 * handler with R0 (the command) and RETS untouched. R1, P0-P2 and CC are scratch at the entry (the handlers
 * read only R0).
 *
 *      lift-all bound   0x8c0c2676   the test of the loop's count against four, and its step            -> continue at 0x8c0c267e
 *                       (the loop of 0x8c0c1c98 over the tracks with a clip at the cursor: the bound becomes
 *                       6; R0, R4, R6 and CC are what stock computes there, P2 is unused by that function)
 *      lift-all handler 0x8f00b24c  -> its room, then 0x8f009ff4
 *      drop-all handler 0x8f00ac78  -> its room, then 0x8f009a28
 *      MIDI bound       0x8f00f96c   the test of the channel against four, and a load of 9              -> continue at 0x8f00f974
 *                       (R5 = the channel, R4 = the type, R7 = the value; channels 0-3 continue in stock with
 *                       R0 as the displaced load left it; channels 4-5 call op1f_midi_mixer(channel, type,
 *                       value) and leave through stock's epilogue at 0x8f00f9f0 like any other channel; P1,
 *                       the constant pair stock's path still reads, is preserved; P2 is free at the site)
 *
 * The rooms (packs L3): until L3 the two handler stubs ran the instructions
 * their hooks displaced themselves, as a copy. Now each holds none of them and ends in a jump to its room: sixteen
 * bytes of the pack's zeroed data into which the base copies those instructions from the running firmware, with a
 * jump back behind them, before it writes the hook (module/build_m4_loader.cpp, make_trampoline; the table entry
 * names the room). P2, the register a stub jumps through, is the one it jumped through before. The two bounds are
 * hooks in the middle of a function: they do what the instructions under them did with another number, and stay.
 */
.set LIFT_ALL_BOUND_CONTINUE, 0x8c0c267e
.set MIDI_CONTINUE,           0x8f00f974
.set MIDI_RETURN,             0x8f00f9f0

.section .text
.align 2

.global op1f_lift_all_bound
.type op1f_lift_all_bound, STT_FUNC
op1f_lift_all_bound:
    R0 = 8;                             /* the four displaced stock instructions, with the bound 8 (F2) */
    CC = R6 < R0;
    R4 = R6;
    R6 += 1;
    P2.H = LIFT_ALL_BOUND_CONTINUE;
    P2.L = LIFT_ALL_BOUND_CONTINUE;
    JUMP (P2);
.size op1f_lift_all_bound, .-op1f_lift_all_bound

.global op1f_lift_all
.type op1f_lift_all, STT_FUNC
op1f_lift_all:
    P2 = R0;                            /* the command: its vector of tracks begins at +8 and ends at +0xc */
    P0 = [P2 + 0x8];
    P1 = [P2 + 0xc];
.Llift_all_next:
    CC = P0 == P1;
    IF CC JUMP .Llift_all_stock;
    R1 = [P0++];
    CC = R1 < 4 (IU);
    IF !CC JUMP .Llift_all_ext;
    JUMP .Llift_all_next;
.Llift_all_stock:
    P2.H = op1f_lift_all_room;          /* the instructions the hook displaced, then 0x8f009ff4 */
    P2.L = op1f_lift_all_room;
    JUMP (P2);
.Llift_all_ext:
    P2.H = op1f_lift_all_ext;
    P2.L = op1f_lift_all_ext;
    JUMP (P2);
.size op1f_lift_all, .-op1f_lift_all

.global op1f_drop_all
.type op1f_drop_all, STT_FUNC
op1f_drop_all:
    P2 = R0;                            /* the command, as above */
    P0 = [P2 + 0x8];
    P1 = [P2 + 0xc];
.Ldrop_all_next:
    CC = P0 == P1;
    IF CC JUMP .Ldrop_all_stock;
    R1 = [P0++];
    CC = R1 < 4 (IU);
    IF !CC JUMP .Ldrop_all_ext;
    JUMP .Ldrop_all_next;
.Ldrop_all_stock:
    P2.H = op1f_drop_all_room;          /* the instructions the hook displaced, then 0x8f009a28 */
    P2.L = op1f_drop_all_room;
    JUMP (P2);
.Ldrop_all_ext:
    P2.H = op1f_drop_all_ext;
    P2.L = op1f_drop_all_ext;
    JUMP (P2);
.size op1f_drop_all, .-op1f_drop_all

.global op1f_midi_bound
.type op1f_midi_bound, STT_FUNC
op1f_midi_bound:
    R0 = 4;                             /* the displaced compare: channels 0-3 are stock's */
    CC = R5 < R0;
    IF !CC JUMP .Lmidi_ext;
    R0 = 9;                             /* the displaced load stock continues with */
    P2.H = MIDI_CONTINUE;
    P2.L = MIDI_CONTINUE;
    JUMP (P2);
.Lmidi_ext:
    R0 = 8;
    CC = R5 < R0;
    IF !CC JUMP .Lmidi_return;
    R0 = R5;                            /* the channel */
    R1 = R4;                            /* the type: 7 volume, 9 mute, 10 pan */
    R2 = R7;                            /* the value, 0..127 */
    [--SP] = RETS;
    CALL op1f_midi_mixer;
    RETS = [SP++];
.Lmidi_return:
    P2.H = MIDI_RETURN;
    P2.L = MIDI_RETURN;
    JUMP (P2);
.size op1f_midi_bound, .-op1f_midi_bound

/* The rooms: zeroed data, which the base clears when it loads the pack and fills when it applies the table. Each is
 * the eight bytes its trampoline copies and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_lift_all_room
.type op1f_lift_all_room, STT_OBJECT
op1f_lift_all_room:
    .space 16
.size op1f_lift_all_room, .-op1f_lift_all_room
.global op1f_drop_all_room
.type op1f_drop_all_room, STT_OBJECT
op1f_drop_all_room:
    .space 16
.size op1f_drop_all_room, .-op1f_drop_all_room
