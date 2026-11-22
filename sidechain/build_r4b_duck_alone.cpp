// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R4b, the pack sidechain (version 9): J5g's unit for the sidechain without 8 tracks with its two functions of the tape's file
// taken out, and nothing else changed. Base 26 holds the tape file's two places (the save's set of "mixer_params" at
// 0x8c0c1db8, the load's entry at 0x8c0c348c), where this unit put the duck's own routines while the pack 8 tracks was
// away; the ducks go with the tape through level 7 now, with 8 tracks or without it (sidechain 9's values unit).
// The two other places (the tape driver's read_block, the signal flow screen's VU vector) and the pack's choice among
// its pictures are the elder's. What follows is the elder's description, which holds but for the tape json's two rows.
//
// Build J packs J5g: the sidechain without the pack `8 tracks` (OS 1.7.0+0).
//
// The duck (Build G's duck unit) is driven from four places of stock, and since G1 and G2 the tape's units have had
// those four sites and called the duck from their own functions there, through weak references:
//
//   the tape driver's call of read_block, 0x11a0825e (L1)   op1f_read_block_ext (build_b_play.cpp): the duck's gains
//                                                           before stock's read_block, the detector after it
//   the signal flow screen's call for its VU vector,        op1f_signalflow_vu (build_e_screens): stock's vector, the
//   0x8fc35738                                              meters of tracks 5 to 8, then the duck's page
//   the tape json's writer, 0x8c0c1db8                      op1f_tape_json_set_mixer (Build B's mixer unit): stock's set,
//                                                           the tape units' own keys, then the duck's settings
//   the tape json's loader, 0x8c0c348c                      op1f_tape_json_load / op1f_tape_json_loaded: after stock's
//                                                           loader the tape units' keys, and the duck's settings
//
// Where the tape's units are away (the pack `sidechain` on a unit without the pack `8 tracks`), this unit has the
// same four sites: the sidechain's table names the functions below there, each entry marked "only when the pack
// 8 tracks is away", so that the base makes them when that pack does not run and passes them over when it does
// (J5c's base unit, left_to_another). Each function does what the tape's function at that site does for
// the duck, and nothing else: there are no tracks 5 to 8 here.
//
// The picture: the pack carries the signal flow screen's picture twice, with the duck's page for eight tracks
// (signalflow.svg, on F3's drawing of eight meters) and for four (signalflow4.svg, on stock's own drawing).
// op1f_art_pick_sidechain is asked by the picture loader of `kit` (J5g's picture loader) which of the pack's pictures
// a name stands for, and answers the page for four when the tape's units are away.
//
// Threads: op1f_duck_read_block on the audio thread (no logging); the rest on the GUI's.
// Freestanding like the other units.

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*ReadBlockFn)(int speed, void* state, int rolling);   // 0x8f01ec34: fills the four window buffers, answers the window's first frame
typedef void* (*VuVectorFn)();                                     // 0x8d516c90: ReadInterface's four-float VU vector, in dB

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_READ_BLOCK ((ReadBlockFn)0x8f01ec34u)
#define STOCK_VU_VALUES ((VuVectorFn)0x8d516c90u)

static const char LOG_FILE[] = "../src/gui/op1f_duck_alone.cpp";
enum { LOG_BUDGET = 4 };

// Build G's duck unit
extern "C" void op1f_duck_block_gains() asm("op1f_duck_block_gains");
extern "C" void op1f_duck_detect(int speed, int w0) asm("op1f_duck_detect");
extern "C" void op1f_duck_screen(u8* screen, const u8* vector, const u32* ext_db) asm("op1f_duck_screen");
// The tape's units, when they are there (build_b_cache.cpp).
extern "C" u32 __attribute__((weak)) op1f_cache_tracks asm("op1f_cache_tracks");

// Counters, for the log.
extern "C" u32 op1f_duck_alone_blocks asm("op1f_duck_alone_blocks");     // blocks the sidechain's own hook saw
extern "C" u32 op1f_duck_alone_updates asm("op1f_duck_alone_updates");   // updates of the signal flow screen through its own hook
u32 op1f_duck_alone_blocks, op1f_duck_alone_updates;

// In place of the driver's call of 0x8f01ec34 at 0x11a0825e: this block's duck gains into the hold-to-solo words
// before stock reads them, stock's read_block, the detector over this block's frames. What op1f_read_block_ext does
// for the duck, in its order.
extern "C" int op1f_duck_read_block(int speed, void* state, int rolling) asm("op1f_duck_read_block");
extern "C" int op1f_duck_read_block(int speed, void* state, int rolling) {
    op1f_duck_block_gains();
    int w0 = STOCK_READ_BLOCK(speed, state, rolling);
    op1f_duck_detect(speed, w0);
    ++op1f_duck_alone_blocks;
    return w0;
}

// From op1f_duck_vu (the stub at the signal flow screen's call of 0x8d516c90, 0x8fc35738): stock's vector for
// stock's own loop, and the duck's page drawn from it. There are no levels of tracks 5 to 8 to hand over.
extern "C" void* op1f_duck_vu_ext(u8* screen) asm("op1f_duck_vu_ext");
extern "C" void* op1f_duck_vu_ext(u8* screen) {
    void* vector = STOCK_VU_VALUES();
    op1f_duck_screen(screen, (const u8*)vector, 0);
    ++op1f_duck_alone_updates;
    return vector;
}

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// Asked by the picture loader of `kit` for every name, before it looks into this pack's pictures: the name of the
// picture to take for it. The signal flow screen's picture is the page for four tracks on stock's own drawing when
// the tape's units are away, and the page for eight on F3's drawing when they are there.
extern "C" const char* op1f_art_pick_sidechain(const char* name) asm("op1f_art_pick_sidechain");
extern "C" const char* op1f_art_pick_sidechain(const char* name) {
    if (&op1f_cache_tracks == 0 && same(name, "signalflow.svg")) return "signalflow4.svg";
    return name;
}
