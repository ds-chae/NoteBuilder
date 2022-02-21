#ifndef _CIS_H_
#define _CIS_H_

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////// CIS Info
#define CIS_ENC						2                      // 0.5�϶� 2 line  

#define NOTE_MIN_X					100                    // 120�� 5 EURO �����̴�..
#define SIZEM_X						(Uint16)(3/X_RATE)  // CIS�� �������� Y���� ����
#define SIZEM_Y						(Uint16)(3/ENC_MM)  // counter sensor�������� X���� ����

// �� ��ġ�� ������ 13.5���µ� �̷��� ���� �߸���. ��Ʋ���� ���� �¿� 90-4.5mm�� �� ����ϱ� ������ �� �߸� ���̴�.
#define CIS_POS_Y					(double)(  10.5            /ENC_MM)  // 18.5
#define CIS_POS_LX					(double)((-90 + FSEN_SHIFTX)/ENC_MM)
#define CIS_POS_RX					(double)(( 90 - FSEN_SHIFTX)/ENC_MM)
// �������� 4.5mm ������ ����ϹǷ� ���������� 4.5mm ������ ����ؾ� �Ѵ�. ������ �ڵ�� ������ 4.5mm �� ������ ����ߴ�.
////////////////////////////// CIS Info

#define NOTE_BUF					26
#define IMG_BUF_X					1536	// 1728				
#define IMG_BUF_Y					800
// vertical size of serial image
#define	SER_BUF_Y					30
// horizontal size of serial image
#define	SER_BUF_X					500
// How many serial images are stored at one batch
#define	SER_IMG_MAX					300

#define CIS_LEFT_LIMIT				24
#define	CIS_RIGHT_LIMIT				(IMG_BUF_X-CIS_LEFT_LIMIT)

#define IMG_CENTER_X				(Uint16)((CIS_RIGHT_LIMIT+CIS_LEFT_LIMIT)/2)

#define CIS_MARGIN_Y				1 // �������� 2����. 20010413 dschae, �̹����� �߸��� ������ �־ 1�� �ٲ�

#define IMG_LV_W					30
#define IMG_LV_IR					180 //ryuhs74@20121126 - New CIS ������ D_CIS_Find_Outline.c ���Ͽ��� Note OutLine ���������θ� ���δ�. Old CIS������ TestCISClear/WaitStableCIS �Լ������� ����Ѵ�.
#define IMG_LV_REFLECT				80 // ��� �ݻ� �̹����� ��쿡 ��ο� ������ ���� �̹����� �ö�´�. �� ���� ���� ��輱���� ����.

#define CIS_RATE_X					0.127
#define CIS_RATE_Y					0.508
#define CIS_RATE_Y1					0.254

#define CIS_RATE_XY					(double)(CIS_RATE_X/CIS_RATE_Y)
#define CIS_RATE_YX					(double)(CIS_RATE_Y/CIS_RATE_X)
#define CIS_RATE_XY1				(double)(CIS_RATE_X/CIS_RATE_Y1)
#define CIS_RATE_YX1				(double)(CIS_RATE_Y1/CIS_RATE_X)

#define IQ_CIS_RATE_XY				_IQ20(CIS_RATE_XY)
#define IQ_CIS_RATE_YX				_IQ20(CIS_RATE_YX)
#define IQ_CIS_RATE_XY1				_IQ20(CIS_RATE_XY1)
#define IQ_CIS_RATE_YX1				_IQ20(CIS_RATE_YX1)

#define NOTE_MIN_SIZE_X				(Uint16)(120       /CIS_RATE_X)		

#define POINT_NUM_TB				20	// Top Point Pixel Number
#define	POINT_NUM_LR				20	// Left Point Pixel Number

#define IMG_BUF_NUM					7000	// ���� ����ϴ� ����� 96x50=4800������, ���Ŀ� 104x60���� �ø� ���ɼ��� �־ 7000���� ��Ҵ�.
//////////////////////////// calibration define
#define	PIXEL_START					200
#define	PIXEL_END					1200

#define	RAW_Y_SIZE	100

#define	ADJ_PIXEL_START	100
#define	ADJ_PIXEL_END	1400

#define	GET_Y_START					10
#define GET_Y_SIZE					64
#define	GET_Y_END					(GET_Y_START+GET_Y_SIZE)	// �� ���� ���� RAW_IMAGE_SIZE�� �ʰ��ϸ� �ȵȴ�.

#if (GET_Y_END >= RAW_Y_SIZE)
#error GET_Y_END should be less than RAW_Y_SIZE
#endif

#define CIS_CH						4
#define CIS_TIR						0
#define CIS_BW						1
#define CIS_TW						2
#define CIS_BIR						3

#define BRIGHT_CH					4
#define BRIGHT_TIR					0 // - ��� IR_BrightByCurrency�� ��ü �ȴ�. 20110408 dschae
#define BRIGHT_TWR					1
#define BRIGHT_TWG					2
#define BRIGHT_TWB					3
/////////////////////////// calibration define
// Direction Define

typedef struct ST_IMG_REGION{

	ST_SQUARE Region;

	Int16 Shift_X;       //�̹��� ���� ����
	Int16 Shift_Y;
	Int16 Sum_X;         //�̹��� ���� ����
	Int16 Sum_Y;
#ifdef _WINDOWS
	float Threshold;
#endif
} ST_IMG_REGION;

#define	REGION_IMG_TOPREF	1	// Top reflection Image
#define	REGION_IMG_IRTRNS	2	// IR Transmission Image

#define	REGION_MTD_1		1	// First Method - EUR, INR, RUB, etc.
#define	REGION_MTD_2		2	// Second Method - for USD, Under development at July 2011

typedef struct ST_IMG_RATE{

	double Rate_XY;    //x�� y�� �� ��ʰ��迡 ���� ����
	double Rate_YX;    //�ӵ��� �������� ��ʰ��谡 ���Ҽ� �ִ�.
	_iq    iqRate_XY;
	_iq    iqRate_YX;

}ST_IMG_RATE;

typedef struct ST_CIS_INFO_FLAG{

	unsigned char Bright_TIR;
	unsigned char Bright_TW;
	unsigned char Offset;
	unsigned char Gain;
	Uint16        Cnt;

}ST_CIS_INFO_FLAG;

#define	ISO_CURRENCY_COUNT	249

#ifndef _WINDOWS
typedef struct ST_CIS_INFO{

//	ST_CIS_INFO_FLAG Flag;
	ST_DMA_INFO      Odd_DMA_Ch;
	ST_DMA_INFO      Even_DMA_Ch;

	Uint16 Bright[BRIGHT_CH];
	Uint16 TW_Offset[IMG_BUF_X]; // Top Grey Offset, // Dark Coef. Memory  [DDR2]
	Uint16 TIR_Offset[IMG_BUF_X]; // Top IR Offset
	Uint16 TW_Gain[IMG_BUF_X];	// Top Grey Gain	// White Coef. Memory [DDR2]
	Uint16 TIR_Gain[IMG_BUF_X]; // Top IR Gain
// dschae20120730, IR ��⸦ ���� Ŀ���ø��� �����ϰ� �Ǿ��ִ� �κ��� �����Ѵ�.
// EUR���� ���� ��� �簳�� �Ǿ����Ƿ� IR�� �� �ϳ��� ���� �����ϸ� �ȴ�.
#define IR_BRIGHT_TO_EACH_CURRENCY	0
#if IR_BRIGHT_TO_EACH_CURRENCY
	// �� �������� �ƿ� ���ֹ�����, ���� DSP �������� FRAM�� ��ϵǾ� �ִ� �������� �ջ�� ������ �ִ�.
	// ���� �������� 1�� ���Ͽ� �������� ����� ������ �ǰ� �Ѵ�. dschae20120730
	Uint16 IR_BrightByCurrency[ISO_CURRENCY_COUNT];
	Uint16 IR_TargetByCurrency[ISO_CURRENCY_COUNT];
#else
	Uint16 IR_BrightByCurrency1[ISO_CURRENCY_COUNT];
	Uint16 IR_TargetByCurrency1[ISO_CURRENCY_COUNT];
#endif
	Uint16 W_Target;
	Uint16 LastRefSX;
	Uint16 RefX[2];
} ST_CIS_INFO;
#endif

#define SIFT_KEY_VECTOR				8
#define V1_SIFT_DESCRIPTER_NUM			20
#define SIFT_DESCRIPTER_NUM			100
typedef struct ST_IQDESCRIPTOR{

	int	x,y;
	int dir;
	_iq V1_DesVector[SIFT_KEY_VECTOR];
	Uint8 DesVector;

}ST_IQDESCRIPTOR;


typedef struct {

	Uint8  SEn, EEn;
	Uint8  CISAnalysisEnd, REn;

	Int16  SL,   SR;
	Int16  SPos, EPos;

	Uint16 DDValue;
	Uint16 DDValueW;
	Uint16 CrackCnt;

	Int16 Enc_Cnt;
	Int16 IRCnt_Y;
	Int16 WCnt_Y;
	Int16 Center_X;
	Int16 Center_Y;
	Int16 ImgCenterX;

	Int16 Start_X;
	Int16 End_X;

	Int16 Size_X;
	Int16 Size_Y;

	Int16 Feature_X;
	Int16 Feature_Y;

	double Degree;
	double Degree_T,  Degree_B,  Degree_L,  Degree_R;
	double Contect_T, Contect_B, Contect_L, Contect_R;

	ST_POINT stPoint_T[POINT_NUM_TB];
	ST_POINT stPoint_B[POINT_NUM_TB];
	ST_POINT stPoint_L[POINT_NUM_LR];
	ST_POINT stPoint_R[POINT_NUM_LR];
	ST_POINT stAngular_TL;// 4 �𼭸�.	Top Left Point
	ST_POINT stAngular_TR;		// Top Right Point
	ST_POINT stAngular_BL;	// Bottom Left Point
	ST_POINT stAngular_BR;		// Bottom Right Point

	_iq iqSin; //�׳� angle��
	_iq iqCos;

	_iq iqBase_X;
	_iq iqBase_Y;

	Uint16 KeyCount;

	Uint16 Max_First_Denom;
	Uint16 Max_First_Cnt;
	Uint16 Max_Second_Denom;
	Uint16 Max_Second_Cnt;

	Uint16 VerCnt[10];						// version ���п�, [0] Old or New, [1] VNew or not
} IMG_ANALYSIS_RESULT;


#define AREA_NUM			4

#define NNR_HIDDEN_BUF		80 // 40->80 20120621 dschae
#define NNR_OUT_BUF			80

/////////////////////////////////////
// ���� IR ������ ���� �߰�.
typedef struct ST_IR{					// ���� ������ IR ��ġ�� OS_TX �� �����Ͽ� Ȯ���ϱ� ���Ͽ� ���� ������ �߰���.
	Uint8  Standard;					// IR ���� ��質 �ð����� ���ϹǷ�, ������ �Ͼ� ������ ��հ��� �����Ͽ� bright �� ���ϴ� ���ذ����� ����.
	Uint16 RgnVal[10];					// IR �� ������ ��� ���� ������.
}ST_IR;


struct ST_AREA{
	int sx, sy;
	int ex, ey;
	Uint16 state;
};

struct ST_NNR_REGION{

	struct ST_AREA  Area[AREA_NUM];

	Uint16 XRate;
	Uint16 YRate;
	Uint16 Normalize;
};

struct ST_NNR_EXTRACT{

	struct ST_AREA  Area[AREA_NUM];
	struct ST_POINT Size[AREA_NUM];

	Uint16 XRate;
	Uint16 YRate;

	long   ShiftVal;   //centering�� ���� value
	long   PixelMin;   //stretching�� �ϱ� ���� minvalue
	double StretchRate;
	double HalfStretchRate;
};

#ifndef ST_NNR_RESULT_DEF
#define	ST_NNR_RESULT_DEF
struct ST_NNR_RESULT {
	float  MaxRate;
	Uint16 MaxIndex;

	float  SecondMaxRate;
	Uint16 SecondMaxIndex;

	Uint16 SecondVers;
	Uint32 SecondDenom;
	Uint16 SecondDir;
};
#endif

struct ST_CIS_IQ{
	_iq iqSin;
	_iq iqCos;

	_iq iqSinS;
	_iq iqCosS;

	_iq iqBaseLTX;
	_iq iqBaseLTY;

	_iq iqBaseCX;
	_iq iqBaseCY;
};

#define X_RATE				0.125//mm����
#define Y_RATE				(double)(ENC_MM*CIS_ENC)//2.4mm
#define YX_RATE				(double)(Y_RATE/X_RATE)
#define XY_RATE				(double)(X_RATE/Y_RATE)
#define IQ_YX_RATE			_IQ20(YX_RATE)
#define IQ_XY_RATE			_IQ20(XY_RATE)


#ifdef	GLOBAL_CIS_DEF
#define GLOBAL_CIS
#else
#define GLOBAL_CIS extern
#endif

#define	MAX_REGIONS	8

#ifndef _WINDOWS
GLOBAL_CIS ST_CIS_INFO gst_CIS_Info;
#endif

//GLOBAL_CIS ST_CIS      gst_Coordinate;
GLOBAL_CIS ST_SQUARE   gst_Region;


//�̹��� ����� 1.2M

GLOBAL_CIS Uint8 g_Line_OddBuf[2][IMG_BUF_X];			    // Line Image [L2RAM] 
GLOBAL_CIS Uint8 g_Line_EvenBuf[2][IMG_BUF_X];			// Line Image [L2RAM] 

#define	EUR_SERIMG_XSIZE	280
#define	EUR_SERIMG_YSIZE	13

#define	TRY_SERIMG_XSIZE	270
#define	TRY_SERIMG_YSIZE	18

#define	USD_SERIMG_XSIZE	370
#define	USD_SERIMG_YSIZE	14

//GLOBAL_CIS Uint8 g_ImgW_Buf[NOTE_BUF][IMG_BUF_Y][IMG_BUF_X];	//�̰� �̹��� �޴� �� //ryuhs74@20120511 - Reject �� �̹��� ���� �߰� 
//GLOBAL_CIS Uint8 g_ImgIR_Buf[NOTE_BUF][IMG_BUF_Y][IMG_BUF_X];
GLOBAL_CIS Uint8 g_ImgSerBuf[SER_IMG_MAX+1][SER_BUF_Y*SER_BUF_X]; // Note.Num�� 1���� �����ϹǷ� ����ϱ� ���� ���۸� �ϳ� �� ��´�.
GLOBAL_CIS Uint8 g_SerImgRdy[SER_IMG_MAX+1];

#define	REGION_BUF_SIZE	30000
#define SIFT_IMG_BUF		10000

GLOBAL_CIS char g_KeyTxBuf[REGION_BUF_SIZE]; // 16384 OS�� ������ Key �ӽ� ���� ����


GLOBAL_CIS long  g_iqRegionW_Buf[MAX_REGIONS][REGION_BUF_SIZE];
GLOBAL_CIS long  g_iqRegionIR_Buf[MAX_REGIONS][REGION_BUF_SIZE];
//GLOBAL_CIS int   t_IMG[576];
#if _WINDOWS // ���� ȯ�濡�� ���� ���⸦ ���� ����
GLOBAL_CIS int WinExtPosX[2][2];
GLOBAL_CIS int WinExtPosY[2][2];
#endif

GLOBAL_CIS Uint32 TxImgDelay; // Sys_Check_Prd�� �̹��� ���� �߿� �����ϰ� �ϱ� ���� ��

GLOBAL_CIS Uint8 g_RegionW_Buf[MAX_REGIONS][REGION_BUF_SIZE];
GLOBAL_CIS Uint8 g_RegionIR_Buf[MAX_REGIONS][REGION_BUF_SIZE];
GLOBAL_CIS int   g_RegionWidth[MAX_REGIONS];
GLOBAL_CIS int   g_RegionHeight[MAX_REGIONS];

GLOBAL_CIS Uint8 g_FitW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_temp_FitW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_temp_diffW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_stainW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_deinkedW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_soiledW_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_soiledm_Buf[MAX_Fit_Buf_Num];
GLOBAL_CIS Uint8 g_RegionCIS[60];				// max x�� 60���� �Ѵ�. �ٲ�� �ٲ���
GLOBAL_CIS int g_CF_Level[2];
GLOBAL_CIS int Size_Margin[2];

GLOBAL_CIS unsigned char g_TestImageBCC;
				
#define LED_IR_VALUE		45//45	// 20
#define LED_TOP_R_VALUE		70//100	// 120
#define LED_TOP_G_VALUE		70//100	// 120
#define LED_TOP_B_VALUE		70//100	// 120

GLOBAL_CIS struct ST_NNR_EXTRACT gst_Extract;
GLOBAL_CIS struct ST_CIS_IQ      gst_IQ;

//NNR ���� 
#define	NNR_FEATURE_BUF	1200
GLOBAL_CIS long   g_Feature[NNR_FEATURE_BUF];
GLOBAL_CIS long   g_HiddenBuf[NNR_HIDDEN_BUF];
GLOBAL_CIS float  g_OutBuf[NNR_OUT_BUF];

GLOBAL_CIS Uint16 g_FeatureCnt;

GLOBAL_CIS Uint8  V3_dogs[IMG_BUF_NUM];
GLOBAL_CIS Uint8  V3_blur[2][IMG_BUF_NUM];

#if V2_VARIABLE_INIT_TEST1
GLOBAL_CIS _iq iq_gau[9][9];
#endif

// SYP ������ �߰���
#if _DEBUG
GLOBAL_CIS Uint16 Max_Rate[10];
GLOBAL_CIS Uint16 Min_Rate[10];
#endif

#ifdef __cplusplus
}
#endif

#endif         /* _CIS_H_ */


