// Written by art/make_picture.py with garden.svg.make: the parts of the picture by index. Do not edit.
#ifndef GARDEN_PARTS_H
#define GARDEN_PARTS_H
#define CHIP_AT 5
#define DOT_AT 21
#define RAYS_AT 36
#define LEAD_AT 53
#define GROUND_AT 70
static const unsigned char GROUND_PERIOD[8] = { 0, 48, 26, 144, 32, 16, 0, 0 };
#define HARP_AT 79
static const unsigned short HARP_X[19] = { 20, 44, 68, 92, 116, 140, 164, 188, 212, 236, 260, 284, 308, 332, 356, 380, 404, 428, 452 };
#define HARP_N 19
#define MOLE_AT 98
#define PETAL_AT 103
static const unsigned short PETAL_OF[12] = { 0, 36, 72, 96, 126, 156, 174, 210, 246, 258, 282, 306 };
static const unsigned char PETALS[12] = { 6, 6, 4, 5, 5, 3, 6, 6, 2, 4, 4, 5 };
#define KRING_AT 441
#define KRINGW_AT 449
#define HELD_AT 457
#define HELDW_AT 465
#define MUTE_AT 473
#define CRITTER_AT 477
#define SMALL_AT 489
#define SMALL_COPIES 3
#define ICON_AT 525
#define ICON_COPIES 8
#define ICONON_AT 621
#define FRAME_AT 632
#define CURSOR_AT 640
#define GBOX_AT 644
#define MARK_AT 670
static const unsigned short THIN_AT[20] = { 681, 693, 717, 757, 761, 773, 785, 797, 809, 821, 833, 845, 857, 869, 881, 893, 905, 906, 907, 908 };
static const unsigned char THIN_N[20] = { 12, 24, 40, 4, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 1, 1, 1, 1 };
enum { THIN_OCHRE = 0, THIN_ORANGE = 1, THIN_WHITE = 2, THIN_DGRAY = 3, THIN_HUE0 = 4, THIN_HUE1 = 5, THIN_HUE2 = 6, THIN_HUE3 = 7, THIN_HUE4 = 8, THIN_HUE5 = 9, THIN_HUE6 = 10, THIN_HUE7 = 11, THIN_HUE8 = 12, THIN_HUE9 = 13, THIN_HUE10 = 14, THIN_HUE11 = 15, THIN_KNOB0 = 16, THIN_KNOB1 = 17, THIN_KNOB2 = 18, THIN_KNOB3 = 19 };
static const unsigned short THICK_AT[14] = { 909, 925, 941, 957, 973, 989, 1005, 1021, 1037, 1053, 1069, 1085, 1101, 1117 };
static const unsigned char THICK_N[14] = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 };
enum { THICK_OCHRE = 0, THICK_ORANGE = 1, THICK_HUE0 = 2, THICK_HUE1 = 3, THICK_HUE2 = 4, THICK_HUE3 = 5, THICK_HUE4 = 6, THICK_HUE5 = 7, THICK_HUE6 = 8, THICK_HUE7 = 9, THICK_HUE8 = 10, THICK_HUE9 = 11, THICK_HUE10 = 12, THICK_HUE11 = 13 };
static const unsigned char TEXT_ALIGN[93] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0 };
#define PAGE1 0
#define PAGE2 1
#define PAGE3 2
#define PAGE4 3
#define STAIR 4
#define SUN 33
#define SUNFILL 34
#define HORIZON 35
#define GMASK 78
#define DISC1 439
#define DISC2 440
#define LOOPMARK 630
#define PLAYHEAD 631
#define GSEL 668
#define GSQ 669
#define WALKON 678
#define WALKBOX 679
#define HELP 680
#define TX_ST0 0
#define TX_ST1 1
#define TX_ST2 2
#define TX_ST3 3
#define TX_ROOT 4
#define TX_KLW0 5
#define TX_KLW1 6
#define TX_KLW2 7
#define TX_KLW3 8
#define TX_KLW4 9
#define TX_KLW5 10
#define TX_KLW6 11
#define TX_KLW7 12
#define TX_KLG0 13
#define TX_KLG1 14
#define TX_KLG2 15
#define TX_KLG3 16
#define TX_KLG4 17
#define TX_KLG5 18
#define TX_KLG6 19
#define TX_KLG7 20
#define TX_NOTE0 21
#define TX_NOTE1 22
#define TX_NOTE2 23
#define TX_NOTE3 24
#define TX_NOTE4 25
#define TX_NOTE5 26
#define TX_NOTE6 27
#define TX_NOTE7 28
#define TX_NOTE8 29
#define TX_T1_LAST 29
#define TX_T2L0 30
#define TX_T2V0 31
#define TX_T2L1 32
#define TX_T2V1 33
#define TX_T2L2 34
#define TX_T2V2 35
#define TX_T2L3 36
#define TX_T2V3 37
#define TX_T2_LAST 37
#define TX_T3L0 38
#define TX_T3L1 39
#define TX_T3L2 40
#define TX_T3L3 41
#define TX_T3L4 42
#define TX_T3L5 43
#define TX_T3L6 44
#define TX_T3L7 45
#define TX_T3L8 46
#define TX_T3L9 47
#define TX_T3L10 48
#define TX_T3L11 49
#define TX_T3L12 50
#define TX_T3L13 51
#define TX_T3L14 52
#define TX_T3L15 53
#define TX_T3_LAST 53
#define TX_T4N0 54
#define TX_T4N1 55
#define TX_T4N2 56
#define TX_T4N3 57
#define TX_T4N4 58
#define TX_T4N5 59
#define TX_T4N6 60
#define TX_T4N7 61
#define TX_T4K0 62
#define TX_T4K1 63
#define TX_T4K2 64
#define TX_T4K3 65
#define TX_T4K4 66
#define TX_T4K5 67
#define TX_T4K6 68
#define TX_T4K7 69
#define TX_T4TOP 70
#define TX_T4WALK 71
#define TX_T4S0 72
#define TX_T4S1 73
#define TX_T4S2 74
#define TX_T4_LAST 74
#define TX_HTW0 75
#define TX_HTW1 76
#define TX_HTW2 77
#define TX_HTW3 78
#define TX_HTG0 79
#define TX_HTG1 80
#define TX_HTG2 81
#define TX_HTG3 82
#define TX_HR0 83
#define TX_HR1 84
#define TX_HR2 85
#define TX_HR3 86
#define TX_HP0 87
#define TX_HP1 88
#define TX_HP2 89
#define TX_HP3 90
#define TX_HK0 91
#define TX_HK1 92
#define PART_COUNT 1133
#define TEXT_COUNT 93
#endif
