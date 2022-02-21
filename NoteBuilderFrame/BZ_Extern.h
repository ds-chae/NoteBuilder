#ifndef _BZ_EXTERN_H_
#define _BZ_EXTERN_H_

#ifndef _WINDOWS
extern EDMA3_DRV_Handle hEdma;

EDMA3_DRV_Result edma3init (void);
EDMA3_DRV_Result edma3deinit (void);
EDMA3_DRV_Result Edma3_CacheFlush(Uint32 mem_start_ptr, Uint32 num_bytes);
EDMA3_DRV_Result Edma3_CacheInvalidate(Uint32 mem_start_ptr, Uint32 num_bytes);

////////////////////////////////////////////Z_Convert.c
extern void ftoa(float Val, char *Buf); // 999.999까지

////////////////////////////////////ctl DMA
extern void InitImageDMA(void);
extern EDMA3_DRV_Result Get_DMA(ST_DMA_INFO * stDma);
extern EDMA3_DRV_Result edma3_MemCopy(ST_DMA_INFO stDma);
extern void callbackCheck(Uint32 tcc, EDMA3_RM_TccStatus status, void *appData);

////////////////////////////////////ctl FRam
extern unsigned char FRam_Read(Uint16 Addr);
extern void FRam_Write(Uint16 Addr, Uint8 Data);

////////////////////////////////////Adj cis coef

////////////////////////////////////Init Sys
extern void _wait(int delay);
extern void OnTargetConnect();

///////////////////////////////////Init gpio
extern void Init_GPIO(void);
extern void Init_GPIO_Mux(void);
extern void Init_GPIO_FPGA(void);
extern void Init_GPIO_ARGOS(void);
extern void Init_Device_GPIO(void);
extern void SPI_CS_FPGA(Uint8 nDevice);
extern void GPIO_Write(Uint32 port, Uint32 state);
extern Uint8 GPIO_Read(Uint32 port);

///////////////////////////////////Init Mcbsp
extern void Change_Frq_Mcbsp(Uint32 Frq);
extern void Change_TxBit_Mcbsp(Uint32 Bit);

extern void Init_Mcbsp(void);
extern void Init_Device_Mcbsp(void);

extern void Init_Uart1(void);
extern void PutUart1(char c);
///////////////////////////////////Init CIS
extern void Init_CIS(void);
extern void Init_CIS_ADC(void);
extern void CIS_ADC_Default_Load(void);

void CIS_Coef_Tx(Uint16 Ch, Uint16 *OffsetArr, Uint16 *GainArr);
extern Uint16 CIS_LED_Tx(Uint16 Ch, Uint16 Data);
extern void CIS_ADC_Tx(Uint16 Data);
extern unsigned char CIS_ADC_Rx(Uint16 Addr);

///////////////////////////////////Init PWM
extern void Adj_KickerSpeed(Uint32 Speed);
extern void Adj_MainSpeed(Uint32 Speed);
extern void Adj_StackerSpeed(Uint32 Speed);

extern void Init_PWM0(void);
extern void Init_PWM1(void);
extern void Init_PWM2(void);
extern void Init_Device_PWM0(void);
extern void Init_Device_PWM1(void);
extern void Init_Device_PWM2(void);

///////////////////////////////////CA Seq F
extern void Main_Enc_isr(void);
extern void Kicker_Enc_isr(void);

void Act_Seq(void);
void Seq_FCntStartInit(NOTE_STRUCT * Note);
void Seq_FCntEndInit(NOTE_STRUCT * Note);
void Seq_FCntNote(NOTE_STRUCT * Note);//각종 note의 error및 start pos및 length등을 구하는 루틴
void CountFCntSensorOnOff(Uint8 SenL, Uint8 SenC, Uint8 SenR, NOTE_STRUCT *Note);

//////////////W_Protocol_OS
void OS_RxData(char * Buf, Uint16 Cmd);
void OS_RxParse(void);
void OS_TxState(void);
void OS_TxKey(int version, NOTE_STRUCT* Note);
void OS_TxFit(NOTE_STRUCT* Note);
void OS_TxMG(NOTE_STRUCT* Note);
void OS_TxTape(NOTE_STRUCT* Note);
void OS_TxUV(NOTE_STRUCT* Note);
#if TX_IMG_BATCH
void OS_TxImg(NOTE_STRUCT* Note, int ySize, char imgType, Uint8 ImgBuf[][IMG_BUF_X]);
void OS_TxSerImg(short Num);
#else
void OS_TxImg(NOTE_STRUCT* Note, Uint8 ImgBuf[][IMG_BUF_X]);
#endif
void TxAllImage();
void TxAllSerImage();

void OS_TxCnt(NOTE_STRUCT* Note, ST_COUNT Cnt);
void OS_CheckTxBuf(void);

void OS_TxUVLowHigh();
void OS_TxInitStruct(TX_STRUCT *pTx);
void OS_TxStream(TX_STRUCT *pTx, const char * ptr);
void OS_TxStream1(const char * ptr);
void OS_TxSubmit(TX_STRUCT *pTx);
void OS_TxBits16(TX_STRUCT* pTx, Uint16 v16);
void OS_TxFlush(TX_STRUCT* pTx);
void InitializeOS_TxSystem();
long FPGA_GetVersion();
void OS_TxHex16(TX_STRUCT* pTx, Uint16 u16);
void OS_TxLong(TX_STRUCT* pTx, long l);

#endif // ifndef _WINDOWS

//////////////D_CIS_DefineOuter
#ifdef __cplusplus
extern "C" {
#endif

void InitializeGaussian9x9();
void FindOutline(IMG_ANALYSIS_RESULT* CIS, ST_IMG_RATE * Rate, Uint8 ImgBuf[][IMG_BUF_X]);
void V1_Recognition(float Threshold, NOTE_STRUCT * Note, long  RegionBuf[]);
void Recognition(float Threshold, int RegionNumber, NOTE_STRUCT * Note, long  RegionBuf[]);
void Recognition_8bit(int RegionNumber, NOTE_STRUCT * Note, Uint8 RegionBuf[], Uint8 RegionIRBuf[]);
void ExtRegion(IMG_ANALYSIS_RESULT* CIS, ST_IMG_RATE * Rate, ST_IMG_REGION * Region, long ExtBuf_W[], Uint8 ImgBuf_W[][IMG_BUF_X]);// base center
// 8비트 이미지 영역 추출
void ExtRegion8(IMG_ANALYSIS_RESULT* CIS, ST_IMG_RATE * Rate, ST_IMG_REGION * Region, Uint8 ExtBuf_W[], Uint8 ExtBuf_IR[], Uint8 ImgBuf_W[][IMG_BUF_X], Uint8 ImgBuf_IR[][IMG_BUF_X]);// base center
void V3_Recognition(NOTE_STRUCT * Note, Uint8  RegionBuf[]);
void V3_ExtRegion(IMG_ANALYSIS_RESULT* CIS, ST_IMG_RATE * Rate, ST_IMG_REGION * Region, Uint8 ExtBuf_W[], Uint8 ExtBuf_IR[], Uint8 ImgBuf_W[][IMG_BUF_X], Uint8 ImgBuf_IR[][IMG_BUF_X]);
void CalAngular(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate);
void CalcLRSlop(struct ST_POINT Point[], double * Angle, double * XConst);

#if TEST_DUAL_DSP
// Upper 쪽 반사이미지는 경계선 찾는 방법이 다르다. 따라서 새로운 함수를 정의해야 한다.
void TBDegreeEx(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);
void LRDegreeEx(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);
void CalSizeCenterEx(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate);
void CalPosiqEx(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate);
void ViewPointEx(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);//외곽선등을 정확하게 찾았는지 확인하기 위한.
//
// FindOutlineEx - Upper DSP의 반사 이미지용 경계선 찾기 프로그램
//
void FindOutlineEx(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate, Uint8 ImgBuf[][IMG_BUF_X]);
void MarkPointEx(ST_POINT p, Uint8 ImgBuf[][IMG_BUF_X]);
void TPFindEx(Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterY);
void CenterPosDecisionEx(IMG_ANALYSIS_RESULT* CIS, Uint8 ImgBuf[][IMG_BUF_X]);
void BPFindEx(Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterY);
void RPFindEx(IMG_ANALYSIS_RESULT* CIS, Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterX);
void LPFindEx(IMG_ANALYSIS_RESULT* CIS, Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterX);
#endif

void   NeuralNetwork(const long npHiddenWeight[], const long npOutputWeight[], Uint16 nInputNode, Uint16 nHiddenNode, Uint16 nOutputNode);
void   NeuralNetworkResult(Uint16 nOutputNode);
void   DataNormalization(long Min, double Rate);
int GetLengthInBytesForNNROCR(int width, int height);
int MakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height);
void AdjustImageDirection(unsigned short Dir, unsigned char* w_data, unsigned char* ir_data, int width, int height);
int OCR_EUR(char* Serial, unsigned char* ocr_data, int FSize_X, int FSize_Y, int RegionSize, int x_window, int y_window, int f_height);
void OCR_GetImage(unsigned char* dst, unsigned short lx, unsigned short ly);
#ifdef __cplusplus
}
#endif

extern void TBDegree(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);
extern void LRDegree(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);

extern void CalSizeCenter(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate);
extern void CalPosiq(IMG_ANALYSIS_RESULT * CIS, ST_IMG_RATE * Rate);

extern void ViewPoint(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);//외곽선등을 정확하게 찾았는지 확인하기 위한.

//////////////D_CIS_Find_OutLine
extern void CenterPosDecision(IMG_ANALYSIS_RESULT * CIS, Uint8 ImgBuf[][IMG_BUF_X]);

extern void TBMakePoint(IMG_ANALYSIS_RESULT* CIS);
extern void LRMakePoint(IMG_ANALYSIS_RESULT* CIS);

extern void TPFind(Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterY);
extern void BPFind(Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterY);
extern void LPFind(IMG_ANALYSIS_RESULT* CIS, Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterX);
extern void RPFind(IMG_ANALYSIS_RESULT* CIS, Uint8 ImgBuf[][IMG_BUF_X], struct ST_POINT Point[], int CenterX);

extern void CalcTBSlop(struct ST_POINT Point[], double * Angle, double * YConst);
extern void CalcLRSlop(struct ST_POINT Point[], double * Angle, double * XConst);
#ifdef TEST_IR_ANGLE
extern Uint16 PointDistAdjust(struct ST_POINT Point[], Uint16 Num, double Angle, double Const, int tag);
#else
extern Uint16 PointDistAdjust(struct ST_POINT Point[], Uint16 Num, double Angle, double Const);
#endif
extern void TransformLRData(double * Angle, double * XConst, double Rate_XY);
extern void FindOrgAngle(IMG_ANALYSIS_RESULT * CIS);

Uint16 CIS_LED_Tx(Uint16 Ch, Uint16 Data);
void CIS_ADC_Tx(Uint16 Data);
Uint8 CIS_ADC_Rx(Uint16 Addr);
Uint16 CIS_LED_Rx(Uint16 Addr);

void ReadDpramBytes(int len);

extern int   c_EUR_FitUV[4];

extern struct ST_LOCAL * g_Local[LOCAL_BUF];

#define C64_EINT4							(1 << 4)
#define C64_EINT5							(1 << 5)
#define C64_EINT6							(1 << 6)
#define C64_EINT7							(1 << 7)
#define C64_EINT8							(1 << 8)
#define C64_EINT9							(1 << 9)
#define C64_EINT10							(1 << 10)
#define C64_EINT11							(1 << 11)
#define C64_EINT12							(1 << 12)
#define C64_EINT13							(1 << 13)
#define C64_EINT14							(1 << 14)
#define C64_EINT15							(1 << 15)

// Event Register 0
#define	EVT0								0		// Output of Event Combiner 0, for EVT  1 through  31
#define	EVT1								1		// Output of Event Combiner 1, for EVT 32 through  63
#define	EVT2								2		// Output of Event Combiner 2, for EVT 64 through  95
#define	EVT3								3		// Output of Event Combiner 3, for EVT 96 through 127

#define	EVENT_TINTL0						4
#define	EVENT_TINTH0						5
#define	EVENT_TINTL1						6
#define	EVENT_TINTH1						7

#define	EVENT_WDINT							8
#define	EVENT_EMU_DTDMA						9
#define	EVENT_EMU_RTDXRX					11
#define	EVENT_EMU_RTDXTX					12
#define	EVENT_IDMAINT0						13
#define	EVENT_IDMAINT1						14

#define	EVENT_VDINT0						24
#define	EVENT_VDINT1						25
#define	EVENT_VDINT2						26
#define	EVENT_HISTINT						27
#define	EVENT_H3AINT						28
#define	EVENT_PRVUINT						29
#define	EVENT_RSZINT						30

// Event Register 1
#define	EVENT_EDMA3CC_INTG					34
#define	EVENT_EDMA3CC_INT0					35
#define	EVENT_EDMA3CC_INT1					36
#define	EVENT_EDMA3CC_ERRINT				37
#define	EVENT_EDMA3TC_ERRINT0				38
#define	EVENT_EDMA3TC_ERRINT1				39
#define	EVENT_EDMA3TC_ERRINT2				40

#define	EVENT_PSCINT						41
#define	EVENT_EMACINT						43
#define	EVENT_HPINT							47
#define	EVENT_MBXINT0						48
#define	EVENT_MBRINT0						49
#define	EVENT_DDRINT						53
#define	EVENT_EMIFAINT						54
#define	EVENT_VLQINT						55

#define	EVENT_HECC0INT						57
#define	EVENT_HECC1INT						58
#define	EVENT_AXINT0						59
#define	EVENT_ARINT0						60

// Event Register 2
#define	EVENT_GPIO0							64
#define	EVENT_GPIO1							65
#define	EVENT_GPIO2							66
#define	EVENT_GPIO3							67
#define	EVENT_GPIO4							68
#define	EVENT_GPIO5							69
#define	EVENT_GPIO6							70
#define	EVENT_GPIO7							71

#define	EVENT_GPIOBNK0						72
#define	EVENT_GPIOBNK1						73
#define	EVENT_GPIOBNK2						74
#define	EVENT_GPIOBNK3						75
#define	EVENT_GPIOBNK4						76
#define	EVENT_GPIOBNK5						77
#define	EVENT_GPIOBNK6						78

#define	EVENT_PWM0							80
#define	EVENT_PWM1							81
#define	EVENT_PWM2							82
#define	EVENT_IICINT0						83
#define	EVENT_UARTINT0						84
#define EVENT_UARTINT1						85

// Event Register 3
#define	EVENT_INTERR						96
#define	EVENT_EMC_IDMAERR					97
#define	EVENT_PMC_ED						113
#define	EVENT_UMCED1						116
#define	EVENT_UMCED2						117
#define	EVENT_PDCINT						118
#define	EVENT_SYSCMPA						119
#define	EVENT_PMCCMPA						120
#define	EVENT_PMCDMPA						121
#define	EVENT_DMCCMPA						122
#define	EVENT_DMCDMPA						123
#define	EVENT_UMCCMPA						124
#define	EVENT_UMCDMPA						125
#define	EVENT_EMCCMPA						126
#define	EVENT_EMCBUSERR						127

#endif         /* _BZ_EXTERN_H_ */



