#ifndef _MAINSTRUCT_H_
#define _MAINSTRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define ADC_SENSOR_FOR_NEW_MERLIN //ryuhs74@20120919 - New Merlin Board�� Reject/Stacker/Hopper ADC �߰� 

///////////////////////////////active struct
typedef struct ST_PTR {

//	Uint16 NoteCnt;
	Uint16 FCnt;
	Uint16 InCnt1;
	Uint16 InCnt2;

	Uint16 CIS;
	Uint16 CISPre;
	Uint16 CISR;
	Uint16 UV;
	Uint16 UVR;
	Uint16 MG;
	Uint16 MGR;
	Uint16 TDS;
	Uint16 TDSR;

	Uint16 Result;
	
	Uint16 Info_OS;

	Uint16 SerIndex;
	//	Uint16 Mux;
} ST_PTR;

typedef struct ST_FCNT{
	
	Uint16 StartedAtFcnt;
	Uint16 AEn;
	Uint16 EEn;

	Int16  Degree;
	double Angle;

	Int16 PrvDist;

	Int16 L_On, C_On, R_On;
	Int16 L_Of, C_Of, R_Of;
} ST_FCNT;

typedef struct ST_INCNT{
	
	Uint16 SEn;
	Uint16 EEn;
//	Uint16 SolEn;

	Int16 PrvDist;

	Int16 On;
	Int16 Of;

}ST_INCNT;

typedef struct ST_CNT_INFO{
	
	Uint16 FL;
	Uint16 FC;
	Uint16 FR;

	Uint16 In1;
	Uint16 In2;

}ST_CNT_INFO;

typedef struct ST_SEN{

	char   MsgBuf[5];
	Uint16 En;
	
	Uint16 OnCnt;
	Uint16 OffCnt;

#ifdef ADC_SENSOR_FOR_NEW_MERLIN 
	Uint8* ADCValue; //ryuhs74@20120919 - Hopper/Reject/Stacker ADC�� ���� �߰� 
#endif
}ST_SEN;

#define VELOCITY_BUF			100

typedef struct ST_TEST_INFO{
	
//	Uint16 Sol;
//	Uint16 Img;
	Uint16 Mode;


}ST_TEST_INFO;


typedef struct {

	Int16 Pos;
	Int16 Length;

	Uint16 Num;
	Uint16 Currency;
	Uint16 Dir;
	Uint16 Vers;
	Uint16 DenomIndex;
	Uint16 DenomIndex2;
	Uint32 DenomValue;

	int ratio0,ratio1,ratio2,ratio3;
	int ratioh0,ratioh1,ratioh2,ratioh3;

	char  Serial[24];

	char   ErMsgBuf[100];
	Uint16 ErMsgSize;	
	Uint16 ErFlag;

	ST_FCNT  FCnt;
	ST_INCNT InCnt1;
	ST_INCNT InCnt2;
	Uint16   SolFlag;
	Uint16   SolTime;

 // 2010/08/23 dschae, �� �̻� �ʿ� �����Ƿ� ���� ����. ������ ARGOS�� ����� MERLIN�� ��û�� ���� ������ ��������
 // �� ��° ������ ���������, ARGOS�� �����͸� ������ ����� MERLIN�� ����ϴ� ������ �ʿ䰡 ���� ������.
 //	Uint16   ArgosRCnt;
//	Uint16   ArgosResult; -- 20120207 ArgosResult��� �� �ʱ⿡ ����̽��� ���� �����̸�, TAPE �� ����� FResult�� �������Ƿ� �����Ѵ�.

	Uint16 Result; // UV, MG, TDS ������ �ִ��� ������ �δ� ����, dschae 2010/08/23�� �߰� ��
	Uint16 IsInCalibration;

	IMG_ANALYSIS_RESULT	IAR_A;	// A Side
	IMG_ANALYSIS_RESULT	IAR_B;	// B Side

	ST_UV    UV;
	ST_MG    MG;
	ST_TDS   TDS;
#ifdef _WINDOWS
	// _WINDOWS ȯ�濡���� ���� ����� ��� �ν��ϴ� �ùķ������̹Ƿ� ���� �� �鿡 ���� ���� �ʿ��ϴ�
	Uint16 UP_Dir;
	Uint16 UP_Vers;
	Uint16 UP_DenomIndex;
	Uint16 UP_DenomIndex2;
	Uint32 UP_DenomValue;
	// _WINDOWS ȯ�濡���� ���� ����� ��� �ν��ϴ� �ùķ������̹Ƿ� ���� �� �鿡 ���� ���� �ʿ��ϴ�
	Uint16 DN_Dir;
	Uint16 DN_Vers;
	Uint16 DN_DenomIndex;
	Uint16 DN_DenomIndex2;
	Uint32 DN_DenomValue;
#endif // _WINDOWS
} NOTE_STRUCT;

typedef struct ST_DENOM_VERSION {
	Uint16 Vers;
	Uint16 DenomIndex;
} ST_DENOM_VERSION;

#ifdef _WINDOWS
#define _CONST
#define	MAX_IMG_FOLDERS	64
#else
#define	_CONST const
#endif

#ifdef _WINDOWS
typedef struct tagST150SIZE {
	int cx;
	int cy;
} ST150SIZE;
#endif

typedef struct ST_LOCAL {
	_CONST char   *Currency;
	_CONST Uint16 CurrencyNum;    // num of Currency
	_CONST Uint16 MaxDenom;
	_CONST Uint32 *pDenom;   // Value of denomination  �׸��� 32bit�� �Ǵ� ��ȭ�� ���� �ִ�
	_CONST Uint16 *pDDLv;

	void (*CIS_Analysis)(NOTE_STRUCT * Note);
	void (*CIS_Verify  )(NOTE_STRUCT * Note);
	void (*CIS_Result  )(NOTE_STRUCT * Note);
	void (*TDS_Result  )(NOTE_STRUCT * Note);
	void (*MG_Result   )(NOTE_STRUCT * Note);
	void (*UV_Result   )(NOTE_STRUCT * Note);

	_CONST Uint16 Region_Value_Num;
	_CONST struct ST_IMG_REGION * pRegion_Value;
	_CONST Uint16 Region_Value_Num_NNR;
	_CONST struct ST_IMG_REGION* pRegion_Value_NNR;

	Uint16 ChainSizeMM;
	Uint16 ChainSizeTICK;
	Uint16 Fit_Buf_Num;
	Uint16 RecognitionType;
	Uint16 ImageAverageReinforce;

//#ifdef HORIZONTAL_INSER_TEST
//	Uint16 nDetectionVerticalLen; //ryuhs74@20120716 - Add Variable For Vertical Detection
//#endif
} ST_LOCAL;

#define	RECOG_METHOD_V1		1
#define	RECOG_METHOD_V2		2
#define	RECOG_METHOD_V3		3
#define	RECOG_METHOD_V4		4

#define	IMG_REINFORCE_NONE	0
#define	IMG_REINFORCE_TYPE1	1
#define	IMG_REINFORCE_TYPE2	2

#define OS_START				1
typedef struct ST_OSCOM{

	char  * RLength;
	char  * TLength;
	char  * RBuf;
	char  * TBuf;
	Uint8 * RFlag;
	Uint8 * TFlag;
	Uint16  RData;
	Uint16  RCnt;
	Uint16  TCnt;

	Uint16  Packet;
	Uint8   Active;	//DP data���� flag

} ST_OSCOM;

typedef struct ST_COUNT{

	Uint32 FCnt;

	Uint32 NCnt, NRej, NVal;
}ST_COUNT;

typedef struct ST_BATCHCNT {
	Uint32 NCnt;
	Uint32 NRej;
} ST_BATCHCNT;

// OS�� ���� �����͸� �غ��ϴ� ����
typedef struct ST_TX_STRUCT {
	int length;
	char buf[8192-2048];
} TX_STRUCT;

typedef struct ST_STATE {
	Uint16 EncCnt;
	Uint16 Prd_10ms;
	Uint16 Condition;
	Uint16 OSCondition;
	Uint16 CisCalibrated;
	Uint16 EnableRGB;
#if GET_DBL_IMAGE_IN_SERIAL
	Uint16 CisEnc;
#endif

	unsigned short SerImgNext;
	unsigned short SerImgSize;
	unsigned short SerImgSizeX;
	unsigned short SerImgSizeY;

	char   ErMsgBuf[100];
	Uint16 ErMsgSize;
	Uint16 ErFlag;

	ST_LOCAL *pCurrentLocal; // �ε��� ��� ������.
	Uint16 Mode;
	Uint16 Dir;
	Uint16 SavedVer; // �ʱ�ġ�� 9999
	Uint32 Denom;
	Uint16 Batch;
	Uint16 RejectBatch;
	Uint16 IsInCalibration;
	Uint16 DataTxReqStatus;
	Uint16 DataTxBits;
	Uint16 NoteTxNumBase;

	Uint32 TimeNano10_Prv;	//motor�� �ӵ� ��� ���� ������
	Uint32 Velocity_PrvPrv;
	Uint32 Velocity_Prv;
	Uint32 Velocity[VELOCITY_BUF];
	Uint16 Velocity_BufCnt;
	Uint16 Velocity_PrdCnt;

	Uint32 TimeMilli_Prv;  //note�� cnt �ӵ��� �����ϱ� ���� ������
	Uint32 Velocity_Note;

	Uint16 Type;
	Uint16 Speed;

	Uint16 UVMG_ADBuf[8];
	Uint16 TDS_ADBuf[12];

	ST_SEN Hopper;
	ST_SEN Stacker;
	ST_SEN Reject;
	Uint16 Sol_Dir;

	Uint16 SPIMode;
	Int16 Mux;
	Uint16 JamCnt;

	ST_CNT_INFO   Info_Cnt;
	ST_UV_INFO    Info_UV;
	ST_TDS_INFO   Info_TDS;

	ST_IMG_RATE   Rate_General;     // scan size�� ���� xy ����

	Uint16 ControlByHost;	// ARM HOST�� ���� �����ϴ� ���. 0�̸� DSP ȥ�� ����.
	Uint16 OS_Request;
	Uint16 SPByVersion;

// Fitness Level
	Uint16 Fit_Tape_Level;
	Uint16 Fit_Hole_Level;
	Uint16 Fit_Soiled_Level[10]; //ryyuhs74@20120618 - For RSD
//////////////// Moon. ���� EUR�� fixed��. �Ŀ� ��� �������� ���ľ� ��
//////////////// �׷����� ���Ⱑ �ƴ� ������ ���� �� ��
	Uint16 Fit_Deinked_Level;
	Uint16 Fit_Stain_Level;
	Uint16 Fit_Washed_Level;

// CF on/off
	Uint16 CF_On_Tag;
	Uint16 MG_On_Tag;
	Uint16 MT_On_Tag; //ryuhs74@20120322 - SYP ���÷� ���� ���� 
	Uint16 CNT_DDLV;
	Uint16 TxMethod;
	Uint16 CNT_SIZE_YMM; // in millimeter
	Uint16 CNT_SIZE_YTICK; // in millimeter
	Uint16 ATM_On_Tag;
	Uint16 Graffiti_On_Tag;

	Uint16 CNT_MG_LEVEL;	//CNT�� MG Level�� �ޱ����� ����
	Uint16 CNT_UV_LEVEL;	//CNT�� UV Level�� �ޱ����� ����
	Uint16 VALUE_RESET;
	Uint16 CIS_TYPE;
#if 0 // �� ������ ���������� �����ε� ������ �ҽ��� �����ִ�. ���� ����. 20120331 dschae
	Uint16 Encoder4Rate; // Main Encoder 4times.
#endif // �� �ּ��� ������ ������ ������ �𸣴� ����� ������ �߰��� �ҽ��� ������ �� �ٽ� �߰��� ������ �ִ�.
	Uint16 RotationTick;

	Uint16 ArgosVers;

	Uint16 Serial_Img_On;

	int LastRefIR;
	int LastRefW;

#ifdef CNT_SIZE_ON_TEST
	Uint16 CNT_SIZE_ON;
	Uint16 Saved_CNT_SIZE;
	Uint16 CNT_SIZE_X;
	Uint16 CNT_SIZE_Y;
	Uint16 CNT_SIZE_DIFF;
#endif

} ST_STATE;

#define	GRAY_IMAGE_TX_ENABLED	1
#define	IR_IMAGE_TX_ENABLED	2
#define	KEY_TX_ENABLED	4

#define	OS_TX_DMA	1
#define	OS_TX_1BYTE	2
#define	OS_TX_4BYTES	3

#define	NO_DATA_XFER		0
//#define	IMG_TX_REQUESTED	1
#define	DATA_TX_PROCESSING	2
//#define	MG_TX_REQUESTED		3
//#define	MG_TX_PROCESSING	4
//#define	TAPE_TX_REQUESTED	5
//#define	TAPE_TX_PROCESSING	6
#define	TEXT_TX_PROCESSING	7

#define	IMG_TX_BIT	0x01
#define	MG_TX_BIT	0x02
#define	TAPE_TX_BIT	0x04
#define	SER_TX_BIT	0x08

#define TEST_VOLTEST //ryuhs74@20120515 - Volume Test Image Capture, test �Ϸ��� ����� ���� ����� define
#ifdef TEST_VOLTEST
#define IMG_TX_BIT_VOLTEST 0x10 //ryuhs74@20120515 - Volume Test Image Capture
#endif

#define	ERROR_CIS_CLEAR_SENSOR	1
#define	ERROR_CIS_NO_REFMARKER	2
#define	ERROR_CIS_UNSTABLE		3
#define	ERROR_CIS_ADJUNSTABLE	4
#define	ERROR_CIS_NEEDADJUST	5
#define	ERROR_CIS_NOIMAGE		6
#define	ERROR_CIS_CHECK_SENSOR	7
#define	ERROR_CIS_CLEAR_SENSOR2	8

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // _MAINSTRUCT_H_
