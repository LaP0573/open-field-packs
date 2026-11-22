// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build D, step D2: the disk mode's track files for tracks 5 and 6 (OS 1.7.0+0).
//
// What stock does (src/usb/mtp in record 309): the USB disk is an MTP device. basset_device.cpp populates
// one folder per labelled tape with tape.json, readme.txt and track_1.wav to track_4.wav (a loop to 4 at
// 0x8fcaf730). When the host opens a file, tape_behavior.cpp's lookup 0x8fcb9cac compares the name with its
// six names and, for a track file, takes the track from the name's digit and builds an EmmcWav object
// (0x8fcb9a84, asserting track < 4 at tape_behavior.cpp:160 and again in emmc_wav.h:29; the partition name
// helper 0x8d4fa4f8 asserts it a third time). The object opens the tape with tape_open (the module's map
// loader), streams a 32-bit stereo WAV out of tape_io_read through a prefetching reader thread, or takes a
// 16- or 32-bit WAV in through a writer thread and tape_io_write (both six-track since B2b); its finalize
// zeroes the rest of the last chunk, unmaps every chunk after the imported audio (unmap_chunk, four maps),
// closes the tape (the module's saver) and rewrites the tape's clip list for the track.
//
// This unit (profile group "build-d2"; the five trampolines in build_d_mtp.s): the folder lists track_5.wav
// and track_6.wav, the lookup accepts them (op1f_mtp_extra_name below), the three bounds read 6, and
// unmap_chunk serves six tracks (tracks 4-5 through build_b_storage.cpp's op1f_ext_unmap_chunk). Nothing
// else is changed: the data path, the clip extent from the tape JSON and the clip regeneration take the
// track number as it comes.

typedef unsigned int u32;

extern "C" u32 op1f_mtp_extra_lookups asm("op1f_mtp_extra_lookups");
u32 op1f_mtp_extra_lookups;

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// 1 when the name is track_5.wav .. track_8.wav (the caller then takes the track from the digit; F2 adds 7 and 8).
extern "C" int op1f_mtp_extra_name(const char* name) asm("op1f_mtp_extra_name");
extern "C" int op1f_mtp_extra_name(const char* name) {
    if (!name) return 0;
    if (!same(name, "track_5.wav") && !same(name, "track_6.wav") && !same(name, "track_7.wav") && !same(name, "track_8.wav")) return 0;
    ++op1f_mtp_extra_lookups;
    return 1;
}
