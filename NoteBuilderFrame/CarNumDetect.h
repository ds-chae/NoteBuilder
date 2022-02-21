////////////////////////////////////////////////////
//
//  Ŭ���� : CCarNumDetect
//
//                    by HDS
#pragma once

using namespace cv;

struct blob_tmp_st {
	int x;
	int index;
    float w_h;
};

#define BT_T4 1370
#define BT_T3_1_2_3 136
#define BT_T3_2_3_4 1369
#define BT_T3_3_4_1 1358
#define BT_T3_4_1_2 1247
#define BT_T2_1_2 13
#define BT_T2_2_3 145
#define BT_T2_3_4 1377
#define BT_T2_4_1 1235
#define BT_T2_4_2 1246
#define BT_T2_2_3 124
#define BT_T1_1 1
#define BT_T1_2 12
#define BT_T1_3 123
#define BT_T1_4 1234



struct blob_info {
	int pixel_num;  // blob��ȣ ...blob_id�� �ؾ���.. �̸� �߸� ������... pixel_num�� ���� -.-
	int sx, sy, ex, ey;
	int width, height; 
// ���߿� ���⸦ ����ϱ� ���Ͽ� �ֻ�ܰ� ���ϴ��� x��ǥ�� ���Ѵ�. - dschae 20140909
	int sy_x, ey_x; // dschae
	int cx; // x �� �� �߽��� ��ǥ
// ���� ���� ������ ���� ������ ǥ��
	int bx, by; // begining x, y


	// HDS-TEST-140926
    int line_count; // ã�Ƴ� ����,������ ���� 
	int up_line[4], up_maxline[4]; //sx:0, sy:1, ex:2, ey:3; // ��� ���� ����.. 
	int down_line[4], down_maxline[4]; // �ϴ� ���� ����.. 
	int left_line[4], left_maxline[4]; // ���� ������ ����.. 
	int right_line[4], right_maxline[4]; // ������ ������ ����.. 
	float up_degree, down_degree, left_degree, right_degree; // ���� ����... 

	//HDS-TEST-140929
	int line_type_id;  // ������, ������ ����� ������ �ִ� ��κ����� �������ִ� �÷��װ�                                                                         1 : 1
	// ��ܼ���(1) : 1,  ��������(2) : 12, �ϴܼ���(3) : 123, ��������(4) : 1234 �� �ؼ� ���������� �����ָ� ����  ID �� ���������                              --------------  
	                   // 1 �� : 1 = 1, 2 = 12, 3 = 123, 4 = 1234                                                                                               |             | 
	                   // 2 �� : 1+2 = 13, 2+3 = 145, 3+4=1377, 4+1=1235, 4+2=1246, 2+3=124                                                           4 : 1234  |             | 2 : 12
	                   // 3 �� : 1+2+3 = 136, 2+3+4=1369, 3+4+1 = 1358, 4+1+2 = 1247                                                                            |             |
	                   // 4 �� : 1+2+3+4 = 1370                                                                                                                 -------------- 
	                   // 0 �� : 0                                                                                                                                     3 : 123

};
//HDS-TEST-140929
typedef struct _tPlateCandidateBox {
	int Box[8]; // px1,py1,px2,py2,px3,py3,px4,py4
	int width;
	int height;
	float rate; // ���� ���� ����...
	float vert_degree, hori_degree;
	bool selected;  // ���� ���ùڽ� ����....


	int EdgeTable_index;
} tPlateCandidateBox;




extern int m_nSCR_WIDTH;
extern int m_nSCR_HEIGHT;

extern int EdgeLinePoints; // ���� ����ϱ� ���� ���� ��
extern int EdgeLineMinX; // �� ����� ���� X�� �ּ� ũ��
extern int EdgeLineMaxX; // �� ����� ���� X�� �ִ� ũ��
extern int EdgeLineMinY; // �� ����� ���� Y�� �ּ� ũ��
extern int EdgeLineMaxY; // �� ����� ���� Y�� �ִ� ũ��

extern double EdgeSlopeMaxX;
extern double EdgeSlopeMinX;

extern double EdgeSlopeMaxY;
extern double EdgeSlopeMinY;

extern double EdgeMaxVarX;
extern double EdgeMaxVarY;

#define	PREDEFINED_SIZE	0
// TypedMat�̶� �� Mat ���� �����͸� [][]�� �����ϰ� ���ִ� �Ϳ� �Ұ��ϴ�.
// ���� �׳� ��ũ�� ġȯ���� �����ϰ� �ٲ� - �ٽ� ����ϰ� �ٲ�

#define	MAX_EDGE_LINES	100000
#define	MAX_EDGE_CANDIDATE	1000

typedef struct _st_test_diff {
	int found[4];
	float dist_avg;
	float dist_var;
} st_test_diff;

typedef struct _tLineParams {
	double a;
	double b;
	double var;
	double radius;
	double theta;
	int x1, y1;
	int x2, y2;
	int crx, cry;
	int axis; // 0 : base is X, 1 : base is Y
} tLineParams;

#define	MAX_TEST_DIFF	100

class  CCarNumDetect
{
public:
	CCarNumDetect();
public:
	~CCarNumDetect(void);

public:
	void OpenNamedWindow();
	void OpenSrcImageFile(char* filename);
	void BlurImage(float fSigmaVal);
    void GenerateEdgeFromOriginal(int blur_type, float fSigmaVal);
	void SetSrcImg(Mat& newSrc);
	int call_check_condition(int *select_arr);
	void PrintTestDifference(FILE* retf);
	
	void DrawTestedEdges(CDC* pdc);
	void DrawMarkedEdge(CDC *pdc, int edge_to_mark);

	void DrawEdgeLines(Mat& dst);
	void RearrangeBlob();
	void AdjustBlob(blob_info& inf);
	void TryToSeperateBigEdge(blob_info* p_inf);
	void GenerateFilledEdges();
	int FillAndDetect(FILE* retf, int blur_type, Mat* CarNumPlate);
	float get_dist_var(int* found_arr);
	void LabelEdges(); // ������ ��ȣ�� �ű��
	int GetPlateRectFromEdges(FILE* retf, bool only_check_4_or_5, int first_run);
	int GetPlateRectFromFill(FILE* retf, bool only_check_4_or_5, int first_run);
	int GetNumberArea(FILE* retf, int blur_type, Mat* CarNumPlate);
	void GetSlope();
	void DrawPlateArea(CCarNumDetect* pPlateOnly, Mat& NumberPlate, int sel);
	int FindLinesByEdge(); // ������ ���� ������ ���Ѵ�. houghlines���� ����� �ٸ���.
	void GetNumberCandidates(int first_run);
	void DebugSelected4();
	void GetTestedEdgeText(CString &txt);

    //Test....
	void View_TestLogic();
	static void Erosion( int, void* );

	// HDS-TEST-140926
	int FindLinesByEdge_HDS(FILE* retf);
	int FindLinesByEdge_HDS_sub(FILE* retf);
	void DrawEdgeLines_HDS(Mat& dst);


private:
	bool recurSearch(int cx, int cy);
	int check_3_condition(int *found_arr, float *w_h_arr); 
	int equidistance_test(FILE* retf, int *found_arr, float *w_h_arr); 
	int check_5_condition(int *found_arr, float *w_h_arr); 
	int check_6_condition(int *found_arr, float *w_h_arr); 
	int check_3_4_5_6_condition(FILE* retf, int *found_arr, float *w_h_arr, bool only_check_4_or_5);
	void get_check_cond(FILE* retf, bool only_check_4_or_5);

	// HDS-TEST-140926
	void find_blob_hori_line(blob_info* pedge, bool bUp_line);
	void find_blob_vert_line(blob_info* pedge, bool bLeft_line);
	float getAngle(const int ori_line[], int *maxline, bool bHoriline);
	//HDS-140928
	float getAngle2(const float degree, const int ori_line[], int *maxline, bool bHoriline);
        void get_cross_point(int *px, int *py, int *line1, int *line2);
	bool adjust_box_horiline_fill_down_line(blob_info* check_pedge, blob_info* pedge);
	bool adjust_box_horiline_fill_up_line(blob_info* check_pedge, blob_info* pedge);



public:
	TCHAR* window_original;
	TCHAR* window_process;
	TCHAR* window_result;
	TCHAR* edge_name;
	TCHAR* lab_name;

	Mat src;  // Original Image...
	Mat blur_src;  // Original Image...
	Mat detected_edges;  // Edge Image....
	Mat new_edges;
	Mat labeledColorImage; //

	Mat	matWhiteFilled;
	Mat matBlackFilled;

	TypedMat<unsigned char> white_m;
	TypedMat<unsigned char> black_m;
	TypedMat<unsigned char> blurr_m;

	Mat PlateROI;
	int PlateROI_x;
	int PlateROI_y;
	int PlateROI_w;
	int PlateROI_h;

	Mat flagImage;

	TypedMat<int> flag_m;
	TypedMat<unsigned char> dte_m;

	struct blob_info  m_EdgeTable[MAX_EDGE_LINES];
	int m_nBlobIndex;
	int m_nEdgeCount;
	int TestedEdgeCount;

	tLineParams fpLines[1000];
	int EdgeLineCount;

	int blobCandidateIndex[MAX_EDGE_CANDIDATE];
	int blobCandidateCount;

    struct blob_tmp_st tmp_blob[MAX_EDGE_CANDIDATE];
	float w_h[MAX_EDGE_CANDIDATE]; // ���� ��� ���� ����, check_4_5_6_condition() ����4 üũ�� Ȱ���ϱ� ���ص�...


	int sv_found_arr[10][4]; // �ִ� 10������ ������ ����Ʈ�� ���Ѵ�.....
	int sv_found_count;  
	float sv_found_var[4];

	int found_arr[4]; // �ֿ켱 �ĺ�....

	float ActiveSigma;

	st_test_diff test_diff_table[MAX_TEST_DIFF];
	int test_diff_count;

	//Test...
	char* window_test_view;
	Mat Test_src;  // Original TestImage...

	// HDS-TEST-140926
	tPlateCandidateBox  PlateCandidateBox[500]; 
        int PlateCandidateBoxCount;
    int Found_PlateCandidateBoxIndex;// ã�� �����̸� �ε�����ȣ.. ���и� -1...
	
	void ApplyFFT();
};