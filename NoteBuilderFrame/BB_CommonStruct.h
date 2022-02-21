#ifndef _STRUCT_H_
#define _STRUCT_H_


#define EDMA3_ENABLE_DCACHE                 (1u)
#define EDMA3_CACHE_LINE_SIZE_IN_BYTES      (128u)

/* OPT Field specific defines */
#define OPT_SYNCDIM_SHIFT                   (0x00000002u)
#define OPT_TCC_MASK                        (0x0003F000u)
#define OPT_TCC_SHIFT                       (0x0000000Cu)
#define OPT_ITCINTEN_SHIFT                  (0x00000015u)
#define OPT_TCINTEN_SHIFT                   (0x00000014u)

/* Error returned in case of data mismatch */
#define EDMA3_DATA_MISMATCH_ERROR           (-1)

typedef struct ST_DMA_STATE{
short Finish;
short Miss;
short State;
	short IsTxToOS;
}ST_DMA_STATE;

#ifndef _WINDOWS
typedef struct ST_DMA_INFO{

    unsigned int    chId;
    unsigned int    tcc;
	unsigned char * srcBuf;
	unsigned char * dstBuf;

	Uint16          acnt;
	Uint16          bcnt;
	Uint16          ccnt;

	EDMA3_DRV_SyncType syncType;
}ST_DMA_INFO;
#endif

typedef struct ST_POINT{
	Int16 x,  y;
	Uint8 State;
}ST_POINT;

typedef struct ST_SQUARE{
	Int16 Sx, Sy;
	Int16 Ex, Ey;
	Uint8 State;
} ST_SQUARE;

typedef struct ST_PEAK{
	Uint16 Value;
	Int16  Pos;
}ST_PEAK;

typedef struct ST_AVEG{
	Uint32 Total;
	Uint16 Cnt;
}ST_AVEG;



#endif  // struct.h



