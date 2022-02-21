////////////////////////////////////////////////////
//
//  클래스 : CCarNumDetect
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
	int pixel_num;  // blob번호 ...blob_id로 해야지.. 이름 잘못 지어줌... pixel_num이 뭐냐 -.-
	int sx, sy, ex, ey;
	int width, height; 
// 나중에 기울기를 계산하기 위하여 최상단과 최하단의 x좌표를 구한다. - dschae 20140909
	int sy_x, ey_x; // dschae
	int cx; // x 축 상 중심의 좌표
// 에지 직선 추정을 위한 시작점 표시
	int bx, by; // begining x, y


	// HDS-TEST-140926
    int line_count; // 찾아낸 수평,수직선 갯수 
	int up_line[4], up_maxline[4]; //sx:0, sy:1, ex:2, ey:3; // 상단 수평선 정보.. 
	int down_line[4], down_maxline[4]; // 하단 수평선 정보.. 
	int left_line[4], left_maxline[4]; // 왼쪽 수직선 정보.. 
	int right_line[4], right_maxline[4]; // 오른쪽 수직선 정보.. 
	float up_degree, down_degree, left_degree, right_degree; // 선분 각도... 

	//HDS-TEST-140929
	int line_type_id;  // 수직선, 수평선을 어떤것을 가지고 있는 블로브인지 구분해주는 플래그값                                                                         1 : 1
	// 상단수평(1) : 1,  우측수직(2) : 12, 하단수평(3) : 123, 좌측수직(4) : 1234 로 해서 있을때마다 더해주면 고유  ID 가 만들어진다                              --------------  
	                   // 1 개 : 1 = 1, 2 = 12, 3 = 123, 4 = 1234                                                                                               |             | 
	                   // 2 개 : 1+2 = 13, 2+3 = 145, 3+4=1377, 4+1=1235, 4+2=1246, 2+3=124                                                           4 : 1234  |             | 2 : 12
	                   // 3 개 : 1+2+3 = 136, 2+3+4=1369, 3+4+1 = 1358, 4+1+2 = 1247                                                                            |             |
	                   // 4 개 : 1+2+3+4 = 1370                                                                                                                 -------------- 
	                   // 0 개 : 0                                                                                                                                     3 : 123

};
//HDS-TEST-140929
typedef struct _tPlateCandidateBox {
	int Box[8]; // px1,py1,px2,py2,px3,py3,px4,py4
	int width;
	int height;
	float rate; // 가로 세로 비율...
	float vert_degree, hori_degree;
	bool selected;  // 최종 선택박스 여부....


	int EdgeTable_index;
} tPlateCandidateBox;




extern int m_nSCR_WIDTH;
extern int m_nSCR_HEIGHT;

extern int EdgeLinePoints; // 선을 계산하기 위한 점의 수
extern int EdgeLineMinX; // 선 계산을 위한 X축 최소 크기
extern int EdgeLineMaxX; // 선 계산을 위한 X축 최대 크기
extern int EdgeLineMinY; // 선 계산을 위한 Y축 최소 크기
extern int EdgeLineMaxY; // 선 계산을 위한 Y축 최대 크기

extern double EdgeSlopeMaxX;
extern double EdgeSlopeMinX;

extern double EdgeSlopeMaxY;
extern double EdgeSlopeMinY;

extern double EdgeMaxVarX;
extern double EdgeMaxVarY;

#define	PREDEFINED_SIZE	0
// TypedMat이란 게 Mat 속의 데이터를 [][]로 접근하게 해주는 것에 불과하다.
// 따라서 그냥 마크로 치환으로 동작하게 바꿈 - 다시 사용하게 바꿈

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
	void LabelEdges(); // 에지에 번호를 매긴다
	int GetPlateRectFromEdges(FILE* retf, bool only_check_4_or_5, int first_run);
	int GetPlateRectFromFill(FILE* retf, bool only_check_4_or_5, int first_run);
	int GetNumberArea(FILE* retf, int blur_type, Mat* CarNumPlate);
	void GetSlope();
	void DrawPlateArea(CCarNumDetect* pPlateOnly, Mat& NumberPlate, int sel);
	int FindLinesByEdge(); // 에지에 따라 라인을 구한다. houghlines과는 상당히 다르다.
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
	float w_h[MAX_EDGE_CANDIDATE]; // 높이 대비 폭의 비율, check_4_5_6_condition() 조건4 체크시 활용하기 구해둠...


	int sv_found_arr[10][4]; // 최대 10개까지 성공한 리스트를 구한다.....
	int sv_found_count;  
	float sv_found_var[4];

	int found_arr[4]; // 최우선 후보....

	float ActiveSigma;

	st_test_diff test_diff_table[MAX_TEST_DIFF];
	int test_diff_count;

	//Test...
	char* window_test_view;
	Mat Test_src;  // Original TestImage...

	// HDS-TEST-140926
	tPlateCandidateBox  PlateCandidateBox[500]; 
        int PlateCandidateBoxCount;
    int Found_PlateCandidateBoxIndex;// 찾기 성공이면 인덱스번호.. 실패면 -1...
	
	void ApplyFFT();
};