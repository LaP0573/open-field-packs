/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R2: the pack `fx train` makes and shows its own page through its card (fx train 11's train unit,
 * build_r2_presets.cpp, fx train 11's frame unit): the train, the UFOs and the presets' list and save screen. A
 * base that still holds that work for it would make the page a second time and take the train's page from it. So
 * this pack names the mark of a base that holds nothing made for one pack, outright: the loader leaves a pack out
 * when a name it needs is offered by no pack that runs, and says so on the start screen and on the log. Nothing
 * reads this word.
 */
.section .rodata
.balign 4
.global op1f_fx_train_needs
.type op1f_fx_train_needs, STT_OBJECT
op1f_fx_train_needs:
    .long op1f_bare_1
.size op1f_fx_train_needs, .-op1f_fx_train_needs
