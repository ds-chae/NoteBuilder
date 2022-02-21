#if 0
#pragma once

#define	REGION_TABLE_SIZE	50

typedef struct {
	ST_IMG_REGION *pRegion;
	CPoint mp0, mp1, mp2, mp3;

	int ExtAvgIR;
	int ExtAvgW;
	int ExtMinW;
	int ExtMaxW;
	int ExtMinIR;
	int ExtMaxIR;
	int ExtVarW;
	int ExtVarIR;

	int ExtSizeX, ExtSizeY;
} REGION_RECORD;

class CRegionTable
{
public:
	CRegionTable(void);
	~CRegionTable(void);

public:
	int RegionCount;
	REGION_RECORD Data[REGION_TABLE_SIZE];

public:
	REGION_RECORD* FindRegion(ST_IMG_REGION *addReg);
	int FindRegion(int Sx, int Sy, int Ex, int Ey);
	void SetRegion(ST_IMG_REGION *addReg, CPoint& p0, CPoint& p1, CPoint& p2, CPoint& p3,
		int ExtAvgIR, int ExtAvgW,
		int ExtMinW, int ExtMaxW,
		int ExtMinIR, int ExtMaxIR,
		int ExtVarW, int ExtVarIR, int SizeX, int SizeY);
	REGION_RECORD* CRegionTable::AddRegion(ST_IMG_REGION *addReg);

	void DelRegion(CPoint p1, CPoint p2, int shiftx, int shifty, int sumx, int sumy);
	void DelRegion(int Sx, int Sy, int Ex, int Ey, int shitx, int shifty, int sumx, int sumy);
};

extern CRegionTable DisplayRegion;
int IsSameRegion(ST_IMG_REGION* reg1, ST_IMG_REGION* reg2);
#endif
