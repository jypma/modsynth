#ifndef TRIANGLE_H_
#define TRIANGLE_H_
 
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "mozzi_pgmspace.h"
 
#define TRIANGLE_NUM_CELLS 256
#define TRIANGLE_SAMPLERATE 256
 
CONSTTABLE_STORAGE(uint16_t) TRIANGLE_DATA [] = {0, 21, 42, 63, 84, 105, 127,
148, 169, 190, 211, 233, 254, 275, 296, 317, 339, 360, 381, 402, 423, 444, 466,
487, 508, 529, 550, 572, 593, 614, 635, 656, 678, 699, 720, 741, 762, 783, 805,
826, 847, 868, 889, 911, 932, 953, 974, 995, 1017, 1038, 1059, 1080, 1101, 1122,
1144, 1165, 1186, 1207, 1228, 1250, 1271, 1292, 1313, 1334, 1356, 1377, 1398,
1419, 1440, 1461, 1483, 1504, 1525, 1546, 1567, 1589, 1610, 1631, 1652, 1673,
1695, 1716, 1737, 1758, 1779, 1800, 1822, 1843, 1864, 1885, 1906, 1928, 1949,
1970, 1991, 2012, 2034, 2055, 2076, 2097, 2118, 2139, 2161, 2182, 2203, 2224,
2245, 2267, 2288, 2309, 2330, 2351, 2373, 2394, 2415, 2436, 2457, 2478, 2500,
2521, 2542, 2563, 2584, 2606, 2627, 2648, 2669, 2690, 2712, 2690, 2669, 2648,
2627, 2606, 2584, 2563, 2542, 2521, 2500, 2478, 2457, 2436, 2415, 2394, 2373,
2351, 2330, 2309, 2288, 2267, 2245, 2224, 2203, 2182, 2161, 2139, 2118, 2097,
2076, 2055, 2034, 2012, 1991, 1970, 1949, 1928, 1906, 1885, 1864, 1843, 1822,
1800, 1779, 1758, 1737, 1716, 1695, 1673, 1652, 1631, 1610, 1589, 1567, 1546,
1525, 1504, 1483, 1461, 1440, 1419, 1398, 1377, 1356, 1334, 1313, 1292, 1271,
1250, 1228, 1207, 1186, 1165, 1144, 1122, 1101, 1080, 1059, 1038, 1017, 995,
974, 953, 932, 911, 889, 868, 847, 826, 805, 783, 762, 741, 720, 699, 678, 656,
635, 614, 593, 572, 550, 529, 508, 487, 466, 444, 423, 402, 381, 360, 339, 317,
296, 275, 254, 233, 211, 190, 169, 148, 127, 105, 84, 63, 42, 21, };
 
 #endif /* TRIANGLE_H_ */
