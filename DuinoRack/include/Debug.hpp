#pragma once

#ifdef DEBUG_SERIAL
#define debugSerial(txt) Serial.println(txt)
#else
#define debugSerial(txt)
#endif
