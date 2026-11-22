/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R4b, the pack 8 tracks (version 9): Build B's mixer stubs with the tape load's trampoline and its room taken
 * out, and nothing else changed. Base 26 holds the load's entry, 0x8c0c348c; this pack keeps its members with the tape
 * through level 7 (8 tracks 9's values unit). What follows is the elder's description, which holds but for its
 * first point.
 *
 * Build B, step B3d: trampolines and stubs for the tape JSON loader and the MixerController (OS 1.7.0+0).
 * Logic in Build B's mixer unit.
 *
 * 1. The loader post-wrap. A JUMP.a at the first 8 bytes of TapeController's load(this, flag) 0x8c0c348c lands
 *    here. The stub keeps the caller's RETS and the flag in op1f_json_load_state, points RETS at its own
 *    continuation and goes into stock through its room; stock's own return then comes back to the
 *    continuation with the stack as it was at entry, where the C function reads the extension strip.
 *    The stock function uses no stack argument (only its own spill of R0 in the caller's argument space), and
 *    its result is unused by both callers; it is kept anyway.
 *
 *      load  0x8c0c348c  -> its room, then 0x8c0c2ae4
 *
 * 2. Entry trampolines at four MixerController handlers, the construction of B2's TapeController trampolines:
 *    the C filter decides, a taken event returns 1 to the dispatcher, otherwise the stub goes on through its
 *    room and stock continues. R0-R3, P0-P2 and RETS are scratch around the C call; the arguments are saved.
 *
 *      key-down          0x8c0bbef8  -> its room, then 0x8c0bb5c8
 *      key-up            0x8c0bbe00  -> its room, then 0x8c0bb4d4 (the instructions under the hook end twelve
 *                        bytes in: the second is eight bytes long and begins inside the hook's eight)
 *      shifted key-down  0x8c0bb9b0  -> its room, then 0x8c0bb080
 *      on_encoder        0x8c0bb294  -> its room, then 0x8c0ba964
 *
 *    The rooms (packs L3): until L3 each of these five stubs ran the
 *    instructions its hook displaced itself, as a copy or said in other words. Now it holds none of them and ends
 *    in a jump to its room: zeroed data of the pack into which the base copies those instructions from the
 *    running firmware, with a jump back behind them, before it writes the hook (module/build_m4_loader.cpp,
 *    make_trampoline; the table entry names the room and how many bytes the instructions take). P2, the register
 *    a stub jumps through, is the one it jumped through before.
 *
 * 3. A load replacement in the MixerScreen's update: the 8-byte load of 0x11b01eb0 into P4 at 0x8fc30340, the base of the
 *    four level numbers, becomes a CALL.a to op1f_mixer_levels_p4, which loads P4 with the module's strip while
 *    the page is held. R0 and CC are dead at the site (both set before their next use); the function is LINKed.
 */
.set STOCK_MIXER_PARAMS,      0x11b01eb0

.section .text
.align 2

.global op1f_mixer_key_down
.type op1f_mixer_key_down, STT_FUNC
op1f_mixer_key_down:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    CALL op1f_mixer_key_filter;         /* int (void* self, const int* event) */
    R2 = R0;
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R2 == 0;
    IF !CC JUMP .Lkey_consumed;
    P2.H = op1f_mixer_key_down_room;    /* the instructions the hook displaced, then 0x8c0bb5c8 */
    P2.L = op1f_mixer_key_down_room;
    JUMP (P2);
.Lkey_consumed:
    R0 = 1;
    RTS;
.size op1f_mixer_key_down, .-op1f_mixer_key_down

.global op1f_mixer_key_up
.type op1f_mixer_key_up, STT_FUNC
op1f_mixer_key_up:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    CALL op1f_mixer_key_up_filter;      /* int (void* self, int key) */
    R2 = R0;
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R2 == 0;
    IF !CC JUMP .Lkey_up_consumed;
    P2.H = op1f_mixer_key_up_room;      /* the instructions the hook displaced, twelve bytes, then 0x8c0bb4d4 */
    P2.L = op1f_mixer_key_up_room;
    JUMP (P2);
.Lkey_up_consumed:
    R0 = 1;
    RTS;
.size op1f_mixer_key_up, .-op1f_mixer_key_up

.global op1f_mixer_shift_key_down
.type op1f_mixer_shift_key_down, STT_FUNC
op1f_mixer_shift_key_down:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    CALL op1f_mixer_shift_key_filter;   /* int (void* self, const int* event) */
    R2 = R0;
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R2 == 0;
    IF !CC JUMP .Lshift_consumed;
    P2.H = op1f_mixer_shift_key_down_room;   /* the instructions the hook displaced, then 0x8c0bb080 */
    P2.L = op1f_mixer_shift_key_down_room;
    JUMP (P2);
.Lshift_consumed:
    R0 = 1;
    RTS;
.size op1f_mixer_shift_key_down, .-op1f_mixer_shift_key_down

.global op1f_mixer_encoder
.type op1f_mixer_encoder, STT_FUNC
op1f_mixer_encoder:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    [--SP] = R2;
    CALL op1f_mixer_encoder_filter;     /* int (void* self, int index, int delta) */
    R3 = R0;
    R2 = [SP++];
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R3 == 0;
    IF !CC JUMP .Lencoder_consumed;
    P2.H = op1f_mixer_encoder_room;     /* the instructions the hook displaced, then 0x8c0ba964 */
    P2.L = op1f_mixer_encoder_room;
    JUMP (P2);
.Lencoder_consumed:
    R0 = 0;
    RTS;
.size op1f_mixer_encoder, .-op1f_mixer_encoder

.global op1f_mixer_levels_p4
.type op1f_mixer_levels_p4, STT_FUNC
op1f_mixer_levels_p4:
    P4.H = op1f_mixer_page;
    P4.L = op1f_mixer_page;
    R0 = B[P4] (Z);
    CC = R0 == 0;
    P4.H = STOCK_MIXER_PARAMS;
    P4.L = STOCK_MIXER_PARAMS;
    IF CC JUMP .Lp4_stock;
    P4.H = op1f_ext_params;
    P4.L = op1f_ext_params;
.Lp4_stock:
    RTS;
.size op1f_mixer_levels_p4, .-op1f_mixer_levels_p4

/* The rooms: zeroed data, which the base clears when it loads the pack and fills when it applies the table. Each is
 * the bytes its trampoline copies (eight; twelve for the key-up handler) and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_mixer_key_down_room
.type op1f_mixer_key_down_room, STT_OBJECT
op1f_mixer_key_down_room:
    .space 16
.size op1f_mixer_key_down_room, .-op1f_mixer_key_down_room
.global op1f_mixer_key_up_room
.type op1f_mixer_key_up_room, STT_OBJECT
op1f_mixer_key_up_room:
    .space 20
.size op1f_mixer_key_up_room, .-op1f_mixer_key_up_room
.global op1f_mixer_shift_key_down_room
.type op1f_mixer_shift_key_down_room, STT_OBJECT
op1f_mixer_shift_key_down_room:
    .space 16
.size op1f_mixer_shift_key_down_room, .-op1f_mixer_shift_key_down_room
.global op1f_mixer_encoder_room
.type op1f_mixer_encoder_room, STT_OBJECT
op1f_mixer_encoder_room:
    .space 16
.size op1f_mixer_encoder_room, .-op1f_mixer_encoder_room
