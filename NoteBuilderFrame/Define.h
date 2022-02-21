#pragma once

typedef struct _SETTING {
	int nCFLevel[2];
	int nTapeLevel;
	int nHoleLevel;
	int nStainLevel;
	int nWashedLevel;
	int nDeinkedLevel;
	int nSoiledLevel[7];
	int bFitnessMode;
} sSetting;