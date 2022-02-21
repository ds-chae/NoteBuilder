#ifndef _TDS_H_
#define _TDS_H_

#define TDS_BUF					140

#define TDS_MUX					4
#define TDS_CH					12
#define TDS_CH_1				0
#define TDS_CH_2				1
#define TDS_CH_3				2
#define TDS_CH_4				3
#define TDS_CH_5				4
#define TDS_CH_6				5
#define TDS_CH_7				6
#define TDS_CH_8				7
#define TDS_CH_9				8
#define TDS_CH_10				9
#define TDS_CH_11				10
#define TDS_CH_12				11

#define TDS_AD_1				0
#define TDS_AD_2				1
#define TDS_AD_3				2

//15.2
#define TDS_INTERVAL			(double)15.2
#define TDS_CH_1_POS_X			(double)((-83.6 +                       FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_2_POS_X			(double)((-83.6 + (TDS_INTERVAL * 1)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_3_POS_X			(double)((-83.6 + (TDS_INTERVAL * 2)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_4_POS_X			(double)((-83.6 + (TDS_INTERVAL * 3)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_5_POS_X			(double)((-83.6 + (TDS_INTERVAL * 4)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_6_POS_X			(double)((-83.6 + (TDS_INTERVAL * 5)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_7_POS_X			(double)((-83.6 + (TDS_INTERVAL * 6)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_8_POS_X			(double)((-83.6 + (TDS_INTERVAL * 7)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_9_POS_X			(double)((-83.6 + (TDS_INTERVAL * 8)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_10_POS_X			(double)((-83.6 + (TDS_INTERVAL * 9)  + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_11_POS_X			(double)((-83.6 + (TDS_INTERVAL * 10) + FSEN_SHIFTX) /ENC_MM)
#define TDS_CH_12_POS_X			(double)((-83.6 + (TDS_INTERVAL * 11) + FSEN_SHIFTX) /ENC_MM)

#define TDS_POS_Y				(double)(46.5 /ENC_MM)


//#define TDS_REGION			8 //�Ƹ� ���� USD�� ���ؼ� ���ɰ����̴�...
//#define TDS_REGION_SIZE		(Uint16)(TDS_REGION/ENC_MM) 

typedef struct ST_TDS_INFO{

	Uint16 Peak_Max[TDS_CH];// g_State �� �������.
	Uint16 Peak_Min[TDS_CH];

	Uint16 OffPr[TDS_CH];
	Uint16 Off[TDS_CH];
} ST_TDS_INFO;

typedef struct ST_TDS{

	Uint8  SEn, EEn, REn;

	Int16  SPos, EPos;
	Int16  Pos[TDS_CH];
	Uint16 ADCnt[TDS_CH];

	Uint16 ThickBuf[TDS_CH][TDS_BUF];

//-- 2010/08/23 ARGOS���� �ڵ� ������ ���鼭 �߰��� �ʵ� ����
 	Uint32 RawSum[TDS_CH];
 	Uint32 MultSum[TDS_CH];
	Uint32 RawAverage[TDS_CH];
	Int16 g_Tape_Channel;
	Int16 g_Tape_Pos;
//-- 2010/08/23 ARGOS���� �ڵ� ������ ���鼭 �߰��� �ʵ� ��
#ifdef _WINDOWS
	int MaxADC;
#endif
} ST_TDS;

/*-------------- globals value define--------------*/
#ifdef GLOBAL_TDS_DEF
#define GLOBAL_TDS
#else
#define GLOBAL_TDS extern
#endif

GLOBAL_TDS	Uint16 g_TapeGain[EB_TAPE_GAIN_SIZE];
GLOBAL_TDS	Uint16 g_TapeMult[EB_TAPE_GAIN_SIZE];
GLOBAL_TDS	Uint16 TapeRawBuf[TDS_CH][TDS_BUF];
#if 0 // �� ������ ���������� �����ε� ������ �ҽ��� �����ִ�. ���� ����. 20120331 dschae
GLOBAL_TDS	Uint16	Tape_Encoder4RateSkip[TDS_MUX];
#endif // �� �ּ��� ������ ������ ������ �𸣴� ����� ������ �߰��� �ҽ��� ������ �� �ٽ� �߰��� ������ �ִ�.

#define	TAPE_SENSE_AVERAGE	100
#define	TAPE_BASE_THICKNESS	(TAPE_SENSE_AVERAGE*100) // 100 * 100�� �⺻ �β��� ģ��.
#define	TAPE_NO_PAPER_LIMIT	((TAPE_BASE_THICKNESS*60)/100) // 60% �̸��̸� ���̰� �ƴ� �ɷ� �����Ѵ�.
#define	TAPE_ADC_LOW_LIMIT	40

GLOBAL_TDS	Int16	g_PrevVThickChannel;
GLOBAL_TDS	Int16	g_SameVThickCount;
GLOBAL_TDS	Int16	g_SameVThickIgnoreLimit;

#endif  // end of _TDS_H_ definition      

