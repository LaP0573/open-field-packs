/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build D, step D2: the disk mode's track files for tracks 5 and 6 (OS 1.7.0+0). The USB disk is an MTP
 * device (src/usb/mtp): each tape's folder lists tape.json, readme.txt and track_N.wav, rendered from and
 * imported into the tape partition through tape_open, tape_io_read/write and tape_close, which the module
 * already serves for six tracks. What still says four is in five places; each is an 8-byte JUMP.a hook that
 * lands here with the registers as stock left them. Logic (one name helper) in build_d_mtp.cpp.
 *
 *      folder population  0x8fcaf730   the test of the loop's count against four, and the jump back into the loop
 *                         (basset_device.cpp: the loop that adds track_%i.wav for i = 1..4 to a tape's
 *                         folder: the bound becomes 6; continue at 0x8fcaf738, or back into the loop)
 *      name lookup        0x8fcbbb86   the test of the comparison's answer, and the first steps of the way out
 *                         (tape_behavior.cpp: after the strcmp with "track_4.wav", R7 = the name: a match
 *                         continues at 0x8fcbbb96, where the track is the name's digit minus '1'; the module
 *                         also matches track_5.wav and track_6.wav there; anything else leaves through
 *                         stock's not-found epilogue at 0x8fcbbb8e; RETS is in the frame, LINK'ed)
 *      track file bound   0x8fcbb8b4   the test of the track's number against four
 *                         (tape_behavior.cpp:160 "track < 4"; R4 is reused by emmc_wav.h:29's compare at
 *                         0x8fcb9af4, so the one bound 6 serves both asserts; continue at 0x8fcbb8bc)
 *      partition name     0x8d4fb664   the test of the track's number against four
 *                         (the "tape%02d.raw" name helper's "track < 4" assert; continue at 0x8d4fb66c)
 *      unmap_chunk        0x8fc22750  -> its room, then 0x8fc20c68
 *                         (tape_snapshots.cpp: tracks 0-3 continue in stock, 4-5 go to op1f_ext_unmap_chunk;
 *                         the import's finalize unmaps every chunk after the imported audio with it)
 *
 * The room (packs L3): the first four are hooks in the middle of a
 * function, which do what the instructions under them did with another bound, and stay. The fifth stands at a
 * function's entry. Until L3 its stub ran the instructions the hook displaced itself, as a copy; now it holds none
 * of them and ends in a jump to its room: sixteen bytes of the pack's zeroed data into which the base copies those
 * instructions from the running firmware, with a jump back behind them, before it writes the hook
 * (module/build_m4_loader.cpp, make_trampoline; the table entry names the room). The displaced instructions end in
 * a compare of the track with zero, which stock branches on further down: it runs in the room, after this stub's
 * own compare. P2, the register the stub jumps through, is the one it jumped through before.
 */
.set POPULATE_CONTINUE,     0x8fcaf738
.set POPULATE_LOOP,         0x8fcaf70c
.set NAME_FOUND,            0x8fcbbb96
.set NAME_NOT_FOUND,        0x8fcbbb8e
.set FILE_TRACK_CONTINUE,   0x8fcbb8bc
.set FILE_TRACK_ASSERT,     0x8fcbba20
.set PARTITION_CONTINUE,    0x8d4fb66c
.set PARTITION_ASSERT,      0x8d4fb6c0

.section .text
.align 2

.global op1f_mtp_populate_bound
.type op1f_mtp_populate_bound, STT_FUNC
op1f_mtp_populate_bound:
    R3 = 8;                             /* the four displaced stock instructions, with the bound 8 (F2) */
    CC = R6 < R3;
    IF CC JUMP .Lpopulate_loop;
    R0 = [FP - 0x68];
    P2.H = POPULATE_CONTINUE;
    P2.L = POPULATE_CONTINUE;
    JUMP (P2);
.Lpopulate_loop:
    P2.H = POPULATE_LOOP;
    P2.L = POPULATE_LOOP;
    JUMP (P2);
.size op1f_mtp_populate_bound, .-op1f_mtp_populate_bound

.global op1f_mtp_name_bound
.type op1f_mtp_name_bound, STT_FUNC
op1f_mtp_name_bound:
    CC = R0 == 0;                       /* the strcmp with "track_4.wav" */
    IF CC JUMP .Lname_found;
    R0 = R7;                            /* the name */
    CALL op1f_mtp_extra_name;           /* 1 for track_5.wav and track_6.wav; R5-R7 preserved */
    CC = R0 == 0;
    IF !CC JUMP .Lname_found;
    SP += 0xc;                          /* the displaced start of stock's not-found epilogue */
    (R7:5) = [SP++];
    P2.H = NAME_NOT_FOUND;
    P2.L = NAME_NOT_FOUND;
    JUMP (P2);
.Lname_found:
    P2.H = NAME_FOUND;
    P2.L = NAME_FOUND;
    JUMP (P2);
.size op1f_mtp_name_bound, .-op1f_mtp_name_bound

.global op1f_mtp_file_track_bound
.type op1f_mtp_file_track_bound, STT_FUNC
op1f_mtp_file_track_bound:
    R4 = 8;                             /* the four displaced stock instructions, with the bound 8 (F2) */
    CC = R5 < R4;
    IF !CC JUMP .Lfile_track_assert;
    R0 = R7;
    P2.H = FILE_TRACK_CONTINUE;
    P2.L = FILE_TRACK_CONTINUE;
    JUMP (P2);
.Lfile_track_assert:
    P2.H = FILE_TRACK_ASSERT;
    P2.L = FILE_TRACK_ASSERT;
    JUMP (P2);
.size op1f_mtp_file_track_bound, .-op1f_mtp_file_track_bound

.global op1f_mtp_partition_bound
.type op1f_mtp_partition_bound, STT_FUNC
op1f_mtp_partition_bound:
    R0 = 8;                             /* the four displaced stock instructions, with the bound 8 (F2) */
    CC = R5 < R0;
    IF !CC JUMP .Lpartition_assert;
    R6 += 1;
    P2.H = PARTITION_CONTINUE;
    P2.L = PARTITION_CONTINUE;
    JUMP (P2);
.Lpartition_assert:
    P2.H = PARTITION_ASSERT;
    P2.L = PARTITION_ASSERT;
    JUMP (P2);
.size op1f_mtp_partition_bound, .-op1f_mtp_partition_bound

.global op1f_unmap_chunk
.type op1f_unmap_chunk, STT_FUNC
op1f_unmap_chunk:
    CC = R0 < 4 (IU);
    IF !CC JUMP .Lunmap_ext;
    P2.H = op1f_unmap_chunk_room;       /* the instructions the hook displaced, then 0x8fc20c68 */
    P2.L = op1f_unmap_chunk_room;
    JUMP (P2);
.Lunmap_ext:
    P2.H = op1f_ext_unmap_chunk;
    P2.L = op1f_ext_unmap_chunk;
    JUMP (P2);
.size op1f_unmap_chunk, .-op1f_unmap_chunk

/* The room: zeroed data, which the base clears when it loads the pack and fills when it applies the table: the
 * eight bytes its trampoline copies and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_unmap_chunk_room
.type op1f_unmap_chunk_room, STT_OBJECT
op1f_unmap_chunk_room:
    .space 16
.size op1f_unmap_chunk_room, .-op1f_unmap_chunk_room
