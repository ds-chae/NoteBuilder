#include "StdAfx.h"

#include "B_Common.h"

#include "RegionTable.h"

CRegionTable DisplayRegion;

CRegionTable::CRegionTable(void)
{
	RegionCount = 0;
}

CRegionTable::~CRegionTable(void)
{
}

void CRegionTable::SetRegion(ST_IMG_REGION *addReg, CPoint& p0, CPoint& p1, CPoint& p2, CPoint& p3,
	int ExtAvgIR, int ExtAvgW,
	int ExtMinW, int ExtMaxW,
	int ExtMinIR, int ExtMaxIR,
	int ExtVarW, int ExtVarIR, int SizeX, int SizeY)
{
	REGION_RECORD *pRegRec = FindRegion(addReg);
	if(pRegRec == NULL)
		return;

	pRegRec->mp0 = p0;
	pRegRec->mp1 = p1;
	pRegRec->mp2 = p2;
	pRegRec->mp3 = p3;

	pRegRec->ExtAvgIR = ExtAvgIR;
	pRegRec->ExtAvgW = ExtAvgW;
	pRegRec->ExtMinW = ExtMinW;
	pRegRec->ExtMaxW = ExtMaxW;
	pRegRec->ExtMinIR = ExtMinIR;
	pRegRec->ExtMaxIR = ExtMaxIR;
	pRegRec->ExtVarW = ExtVarW;
	pRegRec->ExtVarIR = ExtVarIR;
	pRegRec->ExtSizeX = SizeX;
	pRegRec->ExtSizeY = SizeY;
}

REGION_RECORD* CRegionTable::FindRegion(ST_IMG_REGION *addReg)
{
	int ri = 0;
	for( ri = 0; ri < RegionCount; ri++) {
		if(IsSameRegion(Data[ri].pRegion, addReg)) {
			return &Data[ri];
		}
	}

	return NULL;
}

REGION_RECORD* CRegionTable::AddRegion(ST_IMG_REGION *addReg)
{
	REGION_RECORD* pRegRec = FindRegion(addReg);
	if(pRegRec != NULL)
		return NULL;

	if(RegionCount >= REGION_TABLE_SIZE)
		return NULL;
	pRegRec = &Data[RegionCount];
	RegionCount++;

	pRegRec->pRegion = addReg;
	return pRegRec;
}

void CRegionTable::DelRegion(int Sx, int Sy, int Ex, int Ey, int shiftx, int shifty, int sumx, int sumy)
{
	int ri;
	for( ri = 0; ri < RegionCount; ri++) {
		REGION_RECORD *pRegRec = &Data[ri];
		if( pRegRec->pRegion->Region.Sx == Sx && pRegRec->pRegion->Region.Sy == Sy &&
			pRegRec->pRegion->Region.Ex == Ex && pRegRec->pRegion->Region.Ey == Ey &&
			pRegRec->pRegion->Shift_X == shiftx && pRegRec->pRegion->Shift_Y == shifty &&
			pRegRec->pRegion->Sum_X == sumx && pRegRec->pRegion->Sum_Y == sumy
			)
			break;
	}

	if(ri < RegionCount) {
		ri++;
		while(ri < RegionCount) {
			Data[ri-1] = Data[ri];
			ri++;
		}
		RegionCount -= 1;
	}
}

void CRegionTable::DelRegion(CPoint p1, CPoint p2, int shiftx, int shifty, int sumx, int sumy)
{
	DelRegion(p1.x, p1.y, p2.x, p2.y, shiftx, shifty, sumx, sumy);
}

int CRegionTable::FindRegion(int Sx, int Sy, int Ex, int Ey)
{
	int ri;
	for( ri = 0; ri < RegionCount; ri++) {
		REGION_RECORD *pRegRec = &Data[ri];
		if(pRegRec->pRegion->Region.Sx == Sx && pRegRec->pRegion->Region.Sy == Sy &&
			pRegRec->pRegion->Region.Ex == Ex && pRegRec->pRegion->Region.Ey == Ey)
			return ri;
	}
	return -1;
}

int IsSameRegion(ST_IMG_REGION* reg1, ST_IMG_REGION* reg2)
{
	return
		reg1->Region.Sx == reg2->Region.Sx && reg1->Region.Sy == reg2->Region.Sy &&
		reg1->Region.Ex == reg2->Region.Ex && reg1->Region.Ey == reg2->Region.Ey &&
		reg1->Shift_X == reg2->Shift_X && reg1->Shift_Y == reg2->Shift_Y &&
		reg1->Sum_X == reg2->Sum_X && reg1->Sum_Y == reg2->Sum_Y;
}

