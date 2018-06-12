#ifndef _CALIBRATION_H_
#define	_CALIBRATION_H_

/* ************************************************************************** */

// Swr computation result definitions. Definitely hardware dependent!
#define SWR1_0 0
#define SWR1_1 1
#define SWR1_3 7
#define SWR1_5 10
#define SWR1_7 20
#define SWR2_0 40
#define SWR2_5 75
#define SWR3_0 100
#define SWR3_5 120

/* ************************************************************************** */

// This is a little above the 125W values below, and will hopefully not trigger
// a false positive
#define HIGH_OVER_POWER     235

// 125W on Alpha Power 4510A, from Kenwood TS-480X into 50ohm dummy load
// Updated 3/20/17 to adjust for new SWR math
#define OVER_POWER_2MHZ     171
#define OVER_POWER_3MHZ     202
#define OVER_POWER_5MHZ     212
#define OVER_POWER_7MHZ     215
#define OVER_POWER_10MHZ    218
#define OVER_POWER_14MHZ    220
#define OVER_POWER_18MHZ    216
#define OVER_POWER_21MHZ    216
#define OVER_POWER_24MHZ    218
#define OVER_POWER_28MHZ    215

/*

// key
10
25
50
100
200
300
450
600
750

// 160m
410
680
990
1470
2120
2610
3210
3720
4160

// 80m
430
720
1060
1580
2330
2860
3540
4090
4620

// 40m
420
710
1050
1570
2320
2860
3540
4120
4650

// 20m
400
700
1000
1500
2200
2800
3500
4000
4500

// 15m
380
650
970
1480
2190
3750
3420
4000
4360

// 10m
360
610
910
1370
2080
2610
3260
3800
4100

// 6m
370
670
1080
*/

#endif