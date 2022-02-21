#include "StdAfx.h"

#include "NoteBuilderFrame.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ScrollWnd.h"

using namespace std;
using namespace cv;

int CmpBlobVal(const void *arg1,  const void *arg2)
{
	return (  ((struct blob_tmp_st *)arg2)->x - ((struct blob_tmp_st *)arg1)->x ); 
}

int save_index = 0;

float fSigmaVal[4] = { 0, 1.5, 0.5, 0.1 };

int	m_nSCR_WIDTH = 960; //640;//960;//640;   
int m_nSCR_HEIGHT = 669;//446;//669;//446;

int EdgeLinePoints = 10; ///20  // ���� ����ϱ� ���� ���� ��
int EdgeLineMinX = 50; // �� ����� ���� X�� �ּ� ũ��
int EdgeLineMaxX = 600; // �� ����� ���� X�� �ִ� ũ��
int EdgeLineMinY = 30; // �� ����� ���� Y�� �ּ� ũ��
int EdgeLineMaxY = 150; // �� ����� ���� Y�� �ִ� ũ��

double EdgeSlopeMaxX = (457.-408.) / (503.-293.);
double EdgeSlopeMinX = -10. / (503.-293.);

double EdgeSlopeMaxY = 10.0 / (447.-384.);
double EdgeSlopeMinY = -5. / (503.-293.);

double EdgeMaxVarX = 10;
double EdgeMaxVarY = 10;

int horiline_min_pixels  = 80;  //���� �ּҿ��� �ȼ���
int horiline_var_allow  = 25;   // ���� �л� ���ذ�

int vertline_min_pixels  = 25;  // ������ �ּҿ��� �ȼ��� 
int vertline_var_allow  = 10;   // ������ �л� ���ذ�..



extern int equitest_method;


int inv_fft_circle[2000][2];
int inv_fft_init = 0;
int fft_radius = 664/3;

extern Rect cropRect;

CCarNumDetect::CCarNumDetect()
{
#if PREDEFINED_SIZE
	// �̸� �����ص�....
	flagImage = Mat::zeros(m_nSCR_HEIGHT, m_nSCR_WIDTH, CV_32SC1);
	labeledColorImage = Mat::zeros(m_nSCR_HEIGHT, m_nSCR_WIDTH, CV_8UC3);
#endif
	window_original = L"original";
	window_process = L"process";
	window_result = L"result";
	edge_name = L"edge";
	lab_name = L"lab";

	if(!inv_fft_init) {
		inv_fft_init = 1;
		for(int y = 0; y < cropRect.height; y++) {
			inv_fft_circle[y][0] = 0;
			inv_fft_circle[y][1] = cropRect.width;
		}
		for(int y = 0; y < fft_radius; y++) {
			double sin_theta = y / (double)fft_radius;
			double theta = asin(sin_theta);
			int x = (int)(fft_radius*cos(theta));
			inv_fft_circle[y][0] = x;
			inv_fft_circle[y][1] = cropRect.width - x;
			inv_fft_circle[cropRect.height-1-y][0] = x;
			inv_fft_circle[cropRect.height-1-y][1] = cropRect.width - x;
		}
	}
}

// ���������ϴ� �����̴�.

CCarNumDetect::~CCarNumDetect(void)
{


	//if( m_Image != NULL )	cvReleaseImage( &m_Image );	

	//if( m_recBlobs != NULL )
	//{
	//	delete m_recBlobs;
	//	m_recBlobs = NULL;
	//}



}

void CCarNumDetect::OpenNamedWindow()
{
	/// Create a window
	/*-- ���� ���� ����� �ʿ䰡 ����. 20140910dschae
	namedWindow( window_original, CV_WINDOW_AUTOSIZE );
	namedWindow( window_process, CV_WINDOW_AUTOSIZE );
	namedWindow( window_result, CV_WINDOW_AUTOSIZE );
	*/
}

extern int bCropImage;
extern int bScaleImage;
extern int apply_undistort;
void undistort_center(Mat& src, Mat& dst);

void CCarNumDetect::OpenSrcImageFile(char* filename)
{
	Mat readImg;
	Mat subsrc;

	readImg = imread( filename , CV_LOAD_IMAGE_GRAYSCALE);

	if( !readImg.data )
	{ 
		cout << "File open Fail.." << endl;
		return; 
	}

	if(apply_undistort) {
		Mat und;
		undistort_center(readImg, und);
		readImg = und;
	}

	if(bCropImage) {
		Rect activeRect = cropRect;
		if(activeRect.width > readImg.cols-activeRect.x)
			activeRect.width = readImg.cols-activeRect.x;
		if(activeRect.height > readImg.rows-activeRect.y)
			activeRect.height = readImg.rows-activeRect.y;

		subsrc = readImg(activeRect);
		if(bScaleImage)
			resize(subsrc, src, Size(m_nSCR_WIDTH,m_nSCR_HEIGHT));
		else
			subsrc.copyTo(src);
	} else {
		if(bScaleImage)
			resize(readImg, src, Size(m_nSCR_WIDTH,m_nSCR_HEIGHT));
		else
			readImg.copyTo(src);
	}
}

void CCarNumDetect::SetSrcImg(Mat& newSrc)
{
	newSrc.copyTo(src);
}



extern int nBlurType;
extern int kernel_size;

void CCarNumDetect::BlurImage(float fSigmaVal)
{
	// ������ �ʹ� ����ȭ�Ǹ�(�ñ׸� : 1.5�̻����� �ؾ�  16-4774 �ذ�) ��ȣ�� ���� ������ ������ �� ���� ��κ� ������ ��������
	// ������ �ʹ� �ܼ��ϰ� �ϸ�( �ñ׸� : 0.5���Ϸ� �ؾ� 17-2480 �ذ�) ���ڿ����� �������� ��κ갡 ������ �Ǻ��� ���������.
	// ���ÿ� �����ϴ� �ñ׸� ���� ���� ���ϰڴ�....
	// �ñ׸� 0.5���� �ѹ������� �����ϸ�.... 1.5���� �ٽ��ѹ� ������ ������� �غ���....
	if(fSigmaVal == 0.0) {
		blur( src, blur_src, Size(3,3) );
	} else {
		// Gaussian blur
		GaussianBlur( src, blur_src, Size(3,3), fSigmaVal);
	}
	char savepath[MAX_PATH];
	sprintf_s(savepath, "C:\\temp\\src_%d.bmp", save_index);
	imwrite(savepath, src);

	blurr_m = blur_src;
	/*
	for(int y = 0; y < blur_src.rows; y++) {
	for(int x = 0; x < blur_src.cols; x++) {
	if(blurr_m[y][x] < 80) blurr_m[y][x] = 0;
	}
	}
	*/
}

bool CCarNumDetect::recurSearch(int cy, int cx)
{
	bool bFound = false;

	// ������ üũ�ϰ�... �ٽ� 8���� ������....
	if (cx < 0 || cx >= detected_edges.cols)
		return false;
	if (cy < 0 || cy >= detected_edges.rows)
		return false;
	if (dte_m[cy][cx] != 255 )
		return false; // ��輱�� ���� ��.. ��ġ ����...
	if (flag_m[cy][cx] != -1)
		return false;  // �̹� �湮�Ǿ� ��ġ�� ���̹Ƿ� ���̻� ã�� �ʿ� ����

	// ��輱���� ���̰� ó�� ������ ���̴�...

	// ����� ���̹Ƿ� ��ŷ....
	flag_m[cy][cx] = m_nBlobIndex;
	bFound = true;

	// �����ڽ� ������Ʈ....
	if (m_EdgeTable[m_nBlobIndex].sy > cy) {
		m_EdgeTable[m_nBlobIndex].sy = cy;
		m_EdgeTable[m_nBlobIndex].sy_x = cx; // dschae20140909 ���ϴ��� X��ǥ
	}
	if (m_EdgeTable[m_nBlobIndex].ey < cy) {
		m_EdgeTable[m_nBlobIndex].ey = cy;
		m_EdgeTable[m_nBlobIndex].ey_x = cx; // dschae20140909 �ֻ���� X��ǥ
	}
	if (m_EdgeTable[m_nBlobIndex].sx > cx) m_EdgeTable[m_nBlobIndex].sx = cx;
	if (m_EdgeTable[m_nBlobIndex].ex < cx) m_EdgeTable[m_nBlobIndex].ex = cx;

	// 8 �������� ��Ŀ�ú�� search...

#if 0
	// ����Ǿ� �̵��ϴ��߿�... ���ο� ���� 2���̻� �����ϸ�.. 1���� �����ϸ� ��������...
	// ���ɿ����� ��ȣ�� ���������� �ַ� ���� ��輱���� ���ڿ� ���� ���ɼ��� ���� ... ���� ��� ��ȣ��.... 26-17-2480
	// ���� �����Ұ��� 2���̻��̸� 1���� �������Ѵ�... ���� �켱������.....	 
	// �� > �� > �� > ���� > �»� > ���� > ��� > �� ������ Ž������...  1���� �����Ǹ� Ž���� ��ģ��...

	if (recurSearch(cy+1, cx))  return bFound;

	if (recurSearch(cy-1, cx))  return bFound;
	if (recurSearch(cy, cx-1))  return bFound;
	if (recurSearch(cy+1, cx-1))  return bFound;
	if (recurSearch(cy-1, cx-1))  return bFound;
	if (recurSearch(cy+1, cx+1))  return bFound;
	if (recurSearch(cy-1, cx+1))  return bFound;

	recurSearch(cy, cx+1);
#else
	recurSearch(cy+1, cx);
	recurSearch(cy-1, cx);
	recurSearch(cy, cx-1);
	recurSearch(cy+1, cx-1);
	recurSearch(cy-1, cx-1);
	recurSearch(cy+1, cx+1);
	recurSearch(cy-1, cx+1);
	recurSearch(cy, cx+1);

#endif
	return bFound;
}

//
// ũ�Ⱑ ū ������ �Լ� ������ ���� �ʵ��� �Ѵ�.
// �Լ� ������ ū ������ ������ ��� ���ÿ� ������ �ȴ�.

// �¿� �Ǵ� ���ϼ��� ���� �����ִٰų� �� ��츦 ������ �ش�.
void CCarNumDetect::AdjustBlob(blob_info& inf)
{
	if(inf.height > 50 || inf.width > 100) // ���� ū ���� �����Ѵ�.
		return;

	int b_index = inf.pixel_num;

	// �������� �ٿ����� - ��, �� 3�������� ���δ�.
	int x_limit = inf.sx + 3;
	if(x_limit > inf.ex)
		x_limit = inf.ex;
	for(int x = inf.sx; x <= x_limit; x++) {
		int sum = 0;
		for(int y = inf.sy; y <= inf.ey; y++) {
			if(flag_m[y][x] == b_index)
				sum++;
		}
		if(sum > 1)
			break;
		if(x != inf.sx) { // ���� ���� 1�����̰�, ���� ���� 1���Ͽ��� ���� �����Ƿ�
			inf.sx = x;
			inf.width -= 1;
		}
	}

	// ���� ���̱� - ��, �� 3�������� ���δ�.
	x_limit = inf.ex - 3;
	if(x_limit < inf.sx)
		x_limit = inf.sx;
	for(int x = inf.ex; x >= x_limit; x--) {
		int sum = 0;
		for(int y = inf.sy; y <= inf.ey; y++) {
			if(flag_m[y][x] == b_index)
				sum++;
		}
		if(sum > 1)
			break;
		if(x != inf.ex) { // ���� ���� 1�����̰�, ���� ���� 1���Ͽ��� ���� �����Ƿ�
			inf.ex = x;
			inf.width -= 1;
		}
	}
}

int min_char_height = 32;//36;
int max_char_height = 56; //54;//43;//39; 

// ũ��� �Ÿ��� ���Ͽ� �ٿ��ش�.
void CCarNumDetect::RearrangeBlob()
{
	for (int i = 0; i <= m_nBlobIndex-1; i++) {
		blob_info *p = &m_EdgeTable[i];
		// �� ������ '1'������ ���ϰ� �� �� ����.
		if(p->width < 3 || p->width > 50)
			continue; // �� ���� ����

		for(int j = i+1; j <= m_nBlobIndex; j++) {
			blob_info *q = &m_EdgeTable[j];
			// �ϴ� ���� ���� ������ �ִ��� ���Ѵ�
			if( abs(p->sx - q->sx) > 2 || abs(p->ex - q->ex) > 2)
				continue; // ���� ��ǥ�� ���� ���� ��
			int merge = 0;
			int sy = p->sy < q->sy ? p->sy : q->sy;
			int ey = p->ey > q->ey ? p->ey : q->ey;
			int new_height = ey - sy + 1;
			if(new_height > min_char_height && new_height < max_char_height) { // ��ĥ ���ΰ�?
				if(p->sy < q->sy) { // p�� ����
					if(q->sy - p->ey <= 2) { // ���Ϸ� ������
						merge = 1;
					}
				} else { // q�� ����
					if(p->sy - q->ey <= 2) { // ���Ϸ� ������
						merge = 1;
					}
				}
			}
			if(merge) { // ������ q�� p�� ���̴� �ɷ� �Ѵ�.
				p->sx = p->sx < q->sx ? p->sx : q->sx;
				p->ex = p->ex > q->ex ? p->ex : q->ex;
				p->sy = sy;
				p->ey = ey;
				p->width = p->ex - p->sx + 1;
				p->height = p->ey - p->sy + 1;
				q->sx = q->ex = q->sy = q->ey = q->width = q->height = 0;
				break; // ������ �����Ƿ� �� ���� ����� ����.
			}
		}
	}
}

//////////////////////////////////
void CCarNumDetect::LabelEdges() // ������ ��ȣ�� �ű��
{
	// flagImage�� �̰����� �����Ѵ�.
	flagImage = Mat::zeros(detected_edges.rows, detected_edges.cols, CV_32SC1);
	// �÷��� ���� �ʱ�ȭ..
	flagImage = Scalar::all(-1); // �� ���� �� ���� 

	flag_m = flagImage;

	m_nBlobIndex = -1;
	m_nEdgeCount = 0;

	for (int i=0; i< detected_edges.rows; i++) {
		for (int j = 0; j < detected_edges.cols; j++) {
			if (dte_m[i][j] == 255 && flag_m[i][j] == -1)  {
				// ���ο� �������ο� �ش��ϴ� ���� �������Ƿ� �߰��ϰ�.. 
				if(m_nEdgeCount >= MAX_EDGE_LINES)
					break;
				m_nBlobIndex = m_nEdgeCount++;

				flag_m[i][j] = m_nBlobIndex;
				m_EdgeTable[m_nBlobIndex].sy = m_EdgeTable[m_nBlobIndex].ey = i; 
				m_EdgeTable[m_nBlobIndex].sx = m_EdgeTable[m_nBlobIndex].ex = j;
				m_EdgeTable[m_nBlobIndex].sy_x = m_EdgeTable[m_nBlobIndex].ey_x = j; // �ֻ�ܰ� ���ϴ��� x��ǥ�� ���صд�. dschae20140909
				m_EdgeTable[m_nBlobIndex].bx = j; // �� ������ ���� ����Ʈ, ���߿� ���� ������ �������� �ȴ�.
				m_EdgeTable[m_nBlobIndex].by = i;
				m_EdgeTable[m_nBlobIndex].pixel_num = m_nBlobIndex;

				//cout << "XY" << i << "," << j;
				//if (i == 411 && j == 335)
				//	cout << "XY" << i << "," << j;

				// 8 �������� ��Ŀ�ú�� search...
				recurSearch(i, j-1);
				recurSearch(i-1, j-1);
				recurSearch(i-1, j);
				recurSearch(i-1, j+1);
				recurSearch(i, j+1);
				recurSearch(i+1, j+1);
				recurSearch(i+1, j);
				recurSearch(i+1, j-1);
			}
		}		
	}		

	// width, height �ϰ����......
	for (int i = 0; i <= m_nBlobIndex; i++) {
		m_EdgeTable[i].width = m_EdgeTable[i].ex - m_EdgeTable[i].sx + 1;	
		m_EdgeTable[i].height = m_EdgeTable[i].ey - m_EdgeTable[i].sy + 1;
		m_EdgeTable[i].cx = (m_EdgeTable[i].ex + m_EdgeTable[i].sx ) / 2; // 20140911dschae, x�� �߽� ��ǥ�� ���Ѵ�. ��ȣ������ �߽ɰŸ��� ��� �����ϴ�.
		AdjustBlob(m_EdgeTable[i]); // �¿� �Ǵ� ���ϼ��� ���� �����ִٰų� �� ��츦 ������ �ش�.
	}
	RearrangeBlob(); // ũ��� �Ÿ��� ���Ͽ� ���̰ų� �� �ش�.

	// ������ ��κ����� Ȯ��.......
	// ã���� �׷�����......
	// DEBUG...
	cvtColor(detected_edges, labeledColorImage, CV_GRAY2BGR);

	//int alpha = 32;//36;
	//int beta  = 43;//39;

	if(0) {
		// ���� ã�� ���� �׸��� �κ��ε� �ʹ� ���� �׷����� ����ϹǷ� �ϴ� ���Ƶд�. dschae20140908
		for (int i = 0; i <= m_nBlobIndex; i++) {

			//cout << "c:" << i << ":" << m_EdgeTable[i].sx << "," << m_EdgeTable[i].sy << "," << m_EdgeTable[i].ex << "," << m_EdgeTable[i].ey << endl;  

			//if (((m_EdgeTable[m_nBlobIndex].ey - m_EdgeTable[m_nBlobIndex].sy + 1) > alpha) && ((m_EdgeTable[m_nBlobIndex].ey - m_EdgeTable[m_nBlobIndex].sy + 1) < beta) ) {

			cv::rectangle(labeledColorImage, 
				cv::Point(m_EdgeTable[i].sx, m_EdgeTable[i].sy),
				cv::Point(m_EdgeTable[i].ex, m_EdgeTable[i].ey),
				cv::Scalar(0,0,255));
			//}

		}
		CreateScrollWnd(lab_name)->matShow(labeledColorImage);
	}

	/*--- dschae 20140918 - �̰� �� �ϴ��� �𸣰ڴ�. �ĺ��ڸ� �߸��� �κ��� �ٸ� ���� �ִ�.
	int alpha = 32;//36; ������ �ּ�ġ
	int beta  = 56; // 54;//43;//39; ������ �ִ�ġ

	int count = 0;
	for (int i = 0; i < m_nBlobIndex; i++)
	{
	if (m_EdgeTable[i].height > alpha
	&& m_EdgeTable[i].height < beta
	&& m_EdgeTable[i].width > 2 )  {	
	tmp_blob[count].x = m_EdgeTable[i].sx;
	tmp_blob[count].index = i;
	count++;
	if(count >= MAX_EDGE_CANDIDATE)
	break;
	}
	}

	AdjustCandidateBlob(); // ���� ���� �����ִٰų� �� ��츦 ������ �ش�.

	// ȭ�� �����ʿ��� �������� ���� ����...
	qsort((void *)tmp_blob, count, sizeof(struct blob_tmp_st), CmpBlobVal);

	for (int i = 0; i < count; i++) {
	cv::rectangle(labeledColorImage, 
	cv::Point(m_EdgeTable[tmp_blob[i].index].sx, m_EdgeTable[tmp_blob[i].index].sy),
	cv::Point(m_EdgeTable[tmp_blob[i].index].ex, m_EdgeTable[tmp_blob[i].index].ey),
	cv::Scalar(0, 0, 255));
	}
	------ 20140918 ������ �κ� */
}


int CCarNumDetect::check_3_condition(int *found_arr, float *w_h_arr) 
{
	int i;


	// ���� 3 - 1 �߰�...
	// ������ ��κ갡 �����ϸ� �ȵȴ�.....26-17-1044
	// �����ʿ��� �������� ���ĵǾ� �����Ƿ�.. ������.. ���� ��κ갡 ������κ긦 ���ΰ� �ִ°��� ã�Ƴ���...

	// 26-16-9521 �� ���̻� ��κ���� ���ĵ� ��������....



	// ���� 3 - 2 �߰�...
	// ��κ� ���̰� ������� �����ؾ� �Ѵ�.... �ʹ� ���̰� ���� ��κ� �����ϸ� ����ó������.....26-15-2213 ����
	// ���ذ� 8 �ȼ� �̻� ���̳��� ����ó��...

	for (i = 1; i < 4; i++) {
		// ���� 3 - 1 ...
		if ( m_EdgeTable[found_arr[i-1]].cx <= m_EdgeTable[found_arr[i]].ex ) 			
			break ; // ���� ���긦 �����ϰų�, ���̻� ������ �ִ»���.. ���ܰ� ����!

		// ���� 3 - 2 ...
		if ( (m_EdgeTable[found_arr[i-1]].height -   m_EdgeTable[found_arr[i]].height) >= 8 )
			break;
	}

	return i == 4 ? 4 : -i; // 4�̸� ������.
}

int error_x()
{
	return 0;
}

int CCarNumDetect::equidistance_test(FILE* retf, int *found_arr, float *w_h_arr) 
{
	int i;

	// ���� 4 ... Gap < 0.3 * H
	// ��ȣ���� 1 �ΰ��(w_h �� 0.3 ���� �� ��κ� : 26-21-4246 ) üũ �����ذ��� �÷���� �Ѵ�.
	// ���ʴ� 1 �ΰ�� 0.55 . ���ʸ� 1�� ���.. 0.45 �� ���� �غ���...
	//      26-18-9113 ����...
	// ��... 1 �̶�� �ݵ�� �ּ� ���� �־���Ѵ�.... 26-21-4246 ���� : 1�� ��ó�� ���̴� ��κ� ����...
	//         �ּҰ� ���ذ�..  0.2�� �����غ���...20-18-6186 ����... �ణ ���ڶ�.. 0.07�� �غ���....
	//         �ּҰ� ���ذ�..  ..26-12-3614 ����... �ణ ���ڶ�.. 0.05�� �غ���....
	// ���ذ� : 0.3 -> 0.35�� ���÷���.. 26-15-8813 ������....
	// ���ذ� : 0.35 -> 0.4�� ���÷���.. 26-06-1553 ������... ���ҳ��� 1��.,...
	float th_4 = 0.4;  // üũ�� ���ذ�..
	float least_th_4 = 0.0;  // �ּҰ� ���ذ�
	bool bLeastGapCheck;

	float blobGap;
	float blobLeastGap;

	int curGap;
	// w_h_arr�� ���� ���̷� ���� ���̴�. 0.3���ϸ� 1�� �ǹ��ϴ� ����.
	for (i = 1; i < 4; i++) {
		bLeastGapCheck = true;
		if (w_h_arr[i-1] <= 0.3) {
			least_th_4 = 0.05;
			if (w_h_arr[i] <= 0.3) 
				th_4 = 0.55;   // ���ʴ� 1 �ΰ��... �Ÿ� ���. ���̿� ���ؼ� ������ ����� �� ����Ѵ�.
			else
				th_4 = 0.45;   // ���ʸ� 1 �ΰ��...

		}
		else {
			if (w_h_arr[i] <= 0.3) { 
				th_4 = 0.45;   // ���ʸ� 1 �ΰ��...
				least_th_4 = 0.05;
			}
			else {
				th_4 = 0.4;   // ���ʴ� 1�� �ƴѰ��...
				least_th_4 = 0.0;
				bLeastGapCheck = false;
			}
		}

		blobGap = th_4 * m_EdgeTable[found_arr[i-1]].height;
		blobLeastGap = least_th_4 * m_EdgeTable[found_arr[i-1]].height;

		curGap = m_EdgeTable[found_arr[i-1]].sx - m_EdgeTable[found_arr[i]].ex - 1; 

		if (curGap >= blobGap)
			break; //����! 
		if (bLeastGapCheck && (curGap < blobLeastGap))
			break; //����!... �����̶� 1 ����� ��츸 üũ..... �ٸ����� ��κ갡 ��ĥ���� ����....ȸ������ ����... 
	}

	return i == 4 ? 4 : -i;// 4�� �ƴϸ� ������.
}


int CCarNumDetect::check_5_condition(int *found_arr, float *w_h_arr) 
{
	int i;

	// ���� 5 DIf_up < 0.3 * W  and DIf_down < 0.3 * W
	//    0.4�� ����.... 20-18-6186 
	//    0.5�� ����.... 20-18-7871
	// ���⼭ width�� 4���� ��κ��� ���� ū�� �������� �Ѵ�... 1 ��κ��� width�� �ʹ��۴�... (26-09-4103)
	// �����߰�(26-10-9043)....  DIf_up < 0.2 * H  and DIf_down < 0.2 * H

	// �����߰�(26-08-8494).... ������ ��κ���  width�� �ִ� width�� 0.15 �� �̻��� �Ǿ�� �Ѵ�..
	int max_width = m_EdgeTable[found_arr[0]].width;
	for (i = 1; i < 4; i++)
		if (max_width < m_EdgeTable[found_arr[i]].width )  max_width = m_EdgeTable[found_arr[i]].width;

	// �����߰�(26-08-8494).... ������ ��κ���  width�� �ִ� width�� 0.15 �� �̻��� �Ǿ�� �Ѵ�..
	for (i = 0; i < 4; i++) {
		if (max_width * 0.15 >= m_EdgeTable[found_arr[i]].width )
			break;
	}
	if (i != 4)
		return -i;


	float blob_Diff = 0.4 * max_width;
	for (i = 1; i < 4; i++) {
		//float blob_Diff = 0.5 * m_EdgeTable[found_arr[i-1]].width;
		float blob_high_Diff = 0.2 * m_EdgeTable[found_arr[i-1]].height;

		int dif_up = abs(m_EdgeTable[found_arr[i]].sy -  m_EdgeTable[found_arr[i-1]].sy);
		int dif_down = abs((m_EdgeTable[found_arr[i]].ey) - (m_EdgeTable[found_arr[i-1]].ey));

		// ���ǿ� �ȸ�����...����������..
		if ((dif_up >= blob_Diff) || (dif_down >= blob_Diff))
			break;
		if ((dif_up > blob_high_Diff) || (dif_down > blob_high_Diff))
			break;
	}

	return i == 4 ? 4 : -i;
}

int CCarNumDetect::check_6_condition(int *found_arr, float *w_h_arr) 
{
	// ���� 6 üũ.....Ws < 4 * H �� �����ϴ���...
	int Ws =  m_EdgeTable[found_arr[0]].ex - m_EdgeTable[found_arr[3]].sx + 1;

	if (Ws < ( 4 * m_EdgeTable[found_arr[0]].height ) ) 
		return 4;
	else
		return -3;  // �� �ܰ迡�� �����ϸ� �ٷ� �����ܰ迡�� �ٽ� üũ�غ���...

}


int CCarNumDetect::check_3_4_5_6_condition(FILE* retf, int *found_arr, float *w_h_arr, bool only_check_4_or_5) 
{
	/*
	Mat testimg;
	cvtColor(blur_src, testimg, CV_GRAY2BGR);
	for(int ti = 0; ti < 4; ti++) {
	int index = found_arr[ti];
	blob_info info = m_EdgeTable[index];
	cv::rectangle(testimg, 
	cv::Point(info.sx, info.sy), cv::Point(info.ex, info.ey),
	cv::Scalar(0,0,255));
	}
	imshow("testimg", testimg);
	if(found_arr[0] == 297 || found_arr[0] == 422) MessageBox(NULL, L"XX", L"YY", MB_OK);
	*/
	int i = 0;
	// ���� 4�� ���� 5�� 1���� �����ص� ok.....	// 27 - 08-4990 --> ���۳� ��ȣ��...

	if (only_check_4_or_5) {

		// ���� 4 ... 
		i = equidistance_test(retf, found_arr, w_h_arr);

		if (i == 4) {
			return i;
		}


		// ���� 5 
		i = check_5_condition(found_arr, w_h_arr);
		return i;

	}


	// ���� 3-1, 3-2
	i = check_3_condition(found_arr, w_h_arr);

	if (i != 4) {
		return i;
	}


	// ���� 4 ... 
	i = equidistance_test(retf, found_arr, w_h_arr);

	if (i != 4) {
		return i;
	}


	// ���� 5 
	i = check_5_condition(found_arr, w_h_arr);

	if (i != 4) {
		return i;
	}


	// ���� 6 üũ.....Ws < 4 * H �� �����ϴ���...
	i = check_6_condition(found_arr, w_h_arr);
	return i;
}

float CCarNumDetect::get_dist_var(int* found_arr)
{
	float high_sum = 0;
	for(int bi = 0; bi < 4; bi++)
		high_sum += m_EdgeTable[found_arr[bi]].height;
	float high_avg = high_sum / 4;

	float dist_sum = 0;
	for (int bi = 1; bi < 4; bi++)
		dist_sum += (m_EdgeTable[found_arr[bi-1]].cx - m_EdgeTable[found_arr[bi]].cx);// less index has bigger x coord
	float dist_avg = dist_sum / 3;

	float dist_var = 0;
	for (int bi = 1; bi < 4; bi++) {
		float dist_each = (m_EdgeTable[found_arr[bi-1]].cx - m_EdgeTable[found_arr[bi]].cx);
		dist_each -= dist_avg;
		dist_var += (dist_each * dist_each);
	}

	return dist_var;
}

void CCarNumDetect::get_check_cond(FILE* retf, bool only_check_4_or_5)
{
	sv_found_count = 0;  // �ִ� 10������ ������ ����Ʈ�� ���Ѵ�.....
	int found_count = 0;

	for( int i = 0; i < blobCandidateCount; i++ )
	{
		found_arr[0] = tmp_blob[i].index;
		w_h[0] = tmp_blob[i].w_h; // w_h[] ���� ��Ȱ��... ������ �̽����� 1 ������ 4���� ���� Ȱ��ȴ�.
		for( int j = i+1; j < blobCandidateCount; j++ )
		{
			found_arr[1] =tmp_blob[j].index;
			w_h[1] = tmp_blob[j].w_h; 
			for( int k = j+1; k < blobCandidateCount; k++ )
			{
				found_arr[2] = tmp_blob[k].index;
				w_h[2] = tmp_blob[k].w_h; 
				for( int l = k+1; l < blobCandidateCount; l++ )
				{
					found_arr[3] = tmp_blob[l].index;
					w_h[3] = tmp_blob[l].w_h; 

					// ���� : 4
					// ���� : 0,-1,-2,-3 ���� ���� 
					//	if (found_arr[0] == 240 && found_arr[1] == 203 && found_arr[2] == 123 && found_arr[3] == 51  ) 
					//	if (found_arr[0] == 240 ) 
					found_count = check_3_4_5_6_condition(retf, (int *)found_arr, (float *)w_h, only_check_4_or_5 );

					if (found_count == 4)  {
						TCHAR msg[256];
						wsprintf(msg, L"success %d %d %d %d\r\n", found_arr[0], found_arr[1], found_arr[2], found_arr[3]);
						OutputDebugString(msg);
						//  ���� ... ����Ʈ�� �����ϰ�.......��� Ž���Ѵ�... �� �ƽ� 10���̸� ����������.....	
						memmove(sv_found_arr[sv_found_count], found_arr, sizeof(int)*4);
						//						sv_found_var[sv_found_count] = get_dist_var(found_arr); // �л� ���� ���Ѵ�.
						sv_found_count++;
						if (sv_found_count >= 10)
							break; // 10�� �ϼ�... ����������...
						found_count = -3; // ��� Ž���ϰ� �Ѵ�...
					}
					if (found_count != -3)
						break; // ���� ���������� �� �����Ҽ� ����..
					found_count = -2; // ���������� ���� Ż��� �ٷ� �������� �����Ҽ� �ֵ��� �ϱ� ����....
				}
				if (found_count == 4)
					break;
				if (found_count != -2)
					break; // ���� ���������� �� �����Ҽ� ����..
				found_count = -1; // ���������� ���� Ż��� �ٷ� �������� �����Ҽ� �ֵ��� �ϱ� ����....
			}
			if (found_count == 4)
				break;
			if (found_count != -1)
				break; // ���� ���������� �� �����Ҽ� ����..
			found_count = 0; // ���������� ���� Ż��� �ٷ� �������� �����Ҽ� �ֵ��� �ϱ� ����....
		}	
		if (found_count == 4)
			break;
	}
}

void CCarNumDetect::GetNumberCandidates(int first_run)
{
	////////////////////////////////////////////
	//beta 53: 06-26-20-3382
	//beta 54: 06-26-12-8066
	int min_alpha = min_char_height;
	int max_beta = max_char_height;
	if (!first_run) {
		max_beta  += 2; // 56 -> 58��  ������ ���� �� Ű��... Ȯ��Ǹ鼭 Ŀ����...
	}

	// 2 - 1���� ����....  alpha < H < beta
	// 2 - 2����...  W/H < 0.9
	for( int i=0; i <= m_nBlobIndex; i++ )
	{
		//2 - 1����
		blob_info* p_inf = &m_EdgeTable[i];
		if (p_inf->height > min_alpha && p_inf->height < max_beta) {
			// 2 -2���� 
			float wh = p_inf->width * 1.0 / p_inf->height;

			//if ((wh < 0.9) && (wh > 0.26) ) {
			if ( wh < 0.9 ) {
				w_h[blobCandidateCount] = wh;
				blobCandidateIndex[blobCandidateCount++] = i;
			}
		}
		if (blobCandidateCount >= MAX_EDGE_CANDIDATE)
			break;		
	}

	// ���� ��κ���� �����ʿ��� �������� �迭�Ѵ�.
	for (int i = 0; i < blobCandidateCount; i++)
	{
		tmp_blob[i].x = m_EdgeTable[blobCandidateIndex[i]].sx;
		tmp_blob[i].index = blobCandidateIndex[i];
		tmp_blob[i].w_h = w_h[i];
	}
	qsort((void *)tmp_blob, blobCandidateCount, sizeof(struct blob_tmp_st), CmpBlobVal);
	for (int i = 0; i < blobCandidateCount; i++) {
		cv::rectangle(labeledColorImage, 
			cv::Point(m_EdgeTable[tmp_blob[i].index].sx, m_EdgeTable[tmp_blob[i].index].sy),
			cv::Point(m_EdgeTable[tmp_blob[i].index].ex, m_EdgeTable[tmp_blob[i].index].ey),
			cv::Scalar(0, 0, 255));
	}
	/*
	//DEBUG
	for (int i = 0; i < blobCandidateCount; i++)
	{
	CString str;
	str.Format(L"(id23=%d: %d,%d,%d,%d) \r\n", tmp_blob[i].index, m_EdgeTable[tmp_blob[i].index].width, m_EdgeTable[tmp_blob[i].index].height, m_EdgeTable[tmp_blob[i].index].sx, m_EdgeTable[tmp_blob[i].index].sy);
	OutputDebugString(str);
	}
	*/
}

void CCarNumDetect::DebugSelected4()
{
	for( int i = 0; i < 4; i++ ) {
		// �� ���̺� ǥ��
		cv::rectangle(labeledColorImage, 
			cv::Point(m_EdgeTable[found_arr[i]].sx, m_EdgeTable[found_arr[i]].sy),
			cv::Point(m_EdgeTable[found_arr[i]].ex, m_EdgeTable[found_arr[i]].ey),
			cv::Scalar(0,255,0));

		//cout << "pos:" <<m_EdgeTable[tmp_blob[i].index].sx << "," << m_EdgeTable[tmp_blob[i].index].sy << "," <<  m_EdgeTable[tmp_blob[i].index].width << "." <<  m_EdgeTable[tmp_blob[i].index].height << endl;

		CString str;
		str.Format(L"(ids=%d: %d,%d,%d,%d,ex=%d,ey=%d) \r\n", found_arr[i], m_EdgeTable[found_arr[i]].width, m_EdgeTable[found_arr[i]].height, m_EdgeTable[found_arr[i]].sx, m_EdgeTable[found_arr[i]].sy, m_EdgeTable[found_arr[i]].ex, m_EdgeTable[found_arr[i]].ey);
		OutputDebugString(str);
	}
	cv::rectangle(labeledColorImage, 
		cv::Point(m_EdgeTable[found_arr[0]].sx, m_EdgeTable[found_arr[0]].sy),
		cv::Point(m_EdgeTable[found_arr[3]].ex, m_EdgeTable[found_arr[3]].ey),
		cv::Scalar(128,255,128));
}				

// ��κ� ã��.....
int CCarNumDetect::GetPlateRectFromEdges(FILE* retf, bool only_check_4_or_5, int first_run)
{
	// ��ȣ�� ��κ� ã��.....
	blobCandidateCount = 0;

	GetNumberCandidates(first_run);

	get_check_cond(retf, only_check_4_or_5);

	TRACE2("sv count %d\r\n", sv_found_count, 0);
	if (sv_found_count > 0) {

		// 2���̻� ã����(26-17-1044)... �ϴܿ� ����� ����� �����ĺ��� �Ѵ�.....
		// 26-11-1818 �� �ϴܿ� �ִ°��� �ƴϴ�....
		//   ��Ģ���� : �ϴܺ��ٴ� ������ ������... 

		int max_ex = m_EdgeTable[sv_found_arr[0][0]].ex;
		int target_index = 0;
		for (int i = 1; i < sv_found_count; i++) {
			if (m_EdgeTable[sv_found_arr[i][0]].ex > max_ex) {
				max_ex = m_EdgeTable[sv_found_arr[i][0]].ex;
				target_index = i;
			}
		}
		// ����... found_arr[] �� ����....
		memmove(&found_arr[0], &sv_found_arr[target_index][0], sizeof(int)*4);

		// �׷�����... �������..
		// ����.... found_arr[4] �� �����ĺ� �ε��� �������...	
		DebugSelected4();
		return 1; // ����...
	}

	return -1; // ����....
}

extern int canny_th_min[];
extern int canny_th_max[];
extern int canny_th_cnt;

void saveimage(Mat& img, char* prefix, int index1, int index2)
{
	char szsavename[MAX_PATH];
	sprintf_s(szsavename, MAX_PATH, "C:\\temp\\%s_%d_%d_%d.bmp", prefix, save_index, index1, index2);
	imwrite(szsavename, img);
}

int CCarNumDetect::GetNumberArea(FILE* retf, int blur_type, Mat* CarNumPlate)
{
	save_index++;
// ����� ���� �����ϴ� �������� ���� �ϴ°� ���ڴ�....
	// 26-08-9216(9716) ����! --> canny threshold 100 ���� �ϸ� ��..
	// 26-08-4990 ����!
	// 27 - 08-8645 08-4990 12-8162 12-4530 14-6082 14-1107 14-3861 14-8210 15-3409 19-6027
	// 2���̻�...
	//  26 18-6008 
	//  27 11-9427 21-4359

#if 0  // debug... �׽�Ʈ ����.......
	View_TestLogic();
	return;
#endif 
	test_diff_count = 0;

	// 0.1���.... 26-14-3497 �� ���⼭ ������ ���������...
	int ret = 0;

	if(bApplyFFT)
		ApplyFFT();

	int blur_start = 0;
	int blur_end = 4;
	if(blur_type == 0) blur_end = 1; // normal only
	if(blur_type == 1) blur_start = 1; // gaussian only
	for (int blur_index = blur_start; blur_index < blur_end; blur_index++) {
		BlurImage(fSigmaVal[blur_index]); // ����þ� ���� �ð��� �ɸ��Ƿ� �� ���� �� ���� ������ �����ϰ� �Ѵ�.
		saveimage(blur_src, "blur", blur_index, 0);

		for(int th_index = 0; th_index < canny_th_cnt; th_index++) {
			int th_min = canny_th_min[th_index];
			int th_max = canny_th_max[th_index];

			if(bNewCanny) {
				unsigned char* pedge = (unsigned char*)malloc(blur_src.step * blur_src.rows);
				CannyEdge(blur_src.data, blur_src.step, blur_src.cols, blur_src.rows, th_max, th_min, pedge);
				new_edges.create(blur_src.rows, blur_src.cols, CV_8U);
				for(int y = 0; y < blur_src.rows; y++) {
					unsigned char *ucp = new_edges.data + blur_src.step * y;
					unsigned char *src = pedge + blur_src.cols * y;
					for(int x = 0; x < blur_src.cols; x++)
						*ucp++ = *src++;
				}
				free(pedge);
				imshow("new edge", new_edges);
			}

			Canny( blur_src, detected_edges, th_min, th_max, kernel_size );
			saveimage(detected_edges, "edge", blur_index, th_index);
			dte_m = detected_edges;

			// ��Ŀ�ú� ���̺�...
			LabelEdges(); // ������ ��ȣ�� �ű��
			if(nPlateType == 3) { // ��ȣ���� �ܰ����� ã�ƺ���
				FindLinesByEdge(); // ������ ���� ������ ���Ѵ�. houghlines���� ����� �ٸ���.
				break; // ������ ã�� ��쿡�� ������̹Ƿ� �� ���� ã�� �����Ѵ�.
			}
			// HDS-TEST-140926
			if(nPlateType == 4) { // ��ȣ���� �ܰ����� ã�ƺ���
				ret = FindLinesByEdge_HDS(retf); // ������ ���� ������ ���Ѵ�. houghlines���� ����� �ٸ���.
				/** 
				CString str;
				str.Format(L"(Sigma=%f, th_min=%d \r\n", fSigmaVal[blur_index], th_min);
				OutputDebugString(str);
				**/ 
				break; // ������ ã�� ��쿡�� ������̹Ƿ� �� ���� ã�� �����Ѵ�.
			}


			// ��κ� ã��.....
			ret = CCarNumDetect::GetPlateRectFromEdges(retf, false, CarNumPlate != NULL);
			if (ret == 1 ) {
				TRACE2("blur=%d, th=%d\r\n", blur_index, th_index);
				// �����ϰų�, ����þ��� �ƴϸ� �극��ũ. �ֳĸ� ����þȸ� �ݺ��ϹǷ�.
				ActiveSigma = fSigmaVal[blur_index];
				break;
			}
		}
		if(ret == 1) {
			break;
		}
	}

	CreateScrollWnd(edge_name)->matShow(detected_edges);

	if(ret != 1)
		return 0;

	if(CarNumPlate != NULL) {
		// ���� �����ؼ� �׷�����.....
		/*
		pt1                   pt2
		f[3].ex, f[3].sy      f[0].ex, f[0].sy

		pt3                   pt4
		f[3].ex, f[3].ey      f[0].ex, f[0].ey        
		*/

		int pt1x = m_EdgeTable[found_arr[3]].ex;
		int pt1y = m_EdgeTable[found_arr[3]].sy;
		int pt2x = m_EdgeTable[found_arr[0]].ex;
		int pt2y = m_EdgeTable[found_arr[0]].sy;
		int pt3x = m_EdgeTable[found_arr[3]].ex;
		int pt3y = m_EdgeTable[found_arr[3]].ey;
		int pt4x = m_EdgeTable[found_arr[0]].ex;
		int pt4y = m_EdgeTable[found_arr[0]].ey;

		// �Ͼ�� ��ȣ�� ����...


#if 0	
		int width = pt2x - pt1x;
		int height = (pt2y - pt1y) * 2;
		int new_pt1x = pt1x - 1.5 * width;
		int new_pt1y = pt1y - height ;
		int new_pt3x = pt3x - 1.5 * width;
		int new_pt3y = pt3y - height;
#else // ��ȣ�� �����׽�Ʈ 20140910dshan
		float width = (pt2x - pt1x) * 1.5;

		//���...
		double theta_0 = (3.141592/2) - atan2( (double)(m_EdgeTable[found_arr[0]].sy_x - m_EdgeTable[found_arr[3]].sy_x + 1), (double)(m_EdgeTable[found_arr[0]].sy - m_EdgeTable[found_arr[3]].sy + 1) );
		float height = width * tan(theta_0);
		int new_pt1x = pt1x - width;
		int new_pt1y = pt1y - height ;

		//�ϴ�...
		theta_0 = (3.141592/2) - atan2( (double)(m_EdgeTable[found_arr[0]].ey_x - m_EdgeTable[found_arr[3]].ey_x + 1), (double)(m_EdgeTable[found_arr[0]].ey - m_EdgeTable[found_arr[3]].ey + 1) );
		height = width * tan(theta_0);

		int new_pt3x = pt3x - width;
		int new_pt3y = pt3y - height;
#endif

		//CString str;
		//str.Format(L"(%d,%d,%d,%d), (%d,%d,%d,%d),%f :: ", anew_pt1x, anew_pt1y, anew_pt3x, anew_pt3y, new_pt1x, new_pt1y, new_pt3x, new_pt3y,theta_0);
		//OutputDebugString(str);



		new_pt1x -= 10; // �� ��� ��ȣ�� ����,...
		new_pt3x -= 10; // �� ��� ��ȣ�� ����,...`

		int blob_heigt = pt4y - pt2y;
		new_pt1y -= blob_heigt * 0.8; // ��� ��ȣ�� ��...
		pt2y -= blob_heigt * 0.8; // ��� ��ȣ�� ��...

		// ��.�Ʒ�,��,�� ���ȼ��� �÷��ش�...
		new_pt1x -= 10;
		new_pt1y -= 10;
		pt2x += 10;
		pt2y -= 10;
		new_pt3x -= 10;
		new_pt3y += 10;
		pt4x += 10;
		pt4y += 10;

		if(new_pt1x < 0)
			new_pt1x = 0;
		if(new_pt1y < 0)
			new_pt1y = 0;
		if(pt2y < 0)
			pt2y = 0;
		if(new_pt3x < 0)
			new_pt3x = 0;

		int New_Width, New_Height;

		New_Width = pt4x - new_pt1x + 1;
		New_Height = pt4y - new_pt1y + 1;

		CarNumPlate->create(New_Height,New_Width, CV_8UC1);

		Point2f inputQuad[4]; 
		Point2f outputQuad[4];

		inputQuad[0] = Point2f( new_pt1x, new_pt1y);
		inputQuad[1] = Point2f( pt2x, pt2y);
		inputQuad[2] = Point2f( pt4x, pt4y);  
		inputQuad[3] = Point2f( new_pt3x, new_pt3y);

		// The 4 points where the mapping is to be done , from top-left in clockwise order
		outputQuad[0] = Point2f( 0,0 );
		outputQuad[1] = Point2f( New_Width-1, 0);
		outputQuad[2] = Point2f( New_Width-1, New_Height-1);
		outputQuad[3] = Point2f( 0, New_Height-1);

		Mat mapMatrix = getPerspectiveTransform(inputQuad, outputQuad);
		warpPerspective(blur_src, *CarNumPlate, mapMatrix, Size(New_Width,New_Height), INTER_CUBIC);

		// copy number plate to labled color image
		PlateROI_x = m_nSCR_WIDTH - New_Width - 1;
		PlateROI_y = m_nSCR_HEIGHT - New_Height - 1;
		PlateROI_w = New_Width;
		PlateROI_h = New_Height;
		PlateROI = labeledColorImage(Rect(PlateROI_x, PlateROI_y, PlateROI_w, PlateROI_h));
		cvtColor(*CarNumPlate, PlateROI, CV_GRAY2BGR);

		// �׷�����....
		line( labeledColorImage, Point(new_pt1x, new_pt1y),
			Point(pt2x, pt2y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(pt2x, pt2y),
			Point(pt4x, pt4y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(pt4x, pt4y),
			Point(new_pt3x, new_pt3y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(new_pt3x, new_pt3y),
			Point(new_pt1x, new_pt1y), Scalar(0,255,0), 2, 8 );
	}

	return 1;
}

int erosion_elem = 0;
int erosion_size = 0;
int dilation_elem = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;

void CCarNumDetect::Erosion( int newValue, void* object)
{
	CCarNumDetect *myCarNumDetect = (CCarNumDetect *)object;

	//myCarNumDetect = (CCarNumDetect *)this;

	int erosion_type;
	if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
	else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
	else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }

	Mat element = getStructuringElement( erosion_type,
		Size( 2*erosion_size + 1, 2*erosion_size+1 ),
		Point( erosion_size, erosion_size ) );

	/// Apply the erosion operation
	GaussianBlur( myCarNumDetect->src, myCarNumDetect->Test_src, Size(3,3), 0.5);  


	erode( myCarNumDetect->Test_src, myCarNumDetect->Test_src, element );

	imshow( myCarNumDetect->window_test_view, myCarNumDetect->Test_src);
}

void CCarNumDetect::View_TestLogic()
{
	window_test_view = "Test View";
	namedWindow( window_test_view, CV_WINDOW_AUTOSIZE );


#if 0 
	cv::Point2f source_points[4];
	cv::Point2f dest_points[4];


	source_points[0].x = 273;
	source_points[0].y = 350;
	source_points[1].x = 396;
	source_points[1].y = 378;
	source_points[2].x = 281;
	source_points[2].y = 429;
	source_points[3].x = 397;
	source_points[3].y = 456;

	dest_points[0].x = 100;
	dest_points[0].y = 100;
	dest_points[1].x = 250;
	dest_points[1].y = 100;
	dest_points[2].x = 100;
	dest_points[2].y = 170;
	dest_points[3].x = 250;
	dest_points[3].y = 170;

	labeledColorImage = src.clone();
	//cvZero(dst);

	cv::Mat _transform_matrix;
	_transform_matrix = cv::getPerspectiveTransform(source_points, dest_points);
	cv::warpPerspective(src, labeledColorImage, _transform_matrix, cv::Size(m_nSCR_WIDTH, m_nSCR_HEIGHT));

	imshow( window_test_view, labeledColorImage);

#endif 

#if 1

	// LIne..............
	BlurImage(0.5); // canny ���ְ�...
	Canny( blur_src, detected_edges, canny_th_min[0], canny_th_max[0], kernel_size );
	dte_m = detected_edges;
	//cvtColor(detected_edges, labeledColorImage, CV_GRAY2BGR);

	vector<Vec4i> lines;
	HoughLinesP(detected_edges, lines, 1, CV_PI/180, 80, 200, 10 );

	//for( size_t i = 0; i < lines.size(); i++ )
	//{
	//    line( labeledColorImage, Point(lines[i][0], lines[i][1]),
	//        Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
	//}

	//imshow( window_test_view, labeledColorImage);

#endif

	////////////////////////////////////////////////////
#if 0 // ħ�� 	//
	createTrackbar( "Element:\n 0: Rect \n 1: Cross \n 2: Ellipse", window_test_view,
		&erosion_elem, max_elem,
		&CCarNumDetect::Erosion, this);

	createTrackbar( "Kernel size:\n 2n +1", window_test_view,
		&erosion_size, max_kernel_size,
		&CCarNumDetect::Erosion, this);

	/// Default start
	Erosion( 0, this );  
#endif

#if 0 // ����ȭ  Adaptive Threshhold...

	//th2 = cv2.adaptiveThreshold(img,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,11,2)
	//th3 = cv2.adaptiveThreshold(img,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,  cv2.THRESH_BINARY,11,2)

	GaussianBlur( src, Test_src, Size(3,3), 1.5);  

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(19,19));
	Mat closed = Mat();
	morphologyEx(Test_src, closed, MORPH_CLOSE, kernel);

	Test_src.convertTo(Test_src, CV_32F); // divide requires floating-point

	divide(Test_src, closed, Test_src, 1, CV_32F);
	normalize(Test_src, Test_src, 0, 255, NORM_MINMAX);
	Test_src.convertTo(Test_src, CV_8UC1); // convert back to unsigned int

	// Threshold each block (3x3 grid) of the image separately to
	// correct for minor differences in contrast across the image.
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Mat block = Test_src.rowRange(144*i, 144*(i+1)).colRange(144*j, 144*(j+1));
			threshold(block, block, -1, 255, THRESH_BINARY_INV+THRESH_OTSU);
		}
	}

	imshow( window_test_view, Test_src);

#endif

#if 0 // ����ȭ  Adaptive Threshhold...
	Mat Temp_src = Mat();
	cv::medianBlur(src, Temp_src, 5);

	//cv::erode( Test_src, Test_src);

	//cv::adaptiveThreshold(Temp_src, Test_src,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,11,2);
	cv::adaptiveThreshold(Temp_src, Test_src,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);

	imshow( window_test_view, Test_src);
#endif
}

double slope_0, theta_0, slope_1, theta_1, slope_2, theta_2;

void CCarNumDetect::GetSlope()
{
	// Get coordinates of bottom of the edges
	// y = ax + b
	struct blob_info  sb = m_EdgeTable[found_arr[0]];
	struct blob_info  eb = m_EdgeTable[found_arr[3]];

	slope_0 = (double)(eb.ey - sb.ey) / (double)(eb.ey_x - sb.ey_x); // �ϴ�
	theta_0 = asin(slope_0);

	slope_1 = (double)(eb.sy - sb.sy) / (double)(eb.sy_x - sb.sy_x); // ���
	theta_1 = asin(slope_1);

	slope_2 = (double)(eb.sy - sb.sy) / (double)(eb.ex - sb.ex); // ���, ������
	theta_2 = asin(slope_2);

	double slope_3 = (double)(eb.sy - sb.sy) / (double)(eb.ex - sb.ey); // ���, ������
	double theta_3 = asin(slope_3);
}

void CCarNumDetect::DrawPlateArea(CCarNumDetect* pPlateOnly, Mat& NumberPlate, int sel)
{
	if(PlateROI.empty())
		return;

	blob_info inf = pPlateOnly->m_EdgeTable[pPlateOnly->found_arr[sel]];
	int x1 = PlateROI_x + inf.sx;
	int x2 = PlateROI_x + inf.ex;
	int y1 = PlateROI_y + inf.sy;
	int y2 = PlateROI_y + inf.ey;
	// �׷� �ִ´�. PlateROI�� labeledColorImage�� Ư�� ������ ��� �ִ�.
	cvtColor(NumberPlate, PlateROI, CV_GRAY2BGR);
	// ���õ� ���ڿ� �簢���� �����ش�.
	line( labeledColorImage, Point(x1, y1), Point(x2, y1), Scalar(2550,0,0), 1, 8);
	line( labeledColorImage, Point(x2, y1), Point(x2, y2), Scalar(2550,0,0), 1, 8);
	line( labeledColorImage, Point(x2, y2), Point(x1, y2), Scalar(2550,0,0), 1, 8);
	line( labeledColorImage, Point(x1, y2), Point(x1, y1), Scalar(2550,0,0), 1, 8);
	CreateScrollWnd(window_result)->matShow(labeledColorImage);
}

int CCarNumDetect::call_check_condition(int *select_arr)
{
	int found_arr[4];
	float w_h_arr[4];

	for(int i = 0; i < 4; i++) {
		int sel = select_arr[i];
		found_arr[i] = tmp_blob[sel].index;
		w_h_arr[i]   = tmp_blob[sel].w_h; 
	}

	int ret = check_3_4_5_6_condition(NULL, found_arr, w_h_arr, false);

	return ret;
}

void CCarNumDetect::PrintTestDifference(FILE* retf)
{
	for(int i = 0; i < test_diff_count; i++) {
		st_test_diff* pdiff = &test_diff_table[i];
		fprintf(retf, "avg=%f var=%f\n", pdiff->dist_avg, pdiff->dist_var);
	}
}

int CCarNumDetect::FillAndDetect(FILE* retf, int blur_type, Mat* CarNumPlate)
{
	// ����� ���� �����ϴ� �������� ���� �ϴ°� ���ڴ�....
	// 26-08-9216(9716) ����! --> canny threshold 100 ���� �ϸ� ��..
	// 26-08-4990 ����!
	// 27 - 08-8645 08-4990 12-8162 12-4530 14-6082 14-1107 14-3861 14-8210 15-3409 19-6027
	// 2���̻�...
	//  26 18-6008 
	//  27 11-9427 21-4359

	test_diff_count = 0;

	// 0.1���.... 26-14-3497 �� ���⼭ ������ ���������...
	int ret = 0;

	int blur_start = 0;
	int blur_end = 4;
	if(nBlurType == 0) blur_end = 1; // normal only
	if(nBlurType == 1) blur_start = 1; // gaussian only
	for (int blur_index = blur_start; blur_index < blur_end; blur_index++) {
		BlurImage(fSigmaVal[blur_index]); // canny ���ְ�...
		Canny( blur_src, detected_edges, canny_th_min[0], canny_th_max[0], kernel_size );
		dte_m = detected_edges;

		// ��Ŀ�ú� ���̺�...
		GenerateFilledEdges();
		imshow("edge", detected_edges);
		return 0;
		// ��κ� ã��.....
		ret = CCarNumDetect::GetPlateRectFromFill(retf, false, CarNumPlate != NULL);
		if(ret == 1 || blur_type != 1) {
			ActiveSigma = fSigmaVal[blur_index];
			break;
		}
	}
	CreateScrollWnd(edge_name)->matShow(detected_edges);

	if(ret != 1)
		return 0;

	if(CarNumPlate != NULL) {
		// ���� �����ؼ� �׷�����.....
		/*
		pt1                   pt2
		f[3].ex, f[3].sy      f[0].ex, f[0].sy

		pt3                   pt4
		f[3].ex, f[3].ey      f[0].ex, f[0].ey        
		*/

		int pt1x = m_EdgeTable[found_arr[3]].ex;
		int pt1y = m_EdgeTable[found_arr[3]].sy;
		int pt2x = m_EdgeTable[found_arr[0]].ex;
		int pt2y = m_EdgeTable[found_arr[0]].sy;
		int pt3x = m_EdgeTable[found_arr[3]].ex;
		int pt3y = m_EdgeTable[found_arr[3]].ey;
		int pt4x = m_EdgeTable[found_arr[0]].ex;
		int pt4y = m_EdgeTable[found_arr[0]].ey;

		// �Ͼ�� ��ȣ�� ����...


#if 0	
		int width = pt2x - pt1x;
		int height = (pt2y - pt1y) * 2;
		int new_pt1x = pt1x - 1.5 * width;
		int new_pt1y = pt1y - height ;
		int new_pt3x = pt3x - 1.5 * width;
		int new_pt3y = pt3y - height;
#else // ��ȣ�� �����׽�Ʈ 20140910dshan
		float width = (pt2x - pt1x) * 1.5;

		//���...
		double theta_0 = (3.141592/2) - atan2( (double)(m_EdgeTable[found_arr[0]].sy_x - m_EdgeTable[found_arr[3]].sy_x + 1), (double)(m_EdgeTable[found_arr[0]].sy - m_EdgeTable[found_arr[3]].sy + 1) );
		float height = width * tan(theta_0);
		int new_pt1x = pt1x - width;
		int new_pt1y = pt1y - height ;

		//�ϴ�...
		theta_0 = (3.141592/2) - atan2( (double)(m_EdgeTable[found_arr[0]].ey_x - m_EdgeTable[found_arr[3]].ey_x + 1), (double)(m_EdgeTable[found_arr[0]].ey - m_EdgeTable[found_arr[3]].ey + 1) );
		height = width * tan(theta_0);

		int new_pt3x = pt3x - width;
		int new_pt3y = pt3y - height;
#endif

		//CString str;
		//str.Format(L"(%d,%d,%d,%d), (%d,%d,%d,%d),%f :: ", anew_pt1x, anew_pt1y, anew_pt3x, anew_pt3y, new_pt1x, new_pt1y, new_pt3x, new_pt3y,theta_0);
		//OutputDebugString(str);



		new_pt1x -= 10; // �� ��� ��ȣ�� ����,...
		new_pt3x -= 10; // �� ��� ��ȣ�� ����,...`

		int blob_heigt = pt4y - pt2y;
		new_pt1y -= blob_heigt * 0.8; // ��� ��ȣ�� ��...
		pt2y -= blob_heigt * 0.8; // ��� ��ȣ�� ��...

		// ��.�Ʒ�,��,�� ���ȼ��� �÷��ش�...
		new_pt1x -= 10;
		new_pt1y -= 10;
		pt2x += 10;
		pt2y -= 10;
		new_pt3x -= 10;
		new_pt3y += 10;
		pt4x += 10;
		pt4y += 10;

		if(new_pt1x < 0)
			new_pt1x = 0;
		if(new_pt1y < 0)
			new_pt1y = 0;
		if(pt2y < 0)
			pt2y = 0;
		if(new_pt3x < 0)
			new_pt3x = 0;

		int New_Width, New_Height;

		New_Width = pt4x - new_pt1x + 1;
		New_Height = pt4y - new_pt1y + 1;

		CarNumPlate->create(New_Height,New_Width, CV_8UC1);

		Point2f inputQuad[4]; 
		Point2f outputQuad[4];

		inputQuad[0] = Point2f( new_pt1x, new_pt1y);
		inputQuad[1] = Point2f( pt2x, pt2y);
		inputQuad[2] = Point2f( pt4x, pt4y);  
		inputQuad[3] = Point2f( new_pt3x, new_pt3y);

		// The 4 points where the mapping is to be done , from top-left in clockwise order
		outputQuad[0] = Point2f( 0,0 );
		outputQuad[1] = Point2f( New_Width-1, 0);
		outputQuad[2] = Point2f( New_Width-1, New_Height-1);
		outputQuad[3] = Point2f( 0, New_Height-1);

		Mat mapMatrix = getPerspectiveTransform(inputQuad, outputQuad);
		warpPerspective(blur_src, *CarNumPlate, mapMatrix, Size(New_Width,New_Height), INTER_CUBIC);

		// copy number plate to labled color image
		PlateROI_x = m_nSCR_WIDTH - New_Width - 1;
		PlateROI_y = m_nSCR_HEIGHT - New_Height - 1;
		PlateROI_w = New_Width;
		PlateROI_h = New_Height;
		PlateROI = labeledColorImage(Rect(PlateROI_x, PlateROI_y, PlateROI_w, PlateROI_h));
		cvtColor(*CarNumPlate, PlateROI, CV_GRAY2BGR);

		// �׷�����....
		line( labeledColorImage, Point(new_pt1x, new_pt1y),
			Point(pt2x, pt2y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(pt2x, pt2y),
			Point(pt4x, pt4y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(pt4x, pt4y),
			Point(new_pt3x, new_pt3y), Scalar(0,255,0), 2, 8 );
		line( labeledColorImage, Point(new_pt3x, new_pt3y),
			Point(new_pt1x, new_pt1y), Scalar(0,255,0), 2, 8 );
	}

	return 1;
}

// ��κ� ã��.....
int CCarNumDetect::GetPlateRectFromFill(FILE* retf, bool only_check_4_or_5, int first_run)
{
	// ��ȣ�� ��κ� ã��.....
	blobCandidateCount = 0;

	GetNumberCandidates(first_run);

	get_check_cond(retf, only_check_4_or_5);

	TRACE2("sv count %d\r\n", sv_found_count, 0);
	if (sv_found_count > 0) {

		// 2���̻� ã����(26-17-1044)... �ϴܿ� ����� ����� �����ĺ��� �Ѵ�.....
		// 26-11-1818 �� �ϴܿ� �ִ°��� �ƴϴ�....
		//   ��Ģ���� : �ϴܺ��ٴ� ������ ������... 

		int max_ex = m_EdgeTable[sv_found_arr[0][0]].ex;
		int target_index = 0;
		for (int i = 1; i < sv_found_count; i++) {
			if (m_EdgeTable[sv_found_arr[i][0]].ex > max_ex) {
				max_ex = m_EdgeTable[sv_found_arr[i][0]].ex;
				target_index = i;
			}
		}
		// ����... found_arr[] �� ����....
		memmove(&found_arr[0], &sv_found_arr[target_index][0], sizeof(int)*4);

		// �׷�����... �������..
		// ����.... found_arr[4] �� �����ĺ� �ε��� �������...	
		DebugSelected4();
		return 1; // ����...
	}

	return -1; // ����....
}

void CCarNumDetect::GenerateFilledEdges()
{
	matWhiteFilled = Mat::zeros(detected_edges.rows, detected_edges.cols, CV_8UC1);
	matBlackFilled = Mat::zeros(detected_edges.rows, detected_edges.cols, CV_8UC1);

	white_m = matWhiteFilled;
	black_m = matBlackFilled;

	int save_xpos = -1;
	unsigned char edge_on = 0;
	int x_limit = detected_edges.cols-10;
	int y_limit = detected_edges.rows-10;
	for(int y = 10; y < y_limit; y++) {
		unsigned char last_edge = dte_m[y][9];
		unsigned char save_value = blurr_m[y][9];
		for(int x = 10; x < detected_edges.cols; x++) {
			unsigned char cur_edge = dte_m[y][x];
			if(cur_edge != 0) { // ������ �߰�
				if(dte_m[y][x-1] == 0) { // �ٷ� ���� ���� ������ �ƴϰ�
					if(x - save_xpos < 200) { // ���̴� ���� �ʰ�
						// �ֺ� ���� ����� ���ؼ�...
						float f_sum = 0;
						float f_cnt = 0;
						for(int xx = save_xpos - 2; xx < x+2; xx++) {
							f_sum += blurr_m[y][xx];
							f_cnt += 1.0;
						}
						f_sum /= f_cnt; // ����� ���ϰ�.
						// ���� ���� ������ ������
						float m_sum = 0;
						float m_cnt = 0;
						for(int xx = save_xpos + 1; xx < x-1; xx++) {
							m_sum += blurr_m[y][xx];
							m_cnt += 1.0;
						}
						m_sum /= m_cnt;
						if(m_sum > f_sum) {
							for(int xx = save_xpos + 1; xx < x-1; xx++) {
								white_m[y][xx] = 255; // �� ĭ�� ĥ�ϰ�
							}
						} else {
							for(int xx = save_xpos + 1; xx < x-1; xx++) {
								black_m[y][xx] = 255; // ��ο� ĭ�� ĥ�Ѵ�.
							}
						}
						/*
						if(save_value < blurr_m[y][x-1]) { // ���� ���� ���� ���� ��ο��
						for(int xx = save_xpos + 1; xx < x-1; xx++) {
						white_m[y][xx] = 255; // �� ĭ�� ĥ�ϰ�
						}
						} else { // ������
						for(int xx = save_xpos + 1; xx < x-1; xx++) {
						black_m[y][xx] = 255; // ��ο� ĭ�� ĥ�Ѵ�.
						}
						}
						*/
					}
					save_value = blurr_m[y][x-1];
				}
			} else { // ������ �ƴ�
				if(dte_m[y][x-1] != 0) {
					save_xpos = x-1;
				}
			}
		}
	}

	imshow("white_m", matWhiteFilled);
	imshow("black_m", matBlackFilled);
}

void CCarNumDetect::TryToSeperateBigEdge(blob_info* p_inf)
{
	/*
	int pixel_num = p_inf->pixel_num;
	for(int y = p_inf->sy; y < p_inf->ey; y++) {
	for(int x = p_inf->sy+1; x < p_inf->ex; x++) {
	if(flag_m[y][x] == p_inf->pixel_num) {
	if(flag_m[y+1][x-1] == pixel_num && flag_m[y+1][x] == pixel_num && flag_m[y+1][x+1] == pixel_num) {
	for(int 

	*/
}

#define	MAX_FIT_POINTS	4096

typedef struct _fPoint {
	float x;
	float y;
} fPoint;

fPoint PointsToFit[MAX_FIT_POINTS];

fPoint PointsToTest[] = {
	{ 0, 1 },
	{ 0, 3 },
	{ 0, 2 },
	{ 2, 2 },
	{ 2, 4 },
	{ 2, 3 },
	{ 2, 3 },
	{ 4, 5 },
	{ 4, 4 },
	{ 10, 7 },
	{ 10, 8 },
	{ 10, 6 }
};

void ComputeRegression(int xs, int xe, tLineParams* ptr)
{
	float x_cnt = (xe - xs + 1);
	float x_sum = 0;
	float y_sum = 0;
	float xy_sum = 0;
	float xx_sum = 0;

	for(int x = xs; x <= xe; x++) {
		x_sum += PointsToFit[x].x;
		y_sum += PointsToFit[x].y;
		xy_sum += PointsToFit[x].x * PointsToFit[x].y;
		xx_sum += PointsToFit[x].x * PointsToFit[x].x;
	}
	double x_bar = x_sum / x_cnt;
	double y_bar = y_sum / x_cnt;

	double b1 = (xy_sum - (x_sum * y_sum) / x_cnt) / (xx_sum - (x_sum*x_sum) / x_cnt);
	ptr->a = b1;
	float b2 = y_bar - b1 * x_bar;
	ptr->b = b2;
	ptr->theta = asin(b1);
	if(-0.0001 < b1 && b1 < 0.0001) { // ���򼱿� ������
		ptr->radius = b2;
		ptr->crx = 0;
		ptr->cry = b2;
	} else {
		double nx = -b2 / (b1 + (1.0 / b1)); // ������ ������ �־��� ���� �����ϴ� ���� �־��� ���� ������ ���� X��ǥ
		double ny = nx * (1.0 / b1);
		ptr->radius = sqrt(nx *nx + ny * ny);
		ptr->crx = (int)nx;
		ptr->cry = (int)ny;
	}

	float y_var = 0;
	for(int x = xs; x < xe; x++) {
		float y_exp = b1 * PointsToFit[x].x + b2;
		float y_dif = y_exp - PointsToFit[x].y;
		y_var += y_dif * y_dif;
	}
	ptr->var = y_var;
}

void TestRegression()
{
	for(int i = 0; i < 12; i++)
		PointsToFit[i] = PointsToTest[i];

	tLineParams result;
	ComputeRegression(0, 11, &result);
}

// �л��� �ּ�ġ�� �Ǵ� ������ ���� ���Ѵ�.
// �־��� �� �߿��� �л��� ū ���� ���� �����ϸ鼭 ���Ѵ�.
int GetLeastVarEquation(int sy, int ey, int minwidth, double var_allow, tLineParams* p)
{
	int height = ey - sy;
	int y1 = sy;
	int y2 = ey;
	while(1) {
		tLineParams t0, t1;
		ComputeRegression(y1, y2-1, &t0);
		ComputeRegression(y1+1, y2, &t1);
		if(t0.var < t1.var) {
			*p = t0;
			y2 -= 1;
		} else {
			*p = t1;
			y1 += 1;
		}
		height -= 1;
		if(height <= minwidth)
			return 0;
		if(p->var < var_allow)
			return 1;
	}
}

#if 0 // Regression Test
int CCarNumDetect::FindLinesByEdge() // ������ ���� ������ ���Ѵ�. houghlines���� ����� �ٸ���.
{
	TestRegression();

	return 0;
}
#else

int TestedEdges[1000];

int CCarNumDetect::FindLinesByEdge() // ������ ���� ������ ���Ѵ�. houghlines���� ����� �ٸ���.
{
	LabelEdges(); // ������ ��ȣ�� �ű��

	EdgeLineCount = 0;
	TestedEdgeCount = 0;

	for(int i = 0; i < m_nEdgeCount; i++) {
		blob_info* pedge = &m_EdgeTable[i];
		if(pedge->height < EdgeLineMinY || pedge->height > EdgeLineMaxY)
			continue;
		if(pedge->width < EdgeLineMinX || pedge->width > EdgeLineMaxX)
			continue;
		TestedEdges[TestedEdgeCount++] = i;

		tLineParams p1, p2;

		// ���� ���� ã�ƺ���
		// ��� ���, �ڷ� ��ô
		for(int x = pedge->sx; x <= pedge->ex; x++) {
			PointsToFit[x].x = x;
			for(int y = pedge->sy; y <= pedge->ey; y++) {
				if(flag_m[y][x] == pedge->pixel_num) {
					PointsToFit[x].y = y;
					break;
				}
			}
		}
		if(GetLeastVarEquation(pedge->sx, pedge->ex, horiline_min_pixels , horiline_var_allow , &p1)) { // ori_80.... ori_25
			p1.axis = 0; // ������ ã������ ǥ���Ѵ�.
			p1.x1 = pedge->sx; p1.y1 = pedge->sy;
			p1.x2 = pedge->ex; p1.y2 = pedge->ey;
			fpLines[EdgeLineCount] = p1;
			EdgeLineCount++;
		}

		// �ϴ� ���, �ڷ���ô
		for(int x = pedge->sx; x <= pedge->ex; x++) {
			PointsToFit[x].x = x;
			for(int y = pedge->ey; y >= pedge->sy; y--) {
				if(flag_m[y][x] == pedge->pixel_num) {
					PointsToFit[x].y = y;
					break;
				}
			}
		}
		if(GetLeastVarEquation(pedge->sx, pedge->ex, horiline_min_pixels , horiline_var_allow , &p1)) {
			p1.axis = 0; // ������ ã������ ǥ���Ѵ�.
			p1.x1 = pedge->sx; p1.y1 = pedge->sy;
			p1.x2 = pedge->ex; p1.y2 = pedge->ey;
			fpLines[EdgeLineCount] = p1;
			EdgeLineCount++;
		}
		/*
		double distance = p1.radius - p2.radius;
		if(distance < -5.0 || distance > 5.0) {
		fpLines[EdgeLineCount] = p1; EdgeLineCount++;
		fpLines[EdgeLineCount] = p2; EdgeLineCount++;
		}
		*/
		// �������� ã�ƺ���
		// �´� ���, �ϴ� ȸ�ͺм� ���̺� �ڷ� ��ô
		for(int y = pedge->sy; y <= pedge->ey; y++) {
			PointsToFit[y].x = y;
			for(int x = pedge->sx; x <= pedge->ex; x++) {
				if(flag_m[y][x] == pedge->pixel_num) {
					PointsToFit[y].y = x;
					break;
				}
			}
		}
		if(GetLeastVarEquation(pedge->sy, pedge->ey, vertline_min_pixels , vertline_var_allow , &p1)) {  //ori_25  ori_10
			p1.axis = 1; // ������ ã������ ǥ���Ѵ�.
			p1.x1 = pedge->sx; p1.y1 = pedge->sy;
			p1.x2 = pedge->ex; p1.y2 = pedge->ey;
			fpLines[EdgeLineCount] = p1;
			EdgeLineCount++;
		}

		// ��� ���, ȸ�ͺм� ���̺� �ڷ� ��~ô!!
		for(int y = pedge->sy; y <= pedge->ey; y++) {
			PointsToFit[y].x = y;
			for(int x = pedge->ex; x >= pedge->sx; x--) {
				if(flag_m[y][x] == pedge->pixel_num) {
					PointsToFit[y].y = x;
					break;
				}
			}
		}
		if(GetLeastVarEquation(pedge->sy, pedge->ey, vertline_min_pixels , vertline_var_allow , &p2)) {
			p2.axis = 1; // ������ ã������ ǥ���Ѵ�.
			p2.x1 = pedge->sx; p2.y1 = pedge->sy;
			p2.x2 = pedge->ex; p2.y2 = pedge->ey;
			fpLines[EdgeLineCount] = p2;
			EdgeLineCount++;
		}
		/*
		distance = p1.radius - p2.radius;
		if(distance < -5.0 || distance > 5.0) {
		fpLines[EdgeLineCount] = p1; EdgeLineCount++;
		fpLines[EdgeLineCount] = p2; EdgeLineCount++;
		}
		*/
	}

	return 0;
}
#endif

void CCarNumDetect::DrawEdgeLines(Mat& dst)
{
	int x1, y1, x2, y2;
	for(int i = 0; i < EdgeLineCount; i++) {
		tLineParams *pLineParam = &fpLines[i];
		if(pLineParam->axis == 0) {
			y1 = (int)(pLineParam->a * pLineParam->x1 + pLineParam->b);
			x1 = pLineParam->x1;
			if(y1 < pLineParam->y1) {
				y1 = pLineParam->y1;
				if(pLineParam->a != 0)
					x1 = (pLineParam->y1 - pLineParam->b) / pLineParam->a;
			} else if(y1 > pLineParam->y2) {
				y1 = pLineParam->y2;
				if(pLineParam->a != 0)
					x1 = (y1 - pLineParam->b) / pLineParam->a;
			}

			x2 = pLineParam->x2;
			y2 = x2 * pLineParam->a + pLineParam->b;
			if(y2 < pLineParam->y1) {
				y2 = pLineParam->y1;
				if(pLineParam->a != 0)
					x2 = (pLineParam->y1 - pLineParam->b) / pLineParam->a;
			} else if(y2 > pLineParam->y2) {
				y2 = pLineParam->y2;
				if(pLineParam->a != 0)
					x2 = (y2 - pLineParam->b) / pLineParam->a;
			}
			line(dst, Point(x1,y1), Point(x2,y2), Scalar(0,255,0));
		} else {
			x1 = (int)(pLineParam->a * pLineParam->y1 + pLineParam->b);
			y1 = pLineParam->y1;
			if(x1 < pLineParam->x1) {
				x1 = pLineParam->x1;
				if(pLineParam->a != 0)
					y1 = (pLineParam->x1 - pLineParam->b) / pLineParam->a;
			} else if(x1 > pLineParam->x2) {
				x1 = pLineParam->x2;
				if(pLineParam->a != 0)
					y1 = (x1 - pLineParam->b) / pLineParam->a;
			}

			y2 = pLineParam->y2;
			x2 = y2 * pLineParam->a + pLineParam->b;
			if(x2 < pLineParam->x1) {
				x2 = pLineParam->x1;
				if(pLineParam->a != 0)
					y2 = (pLineParam->x1 - pLineParam->b) / pLineParam->a;
			} else if(x2 > pLineParam->x2) {
				x2 = pLineParam->x2;
				if(pLineParam->a != 0)
					y2 = (x2 - pLineParam->b) / pLineParam->a;
			}
			line(dst, Point(x1,y1), Point(x2,y2), Scalar(0,255,255));
		}
	}

	//DEBUG..........
	if(1) {
		// ���� ã�� ���� �׸��� �κ��ε� �ʹ� ���� �׷����� ����ϹǷ� �ϴ� ���Ƶд�. dschae20140908
		for (int i = 0; i <= m_nBlobIndex; i++) {
			if (m_EdgeTable[i].sx >= 521 && m_EdgeTable[i].sx <= 531 &&
				m_EdgeTable[i].sy >= 377 && m_EdgeTable[i].sy <= 388 ) {
					cv::rectangle(dst, 
						cv::Point(m_EdgeTable[i].sx, m_EdgeTable[i].sy),
						cv::Point(m_EdgeTable[i].ex, m_EdgeTable[i].ey),
						cv::Scalar(0,0,255));
			}

		}
	}


}

void DrawTestedEdges(void *pDrawer, CDC* pdc)
{
	CCarNumDetect* pDetector = (CCarNumDetect*)pDrawer;
	pDetector->DrawTestedEdges(pdc);
}


void CCarNumDetect::DrawTestedEdges(CDC* pdc)
{
	for(int i = 0; i < TestedEdgeCount; i++) {
		struct blob_info* pinfo = &m_EdgeTable[TestedEdges[i]];
		CString ttext;
		ttext.Format(L"%d", TestedEdges[i]);
		pdc->TextOut(pinfo->sx, pinfo->sy, ttext);
	}
}


void DrawMarkedEdge(void *pEdgeDetector, CDC* pdc, int edge_to_mark)
{
	CCarNumDetect* pDetector = (CCarNumDetect*)pEdgeDetector;
	pDetector->DrawMarkedEdge(pdc, edge_to_mark);
}


void CCarNumDetect::DrawMarkedEdge(CDC *pdc, int edge_to_mark)
{
	if(edge_to_mark >= m_nEdgeCount)
		return;

	struct blob_info* pinfo = &m_EdgeTable[edge_to_mark];
	int index = pinfo->pixel_num;
	for(int y = pinfo->sy; y <= pinfo->ey; y++) {
		for(int x = pinfo->sx; x <= pinfo->ex; x++) {
			if(flag_m[y][x] == index)
				pdc->SetPixel(x, y, RGB(0, 40, 255));
		}
	}
}



//////////////////////////////////////////////////////////////
// HDS-TEST-140926
int th_HORILINE_CONTINUE_PIXELS = 8;
int th_VERTLINE_CONTINUE_PIXELS = 8;


float CCarNumDetect::getAngle(const int ori_line[], int *maxline, bool bHoriline)
{

	int dx = ori_line[2] - ori_line[0];  // ex - sx
	int dy = ori_line[3] - ori_line[1];  // ey - sy


	float radian = atan2((float)dx, (float)dy);
	float degree = abs((radian*180)/CV_PI - 180);  // +y���� 180��, -y���� 0�� �� ���� --> +y���� 0��,-y���� 180�� �� ������ ��ȯ...�ð��������.....
	radian = (CV_PI/180) * degree;

	float rho = ori_line[0] * cos(radian) + ori_line[1] * sin(radian);

	if (bHoriline) { // ����...
		maxline[0] = 0;
		maxline[1] = (rho - 0 * cos(radian))  /  (sin(radian) + 0.0000001);
		maxline[2] = m_nSCR_WIDTH;
		maxline[3] = (rho - m_nSCR_WIDTH * cos(radian))  / (sin(radian) + 0.0000001);

	}
	else {  // ������.....

		maxline[0] = (rho - 0 * sin(radian))  / (cos(radian) + 0.0000001);
		maxline[1] = 0;
		maxline[2] = (rho - m_nSCR_HEIGHT * sin(radian))  / (cos(radian) + 0.0000001);
		maxline[3] = m_nSCR_HEIGHT;	
	}

	//float degree = abs((radian*180)/CV_PI - 180);  // +y���� 180��, -y���� 0�� �� ���� --> +y���� 0��,-y���� 180�� �� ������ ��ȯ...�ð��������.....
	return degree; 

}


//HDS-140928
float CCarNumDetect::getAngle2(const float degree, const int ori_line[], int *maxline, bool bHoriline)
{

	float radian = (CV_PI/180) * degree;
	float rho = ori_line[0] * cos(radian) + ori_line[1] * sin(radian);

	if (bHoriline) { // ����...
		maxline[0] = 0;
		maxline[1] = (rho - 0 * cos(radian))  /  (sin(radian) + 0.0000001);
		maxline[2] = m_nSCR_WIDTH;
		maxline[3] = (rho - m_nSCR_WIDTH * cos(radian))  / (sin(radian) + 0.0000001);

	}
	else {  // ������.....

		maxline[0] = (rho - 0 * sin(radian))  / (cos(radian) + 0.0000001);
		maxline[1] = 0;
		maxline[2] = (rho - m_nSCR_HEIGHT * sin(radian))  / (cos(radian) + 0.0000001);
		maxline[3] = m_nSCR_HEIGHT;	
	}

	//float degree = abs((radian*180)/CV_PI - 180);  // +y���� 180��, -y���� 0�� �� ���� --> +y���� 0��,-y���� 180�� �� ������ ��ȯ...�ð��������.....
	return degree; 

}




int line_sx[50],line_ex[50], line_sy[50], line_ey[50];
int sv_lines_count[50];  // ���� �ȼ���

void CCarNumDetect::find_blob_hori_line(blob_info* pedge, bool bUp_line)  {

	// ���� ã��.....
	int sv_lines = 0; // �ִ� 10������ �ĺ��� ã�Ƽ� ���� ����� �����Ѵ�...
	memset(sv_lines_count, 0, sizeof(int)*50);
	memset(line_sx, 0, sizeof(int)*50);
	memset(line_ex, 0, sizeof(int)*50);
	memset(line_sy, 0, sizeof(int)*50);
	memset(line_ey, 0, sizeof(int)*50);


	int up_count = 0;  // ������ ���� Ƚ�� 
	int down_count = 0;  // �Ʒ����� ���� Ƚ�� 
	int cur_y_pos;
	int mid_pos;



	mid_pos = (int)((pedge->ey - pedge->sy)/2) + pedge->sy;
	// 1.1  ������ ���ϱ�...

	for(int x = pedge->sx; x <= pedge->ex; x++) {

		line_sx[sv_lines] = x; 
		line_sy[sv_lines] = -1;
		sv_lines_count[sv_lines] = 0; 

		if (bUp_line) { // ���.. ������ �Ʒ���...
			for(int y = pedge->sy; y <= mid_pos; y++) {
				if(flag_m[y][x] == pedge->pixel_num) {
					line_sy[sv_lines] = y;
					break;
				}
			}
		}
		else { // �ϴ�.. �Ʒ����� ����..
			for(int y = pedge->ey; y >= mid_pos; y--) {
				if(flag_m[y][x] == pedge->pixel_num) {
					line_sy[sv_lines] = y;
					break;
				}
			}

		}
		if (line_sy[sv_lines] == -1) continue;

		// 1.2 �������� ����...���� �ִµ����� �ִ��� �����Ѵ�...
		cur_y_pos = line_sy[sv_lines];
		sv_lines_count[sv_lines] = 1; 
		up_count = 0; 
		down_count = 0;  
		for(int x = line_sx[sv_lines]+1; x <= pedge->ex; x++) {


			// �����ϴٰ� ���� �������� ���ο� �������� ���� ã�� ���� ���....  ���� x ������ y�� ã�ƺ���....
			if (line_sy[sv_lines] == -1) {
				line_sx[sv_lines] = x; 
				line_sy[sv_lines] = -1;
				sv_lines_count[sv_lines] = 0; 
				if (bUp_line) {
					for(int y = pedge->sy; y <= mid_pos; y++) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sy[sv_lines] = y;
							break;
						}
					}
				}
				else {
					for(int y = pedge->ey; y >= mid_pos; y--) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sy[sv_lines] = y;
							break;
						}
					}
				}
				// ������ ã���� �ʱ�ȭ�ؼ� ����....
				if (line_sy[sv_lines] != -1) {
					cur_y_pos = line_sy[sv_lines];
					sv_lines_count[sv_lines] = 1; 
					up_count = 0; 
					down_count = 0;  
				}

				continue;
			}


			// ������ üũ...
			if(flag_m[cur_y_pos][x] == pedge->pixel_num) {
				line_ex[sv_lines] = x;
				line_ey[sv_lines] = cur_y_pos;
				++(sv_lines_count[sv_lines]);
			}
			// �Ʒ����� üũ..
			else if (((cur_y_pos-1) >= 0) && (flag_m[cur_y_pos-1][x] == pedge->pixel_num)) {
				++down_count;

				if (up_count > 0 ) { // ������ ������ ������ �־�����...

					// ������ üũ�ؼ� ������ ���� ����...
					if (((cur_y_pos+1) < m_nSCR_HEIGHT) && (flag_m[cur_y_pos+1][x] == pedge->pixel_num)) {
						line_ex[sv_lines] = x;
						line_ey[sv_lines] = cur_y_pos+1;
						cur_y_pos = line_ey[sv_lines];
						++(sv_lines_count[sv_lines]);
						up_count++;
					}
					// ������ �ι��̻� ������ �־����� ���������� �ٽ� ī��Ʈ ����.....�ƴϸ� ��� ���� ����.. 
					else {

						if (up_count > 1) {
							// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
							// 
							if (sv_lines_count[sv_lines] >= th_HORILINE_CONTINUE_PIXELS) {
								sv_lines++;
								if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....
							}



							line_sx[sv_lines] = x; 
							line_sy[sv_lines] = cur_y_pos-1;
							cur_y_pos = line_sy[sv_lines];
							sv_lines_count[sv_lines] = 1; 
							up_count = 0; 
							down_count = 0;  
						}
						else {
							line_ex[sv_lines] = x;
							line_ey[sv_lines] = cur_y_pos-1;
							cur_y_pos = line_sy[sv_lines];
							++(sv_lines_count[sv_lines]);
							up_count--;  // ������ ���� ǥ�� �����...

						}
					}
				}
				else { // ������ ������ ������.. �Ʒ��� ����...
					line_ex[sv_lines] = x;
					line_ey[sv_lines] = cur_y_pos-1;
					cur_y_pos = line_ey[sv_lines];
					++(sv_lines_count[sv_lines]);
				}
			}
			// ������ üũ..
			else if (((cur_y_pos+1) < m_nSCR_HEIGHT) && (flag_m[cur_y_pos+1][x] == pedge->pixel_num)) {
				++up_count;

				if (down_count > 0 ) { // ������ �Ʒ����� ������ �־�����... 

					//  �Ʒ����� üũ�ؼ� ������ �Ʒ��� ����...
					if (((cur_y_pos-1) >= 0) && (flag_m[cur_y_pos-1][x] == pedge->pixel_num)) {
						line_ex[sv_lines] = x;
						line_ey[sv_lines] = cur_y_pos-1;
						cur_y_pos = line_ey[sv_lines];
						++(sv_lines_count[sv_lines]);
						down_count++;
					}
					//  ������ �ι��̻� �Ʒ����� �־����� ���������� �ٽ� ī��Ʈ ����.....�ƴϸ� ��� ���� ����.. 
					else {

						if (down_count > 1) {
							// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
							if (sv_lines_count[sv_lines] >= th_HORILINE_CONTINUE_PIXELS) {
								sv_lines++;	
								if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....

							}

							line_sx[sv_lines] = x; 
							line_sy[sv_lines] = cur_y_pos+1;
							cur_y_pos = line_sy[sv_lines];
							sv_lines_count[sv_lines] = 1; 
							up_count = 0; 
							down_count = 0;  
						}
						else {
							line_ex[sv_lines] = x;
							line_ey[sv_lines] = cur_y_pos+1;
							cur_y_pos = line_ey[sv_lines];
							++(sv_lines_count[sv_lines]);
							down_count--; // �Ʒ����� ���� ǥ�� �����...
						}
					}
				}
				else { // ������ �Ʒ����� ������.. ���� ����...
					line_ex[sv_lines] = x;
					line_ey[sv_lines] = cur_y_pos+1;
					cur_y_pos = line_ey[sv_lines];
					++(sv_lines_count[sv_lines]);
				}
			}
			// �̾��� ���� ����.. �� ������.... 1ĭ ��üũ�ؼ� ������ ����...   ������ ���� x ��ġ���� ������ y�� �ٽñ��ϰ�.. ���� �Ѵ�.,...
			else {


				if ( ((x+1) <= pedge->ex) && 
					( (flag_m[cur_y_pos][x+1] == pedge->pixel_num) || 
					(((cur_y_pos-1) > 0) && (flag_m[cur_y_pos-1][x+1] == pedge->pixel_num)) || 
					(((cur_y_pos+1) < m_nSCR_HEIGHT) && (flag_m[cur_y_pos+1][x+1] == pedge->pixel_num))
					)
					)
				{ // 1ĭ �� ������ ������ �����ϸ� �������� �����ؼ� ��� �����ϰ� �Ѵ�...
					line_ex[sv_lines] = x;
					line_ey[sv_lines] = cur_y_pos;
					++(sv_lines_count[sv_lines]);

					continue;
				}

				// ������ �����ϰ�. ���� x ��ġ���� ������ y�� �ٽñ��ϰ�.. ���� �Ѵ�.,...


				// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
				if (sv_lines_count[sv_lines] >= th_HORILINE_CONTINUE_PIXELS) {
					sv_lines++;	
					if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....
				} 

				// ���������� �ٽ� �����Ѵ�...
				sv_lines_count[sv_lines] = 0; 

				// ���� x��ġ���� y �� ���Ѵ�...
				line_sx[sv_lines] = x; 
				line_sy[sv_lines] = -1;
				if (bUp_line) {
					for(int y = pedge->sy; y <= mid_pos; y++) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sy[sv_lines] = y;
							break;
						}
					}
				}
				else {
					for(int y = pedge->ey; y >= mid_pos; y--) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sy[sv_lines] = y;
							break;
						}
					}
				}


				// ������ ã���� �ʱ�ȭ�ؼ� ����....
				if (line_sy[sv_lines] != -1) {
					cur_y_pos = line_sy[sv_lines];
					sv_lines_count[sv_lines] = 1; 
					up_count = 0; 
					down_count = 0;  
				}
			}
		}

		// ���� ��Ƽ� �ٷ� �������...
		if (sv_lines < 50 && line_ex[sv_lines] != 0 && line_ey[sv_lines] != 0 && sv_lines_count[sv_lines] > 0) sv_lines++;


		// ���� ��κ꿡 ���� �� ���� ���� ����.......
		// 
		int max_len_index = -1;
		int width = 0;
		for (int i = 0; i < sv_lines; i++) {
			if (abs(line_ex[i] - line_sx[i]) > width) {
				max_len_index = i;
				width = abs(line_ex[i] - line_sx[i]);
			}
		}

		if (max_len_index != -1) {
			pedge->line_count++;
			if (bUp_line) {
				pedge->line_type_id += 1; 
				pedge->up_line[0] = line_sx[max_len_index];
				pedge->up_line[1] = line_sy[max_len_index];
				pedge->up_line[2] = line_ex[max_len_index];
				pedge->up_line[3] = line_ey[max_len_index];
				pedge->up_degree = getAngle(pedge->up_line, pedge->up_maxline, true);

			}
			else {
				pedge->line_type_id += 123; 
				pedge->down_line[0] = line_sx[max_len_index];
				pedge->down_line[1] = line_sy[max_len_index];
				pedge->down_line[2] = line_ex[max_len_index];
				pedge->down_line[3] = line_ey[max_len_index];

				//HDS-140928
				//pedge->down_degree = getAngle(pedge->down_line, pedge->down_maxline, true);
				//  �Ʒ��ڵ�� ��ܰ� �ϴ��� ���ÿ� ������� ������� ������ ���Ͻ�Ű�� ���ε�... �� �ȸ´°� ���⵵... 30-09-9427.. �ϴ� ���ƺ���....
				// �ƴϴ�.... �����ϱ� ���ۿ� �ξ� ����...

				// ��� ������ ������ �ϴܶ��� ���Ѵ�..
				if (pedge->up_line[0] == -1) {
					pedge->down_degree = getAngle(pedge->down_line, pedge->down_maxline, true);

				} 
				// ��ܶ����� ������....
				else {

					// ��ܶ����� ��� ��� ������ �̿��Ͽ� �ϴ� �ƽ����� ���Ѵ�...
					if (abs(pedge->up_line[2]-pedge->up_line[0]) >= abs(pedge->down_line[2]-pedge->down_line[0])) {
						pedge->down_degree = getAngle2(pedge->up_degree, pedge->down_line, pedge->down_maxline, true);

					}
					// �ϴ� ������ ��� �ϴ� ���� ���ϰ�..  ��� ������ �ϴܰ����� �ٽ� ���Ѵ�...
					else {
						pedge->down_degree = getAngle(pedge->down_line, pedge->down_maxline, true);
						pedge->up_degree = getAngle2(pedge->down_degree, pedge->up_line, pedge->up_maxline, true);
					}
				}


			}
			break; // // ������ ������ ����....���̻� ã�� �ʴ´�... 

		}
	}
}


void CCarNumDetect::find_blob_vert_line(blob_info* pedge, bool bLeft_line)  {

	// ������ ã��.....
	int sv_lines = 0; // �ִ� 10������ �ĺ��� ã�Ƽ� ���� ����� �����Ѵ�...
	memset(sv_lines_count, 0, sizeof(int)*50);
	memset(line_sx, 0, sizeof(int)*50);
	memset(line_ex, 0, sizeof(int)*50);
	memset(line_sy, 0, sizeof(int)*50);
	memset(line_ey, 0, sizeof(int)*50);

	int left_count = 0;  // ���ʹ��� ���� Ƚ�� 
	int right_count = 0;  // �����ʹ��� ���� Ƚ�� 
	int cur_x_pos;
	int mid_pos;



	//
	mid_pos = (int)((pedge->ex - pedge->sx)/2) + pedge->sx;

	// 1.1  ������ ���ϱ�...
	for(int y = pedge->sy; y <= pedge->ey; y++) {

		line_sy[sv_lines] = y; 
		line_sx[sv_lines] = -1;
		sv_lines_count[sv_lines] = 0; 

		if (bLeft_line) { // ����.. ���ʿ��� ����������...
			for(int x = pedge->sx; x <= mid_pos; x++) {
				if(flag_m[y][x] == pedge->pixel_num) {
					line_sx[sv_lines] = x;
					break;
				}
			}
		}
		else { // ������.. �����ʿ��� ��������..
			for(int x = pedge->ex; x >= mid_pos; x--) {
				if(flag_m[y][x] == pedge->pixel_num) {
					line_sx[sv_lines] = x;
					break;
				}
			}

		}

		if (line_sx[sv_lines] == -1) continue;

		// 1.2 �������� ����...���� �ִµ����� �ִ��� �����Ѵ�...
		cur_x_pos = line_sx[sv_lines];
		sv_lines_count[sv_lines] = 1; 
		left_count = 0; 
		right_count = 0;  
		for(int y = line_sy[sv_lines]+1; y <= pedge->ey; y++) {


			// �����ϴٰ� ���� �������� ���ο� �������� ���� ã�� ���� ���....  ���� y ������ x�� ã�ƺ���....
			if (line_sx[sv_lines] == -1) {
				line_sy[sv_lines] = y; 
				line_sx[sv_lines] = -1;
				if (bLeft_line) {
					for(int x = pedge->sx; x <= mid_pos; x++) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sx[sv_lines] = x;
							break;
						}
					}
				}
				else {
					for(int x = pedge->ex; x >= mid_pos; x--) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sx[sv_lines] = x;
							break;
						}
					}
				}
				// ������ ã���� �ʱ�ȭ�ؼ� ����....
				if (line_sx[sv_lines] != -1) {
					cur_x_pos = line_sx[sv_lines];
					sv_lines_count[sv_lines] = 1; 
					left_count = 0; 
					right_count = 0;  
				}

				continue;
			}


			// ������ üũ...
			if(flag_m[y][cur_x_pos] == pedge->pixel_num) {
				line_ey[sv_lines] = y;
				line_ex[sv_lines] = cur_x_pos;
				++(sv_lines_count[sv_lines]);
			}
			// �����ʹ��� üũ..
			else if (((cur_x_pos-1) < m_nSCR_WIDTH) && (flag_m[y][cur_x_pos+1] == pedge->pixel_num)) {
				++right_count;

				if (left_count > 0 ) { // ������ ���ʹ��� ������ �־�����....  

					// ���ʹ��� üũ�ؼ� ������ �������� ����...
					if (((cur_x_pos-1) >= 0) && (flag_m[y][cur_x_pos-1] == pedge->pixel_num)) {
						line_ey[sv_lines] = y;
						line_ex[sv_lines] = cur_x_pos-1;
						cur_x_pos = line_ex[sv_lines];
						++(sv_lines_count[sv_lines]);
						++left_count;
					}
					// ������ �ι��̻� �������� �־����� ������ ���������� �ٽ� ī��Ʈ ����... �ƴϸ� ��� ���� ����..
					else {


						if (left_count >= 2) { 
							// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
							if (sv_lines_count[sv_lines] >= th_VERTLINE_CONTINUE_PIXELS) {
								sv_lines++;	
								if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....

							} 

							line_sy[sv_lines] = y; 
							line_sx[sv_lines] = cur_x_pos+1;
							cur_x_pos = line_sx[sv_lines];
							sv_lines_count[sv_lines] = 1; 
							left_count = 0; 
							right_count = 0;  
						}
						else {
							line_ey[sv_lines] = y;
							line_ex[sv_lines] = cur_x_pos+1;
							cur_x_pos = line_ex[sv_lines];
							++(sv_lines_count[sv_lines]);
							--left_count; 
						}
					}
				}
				else { // ������ ���ʹ��� ������.. ���������� ����...
					line_ey[sv_lines] = y;
					line_ex[sv_lines] = cur_x_pos+1;
					cur_x_pos = line_ex[sv_lines];
					++(sv_lines_count[sv_lines]);
				}
			}
			// ���ʹ��� üũ..
			else if (((cur_x_pos-1) >= 0) && (flag_m[y][cur_x_pos-1] == pedge->pixel_num)) {
				++left_count;

				if (right_count > 0 ) { // ������ �����ʹ��� ������ �־�����..... 

					// �����ʹ��� üũ�ؼ� ������ ���������� ����... 
					if (((cur_x_pos-1) < m_nSCR_WIDTH) && (flag_m[y][cur_x_pos+1] == pedge->pixel_num)) {
						line_ey[sv_lines] = y;
						line_ex[sv_lines] = cur_x_pos+1;
						cur_x_pos = line_ex[sv_lines];
						++(sv_lines_count[sv_lines]);
						++right_count;
					}
					// ������ �ι��̻� ������ ���� ���� ������ ���������� �ٽ� ī��Ʈ ����...�ƴϸ� ����������...
					else {

						if (right_count >= 2) { 
							// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
							if (sv_lines_count[sv_lines] >= th_VERTLINE_CONTINUE_PIXELS) {
								sv_lines++;	
								if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....
							} 

							line_sy[sv_lines] = y; 
							line_sx[sv_lines] = cur_x_pos-1;
							cur_x_pos = line_sx[sv_lines];
							sv_lines_count[sv_lines] = 1; 
							left_count = 0; 
							right_count = 0;  
						}
						else {
							line_ey[sv_lines] = y;
							line_ex[sv_lines] = cur_x_pos-1;
							cur_x_pos = line_ex[sv_lines];
							++(sv_lines_count[sv_lines]);
							--right_count;  

						}
					}
				}
				else { // ������ �����ʹ��� ������.. �������� ����...
					line_ey[sv_lines] = y;
					line_ex[sv_lines] = cur_x_pos-1;
					cur_x_pos = line_ex[sv_lines];
					++(sv_lines_count[sv_lines]);
				}
			}
			// �̾��� ���� ����.. �� ������.... 1ĭ �� ������ ������ �����ϸ� �������.. �ƴϸ� ���� y ��ġ���� ������ x  �ٽñ��ϰ�.. ���� �Ѵ�.,...
			else {


				if ( ((y+1) <= pedge->ey) && 
					( (flag_m[y+1][cur_x_pos] == pedge->pixel_num) || 
					(((cur_x_pos-1) > 0) && (flag_m[y+1][cur_x_pos-1] == pedge->pixel_num)) || 
					(((cur_x_pos+1) < m_nSCR_WIDTH) && (flag_m[y+1][cur_x_pos+1] == pedge->pixel_num))
					)
					)
				{ // 1ĭ �� ������ ������ �����ϸ� �������� �����ؼ� ��� �����ϰ� �Ѵ�...
					line_ey[sv_lines] = y;
					line_ex[sv_lines] = cur_x_pos;
					++(sv_lines_count[sv_lines]);

					continue;
				}

				// ������ �����ϰ�. ���� x ��ġ���� ������ y�� �ٽñ��ϰ�.. ���� �Ѵ�.,...
				// �̹� ���� �ȼ� �̻��� ���̸� ã����� �ߴ����� �ʰ� �ϴ� ������ ��� �����Ѵ�.....
				if (sv_lines_count[sv_lines] >= th_VERTLINE_CONTINUE_PIXELS) {
					sv_lines++;	
					if (sv_lines >= 50) break;  // �Ѱ� ����... ���� �ߴ�����.. �̸�ŭ�̸� ���....
				} 


				// ���������� �ٽ� �����Ѵ�...
				sv_lines_count[sv_lines] = 0; 


				// ���� y��ġ���� x�� ���Ѵ�...
				line_sy[sv_lines] = y; 
				line_sx[sv_lines] = -1;
				if (bLeft_line) {
					for(int x = pedge->sx; x <= mid_pos; x++) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sx[sv_lines] = x;
							break;
						}
					}
				}
				else {
					for(int x = pedge->ex; x >= mid_pos; x--) {
						if(flag_m[y][x] == pedge->pixel_num) {
							line_sx[sv_lines] = x;
							break;
						}
					}
				}


				// ������ ã���� �ʱ�ȭ�ؼ� ����....
				if (line_sx[sv_lines] != -1) {
					cur_x_pos = line_sx[sv_lines];
					sv_lines_count[sv_lines] = 1; 
					left_count = 0; 
					right_count = 0;  
				}
			}
		}

		// ���� ��Ƽ� �ٷ� �������...
		if (sv_lines < 50 && line_ex[sv_lines] != 0 && line_ey[sv_lines] != 0 && sv_lines_count[sv_lines] > 0) sv_lines++;


		// ���� ��κ꿡 ���� �� ������ ���� ����.......
		int max_len_index = -1;
		int height = 0;
		for (int i = 0; i < sv_lines; i++) {
			if (abs(line_ey[i] - line_sy[i]) > height) {
				max_len_index = i;
				height = abs(line_ey[i] - line_sy[i]);
			}
		}

		if (max_len_index != -1) {
			pedge->line_count++;
			if (bLeft_line) {
				pedge->line_type_id += 1234; 
				pedge->left_line[0] = line_sx[max_len_index];
				pedge->left_line[1] = line_sy[max_len_index];
				pedge->left_line[2] = line_ex[max_len_index];
				pedge->left_line[3] = line_ey[max_len_index];
				// ���� ���� ���صд�......
				pedge->left_degree = getAngle(pedge->left_line, pedge->left_maxline, false);
			}
			else {
				pedge->line_type_id += 12; 
				pedge->right_line[0] = line_sx[max_len_index];
				pedge->right_line[1] = line_sy[max_len_index];
				pedge->right_line[2] = line_ex[max_len_index];
				pedge->right_line[3] = line_ey[max_len_index];

				//HDS-140928
				pedge->right_degree = getAngle(pedge->right_line, pedge->right_maxline, false);
				//  �Ʒ��ڵ�� ���ʰ� �������� ���ÿ� ������� ������� ������ ���Ͻ�Ű�� ���ε�... �� �ȸ´°� ���⵵... 30-09-9427.. �ϴ� ���ƺ���....
				// �ƴϴ�.... �����ϱ� ���ۿ� �ξ� ����...

				// ���� ������ ������ ������ �ƽ����� ���Ѵ�..
				if (pedge->left_line[0] == -1) {
					pedge->right_degree = getAngle(pedge->right_line, pedge->right_maxline, false);

				} 
				// ���ʶ����� ������....
				else {

					// ���ʶ����� ��� ���ʶ��� ������ �̿��Ͽ� ������ �ƽ����� ���Ѵ�...
					if (abs(pedge->left_line[3]-pedge->left_line[1]) >= abs(pedge->right_line[3]-pedge->right_line[1])) {
						pedge->right_degree = getAngle2(pedge->left_degree, pedge->right_line, pedge->right_maxline, false);

					}
					// ������ ������ ��� ������ ���� ���ϰ�..  ���� ������ �����ʶ��� ������ �ٽ� ���Ѵ�...
					else {
						pedge->right_degree = getAngle(pedge->right_line, pedge->right_maxline, false);
						pedge->left_degree = getAngle2(pedge->right_degree, pedge->left_line, pedge->left_maxline, false);
					}
				}


			}

			break; // ������ ������ ����....���̻� ã�� �ʴ´�... 
		}
	}
}


bool GetIntersectPoint(const POINT& AP1, const POINT& AP2, 

	const POINT& BP1, const POINT& BP2, POINT* IP) 

{

	double t;

	double s; 

	double under = (BP2.y-BP1.y)*(AP2.x-AP1.x)-(BP2.x-BP1.x)*(AP2.y-AP1.y);

	if(under==0) return false;



	double _t = (BP2.x-BP1.x)*(AP1.y-BP1.y) - (BP2.y-BP1.y)*(AP1.x-BP1.x);

	double _s = (AP2.x-AP1.x)*(AP1.y-BP1.y) - (AP2.y-AP1.y)*(AP1.x-BP1.x); 



	t = _t/under;

	s = _s/under; 



	if(t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;

	if(_t==0 && _s==0) return false; 



	IP->x = AP1.x + t * (double)(AP2.x-AP1.x);

	IP->y = AP1.y + t * (double)(AP2.y-AP1.y);



	return true;

}


void CCarNumDetect::get_cross_point(int *px, int *py, int *line1, int *line2)
{

	double t;
	double s; 

	double under = (line2[3]-line2[1])*(line1[2]-line1[0])-(line2[2]-line2[0])*(line1[3]-line1[1]);

	if(under==0) {
		*px = -1;
		*py = -1;
		return;
	}

	double _t = (line2[2]-line2[0])*(line1[1]-line2[1]) - (line2[3]-line2[1])*(line1[0]-line2[0]);
	double _s = (line1[2]-line1[0])*(line1[1]-line2[1]) - (line1[3]-line1[1])*(line1[0]-line2[0]); 

	t = _t/under;
	s = _s/under; 

	if(t<0.0 || t>1.0 || s<0.0 || s>1.0) {
		*px = -1;
		*py = -1;
		return;
	}

	if(_t==0 && _s==0) {
		*px = -1;
		*py = -1;
		return;
	}


	*px = line1[0] + t * (double)(line1[2]-line1[0]);
	*py = line1[1] + t * (double)(line1[3]-line1[1]);

}


bool CCarNumDetect::adjust_box_horiline_fill_up_line(blob_info* check_pedge, blob_info* pedge)
{
	// 1. �ϴ� ���򼱺��� ���� ������
	// 2. �ϴ� ���򼱰� ������ �����ϰ�
	// 3. ���� �������� ������ �� �� ���ϴ����� �������� ������̰� ������ ���...
	// 4. �հ��̸�.. ��ܼ� ������ �߰����ش�... �ٷ� ������....

	int px1,py1,px2,py2;

	// ��� ���� üũ....
	while (1) {
		// ���� �����ϰ�...
		if (check_pedge->up_line[0] == -1) break;

		// 1. �ϴ� ���򼱺��� ���� ������
		if ( check_pedge->up_line[1] >= pedge->down_line[1]) break;

		// 2. �ϴ� ���򼱰� ������ �����ϰ� 5�� ���� ����....//    14-06-30-11-8225	 113, 100 dptj 1.97�� ���̳�...
		if ( abs(check_pedge->up_degree - pedge->down_degree) >= 5) break;

		// 3. ���� �������� ������ �»���� �� ����ڽ� ���ϴ����� �������� ������̰� ������ ���...
		// �»���� ���ϱ�...
		get_cross_point(&px1, &py1, pedge->left_maxline, check_pedge->up_maxline);
		// ���ϴ��� ���ϱ�...
		get_cross_point(&px2, &py2, pedge->left_maxline, pedge->down_maxline);
		// ����������� ���ϱ�..
		int height = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
		// ������ ����.....1,500 < x < 10,000
		if (height < 1500 || height > 10000) break;

		// 4. �հ�..... ��ܼ� ������ �߰����ش�... �ٷ� ������....
		pedge->line_count++;
		pedge->line_type_id += 1; 
		pedge->up_line[0] = check_pedge->up_line[0];
		pedge->up_line[1] = check_pedge->up_line[1];
		pedge->up_line[2] = check_pedge->up_line[2];
		pedge->up_line[3] = check_pedge->up_line[3];
		pedge->up_maxline[0] = check_pedge->up_maxline[0];
		pedge->up_maxline[1] = check_pedge->up_maxline[1];
		pedge->up_maxline[2] = check_pedge->up_maxline[2];
		pedge->up_maxline[3] = check_pedge->up_maxline[3];
		pedge->up_degree = check_pedge->up_degree;

		return true;

	}

	// �ϴ� ���� üũ....
	while (1) {
		// ���� �����ϰ�...
		if (check_pedge->down_line[0] == -1) break;

		// 1. �ϴ� ���򼱺��� ���� ������
		if ( check_pedge->down_line[1] >= pedge->down_line[1]) break;

		// 2. �ϴ� ���򼱰� ������ �����ϰ� 1�� ���� ����....
		if ( abs(check_pedge->down_degree - pedge->down_degree) >= 1) break;

		// 3. ���� �������� ������ �»���� �� ����ڽ� ���ϴ����� �������� ������̰� ������ ���...
		// �»���� ���ϱ�...
		get_cross_point(&px1, &py1, pedge->left_maxline, check_pedge->down_maxline);
		// ���ϴ��� ���ϱ�...
		get_cross_point(&px2, &py2, pedge->left_maxline, pedge->down_maxline);
		// ����������� ���ϱ�..
		int height = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
		// ������ ����.....1,500 < x < 10,000
		if (height < 1500 || height > 10000) break;

		// 4. �հ�..... ��ܼ� ������ �߰����ش�... �ٷ� ������....
		pedge->line_count++;
		pedge->line_type_id += 1; 
		pedge->up_line[0] = check_pedge->down_line[0];
		pedge->up_line[1] = check_pedge->down_line[1];
		pedge->up_line[2] = check_pedge->down_line[2];
		pedge->up_line[3] = check_pedge->down_line[3];
		pedge->up_maxline[0] = check_pedge->down_maxline[0];
		pedge->up_maxline[1] = check_pedge->down_maxline[1];
		pedge->up_maxline[2] = check_pedge->down_maxline[2];
		pedge->up_maxline[3] = check_pedge->down_maxline[3];
		pedge->up_degree = check_pedge->down_degree;

		return true;

	}

	return false;
}

bool CCarNumDetect::adjust_box_horiline_fill_down_line(blob_info* check_pedge, blob_info* pedge)
{	
	// 1. ��� ���򼱺��� �Ʒ��� ������
	// 2. ��� ���򼱰� ������ �����ϰ�
	// 3. ���� �������� ������ �� �� �»������ �������� ������̰� ������ ���...
	// 4. �հ��̸�.. �ϴܼ� ������ �߰����ش�... �ٷ� ������....

	int px1,py1,px2,py2;

	// ��� ���� üũ....
	while (1) {
		// ���� �����ϰ�...
		if (check_pedge->up_line[0] == -1) break;

		// 1. ��� ���򼱺��� �Ʒ��� ������
		if ( check_pedge->up_line[1] <= pedge->up_line[1]) break;

		// 2. ��� ���򼱰� ������ �����ϰ� 5�� ���� ����....//    
		if ( abs(check_pedge->up_degree - pedge->up_degree) >= 5) break;

		// 3. ���� �������� ������ ���ϴ��� �� ����ڽ� �»������ �������� ������̰� ������ ���...
		// ���ϴ��� ���ϱ�...
		get_cross_point(&px1, &py1, pedge->left_maxline, check_pedge->up_maxline);
		// �»���� ���ϱ�...
		get_cross_point(&px2, &py2, pedge->left_maxline, pedge->up_maxline);
		// ����������� ���ϱ�..
		int height = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
		// ������ ����.....1,500 < x < 10,000
		if (height < 1500 || height > 10000) break;

		// 4. �հ�..... �ϴܼ� ������ �߰����ش�... �ٷ� ������....
		pedge->line_count++;
		pedge->line_type_id += 123; 
		pedge->down_line[0] = check_pedge->up_line[0];
		pedge->down_line[1] = check_pedge->up_line[1];
		pedge->down_line[2] = check_pedge->up_line[2];
		pedge->down_line[3] = check_pedge->up_line[3];
		pedge->down_maxline[0] = check_pedge->up_maxline[0];
		pedge->down_maxline[1] = check_pedge->up_maxline[1];
		pedge->down_maxline[2] = check_pedge->up_maxline[2];
		pedge->down_maxline[3] = check_pedge->up_maxline[3];
		pedge->down_degree = check_pedge->up_degree;

		return true;

	}

	// �ϴ� ���� üũ....
	while (1) {
		// ���� �����ϰ�...
		if (check_pedge->down_line[0] == -1) break;

		// 1. ��� ���򼱺��� �Ʒ��� ������
		if ( check_pedge->down_line[1] <= pedge->up_line[1]) break;

		// 2. ��� ���򼱰� ������ �����ϰ� 1�� ���� ����....
		if ( abs(check_pedge->down_degree - pedge->up_degree) >= 1) break;

		// 3. ���� �������� ������ ���ϴ��� �� ����ڽ� �»������ �������� ������̰� ������ ���...
		// ���ϴ��� ���ϱ�...
		get_cross_point(&px1, &py1, pedge->left_maxline, check_pedge->down_maxline);
		// �»���� ���ϱ�...
		get_cross_point(&px2, &py2, pedge->left_maxline, pedge->up_maxline);
		// ����������� ���ϱ�..
		int height = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
		// ������ ����.....1,500 < x < 10,000
		if (height < 1500 || height > 10000) break;

		// 4. �հ�..... �ϴܼ� ������ �߰����ش�... �ٷ� ������....
		pedge->line_count++;
		pedge->line_type_id += 123; 
		pedge->down_line[0] = check_pedge->down_line[0];
		pedge->down_line[1] = check_pedge->down_line[1];
		pedge->down_line[2] = check_pedge->down_line[2];
		pedge->down_line[3] = check_pedge->down_line[3];
		pedge->down_maxline[0] = check_pedge->down_maxline[0];
		pedge->down_maxline[1] = check_pedge->down_maxline[1];
		pedge->down_maxline[2] = check_pedge->down_maxline[2];
		pedge->down_maxline[3] = check_pedge->down_maxline[3];
		pedge->down_degree = check_pedge->down_degree;

		return true;

	}

	return false;
}




int CCarNumDetect::FindLinesByEdge_HDS_sub(FILE* retf) 
{

	PlateCandidateBoxCount = 0;


	//LabelEdges(); // ������ ��ȣ�� �ű��

	EdgeLineCount = 0;

	blob_info* pedge;

	for(int i = 0; i < m_nEdgeCount; i++) {
		//for(int i = 180; i <= 187; i++) {
		pedge = &m_EdgeTable[i];

		// ���ٴ� ǥ�� �ص�.... ã���� ���� ��..
		pedge->line_count = 0;
		pedge->up_line[0] = -1; 
		pedge->down_line[0] = -1; 
		pedge->left_line[0] = -1; 
		pedge->right_line[0] = -1; 
		pedge->line_type_id = 0; 

		//if ( i != 113) continue;  //debug

		// ����ũ���� ��κ긦 ���ؼ�....
		// HDS-140928
		// ���� ũ�⸸ �����ص� ���غ���... 1���� ���� Ȥ�� ������ �ĺ��� ���ϱ� ����...
		if((pedge->height < EdgeLineMinY || pedge->height > EdgeLineMaxY) && 
			(pedge->width < EdgeLineMinX || pedge->width > EdgeLineMaxX) )
			continue;
		/**
		if(pedge->height < EdgeLineMinY || pedge->height > EdgeLineMaxY)
		continue;
		if(pedge->width < EdgeLineMinX || pedge->width > EdgeLineMaxX)
		continue;
		**/
		// ��κ�� ����, �������� ã�Ƴ���.....

		//	if (i == 176)
		find_blob_hori_line(pedge, true);  // ��� ����...

		//	if (i == 242)
		find_blob_hori_line(pedge, false);  // �ϴ� ����...

		//if (i == 137)
		find_blob_vert_line(pedge, true);  // ���� ������...


		//if (i == 106)
		find_blob_vert_line(pedge, false);  // ������ ������...


		// ���, �ϴ� �� ���ÿ� �ִ°��... ���� ������ ���� �ִ�... ���� ������ ���ֿ� ���� �ϳ��� ���ش�....
		// ���ɼ��� �ִ� Ÿ��.. 4��¥��, 3��¥�� 2��
		if ((pedge->line_type_id == BT_T4) || (pedge->line_type_id == BT_T3_1_2_3) || (pedge->line_type_id == BT_T3_3_4_1)) {

			// ���� �������̰� �ּ�ġ 40(sy) + 40(ey) ���� ���� 50 ���ϸ� ���� �������� ����...
			if (abs(pedge->up_line[1] - pedge->down_line[1]) +  abs(pedge->up_line[3] - pedge->down_line[3]) < 50 ) {
				// �ڽ� �������� 1/2 y ��ġ���� ���� ������ �������.. �Ʒ��� ������ �ϴ����� ��ģ��...
				int mid_y_pos;
				if (pedge->line_type_id == BT_T3_1_2_3) 
					mid_y_pos = pedge->right_line[1] + (int)((pedge->right_line[3] - pedge->right_line[1])/2);
				else
					mid_y_pos = pedge->left_line[1] + (int)((pedge->left_line[3] - pedge->left_line[1])/2);

				// ������� ��ġ��...
				if (pedge->up_line[1] < mid_y_pos) {
					pedge->down_line[0] = -1;
					pedge->line_count--;
					pedge->line_type_id -= 123; 
				}
				// �ϴ����� ��ġ��...
				else {
					pedge->up_line[0] = -1;
					pedge->line_count--;
					pedge->line_type_id -= 1; 
				}
			}
		}

		// ����, �������� ���ÿ� �ִ°��... ���� ������ ���� �ִ�... ���� ������ ���ֿ� ���� �ϳ��� ���ش�....
		// ���ɼ��� �ִ� Ÿ��.. 4��¥��, 3��¥�� 2��
		if ((pedge->line_type_id == BT_T4) || (pedge->line_type_id == BT_T3_2_3_4) || (pedge->line_type_id == BT_T3_4_1_2)) {

			// ������ x��ġ���̰� �ּ�ġ 150(sx) + 150(ex) ���� ���� 200 ���ϸ� ���� �������� ����...30-09-9427 : 197 �ΰ� �߰�...  150���� ����..
			if (abs(pedge->right_line[0] - pedge->left_line[0]) +  abs(pedge->right_line[2] - pedge->left_line[2]) < 150 ) {
				// �ڽ� ������ 1/2 x ��ġ���� ���ʿ� ������ ��������.. �����ʿ� ������ ���������� ��ģ��...
				int mid_x_pos;
				if (pedge->line_type_id == BT_T3_2_3_4) 
					mid_x_pos = pedge->down_line[0] + (int)((pedge->down_line[2] - pedge->down_line[0])/2);
				else
					mid_x_pos = pedge->up_line[0] + (int)((pedge->up_line[2] - pedge->up_line[0])/2);

				// �������� ��ġ��...
				if (pedge->left_line[0] <= mid_x_pos) {
					pedge->right_line[0] = -1;
					pedge->line_count--;
					pedge->line_type_id -= 12; 
				}
				// ���������� ��ġ��...
				else {
					pedge->left_line[0] = -1;
					pedge->line_count--;
					pedge->line_type_id -= 1234; 
				}
			}
		}
	}
	////////////
	// 3�� ¥�� �߿��� ������ ��ĥ���� �ڽ��� ���ļ� ũ�⸦ Ȯ���ؼ� �ĺ����� �ø���....�� �������� ��� 3��¥���߿���....
	for(int i = 0; i < m_nEdgeCount; i++) {
		pedge = &m_EdgeTable[i];

		// ��ܼ��� ���� 3�� ¥��...
		//    14-06-30-11-8225	 113
		if (pedge->line_type_id == BT_T3_2_3_4) {
			// ���� ���̰� ������ ���� �Ѵ�....
			// �ϴ� 2�� ���ؼ� ���� ������� ������ ���Ѵ�
			int px1, py1, px2, py2;
			// ���ϴ�...
			//   �ϴ� ���򼱰� ���ʼ������� ������ ��....
			get_cross_point(&px1, &py1, pedge->down_maxline, pedge->left_maxline);
			// ���ϴ�...
			//   �ϴ� ���򼱰� �����ʼ������� ������ ��....
			get_cross_point(&px2, &py2, pedge->down_maxline, pedge->right_maxline);
			int width_2 = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);

			// ���� ����.....10,000 < x < 100,000
			// ������� ���� 10,000 < x < 100,000
			if ( width_2 > 10000 &&  width_2 < 100000 ) {

				for (int j = 0; j < m_nEdgeCount; j++) {
					if ( j == i ) continue;  // ���� ������Ʈ�̸� ����...

					blob_info* check_pedge = &m_EdgeTable[j];

					// ���, �ϴ� ���� üũ....
					//if ( i == 113 ) // && j == 100) // debug
					if (adjust_box_horiline_fill_up_line(check_pedge, pedge)) break;
				}
			}
		}
		// �ϴܼ��� ���� 3�� ¥��...
		//  
		/** ��ȿ�� �߻�.. �ϴ� ����.... ����ܼ� ä��⸸ ����...
		else if (pedge->line_type_id == BT_T3_4_1_2) {
		// ���� ���̰� ������ ���� �Ѵ�....
		// ��� 2�� ���ؼ� ���� ������� ������ ���Ѵ�
		int px1, py1, px2, py2;
		// �»��...
		//   ��� ���򼱰� ���ʼ������� ������ ��....
		get_cross_point(&px1, &py1, pedge->up_maxline, pedge->left_maxline);
		// ����...
		//   ��ܼ��򼱰� �����ʼ������� ������ ��....
		get_cross_point(&px2, &py2, pedge->up_maxline, pedge->right_maxline);
		int width_2 = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);

		// ���� ����.....10,000 < x < 100,000
		// ������� ���� 10,000 < x < 100,000
		if ( width_2 > 10000 &&  width_2 < 100000 ) {

		for (int j = 0; j < m_nEdgeCount; j++) {
		if ( j == i ) continue;  // ���� ������Ʈ�̸� ����...

		blob_info* check_pedge = &m_EdgeTable[j];

		// ���, �ϴ� ���� üũ....
		//if ( j == 100) // debug
		if (adjust_box_horiline_fill_down_line(check_pedge, pedge)) break;
		}
		}
		}
		**/





	}


	// �ڽ��� ã�ƺ���......
	int selected_count = 0, sv_selected_index;
	float degree_score, sv_degree_score;


	// 1. 4��¥�� ��κ긦 ���� �ɷ���.. �������� �ڽ��� �����Ѵ�.....
	for(int i = 0; i < m_nEdgeCount; i++) {
		blob_info* pedge = &m_EdgeTable[i];
		if (pedge->line_count == 4) {

			//	if ( i != 113) continue;  //debug


			// 4���� ���Ѵ�...
			int px1, py1, px2, py2, px3, py3, px4, py4;
			// �»��...
			//   ��� ���򼱰� ���ʼ������� ������ ��....
			get_cross_point(&px1, &py1, pedge->up_maxline, pedge->left_maxline);

			// ����...
			//   ��� ���򼱰� �����ʼ������� ������ ��....
			get_cross_point(&px2, &py2, pedge->up_maxline, pedge->right_maxline);

			// ���ϴ�...
			//   �ϴ� ���򼱰� ���ʼ������� ������ ��....
			get_cross_point(&px3, &py3, pedge->down_maxline, pedge->left_maxline);


			// ���ϴ�...
			//   �ϴ� ���򼱰� �����ʼ������� ������ ��....
			get_cross_point(&px4, &py4, pedge->down_maxline, pedge->right_maxline);

			// �������� ����� üũ�ؼ�... ��� ����.....
			//int width = abs(px2 - px1) + 1;
			//int height = abs(py3 - py1) + 1;

			// ���� ������ ������...
			int width_2 = (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
			int height_2 = (px3 - px1) * (px3 - px1) + (py3 - py1) * (py3 - py1);
			float rate_2 = width_2 / (height_2 + 0.0000001);




			PlateCandidateBox[PlateCandidateBoxCount].Box[0] = px1;
			PlateCandidateBox[PlateCandidateBoxCount].Box[1] = py1;
			PlateCandidateBox[PlateCandidateBoxCount].Box[2] = px2;
			PlateCandidateBox[PlateCandidateBoxCount].Box[3] = py2;
			PlateCandidateBox[PlateCandidateBoxCount].Box[4] = px3;
			PlateCandidateBox[PlateCandidateBoxCount].Box[5] = py3;
			PlateCandidateBox[PlateCandidateBoxCount].Box[6] = px4;
			PlateCandidateBox[PlateCandidateBoxCount].Box[7] = py4;
			PlateCandidateBox[PlateCandidateBoxCount].width = width_2;
			PlateCandidateBox[PlateCandidateBoxCount].height = height_2;
			PlateCandidateBox[PlateCandidateBoxCount].rate =  rate_2; // width / (height + 0.000001);
			PlateCandidateBox[PlateCandidateBoxCount].hori_degree = pedge->up_degree; 
			PlateCandidateBox[PlateCandidateBoxCount].vert_degree = pedge->left_degree; 

			PlateCandidateBox[PlateCandidateBoxCount].EdgeTable_index = i;
			PlateCandidateBox[PlateCandidateBoxCount].selected = false;
			PlateCandidateBoxCount++;

			// 1. ������ ����.....1,500 < x < 10,000
			if (height_2 < 1500 || height_2 > 10000)  continue;

			// 2. ���� ����.....10,000 < x < 100,000
			if (width_2 < 10000 || width_2 > 100000)  continue;

			// 3. rate ����... 1.65 < x < 7  or 9 < x < 39
			//
			if (!((rate_2 > 1.65 && rate_2 < 7) || (rate_2 > 9 && rate_2 < 39))) continue;

			// ���� 90������ ���̰��� ������ 180�� ���� ���̰��� ���ؼ� ���������� ���Ѵ�.
			degree_score = abs(pedge->up_degree - 90) + abs(pedge->left_degree - 180);

			// �������� : ���� 15�� �̳� ������ 15�� �̳� �հ� 30�� �̳����� �Ѵ�.....11-9964 24.5��
			if (degree_score >= 30) continue;



			// ó�� ���� �ڽ��̸�... ���� ���� ���� ������. ���
			/** ���������� ���� �Ű�µ�.. �߾ȵ� .. 11-4040
			if (selected_count == 0) {
			min_degree_score = degree_score;
			PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
			sv_selected_index = PlateCandidateBoxCount-1;
			selected_count = 1;
			}
			// ���� �ڽ� ���������� ���ؼ� �� ���̵��� ������ ���粬�� ��ü...
			else {
			if (min_degree_score > degree_score) {
			PlateCandidateBox[sv_selected_index].selected = false;
			min_degree_score = degree_score;
			PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
			sv_selected_index = PlateCandidateBoxCount-1;
			}
			}
			**/

			// ��ȣ��κ갡 �����ϸ� �ĺ� �ڽ��� �ݵ�� ��κ��� 1���� �����ϰ� �־�� �Ѵ�.....
			if  ( found_arr[0] != -1) {

				int j;
				for (j = 0; j < 4; j++) {//������

					if (PlateCandidateBox[PlateCandidateBoxCount-1].Box[0] <= m_EdgeTable[found_arr[j]].sx && 
						PlateCandidateBox[PlateCandidateBoxCount-1].Box[1] <= m_EdgeTable[found_arr[j]].sy && 
						PlateCandidateBox[PlateCandidateBoxCount-1].Box[6] >= m_EdgeTable[found_arr[j]].ex && 
						PlateCandidateBox[PlateCandidateBoxCount-1].Box[7] >= m_EdgeTable[found_arr[j]].ey )
						break;
				}

				if (j < 4) { // �����ϹǷ�...�̰� Ʋ�����ٰ� ����... �����ϰ� ������....
					PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
					sv_selected_index = PlateCandidateBoxCount-1;
					selected_count = 1;
					break;
				}
			}


			// ��ȣ��κ갡 ���ٸ�....�׳� ���� �ϴ��ΰŸ� 1������ �Űܺ���...
			else {


				if (selected_count == 0) {
					sv_selected_index = PlateCandidateBoxCount-1;
					sv_degree_score = degree_score; 
					PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
					selected_count = 1;
				}
				// py3 �� ���� ������....�� ���̰� ���� �ȳ��� (5�ȼ��̳�) ���������� ��ȣ�Ѱɷ� ���� 30-09-8625
				// �ƴϴ�..  ��ȣ��κ갡 ���ԵǾ� ������ �װ� �ְ�....
				else {
					int diff_pixels = abs(PlateCandidateBox[PlateCandidateBoxCount-1].Box[5] -  PlateCandidateBox[sv_selected_index].Box[5]);
					if (diff_pixels <= 5) {
						if (sv_degree_score > degree_score) { 
							// ������ �� ��ȣ�� ���粨�� ��ü...
							PlateCandidateBox[sv_selected_index].selected = false;
							PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
							sv_selected_index = PlateCandidateBoxCount-1;
							sv_degree_score = degree_score; 
						}
					}
					else {
						if (PlateCandidateBox[PlateCandidateBoxCount-1].Box[5] >  PlateCandidateBox[sv_selected_index].Box[5]) {
							// 5�ȼ� �̻� �Ʒ��� �ִ°�....
							PlateCandidateBox[sv_selected_index].selected = false;
							PlateCandidateBox[PlateCandidateBoxCount-1].selected = true;
							sv_selected_index = PlateCandidateBoxCount-1;
							sv_degree_score = degree_score; 
						}
					}
				}
			}
		}
	}

	//.. ���õ� �ڽ��� �ִ°�........  
	if (selected_count == 1) {

		Found_PlateCandidateBoxIndex = sv_selected_index;
		return 1;

	}

	Found_PlateCandidateBoxIndex = -1;
	return -1; // ã�� ����....

}


int CCarNumDetect::FindLinesByEdge_HDS(FILE* retf) 
{

	// ���� �����ϴ� ��κ� ������ ���صд�....
	int ret = GetPlateRectFromEdges(retf, false, true);
	if  ( ret != 1) // ã������....
		found_arr[0] = -1;  // ����ǥ��....
	// ã���� found_arr[0,1,2,3] �� ����Ǿ� ����...

	ret = FindLinesByEdge_HDS_sub(retf);
	if (ret != 1) {
		//������...����þ����� 1����....  
		BlurImage( 0.1); 
		Canny( blur_src, detected_edges, 60, 180, 3 );
		dte_m = detected_edges;

		// ��Ŀ�ú� ���̺�...
		LabelEdges(); // ������ ��ȣ�� �ű��

		// ���� �����ϴ� ��κ� ������ �ٽ� ���صд�....����þ� ���� �ѹ��� ���ϴ°� �� ������...30-10-1611
		ret = GetPlateRectFromEdges(retf, false, true);
		if  ( ret != 1) // ã������....
			found_arr[0] = -1;  // ����ǥ��....

		ret = FindLinesByEdge_HDS_sub(retf);
	}

	if (ret != 1) {
		//�Ǵٽ� ������ ����þȿ� ĳ�ϸ� 5, 15�� 1����....  30-09-1280(7290)
		BlurImage( 0.1); 
		Canny( blur_src, detected_edges, 5, 15, 3 );
		dte_m = detected_edges;

		// ��Ŀ�ú� ���̺�...
		LabelEdges(); // ������ ��ȣ�� �ű��

		// ���� �����ϴ� ��κ� ������ �ٽ� ���صд�....����þ� ���� �ѹ��� ���ϴ°� �� ������...30-10-1611
		ret = GetPlateRectFromEdges(retf, false, true);
		if  ( ret != 1) // ã������....
			found_arr[0] = -1;  // ����ǥ��....

		ret = FindLinesByEdge_HDS_sub(retf);
	}

	int ret2;
	//�׷��� ������  ��κ� ���� ������� ���Ѵ�...
	//if (ret != 1) {  // 30-08-3068 : �ڽ��� ����������.. �຿ �������� �����Ƿ�.. ��ȣ��κ굵 ������ ã�Ƶд�......

	// ��������� ��ȣ��κ갡 ���� ���.. �ٽ� ���� �ٲپ ã�ƺ���...30-09-8833
	if (found_arr[0] = -1) {

		//���� ����þ� 1.5��....
		BlurImage( 1.5); 
		Canny( blur_src, detected_edges, 60, 180, 3 );
		dte_m = detected_edges;

		// ��Ŀ�ú� ���̺�...
		LabelEdges(); // ������ ��ȣ�� �ű��
		ret2 = GetPlateRectFromEdges(retf, false, true);
		if  ( ret2 != 1) {

			//���� ����þ� 0.1��....
			BlurImage( 0.1); 
			Canny( blur_src, detected_edges, 60, 180, 3 );
			dte_m = detected_edges;

			// ��Ŀ�ú� ���̺�...
			LabelEdges(); // ������ ��ȣ�� �ű��
			ret2 = GetPlateRectFromEdges(retf, false, true);

		}
		// ���� ����....
		if  ( ret2 != 1) {
			found_arr[0] = -1;  // ����ǥ��....
		}
	}
	//}

	// ���������� ã�⿡�� �����ϸ�.... ��ȣ��κ� ã�� �����ص� �������� �����غ���....
	if (ret == -1  && ret2 == 1) 
		ret = 1;

	return ret;

}




void CCarNumDetect::DrawEdgeLines_HDS(Mat& dst)
{
	CString str;
	OutputDebugString(L"------start---- \r\n");



	for (int i = 0; i < PlateCandidateBoxCount; i++) {
		int green = 255;
		int red = 255;

		if (PlateCandidateBox[i].selected) red = 0;  // �������... �ĺ��� �����?
		//if (1)  continue;
		//if (!PlateCandidateBox[i].selected)  continue;

		cv::line(dst, 
			cv::Point(PlateCandidateBox[i].Box[0], PlateCandidateBox[i].Box[1]),
			cv::Point(PlateCandidateBox[i].Box[2], PlateCandidateBox[i].Box[3]),
			cv::Scalar(0,green,red), 2);
		cv::line(dst, 
			cv::Point(PlateCandidateBox[i].Box[2], PlateCandidateBox[i].Box[3]),
			cv::Point(PlateCandidateBox[i].Box[6], PlateCandidateBox[i].Box[7]),
			cv::Scalar(0,green,red), 2);
		cv::line(dst, 
			cv::Point(PlateCandidateBox[i].Box[4], PlateCandidateBox[i].Box[5]),
			cv::Point(PlateCandidateBox[i].Box[6], PlateCandidateBox[i].Box[7]),
			cv::Scalar(0,green,red), 2);
		cv::line(dst, 
			cv::Point(PlateCandidateBox[i].Box[0], PlateCandidateBox[i].Box[1]),
			cv::Point(PlateCandidateBox[i].Box[4], PlateCandidateBox[i].Box[5]),
			cv::Scalar(0,green,red), 2);

		str.Format(L"(flag=%d-->ID=%d:, width=%d, height=%d, rate=%f)(p1=%d,%d,p2=%d,%d,p3=%d,%d,p4=%d,%d)(h_dg=%f,v_dg=%f \r\n", PlateCandidateBox[i].selected, 
			PlateCandidateBox[i].EdgeTable_index, PlateCandidateBox[i].width, PlateCandidateBox[i].height, 
			PlateCandidateBox[i].rate, 
			PlateCandidateBox[i].Box[0], PlateCandidateBox[i].Box[1],
			PlateCandidateBox[i].Box[2], PlateCandidateBox[i].Box[3],
			PlateCandidateBox[i].Box[4], PlateCandidateBox[i].Box[5],
			PlateCandidateBox[i].Box[6], PlateCandidateBox[i].Box[7],		   
			PlateCandidateBox[i].hori_degree,
			PlateCandidateBox[i].vert_degree
			);


		OutputDebugString(str);



	}

	for (int i = 0; i < m_nEdgeCount; i++) {
		// HDS-140928
		//if (m_EdgeTable[i].line_count  != 4 && m_EdgeTable[i].line_count != 3) continue;
		//if ( i != 106) continue;

		if (m_EdgeTable[i].line_count  != 4) continue; // 4 �� ¥���� �׷�����.... 


		if (m_EdgeTable[i].up_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].up_line[0], m_EdgeTable[i].up_line[1]),
				cv::Point(m_EdgeTable[i].up_line[2], m_EdgeTable[i].up_line[3]),
				cv::Scalar(0,0,255), 1);
		}
		if (m_EdgeTable[i].down_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].down_line[0], m_EdgeTable[i].down_line[1]),
				cv::Point(m_EdgeTable[i].down_line[2], m_EdgeTable[i].down_line[3]),
				cv::Scalar(0,0,255), 1);
		}
		if (m_EdgeTable[i].left_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].left_line[0], m_EdgeTable[i].left_line[1]),
				cv::Point(m_EdgeTable[i].left_line[2], m_EdgeTable[i].left_line[3]),
				cv::Scalar(0,0,255), 1);
		}
		if (m_EdgeTable[i].right_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].right_line[0], m_EdgeTable[i].right_line[1]),
				cv::Point(m_EdgeTable[i].right_line[2], m_EdgeTable[i].right_line[3]),
				cv::Scalar(0,0,255), 1);
		}

		str.Format(L"(flag=%d-->ID=%d:, up[0]=%d, down[0]=%d, left[0]=%d, right[0] = %d) \r\n", i, 
			m_EdgeTable[i].up_line[0], m_EdgeTable[i].down_line[0], m_EdgeTable[i].left_line[0], m_EdgeTable[i].right_line[0]);

		OutputDebugString(str);

	}

	//debug  max line �׸���...

	for (int i = 106; i <= -1; i++) {
		if (m_EdgeTable[i].up_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].up_maxline[0], m_EdgeTable[i].up_maxline[1]),
				cv::Point(m_EdgeTable[i].up_maxline[2], m_EdgeTable[i].up_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].down_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].down_maxline[0], m_EdgeTable[i].down_maxline[1]),
				cv::Point(m_EdgeTable[i].down_maxline[2], m_EdgeTable[i].down_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].left_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].left_maxline[0], m_EdgeTable[i].left_maxline[1]),
				cv::Point(m_EdgeTable[i].left_maxline[2], m_EdgeTable[i].left_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].right_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].right_maxline[0], m_EdgeTable[i].right_maxline[1]),
				cv::Point(m_EdgeTable[i].right_maxline[2], m_EdgeTable[i].right_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
	}


	//debug,... ��ȣ��κ� ���� �׸���...

	if  ( found_arr[0] != -1) {

		// �ĺ���...����..
		for( int i = 0; i < sv_found_count; i++ ) {
			for( int j = 0; j < 4; j++ ) {
				// �� ���̺� ǥ��
				cv::rectangle(dst, 
					cv::Point(m_EdgeTable[found_arr[j]].sx, m_EdgeTable[found_arr[j]].sy),
					cv::Point(m_EdgeTable[found_arr[j]].ex, m_EdgeTable[found_arr[j]].ey),
					cv::Scalar(255,0,0),2);

			}
		}

		// ���� �ҿ켱�ĺ�...������,,
		for( int i = 0; i < 4; i++ ) {
			// �� ���̺� ǥ��
			cv::rectangle(dst, 
				cv::Point(m_EdgeTable[found_arr[i]].sx, m_EdgeTable[found_arr[i]].sy),
				cv::Point(m_EdgeTable[found_arr[i]].ex, m_EdgeTable[found_arr[i]].ey),
				cv::Scalar(0,0,255), 2);

			//cout << "pos:" <<m_EdgeTable[tmp_blob[i].index].sx << "," << m_EdgeTable[tmp_blob[i].index].sy << "," <<  m_EdgeTable[tmp_blob[i].index].width << "." <<  m_EdgeTable[tmp_blob[i].index].height << endl;

			//CString str;
			//str.Format(L"(ids=%d: %d,%d,%d,%d,ex=%d,ey=%d) \r\n", found_arr[i], m_EdgeTable[found_arr[i]].width, m_EdgeTable[found_arr[i]].height, m_EdgeTable[found_arr[i]].sx, m_EdgeTable[found_arr[i]].sy, m_EdgeTable[found_arr[i]].ex, m_EdgeTable[found_arr[i]].ey);
			//OutputDebugString(str);
		}

	}




#if 0	

	for (int i = 0; i <= -1; i++) {
		if (m_EdgeTable[i].up_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].up_line[0], m_EdgeTable[i].up_line[1]),
				cv::Point(m_EdgeTable[i].up_line[2], m_EdgeTable[i].up_line[3]),
				cv::Scalar(0,0,255), 2);
		}
		if (m_EdgeTable[i].down_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].down_line[0], m_EdgeTable[i].down_line[1]),
				cv::Point(m_EdgeTable[i].down_line[2], m_EdgeTable[i].down_line[3]),
				cv::Scalar(0,0,255), 2);
		}
		if (m_EdgeTable[i].left_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].left_line[0], m_EdgeTable[i].left_line[1]),
				cv::Point(m_EdgeTable[i].left_line[2], m_EdgeTable[i].left_line[3]),
				cv::Scalar(0,0,255), 2);
		}
		if (m_EdgeTable[i].right_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].right_line[0], m_EdgeTable[i].right_line[1]),
				cv::Point(m_EdgeTable[i].right_line[2], m_EdgeTable[i].right_line[3]),
				cv::Scalar(0,0,255), 2);
		}
	}

	//for (int i = 0; i <= m_nBlobIndex; i++) {
	for (int i = 0; i <= -1; i++) {
		if (m_EdgeTable[i].up_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].up_maxline[0], m_EdgeTable[i].up_maxline[1]),
				cv::Point(m_EdgeTable[i].up_maxline[2], m_EdgeTable[i].up_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].down_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].down_maxline[0], m_EdgeTable[i].down_maxline[1]),
				cv::Point(m_EdgeTable[i].down_maxline[2], m_EdgeTable[i].down_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].left_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].left_maxline[0], m_EdgeTable[i].left_maxline[1]),
				cv::Point(m_EdgeTable[i].left_maxline[2], m_EdgeTable[i].left_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
		if (m_EdgeTable[i].right_line[0] != -1) {
			cv::line(dst, 
				cv::Point(m_EdgeTable[i].right_maxline[0], m_EdgeTable[i].right_maxline[1]),
				cv::Point(m_EdgeTable[i].right_maxline[2], m_EdgeTable[i].right_maxline[3]),
				cv::Scalar(255,0,0), 1);
		}
	}
#endif 
	//DEBUG..........
	if(0) {
		// ���� ã�� ���� �׸��� �κ��ε� �ʹ� ���� �׷����� ����ϹǷ� �ϴ� ���Ƶд�. dschae20140908
		//for(int i = 0; i < m_nEdgeCount; i++) {
		for (int i = 0; i <= -1; i++) {
			//if (m_EdgeTable[i].sx >= 521 && m_EdgeTable[i].sx <= 531 &&
			//    m_EdgeTable[i].sy >= 377 && m_EdgeTable[i].sy <= 388 ) {
			cv::rectangle(dst, 
				cv::Point(m_EdgeTable[i].sx, m_EdgeTable[i].sy),
				cv::Point(m_EdgeTable[i].ex, m_EdgeTable[i].ey),
				cv::Scalar(0,255,0));
			//}

		}
	}


}


void CCarNumDetect::GetTestedEdgeText(CString &txt)
{
	CString s;
	for(int i = 0; i < TestedEdgeCount; i++) {
		s.Format(L"%d,", TestedEdges[i]);
		txt += s;
	}
}

cv::Mat inputImage;
cv::Mat fImage;
Mat magI;
Mat m;
cv::Mat inverseTransform;
Mat diff;
cv::Mat finalImage;
cv::Mat fourierTransform;

void EraseSomeFrequency(int rows, int cols, int step)
{
	for(int y = 0; y < rows; y++) {
		int xs = inv_fft_circle[y][0];
		int xe = inv_fft_circle[y][1];
		float *fp = (float*)(fourierTransform.data + y * step);
		for(int x = xs; x < xe; x++) {
			fp[x*2] = 0; // real part
			fp[x*2+1] = 0; // imaginary part
		}
	}
/*
	for(int y = rows/4; y < (rows*3)/4; y++) {
		float *fp = (float*)(fourierTransform.data + y * step);
		for(int x = 0; x < cols; x++) {
			fp[x*2] = 0; // real part
			fp[x*2+1] = 0; // imaginary part
		}
	}
	for(int y = 0; y < rows/4; y++) {
		float *fp = (float*)(fourierTransform.data + y * step);
		for(int x = cols/4; x < (cols*3)/4; x++) {
			fp[x*2] = 0; // real part
			fp[x*2+1] = 0; // imaginary part
		}
	}
	for(int y = (rows*3)/4; y < rows; y++) {
		float *fp = (float*)(fourierTransform.data + y * step);
		for(int x = cols/4; x < (cols*3)/4; x++) {
			fp[x*2] = 0; // real part
			fp[x*2+1] = 0; // imaginary part
		}
	}
*/
}

void CCarNumDetect::ApplyFFT()
{
#if 1
	src.copyTo(inputImage);

	// Go float
	inputImage.convertTo(fImage, CV_32F);

	// FFT
	cv::dft(fImage, fourierTransform, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);

	//	doSomethingWithTheSpectrum();
	int rows = fourierTransform.rows;
	int cols = fourierTransform.cols;
	int step = fourierTransform.step;
	EraseSomeFrequency(rows, cols, step);

	// Some processing
	magI.create(fImage.rows, fImage.cols, CV_32F);
	double t;
	double mx = -1000000000;
	double mn = -mx;
	for(int y = 0; y < fourierTransform.rows; y++) {
		float *dp = (float*)(fourierTransform.data + y * fourierTransform.step);
		float *mp = (float*)(magI.data + y * magI.step);
		for(int x = 0; x < fourierTransform.cols; x++) {
			float rpart = dp[x*2];
			float ipart = dp[x*2+1];
			double t = log10(1.0 + sqrt(rpart*rpart + ipart*ipart));
			mp[x] = t;
			if(mx < t) mx = t;
			if(mn > t) mn = t;
		}
	}

	mx -= mn;
	for(int y = 0; y < magI.rows; y++) {
		float *mp = (float*)(magI.data + y * magI.step);
		for(int x = 0; x < magI.cols; x++) {
			float t = mp[x];
			mp[x] = ((t - mn) / mx) * 255;
		}
	}
	magI.convertTo(m, CV_8U, 255);
	imshow("mag", m);

	// IFFT
	cv::dft(fourierTransform, inverseTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

	// Back to 8-bits
	inverseTransform.convertTo(finalImage, CV_8U);
	diff = finalImage - inputImage;
	int diffmax = 0;
	for(int y = 0; y < diff.rows; y++) {
		unsigned char* cp = diff.data + y * diff.step;
		for(int x = 0; x < diff.cols; x++) {
			if(cp[x] > diffmax) diffmax = cp[x];
		}
	}
	for(int y = 0; y < diff.rows; y++) {
		unsigned char* cp = diff.data + y * diff.step;
		for(int x = 0; x < diff.cols; x++) {
			cp[x] = (cp[x]*255) / diffmax;
		}
	}
	imshow("diff", diff);
	finalImage.copyTo(src);
#else
	Mat I;
	src.copyTo(I);

    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize( I.rows );
    int n = getOptimalDFTSize( I.cols ); // on the border add zero values
    copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    dft(complexI, complexI);            // this way the result may fit in the source matrix

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
    split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    Mat magI = planes[0];

    magI += Scalar::all(1);                    // switch to logarithmic scale
    log(magI, magI);

    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    int cx = magI.cols/2;
    int cy = magI.rows/2;

    Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

    Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
                                            // viewable image form (float between values 0 and 1).

    imshow("Input Image"       , I   );    // Show the result
    imshow("spectrum magnitude", magI);
#endif
}

//