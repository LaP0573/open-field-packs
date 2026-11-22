/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Build F images F4 to F6: the tape screen's clip bars lower (OS 1.7.0+0). Three 8-byte CALL.a retargets in record
 * 309, inside TapeScreen.
 *
 * The bar itself (F6). TapeScreen's per-clip draw 0x8fc81e7c, run from the update's loop 0x8fc81c12 for every clip of
 * the window on every track, ends in one call of the rectangle fill 0x11a0a680 at 0x8fc83d00, whichever colour path
 * the clip took (an ordinary track, the selected track, the highlighted clip): R0 = the drawing context, R1 = x1 and
 * R2 = x2 in 22.10 fixed point, [SP + 0xc] = the top row, [SP + 0x10] = the bottom row (whole pixels, both inclusive:
 * the fill hands them to the span writer 0x11a0a1dc, which clamps them to 0..223 and writes bottom - top + 1 rows of
 * every column from x1 to x2), [SP + 0x14] = the colour pointer. The rows are the lane y (the first lane's row
 * [this + 0x14] plus the track times the spacing [this + 0x18], both integers from the artwork) minus 3 and plus 3:
 * seven rows, which is what the unit's screen showed at F3, the bars of neighbouring lanes touching at the 7-row
 * pitch (F5 result). The lane y is the template line's y plus 0.5, truncated (the TapeScreen's static initialiser
 * 0x8fc82fac writes 0x200 to 0x84c9fb74), so the templates at 168.1 give the lanes 168, 175, ..., 217.
 *
 *      at 0x8fc83d00 the call of the rectangle fill, for the rows y - 3 .. y + 3 -> a call of op1f_clip_rect: the rows y - 2 .. y + 2
 *
 * Five rows per bar, two black rows between the lanes.
 *
 * The shift-held overlay (F4's two hooks, kept with F4's values). 0x8fc80e50, run per clip from the same loop only
 * while the shift byte 0x11905a4d is set and no clip is selected (the word 0x848d98e0 holds -1), draws two stroked shapes through
 * the line helper 0x11a0a29c (its arguments: point 1 and point 2, each (x, y) in 22.10 on the caller's frame, then the
 * half-width and the colour pointer, both on the stack): 3.0 and 2.0 at the lane y, the edges y - 3.0 .. y + 3.0 and
 * y - 2.0 .. y + 2.0. F5 had hooked these two alone, which is why the bars did not change on the device.
 *
 *      at 0x8fc82c54 the call of the line helper with the half-width 3.0 -> a call of op1f_clip_bar_outer: 2.5, both points down half a pixel
 *      at 0x8fc82c7e the call of the line helper with the half-width 2.0 -> a call of op1f_clip_bar_inner: 1.5, the same shift
 *
 * The overlay's edges become y - 2.0 .. y + 3.0 and y - 1.0 .. y + 2.0: whole edges, the rows y - 2 .. y + 2 and
 * y - 1 .. y + 1, the same five rows as the bar with a row of the outer colour above and below the stripe, as stock's
 * 6/4 pair has. The served tape.svg's lane backgrounds are 5-pixel strokes on the half-pixel centres y + 0.5
 * (the F4 file), the same whole edges again, so the
 * bar, the band and the overlay light the same five rows. The caller of the overlay stores the two points afresh
 * before the second call, so the first shift does not add up.
 *
 * The stubs: each is entered by the site's CALL.a, so RETS holds the return into the clip draw and SP is the
 * caller's; they change the arguments in place and tail-jump to the stock helper, which returns through RETS (both
 * helpers LINK). R3, P0, P1 and P2 are caller-saved and dead at the three sites; R0, R1 and R2 are the helpers' own
 * arguments and are not touched (the line helper reads the points through R1 and R2; the fill reads x1 and x2).
 */
.set LINE_HELPER, 0x11a0a29c
.set RECT_FILL, 0x11a0a680

.section .text
.align 2

.global op1f_clip_rect
.type op1f_clip_rect, STT_FUNC
op1f_clip_rect:
    R3 = [SP + 0xc];                    /* the top row, y - 3 */
    R3 += 1;                            /* y - 2 */
    [SP + 0xc] = R3;
    R3 = [SP + 0x10];                   /* the bottom row, y + 3 */
    R3 += -1;                           /* y + 2 */
    [SP + 0x10] = R3;
    P2.H = RECT_FILL;
    P2.L = RECT_FILL;
    JUMP (P2);                          /* fills the five rows, then returns to the clip draw through RETS */
.size op1f_clip_rect, .-op1f_clip_rect

.global op1f_clip_bar_outer
.type op1f_clip_bar_outer, STT_FUNC
op1f_clip_bar_outer:
    P1 = 0xa00 (X);                     /* 2.5 pixels (stock 0xc00, 3.0; F5 had 0x800, 2.0) */
    JUMP .Lclip_bar_draw;
.size op1f_clip_bar_outer, .-op1f_clip_bar_outer

.global op1f_clip_bar_inner
.type op1f_clip_bar_inner, STT_FUNC
op1f_clip_bar_inner:
    P1 = 0x600 (X);                     /* 1.5 pixels (stock 0x800, 2.0; F5 had 0x400, 1.0) */
.Lclip_bar_draw:
    [SP + 0xc] = P1;                    /* the half-width argument, in place of stock's */
    P2 = 0x200 (X);                     /* half a pixel in 22.10 fixed point */
    P0 = R1;                            /* point 1: x at +0, y at +4 */
    R3 = [P0 + 0x4];
    P1 = R3;
    P1 = P1 + P2;
    R3 = P1;
    [P0 + 0x4] = R3;
    P0 = R2;                            /* point 2 */
    R3 = [P0 + 0x4];
    P1 = R3;
    P1 = P1 + P2;
    R3 = P1;
    [P0 + 0x4] = R3;
    P2.H = LINE_HELPER;
    P2.L = LINE_HELPER;
    JUMP (P2);                          /* draws the overlay, then returns to the clip draw through RETS */
.size op1f_clip_bar_inner, .-op1f_clip_bar_inner
