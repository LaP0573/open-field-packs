// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K, pack K5: where the chunks of tracks 5 to 8 lie on the tape, and a sum for each. See build_k_sums.cpp.
#ifndef OP1F_BUILD_K_SUMS_H
#define OP1F_BUILD_K_SUMS_H

enum {
    XSUM_NONE = 0,           // an entry's flag in the file of tracks 5 to 8: no sum is known for its chunk
    XSUM_SURE = 1,           // the sum is the chunk's as this pack wrote or read it
    XSUM_HELD = 2,           // the same, and another firmware has written the tape since: the chunk is held to it at its next read
};

// A free chunk for a track of 5 to 8, the highest on the tape, with the live record's bit set; 0xffff when none is free.
extern "C" unsigned int op1f_sums_alloc() asm("op1f_sums_alloc");
// A whole chunk was written from these 131,072 bytes. 1 when what the file should say of the chunk has changed.
extern "C" int op1f_sums_wrote(unsigned int physical, const unsigned char* chunk) asm("op1f_sums_wrote");
// A part of the chunk was written in place, or a write of it failed: its sum is not known any more. 1 as above.
extern "C" int op1f_sums_part(unsigned int physical) asm("op1f_sums_part");
// A whole chunk was read into these bytes. 0 when it is not what this pack wrote: the caller plays silence and
// lets the entry go.
extern "C" int op1f_sums_read(unsigned int physical, const unsigned char* chunk) asm("op1f_sums_read");
// 0 for a chunk that is still to be held to its sum, or failed it: nothing is written into it in place and nothing
// copied out of it before a whole read has looked at it.
extern "C" int op1f_sums_ours(unsigned int physical) asm("op1f_sums_ours");
// Every chunk forgotten (the tape engine's reset of its records).
extern "C" void op1f_sums_reset() asm("op1f_sums_reset");
// A tape's map was loaded: the tables of tracks 5 to 8 (4 x 969 entries) and, entry for entry, the file's sums and
// flags (both 0 when the file has none). `foreign`: another firmware wrote the tape after this pack last did.
extern "C" void op1f_sums_loaded(int tape, const unsigned short* ext, const unsigned int* sums, const unsigned char* flags, int foreign) asm("op1f_sums_loaded");
// For a save of that tape: the sums and flags, entry for entry. 0 (nothing filled) when the sums in memory are not
// that tape's.
extern "C" int op1f_sums_collect(int tape, const unsigned short* ext, unsigned int* sums, unsigned char* flags) asm("op1f_sums_collect");
// From the frame hook: sums learnt by reading reach the file within some ten seconds; the log's lines.
extern "C" void op1f_sums_frame() asm("op1f_sums_frame");

#endif
