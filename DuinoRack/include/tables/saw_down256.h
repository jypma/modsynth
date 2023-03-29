#ifndef SAW_DOWN_H_
#define SAW_DOWN_H_
 
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "mozzi_pgmspace.h"
 
#define SAW_DOWN_NUM_CELLS 256
#define SAW_DOWN_SAMPLERATE 256
 
CONSTTABLE_STORAGE(uint16_t) SAW_DOWN_DATA [] = {2712, 2701, 2690, 2680, 2669,
2659, 2648, 2637, 2627, 2616, 2606, 2595, 2584, 2574, 2563, 2553, 2542, 2531,
2521, 2510, 2500, 2489, 2478, 2468, 2457, 2447, 2436, 2425, 2415, 2404, 2394,
2383, 2373, 2362, 2351, 2341, 2330, 2320, 2309, 2298, 2288, 2277, 2267, 2256,
2245, 2235, 2224, 2214, 2203, 2192, 2182, 2171, 2161, 2150, 2139, 2129, 2118,
2108, 2097, 2086, 2076, 2065, 2055, 2044, 2034, 2023, 2012, 2002, 1991, 1981,
1970, 1959, 1949, 1938, 1928, 1917, 1906, 1896, 1885, 1875, 1864, 1853, 1843,
1832, 1822, 1811, 1800, 1790, 1779, 1769, 1758, 1747, 1737, 1726, 1716, 1705,
1695, 1684, 1673, 1663, 1652, 1642, 1631, 1620, 1610, 1599, 1589, 1578, 1567,
1557, 1546, 1536, 1525, 1514, 1504, 1493, 1483, 1472, 1461, 1451, 1440, 1430,
1419, 1408, 1398, 1387, 1377, 1366, 1356, 1345, 1334, 1324, 1313, 1303, 1292,
1281, 1271, 1260, 1250, 1239, 1228, 1218, 1207, 1197, 1186, 1175, 1165, 1154,
1144, 1133, 1122, 1112, 1101, 1091, 1080, 1069, 1059, 1048, 1038, 1027, 1017,
1006, 995, 985, 974, 964, 953, 942, 932, 921, 911, 900, 889, 879, 868, 858, 847,
836, 826, 815, 805, 794, 783, 773, 762, 752, 741, 730, 720, 709, 699, 688, 678,
667, 656, 646, 635, 625, 614, 603, 593, 582, 572, 561, 550, 540, 529, 519, 508,
497, 487, 476, 466, 455, 444, 434, 423, 413, 402, 391, 381, 370, 360, 349, 339,
328, 317, 307, 296, 286, 275, 264, 254, 243, 233, 222, 211, 201, 190, 180, 169,
158, 148, 137, 127, 116, 105, 95, 84, 74, 63, 52, 42, 31, 21, 10, };
 
 #endif /* SAW_DOWN_H_ */