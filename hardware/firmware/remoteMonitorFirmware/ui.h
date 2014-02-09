#include "lcd.h"
#include "config.h"
#include <stdlib.h>

#include "basestation.h"
#include "int_sqrt.h"

void initUI(void);
void updateUI(void);
void ui_baseStationConnected(void);
void ui_baseStationDisconnected(void);
void ui_baseStationListChanged(int8_t modifiedEntry);
void ui_updatePowerValues(int64_t ch1, int64_t ch2, uint32_t sampleCount);

