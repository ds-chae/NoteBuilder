#ifndef _OSPROTOCOL_H_
#define _OSPROTOCOL_H_

//#define OS_STATE_BUF		2
//#define OS_ON				0
//#define OS_OFF				1

#define OS_ER_BUF			10 //기구상에 발생하는 error(system 정지)
#define OS_OK				0
#define OS_JAM_FCNT			1
#define OS_JAM_IN1			2
#define OS_JAM_IN2			3
#define OS_OPEN_BACK		4
#define OS_OPEN_FRONT		5

#define OS_RXBUF			2043         //  2048 - char 5개
#define OS_TXBUF			(6137 - 10)  //  6144 - char 5개 flag 2개

enum OS_CMD_INDEX {
	OS_INFO = 0,
	OS_STATE = 1,
	OS_CURRENCY = 2,
	OS_MODE = 3,
	OS_DENOM = 4,
	OS_DIR = 5,
	OS_BATCH = 6,
	OS_SPEED = 7,
	OS_IMG = 8,
	OS_TYPE = 9,
	OS_TDS = 10,
	OS_KICKER = 11,
	OS_MAIN = 12,
	OS_STACKER = 13,
	OS_MOTOR = 14,
	OS_FIT_LEVEL_TAPE = 15,
	OS_FIT_LEVEL_HOLE = 16,
	OS_FIT_LEVEL_SOILED = 17,
	OS_FIT_LEVEL_DEINKED = 18,
	OS_FIT_LEVEL_STAIN = 19,
	OS_FIT_LEVEL_WASHED = 20,
// CF on/off
	OS_CF_ON = 21,
	OS_MOTORSPEED = 22,
	OS_LEDBRIGHT = 23,
	OS_TOP_IR = 24,
	OS_TOP_RED = 25,
	OS_TOP_GREEN = 	26,
	OS_TOP_BLUE	= 27,
	OS_REJECTCNT = 28,
	OS_VERSION_REQ = 29,
	OS_UV_REQ = 30,
	OS_TAPEGAIN = 31,
	OS_TAPEMULT = 32,
//	OS_TAPEBASE = 33, // 20110113에 삭제 함. 베이스 계산 없이 계수만 처리하도록 수정하였음. dschae
	OS_MGGAIN = 34,
	OS_MG_ON = 35,
	OS_CNT_DDLV = 36,
	OS_STOPBYCF = 37,
	OS_TAPEGAINMULTI_REQ = 38,
	OS_CALIBRATION = 39,
	OS_CAPTURE = 40,
	OS_CNT_LENGTH = 41,
	OS_CF_LEVEL = 42,
	OS_SIZE_MARGIN = 43,
	OS_CNT_SIZE_Y = 44,
	OS_SER_IMG_ON = 45,
	OS_IGN_VTLIMIT = 46,
	OS_MT_ON = 47,
	OS_ATM = 48,
	OS_GRAFFITI = 49,
	OS_MG_LEVEL = 50,
	OS_UV_LEVEL = 51,
	OS_RESET = 52,
	OS_CIS_TYPE = 53,
#if TEST_DUAL_DSP
	OS_UPTOP_RED = 54,
	OS_UPTOP_GREEN = 55,
	OS_UPTOP_BLUE = 56
#endif
	 };

#define OS_NONE		-1

//for denom
#define MODE_MIX			10
#define MODE_SP_AUTO		11
#define MODE_SP_MANUAL		12
#define MODE_DIR			13
#define MODE_SERIAL			14
#define MODE_DIR_AUTO		15

#define	TX_IMG_BATCH	1 // 이미지 일괄전송

//for cnt
#define MODE_CNT			0
#define MODE_SOLENOID		1
#define MODE_SEPARATOR		2
#if TX_IMG_BATCH
#else
#define MODE_TRANS_IMG		3
#endif
#define MODE_TRANS_KEY		4
#define MODE_TOGGLE			5

//for fitness
#define VALUE_ONLY				0
#define VALUE_FITNESS			1
#define VALUE_SERIAL			2
#define VALUE_FITNESS_SERIAL	3

//extern const char * g_OSComState[OS_STATE_BUF];
extern const char * g_OSComEr[OS_ER_BUF];

#ifdef GLOBAL_MAIN_DEF
#define	GLOBAL_MAIN
#else
#define	GLOBAL_MAIN	extern
#endif

GLOBAL_MAIN	volatile int	g_TxCntTimer;
void SetGainOfUV(char* Buf);
int LoadGainOfUV();

#define	HOST_MSG_SIZE	(OS_TXBUF-256)

GLOBAL_MAIN char szHostMsg[HOST_MSG_SIZE];
GLOBAL_MAIN Uint16  HostMsgLen;


#endif





