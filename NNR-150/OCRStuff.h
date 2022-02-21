#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WINDOWS
/* Handle the 6x ISA */
#if defined(_TMS320C6X)
    /* Unsigned integer definitions (32bit, 16bit, 8bit) follow... */
    typedef unsigned int    Uint32;
    typedef unsigned short  Uint16;
    typedef unsigned char   Uint8;

    /* Signed integer definitions (32bit, 16bit, 8bit) follow... */
    typedef int             Int32;
    typedef short           Int16;
    typedef char            Int8;
                            
/* Handle the 54x, 55x and 28x ISAs */
#elif defined(_TMS320C5XX) || defined(__TMS320C55X__) || defined(_TMS320C28X)
    /* Unsigned integer definitions (32bit, 16bit, 8bit) follow... */
    typedef unsigned long   Uint32;
    typedef unsigned short  Uint16;
    typedef unsigned char   Uint8;

    /* Signed integer definitions (32bit, 16bit, 8bit) follow... */
    typedef long            Int32;
    typedef short           Int16;
    typedef char            Int8;

#else
    /* Other ISAs not supported */
    #error <ti/sdo/pspdrivers/soc/dm6437/dsp/tistdtypes.h> is not supported for this target
#endif  /* defined(_6x_) */
#else
typedef unsigned short Uint16;
typedef unsigned int Uint32;
#endif // _WINDOWS

void   NeuralNetwork(const long npHiddenWeight[], const long npOutputWeight[], Uint16 nInputNode, Uint16 nHiddenNode, Uint16 nOutputNode);
void   NeuralNetworkResult(Uint16 nOutputNode);
void   DataNormalization(long Min, double Rate);
int GetLengthInBytesForNNROCR(int width, int height);
int MakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height);
int MakeFeatureForNNROCR_NoScale(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height);
int ScaleImageXY(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height, int xscale, int yscale);
void RemoveUpperBlank(unsigned char* image, int width, int height);
int FindVerticalWindow(unsigned char* copy_ir_data, int xsize, int ysize, int vwindow);
int FindVerticalWindowSE(unsigned char* copy_ir_data, int xsize, int ysize, int vwindow, int* ys, int* ye);
int FindHorizontalWindows(unsigned char* copy_ir_data, int img_width, int xwindow, int* max_len, int max_cnt, int m, int x_limit, int method);
int FindHorizontalWindowsEx(unsigned char* copied_data, int img_width, int max_cnt, int* x_pos, int *x_len, int *y_len, int* l_move, int* r_move);
void RemoveMetalThread(unsigned char* data, int width, int height, int metal);
int GetThresholdByPixels(int* freq, unsigned char* data, int width, int height, int count);
int GetThresholdByPixelsYY(int* freq, unsigned char* data, int width, int height, int count);
void RemoveSidePixels(unsigned char* srcbuf, int old_w, int old_h, int new_w, int new_h);
void RemoveHSingleDots(unsigned char* srcbuf, int old_w, int old_h);
int CountUnderThreshold(int threshold, unsigned char* w_data, unsigned char* copy_w_data, int width, int height);
void AdjustImageDirection(unsigned short Dir, unsigned char* w_data, unsigned char* ir_data, int width, int height);
void SwapImageHorizontal(unsigned char* _data, int width, int height);
void SwapImageVertical(unsigned char* _data, int width, int height);
void MakeOneImage(unsigned char* dst, unsigned char* copy_ir_data, int i, int* char_x_pos, int x_size, int y_start, int x_window, int y_window, int SaveHeight, int xw);
void MakeOneImageEx(unsigned char* dst, unsigned char* src_data, int src_img_width, int x_pos, int x_len, int y_start, int y_end, int x_dst, int y_dst);
#ifndef _u16
#define	_u16 unsigned short
#endif

void CropImage(unsigned char* dst, _u16 dstw, _u16 dsth, unsigned char* src, _u16 srcx, _u16 srcy, _u16 srcw, _u16 srch);

// OCR Image source definition, dschae 20121222
#define	OCR_IMGSOURCE_IRT	0	// IR Trsmission image
#define	OCR_IMGSOURCE_WR	1	// W Reflection image

#ifndef NNR_FEATURE_BUF
#define	NNR_FEATURE_BUF	1200
extern long   g_Feature[NNR_FEATURE_BUF];
#endif

extern Uint16 g_FeatureCnt;

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

extern struct ST_NNR_RESULT gst_NNR_Result;

#define	OCR_DATA_MAX	30000
extern unsigned char copy_ocr_data[OCR_DATA_MAX];
extern unsigned char dst[30*30];
extern unsigned char FeatureInBytes[30*30];

extern int      ir_freq[256];

#define	MAX_CHAR_X_POS	20

extern int OCR_x_pos[MAX_CHAR_X_POS];
extern int OCR_x_len[MAX_CHAR_X_POS];
extern int OCR_y_len[MAX_CHAR_X_POS];
extern int OCR_max_length;

extern int ORG_x_pos[MAX_CHAR_X_POS];
extern int ORG_x_len[MAX_CHAR_X_POS];
extern int ORG_y_len[MAX_CHAR_X_POS];

extern int OCR_y_pos_0, OCR_y_pos_1;

#ifdef __cplusplus
}
#endif
