// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build K: a tape's block map in two files (K1, K1b) and the tape's identity (K2). See Build K's maps unit.
#ifndef OP1F_BUILD_K_MAPS_H
#define OP1F_BUILD_K_MAPS_H

enum {
    XMAP_V1 = 1,             // stock's file, version 1: tracks 1 to 4; the extension file read beside it
    XMAP_LEGACY = 2,         // this project's old version-2 file: eight tracks in one file
    XMAP_NO_FILE = -1,
    XMAP_BAD_MAGIC = -2,
    XMAP_BAD_VERSION = -3,
    XMAP_SHORT = -4,
};

enum {
    XEXT_NONE = 0,           // no extension file: the tape has four tracks
    XEXT_SAME = 1,           // read, and stock's file is the one it was written with
    XEXT_CHANGED = 2,        // read and the tape's own, but stock's file has changed since: another firmware saved this tape
    XEXT_BAD = 3,            // there, and not usable: taken as empty
    XEXT_LEGACY = 4,         // tracks 5 to 8 came out of a version-2 file
    XEXT_NEWER = 5,          // there, and of a later version than this pack reads: taken as empty, left alone
    XEXT_STALE = 6,          // there, whole, and not this tape's (K2): taken as empty
};

struct XmapLoad {
    int status;              // XMAP_*
    unsigned int version;    // the stock file's version
    unsigned int tracks_in_file;
    unsigned int stock_crc;  // the CRC-32 of stock's file as read (version 1 only)
    int ext;                 // XEXT_*
    unsigned int dropped;    // extension entries dropped by the filter
    unsigned int allocated[4];
    unsigned int ext_version;   // K2: the extension file's version, 0 without one
    unsigned int id;            // K2: the id in the extension file, 0 for none
    unsigned int json_id;       // K2: the id in the tape's json, 0 for none or not asked
    int bound;                  // K2: the two are the same number
    int rewrite;                // K2: the file is right and still to be written anew (an older version, the bound flag)
    int sums;                   // K5: the file carries the sums of its chunks (op1f_xmap_entry_sums, op1f_xmap_entry_flags)
    int foreign;                // K5: another firmware saved the tape's map after this pack last wrote the file of tracks 5 to 8
};

// The tables are 4 x 969 entries each: tracks 1 to 4 in `stock`, tracks 5 to 8 in `ext`.
extern "C" int op1f_xmap_load(int tape, unsigned short* stock, unsigned short* ext, XmapLoad* out) asm("op1f_xmap_load");
extern "C" int op1f_xmap_save(int tape, const unsigned short* stock, const unsigned short* ext) asm("op1f_xmap_save");
extern "C" int op1f_xmap_convert(int tape) asm("op1f_xmap_convert");
// K2, both on the GUI thread: the tape json writer's call (Build B's mixer unit) and the frame hook's (the first frame hook's measuring unit).
extern "C" void op1f_xmap_json_write() asm("op1f_xmap_json_write");
extern "C" void op1f_xmap_frame() asm("op1f_xmap_frame");
// K4, for the copy of a tape (build_k_copy.cpp): a slot's tracks 5 to 8 as a load takes them, in the unit's own
// tables; the CRC-32 of a slot's map file on the disk; a file of tracks 5 to 8 with no identity for a new copy.
extern "C" const unsigned short* op1f_xmap_source(int tape, XmapLoad* out) asm("op1f_xmap_source");
extern "C" int op1f_xmap_file_crc(int tape, unsigned int* crc) asm("op1f_xmap_file_crc");
extern "C" int op1f_xmap_write_plain(int tape, const unsigned short* ext, unsigned int stock_crc, int only_if_there, int with_sums) asm("op1f_xmap_write_plain");
// K5, the sums of the chunks of tracks 5 to 8 (build_k_sums.cpp), entry for entry beside the tables. Two pairs,
// one a thread's side: op1f_xmap_entry_* is what the last load read from the file (all 0 when it has none), and
// what op1f_xmap_write_plain writes for a copy; op1f_xmap_save_entry_* is what a save writes when
// op1f_xmap_save_sums is set just before it (the save clears the word). The flags are XSUM_NONE, XSUM_SURE and
// XSUM_HELD.
extern "C" unsigned int op1f_xmap_entry_sums[4 * 969] asm("op1f_xmap_entry_sums");
extern "C" unsigned char op1f_xmap_entry_flags[4 * 969] asm("op1f_xmap_entry_flags");
extern "C" unsigned int op1f_xmap_save_entry_sums[4 * 969] asm("op1f_xmap_save_entry_sums");
extern "C" unsigned char op1f_xmap_save_entry_flags[4 * 969] asm("op1f_xmap_save_entry_flags");
extern "C" int op1f_xmap_save_sums asm("op1f_xmap_save_sums");

#endif
