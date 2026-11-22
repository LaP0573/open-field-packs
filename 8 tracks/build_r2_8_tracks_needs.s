/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R2: the pack `8 tracks` does its own frame work through its card (8 tracks 8's frame unit): the
 * frame calls of its units, the meters' second and its lines on the log. A base that still holds that work for it
 * would do it a second time. So this pack names the mark of a base that holds nothing made for one pack, outright:
 * the loader leaves a pack out when a name it needs is offered by no pack that runs, and says so on the start screen
 * and on the log. Nothing reads this word.
 */
.section .rodata
.balign 4
.global op1f_8_tracks_needs
.type op1f_8_tracks_needs, STT_OBJECT
op1f_8_tracks_needs:
    .long op1f_bare_1
.size op1f_8_tracks_needs, .-op1f_8_tracks_needs
