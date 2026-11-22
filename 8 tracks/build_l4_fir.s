/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build L, packs L4: the 16-tap filter of tracks 5 to 8, written from what it computes (OS 1.7.0+0).
 * Its elder is op1f_fir_ext in Build B's playback stub, which every pack before L4
 * has and which is kept as it is; the caller, build_b_play.cpp, is as it was.
 *
 * What it computes. For one output frame and each of one to four tracks, two sums of sixteen products:
 *
 *     left  = S(... S(S(0, T(c[0], w[0])), T(c[1], w[2])) ..., T(c[15], w[30]))
 *     right = S(... S(S(0, T(c[0], w[1])), T(c[1], w[3])) ..., T(c[15], w[31]))
 *
 *   w[]   the track's window from the frame the first tap meets: 32-bit fractions, left and right by turns;
 *   c[k]  tap k of this output frame's phase, read where the firmware keeps its table: sixteen rows of sixty-four
 *         phases, so 0x100 bytes lie between a tap and the next of the same phase. The caller hands over the
 *         address of c[0]. No tap stands in this file or in the pack;
 *   T     the processor's 32-bit fractional multiply that cuts the low bits off, written `(T)`;
 *   S     its add that stops at the largest and at the smallest number, written `(S)`.
 *
 * The order is part of the result: a sum that has stopped at a limit goes on from that limit, so the sixteen
 * products are added from tap 0 to tap 15 and in no other order. These are the numbers the firmware gives the
 * tape's own four tracks, and the numbers the elder gave: both routines are run on the PC, instruction by
 * instruction from their assembled bytes, and their sums are held against each other as numbers and as formulas.
 *
 * How. A tap is seven things to do: three words to fetch (the tap and the two samples) and four to work out (two
 * products, two sums). Only the four need an instruction each; a fetch rides beside one of them. So a tap is four
 * instructions, and what is left to choose is where the fetches ride. Here the two products come first, then the
 * two sums, and beside the sums ride the three words of the NEXT tap: from the first tap to the last, no product
 * and no sum is worked out from a word that the instruction right before it fetched or worked out. The first
 * tap's words are fetched beside the two instructions that set the sums to zero. The loop runs through fifteen
 * taps, and the sixteenth follows it with nothing fetched beside it, so that no word behind the window or behind
 * the column is read. A track's two sums are stored as soon as each is whole.
 *
 *   void op1f_fir_ext(FirJob* job)      R0 = job
 *   struct FirJob {                      offsets
 *       const i32* bases[4];             0, 4, 8, 12   the windows of the tracks to filter, in order
 *       u32 count;                       16            how many of them, 1..4
 *       u32 offset_bytes;                20            8 x (n - 8): the first tap's frame in every window
 *       const i32* coefficient;          24            c[0] of this output frame's phase
 *       i32 out[8];                      28            left, right per track
 *   };
 *
 * Registers. R4 is the one register a caller keeps that the routine needs: pushed and popped. Everything else it
 * writes is free to a callee in this ABI (R0-R3, P0-P2, I0-I2, M0-M2, both loop counters with their bounds, the
 * flags), and the compiled caller holds nothing in them across the call: read in its listing, and held by the test.
 * R5-R7, P3-P5, FP, I3, M3 and the B and L registers are not touched. The index registers count straight on
 * because their length registers are zero, which holds wherever compiled code is called; the firmware's own
 * resampler 0x11a07748, which the tape's driver calls right before the call this routine's caller stands in for,
 * counts with all four index registers in the same way. No stack frame, no call, no branch but the two loops.
 *
 * Size. 106 bytes of code, and six bytes of zeros behind the return: with them it takes its elder's 112 bytes, so
 * that every other byte of the pack stands where it stands in the set of L3.
 */
.section .text
.align 2

.global op1f_fir_ext
.type op1f_fir_ext, STT_FUNC
op1f_fir_ext:
    [--SP] = R4;
    P0 = R0;                                /* the job; from here on it walks bases[] */
    R0 += 28;
    R1 = [P0 + 20];
    R2 = [P0 + 24];
    P1 = [P0 + 16];                         /* count */
    I2 = R0;                                /* out[] */
    M2 = R1;                                /* offset_bytes */
    M1 = R2;                                /* c[0] */
    M0 = 0x100 (X);                         /* from a tap to the next of the same phase */
    P2 = 15;                                /* the taps the loop runs through */
    LSETUP (.Ltrack, .Ltrack_end) LC1 = P1;
.Ltrack:
    P1 = [P0++];                            /* this track's window */
    I0 = M1;                                /* the column from its first tap */
    I1 = P1;
    I1 += M2;                               /* the frame the first tap meets */
    R3 = R3 - R3 (NS) || R0 = [I0 ++ M0] || R1 = [I1++];      /* left = 0; c[0]; its left sample */
    R4 = R4 - R4 (NS) || R2 = [I1++] || NOP;                  /* right = 0; its right sample */
    LSETUP (.Ltap, .Ltap_end) LC0 = P2;
.Ltap:
    R1 = R0 * R1 (T);                                         /* tap x left sample */
    R2 = R0 * R2 (T);                                         /* tap x right sample */
    R3 = R3 + R1 (S) || R1 = [I1++] || R0 = [I0 ++ M0];       /* into the left sum; the next left sample, the next tap */
.Ltap_end:
    R4 = R4 + R2 (S) || R2 = [I1++] || NOP;                   /* into the right sum; the next right sample */
    R1 = R0 * R1 (T);                                         /* the sixteenth tap */
    R2 = R0 * R2 (T);
    R3 = R3 + R1 (S);
    R4 = R4 + R2 (S) || [I2++] = R3 || NOP;                   /* out: left */
.Ltrack_end:
    [I2++] = R4;                                              /* out: right */
    R4 = [SP++];
    RTS;
.size op1f_fir_ext, .-op1f_fir_ext
    .space 6                                /* the elder's size: see the head */
