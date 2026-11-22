/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build B, step B2: register-preserving stubs for the record-track hooks (OS 1.7.0+0). Logic in build_b_record.cpp.
 *
 * Two kinds of stub, both installed by profile group "build-b2":
 *
 * 1. Entry trampolines. A JUMP.a at the first 8 bytes of a TapeController key handler lands here with the
 *    handler's arguments untouched (R0 = this, R1 = key event pointer, RETS = the dispatcher's return). The
 *    stub asks the C filter whether the module takes the key; if so it returns 1 (consumed) to the dispatcher,
 *    otherwise it goes on through its room, so the stock handler runs exactly as before. The filter is an
 *    ordinary C call: R0-R3, P0-P2 and RETS are scratch in the Blackfin ABI, so only RETS and the two arguments
 *    are saved around it.
 *
 *      TapeController::on_key_down        0x8c0c3be2  -> its room, then 0x8c0c321a
 *      TapeController::on_shift_key_down  0x8c0c4422  -> its room, then 0x8c0c3a5a
 *
 *    The rooms (packs L3). Until L3 a stub of this kind ran the
 *    instructions its hook displaced itself, as a copy, and jumped to the instruction after them. Now it holds
 *    none of them: it ends in a jump to its room, sixteen bytes of the pack's zeroed data at the end of this
 *    file. The base (module/build_m4_loader.cpp, make_trampoline) copies the displaced instructions from the
 *    running firmware into the room and puts a jump back behind them, before it writes the hook; the table entry
 *    names the room (the feature's table, made with its sums and its trampolines). The
 *    register a stub jumps through, P2, is the one it jumped through before: free at a function's entry, and
 *    left holding an address stock does not read.
 *
 * 2. Load replacements. Three 8-byte `Preg/Rreg = [0x11905a08]` loads of the selected tape track become a
 *    CALL.a to a stub that performs the load and bounds the value for the stock code that indexes a
 *    four-entry table with it. Each site is inside a LINKed function (RETS is saved in its frame) and the
 *    registers the stub uses as scratch are dead at that site (shown in the design report):
 *
 *      0x11a02b60  the load of the selected track into P0: input monitoring through the selected track's mixer strip (L1 core)
 *      0x11a02d3a  the load of the selected track into P1: the selected track's VU peak word (L1 core)
 *      0x8c0c1bee  the load of the selected track into R1: the "track" field written to user/tapeNN.json
 *
 * B2 clamped tracks 4 and 5 to 3 in all three. Since B3a the two L1 stubs instead redirect tracks 4 and 5 to
 * the module's own strip: stock computes the volume as [0x11b01eb0 + 4k], the pan as [0x11b01eb0 + 4(k + 4)],
 * the mute gain as [0x11b01ed0 + 4k] and the VU word as W[0x11905a34 + 2k] from the index k the stub leaves in
 * P0/P1, so the stub returns k = (&op1f_ext_params[track - 4] - 0x11b01eb0) / 4, respectively
 * (&op1f_ext_vu[track - 4] - 0x11905a34) / 2, and the stock loads and the store land on the module's table
 * (build_b_mix.cpp, laid out as stock's: volumes, pans, mute gains). Tracks 6 and above still read as 3. The
 * JSON stub keeps the clamp, so a tape file never names a track stock firmware lacks.
 *
 * 3. Track dispatch at tape_io_read / tape_io_write (added after the first B2 image aborted at record stop in
 *    tape_io.cpp:89). A JUMP.a at the first 8 bytes of each stock
 *    function lands here with R0 = track, R1 = byte offset, R2 = length and the buffer at [SP + 0xc]; a
 *    track below 4 goes on through the stub's room and continues in stock, any other track tail-jumps to
 *    the module's extension version in build_b_storage.cpp with the registers and the stack untouched.
 *
 *      tape_io_read   0x8fc21bac  -> its room, then 0x8fc200b4
 *      tape_io_write  0x8fc21d1c  -> its room, then 0x8fc20224
 */
.set SELECTED_TRACK,        0x11905a08
.set STOCK_MIXER_PARAMS,    0x11b01eb0
.set STOCK_VU_WORDS,        0x11905a34

.section .text
.align 2

.global op1f_tape_key_down
.type op1f_tape_key_down, STT_FUNC
op1f_tape_key_down:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    CALL op1f_tape_key_filter;          /* int (void* self, const int* event): 1 = the module took the key */
    R2 = R0;
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R2 == 0;
    IF !CC JUMP .Lkey_consumed;
    P2.H = op1f_tape_key_down_room;     /* the instructions the hook displaced, then 0x8c0c321a */
    P2.L = op1f_tape_key_down_room;
    JUMP (P2);
.Lkey_consumed:
    R0 = 1;
    RTS;
.size op1f_tape_key_down, .-op1f_tape_key_down

.global op1f_tape_shift_key_down
.type op1f_tape_shift_key_down, STT_FUNC
op1f_tape_shift_key_down:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    CALL op1f_tape_shift_key_filter;    /* int (void* self, const int* event) */
    R2 = R0;
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R2 == 0;
    IF !CC JUMP .Lshift_consumed;
    P2.H = op1f_tape_shift_key_down_room;   /* the instructions the hook displaced, then 0x8c0c3a5a */
    P2.L = op1f_tape_shift_key_down_room;
    JUMP (P2);
.Lshift_consumed:
    R0 = 1;
    RTS;
.size op1f_tape_shift_key_down, .-op1f_tape_shift_key_down

/* TapeController::on_encoder(this, index, delta) 0x8c0c2e54 -> its room, then 0x8c0c24ac. The dispatcher
 * (0x8c0b145a) ignores the result; the stub returns 0 when the filter swallowed the event (the clip lift that
 * shift + the blue encoder starts on a stopped tape). */
.global op1f_tape_encoder
.type op1f_tape_encoder, STT_FUNC
op1f_tape_encoder:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    [--SP] = R2;
    CALL op1f_tape_encoder_filter;      /* int (void* self, int index, int delta) */
    R3 = R0;
    R2 = [SP++];
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    CC = R3 == 0;
    IF !CC JUMP .Lencoder_consumed;
    P2.H = op1f_tape_encoder_room;      /* the instructions the hook displaced, then 0x8c0c24ac */
    P2.L = op1f_tape_encoder_room;
    JUMP (P2);
.Lencoder_consumed:
    R0 = 0;
    RTS;
.size op1f_tape_encoder, .-op1f_tape_encoder

/* Replaces the load of the word at 0x11905a08 into P0 at 0x11a02b60. P1 and CC are dead there (P1 is loaded by the next instruction).
 * Tracks 0-3: the track itself. Tracks 4-5 (B3a): the index that makes stock's three loads hit the module's
 * volume, pan and mute gain of that track. 6 and above: 3. */
.global op1f_monitor_track_p0
.type op1f_monitor_track_p0, STT_FUNC
op1f_monitor_track_p0:
    P0.H = SELECTED_TRACK;
    P0.L = SELECTED_TRACK;
    P0 = [P0];
    P1 = 4;
    CC = P0 < P1 (IU);
    IF CC JUMP .Lmonitor_stock;
    P1 = 8;
    CC = P0 < P1 (IU);
    IF !CC JUMP .Lmonitor_clamp;
    P0 += -4;
    P1.H = op1f_ext_params;
    P1.L = op1f_ext_params;
    P1 = P1 + (P0 << 2);                /* &op1f_ext_params[track - 4]: the volume; pan at +0x10, mute gain at +0x20 */
    P0.H = STOCK_MIXER_PARAMS;
    P0.L = STOCK_MIXER_PARAMS;
    P1 -= P0;
    P1 = P1 >> 2;
    P0 = P1;
    RTS;
.Lmonitor_clamp:
    P0 = 3;
    RTS;
.Lmonitor_stock:
    RTS;
.size op1f_monitor_track_p0, .-op1f_monitor_track_p0

/* Replaces the load of the word at 0x11905a08 into P1 at 0x11a02d3a. P0 and CC are dead there (P0 is loaded by the next instruction).
 * Tracks 4-5 (B3a): the index that makes stock's `W[0x11905a34 + 2k]` the module's VU word of that track. */
.global op1f_meter_track_p1
.type op1f_meter_track_p1, STT_FUNC
op1f_meter_track_p1:
    P1.H = SELECTED_TRACK;
    P1.L = SELECTED_TRACK;
    P1 = [P1];
    P0 = 4;
    CC = P1 < P0 (IU);
    IF CC JUMP .Lmeter_stock;
    P0 = 8;
    CC = P1 < P0 (IU);
    IF !CC JUMP .Lmeter_clamp;
    P1 += -4;
    P0.H = op1f_ext_vu;
    P0.L = op1f_ext_vu;
    P0 = P0 + (P1 << 1);                /* &op1f_ext_vu[track - 4] */
    P1.H = STOCK_VU_WORDS;
    P1.L = STOCK_VU_WORDS;
    P0 -= P1;
    P0 = P0 >> 1;
    P1 = P0;
    RTS;
.Lmeter_clamp:
    P1 = 3;
    RTS;
.Lmeter_stock:
    RTS;
.size op1f_meter_track_p1, .-op1f_meter_track_p1

/* Replaces the load of the selected track into R1 at 0x8c0c1bee, right before the call that sets a number in a json object.
 * P1 and R2 are dead there. A track index above 3 is written as 3 so a tape file never tells stock firmware to
 * record on a track it does not have. */
.global op1f_saved_track_r1
.type op1f_saved_track_r1, STT_FUNC
op1f_saved_track_r1:
    P1.H = SELECTED_TRACK;
    P1.L = SELECTED_TRACK;
    R1 = [P1];
    R2 = 3;
    R1 = MIN (R1, R2);
    RTS;
.size op1f_saved_track_r1, .-op1f_saved_track_r1

/* Replaces the entry of tape_io_read 0x8fc21bac. Neither stock function reads P2 or CC before setting them. */
.global op1f_tape_io_read
.type op1f_tape_io_read, STT_FUNC
op1f_tape_io_read:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lread_ext;
    P2.H = op1f_tape_io_read_room;      /* the instructions the hook displaced, then 0x8fc200b4 */
    P2.L = op1f_tape_io_read_room;
    JUMP (P2);
.Lread_ext:
    P2.H = op1f_tape_io_read_ext;
    P2.L = op1f_tape_io_read_ext;
    JUMP (P2);
.size op1f_tape_io_read, .-op1f_tape_io_read

/* Replaces the entry of tape_io_write 0x8fc21d1c. */
.global op1f_tape_io_write
.type op1f_tape_io_write, STT_FUNC
op1f_tape_io_write:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lwrite_ext;
    P2.H = op1f_tape_io_write_room;     /* the instructions the hook displaced, then 0x8fc20224 */
    P2.L = op1f_tape_io_write_room;
    JUMP (P2);
.Lwrite_ext:
    P2.H = op1f_tape_io_write_ext;
    P2.L = op1f_tape_io_write_ext;
    JUMP (P2);
.size op1f_tape_io_write, .-op1f_tape_io_write

/* 4. The clip-edge crossfades. At record stop (and after a drop) stock blends the audio outside a clip's
 *    edges into the clip through tape_c's crossfade writer 0x8f01dec4(track, from, to, padding, direction) or,
 *    when the outside padding is missing, fades the edge in place (0x8f01f7d0 / 0x8f01f8e0). B2b let the two
 *    helpers that do this return 0 for tracks 4-5 (their callers ignore the result). Since B3b the writer's
 *    tape I/O and the two fade writers serve six tracks (build_b_xfade.s/.cpp), so tracks 0-5 continue in
 *    stock; only a track of 6 or more still returned 0. Build F image F3 raises
 *    the bound to 8: F2 left it at 6, so the record stops and drops on tracks 7-8 got no clip-edge crossfade
 *    (the F2 log counted eight edge fades, all on 5 and 6). Since packs L3 the way into stock is the stub's room.
 *
 *      0x8f00a4cc (clip start edge)  -> its room, then 0x8f009294
 *      0x8f00a584 (clip stop edge)   -> its room, then 0x8f00934c
 */
.global op1f_xfade_start_edge
.type op1f_xfade_start_edge, STT_FUNC
op1f_xfade_start_edge:
    CC = R0 <= 7 (IU);                /* B3b: tracks 0-5 ran stock's helper; F3: 0-7 (the module's I/O and fade writers serve eight; the immediate form takes 0-7) */
    IF !CC JUMP .Lxfade_start_skip;
    P2.H = op1f_xfade_start_edge_room;  /* the instructions the hook displaced, then 0x8f009294 */
    P2.L = op1f_xfade_start_edge_room;
    JUMP (P2);
.Lxfade_start_skip:
    R0 = 0;
    RTS;
.size op1f_xfade_start_edge, .-op1f_xfade_start_edge

.global op1f_xfade_stop_edge
.type op1f_xfade_stop_edge, STT_FUNC
op1f_xfade_stop_edge:
    CC = R0 <= 7 (IU);                /* B3b: tracks 0-5 ran stock's helper; F3: 0-7 (the module's I/O and fade writers serve eight; the immediate form takes 0-7) */
    IF !CC JUMP .Lxfade_stop_skip;
    P2.H = op1f_xfade_stop_edge_room;   /* the instructions the hook displaced, then 0x8f00934c */
    P2.L = op1f_xfade_stop_edge_room;
    JUMP (P2);
.Lxfade_stop_skip:
    R0 = 0;
    RTS;
.size op1f_xfade_stop_edge, .-op1f_xfade_stop_edge

/* The rooms: zeroed data, which the base clears when it loads the pack and fills when it applies the table. Each is
 * the eight bytes its trampoline copies and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_tape_key_down_room
.type op1f_tape_key_down_room, STT_OBJECT
op1f_tape_key_down_room:
    .space 16
.size op1f_tape_key_down_room, .-op1f_tape_key_down_room
.global op1f_tape_shift_key_down_room
.type op1f_tape_shift_key_down_room, STT_OBJECT
op1f_tape_shift_key_down_room:
    .space 16
.size op1f_tape_shift_key_down_room, .-op1f_tape_shift_key_down_room
.global op1f_tape_encoder_room
.type op1f_tape_encoder_room, STT_OBJECT
op1f_tape_encoder_room:
    .space 16
.size op1f_tape_encoder_room, .-op1f_tape_encoder_room
.global op1f_tape_io_read_room
.type op1f_tape_io_read_room, STT_OBJECT
op1f_tape_io_read_room:
    .space 16
.size op1f_tape_io_read_room, .-op1f_tape_io_read_room
.global op1f_tape_io_write_room
.type op1f_tape_io_write_room, STT_OBJECT
op1f_tape_io_write_room:
    .space 16
.size op1f_tape_io_write_room, .-op1f_tape_io_write_room
.global op1f_xfade_start_edge_room
.type op1f_xfade_start_edge_room, STT_OBJECT
op1f_xfade_start_edge_room:
    .space 16
.size op1f_xfade_start_edge_room, .-op1f_xfade_start_edge_room
.global op1f_xfade_stop_edge_room
.type op1f_xfade_stop_edge_room, STT_OBJECT
op1f_xfade_stop_edge_room:
    .space 16
.size op1f_xfade_stop_edge_room, .-op1f_xfade_stop_edge_room
