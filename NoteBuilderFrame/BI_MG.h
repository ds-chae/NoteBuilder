#ifndef _MG_H_
#define _MG_H_

#define MG_POW_HIGH				0
#define MG_POW_LOW				1

#define MG_LV_LOW				50
#define MG_LV_HIGH				40//90//65

#define MG_BUF					480 //2011.10.04 jhmoon
//#define MG_BUF					300 //size 1/2

#define MG_CH					5
#define MG_CH_1					0
#define MG_CH_2					1
#define MG_CH_3					2
#define MG_CH_4					3
#define MG_CH_5					4

#define MG_AD_1					0
#define MG_AD_2					1
#define MG_AD_3					2
#define MG_AD_4					3
#define MG_AD_5					4

	#define MG_CH_4_POS_X			(double)(-57.5/ENC_MM)
	#define MG_CH_5_POS_X			(double)(-43  /ENC_MM)
	#define MG_CH_3_POS_RX			(double)(-32  /ENC_MM) 
	#define MG_CH_3_POS_LX			(double)( 32  /ENC_MM) 
	#define MG_CH_1_POS_X			(double)( 43  /ENC_MM)
	#define MG_CH_2_POS_X			(double)( 57.5/ENC_MM)
// 2011.10.04 jhmoon, 재 본 바로는 119.74쪽이 더 가까운 것으로 측정되어 수정함.
//#define MG_POS_Y				(double)(119.74/ENC_MM)
#define MG_POS_Y				(double)(117/ENC_MM) // 좀 늦게 잡는 거 같아서 수정 함.


//#define MG_REGION			8 //아마 거의 USD를 위해서 사용될것이이다...
//#define MG_REGION_SIZE		(Uint16)(MG_REGION/ENC_MM) 

typedef struct ST_MG_INFO{

	Uint16 Peak_Max[MG_CH];// g_State 에 집어넣자.
	Uint16 Peak_Min[MG_CH];

	Uint16 LFill[4];
	Uint16 LEmpty[4];

	Uint16 RFill[4];
	Uint16 REmpty[4];

}ST_MG_INFO;

typedef struct ST_MG{

	Uint8  SEn, EEn, REn;

	Int16  SPos, EPos;
	Int16  Pos[MG_CH];
	Uint16 ADCnt[MG_CH];

	Uint16 Peak[MG_CH];
	Uint16 PeakSum;			// Peak 를 다 더한 값을 구한다.
	Uint32 Pow[MG_CH];
	Uint16 MaxDuty[MG_CH];//saturation구간을 알고 싶어서 10000원권 위폐 땀시.

	Uint16 PowBuf[MG_CH][MG_BUF];
	Uint16 Buf[MG_CH][MG_BUF];

	Uint16 Real_Start[MG_CH];
	Uint16 Note_MG_Tic;

	Uint16 Shifted_PowBuf[3][MG_BUF];
}ST_MG;

/*-------------- globals value define--------------*/
#ifdef GLOBAL_MG_DEF
#define GLOBAL_MG
#else
#define GLOBAL_MG extern
#endif

GLOBAL_MG int  ga_MG_Trace[MG_CH][3];
#define	MG_GAIN_SIZE	8
GLOBAL_MG Uint16  g_MgGain[8][MG_GAIN_SIZE];
GLOBAL_MG Uint16 AcceptMGFlag;
GLOBAL_MG int MGcnt[MG_CH];
#endif  // end of SNIPER_COMMON_H definition      

