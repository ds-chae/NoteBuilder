#ifndef _UV_H_
#define _UV_H_

#define UV_LED					80

#define UV_BUF					170		// 500유로같이 큰 게 skew가 나면 커지더라


#define UV_MUX					4
#define UV_CH					4
#define UV_CH_1					0
#define UV_CH_2					1
#define UV_CH_3					2
#define UV_CH_4					3

#define UV_AD_F					5
#define UV_AD_R					6
#define UV_AD_C					7

#define UV_CH_1_POS_X			(double)((-46.8 + FSEN_SHIFTX)/ENC_MM)
#define UV_CH_3_POS_X			(double)((-28.2 + FSEN_SHIFTX)/ENC_MM)
#define UV_CH_2_POS_X			(double)(( 28.2 + FSEN_SHIFTX)/ENC_MM)
#define UV_CH_4_POS_X			(double)(( 46.8 + FSEN_SHIFTX)/ENC_MM)

#define UV_POS_Y					(double)(  71.5/ENC_MM)


typedef struct ST_UV_INFO{
	Uint16 UVGain[2];
#if ENABLE_UV_OFFSET_FROM_MERLIN
	Uint16 UVOffset[UV_CH]; // Offset,Offset-Pre, Offset,Offset-Pre
#endif
	Uint16 UVMultF[UV_CH];
	Uint16 UVMultR[UV_CH];
}ST_UV_INFO;

#define	ADJ_UVF(f,b,g) (((f)-(b))*200)/(g)
#define	ADJ_UVR(r,b,g) (((r)-(b))*100)/(g)

#define	UV_T_OFFSET	0		// Transmission values start at 0
#define	UV_R_OFFSET	UV_CH	// Reflection values start at UV_CH

typedef struct ST_UV{

	Uint8  SEn, EEn, REn;

	Int16  SPos, EPos;
	Int16  Pos[UV_CH];
	Uint16 ADCnt[UV_CH];

	Uint32 Sum_F[UV_CH];
	Uint32 Sum_R[UV_CH];

	Uint32 Avg_F[UV_CH];
	Uint32 Avg_R[UV_CH];

	Uint16  Buf_F[UV_CH][UV_BUF];// 이게 투과광이다
	Uint16  Buf_R[UV_CH][UV_BUF];

	Uint16 Real_Start[UV_CH];
	Uint16 SumCnt[UV_CH];
	Uint16 Diff_Sum[UV_CH];

//#ifdef _WINDOWS - DrawUV가 수정되면서 필요 없어져서 삭제했다 20120331 dschae
//	int MaxADC;
//#endif
}ST_UV;

/*-------------- globals value define--------------*/
#ifdef GLOBAL_UV_DEF
#define GLOBAL_UV
#else
#define GLOBAL_UV extern
#endif


//GLOBAL_UV struct ST_UV_INFO gst_CUV;//g_State 에 집어넣자.
GLOBAL_UV int uv_length[UV_CH];


#endif  // end of SNIPER_COMMON_H definition      

