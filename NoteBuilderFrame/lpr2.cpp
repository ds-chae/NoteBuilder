// lpr2.cpp : implementation of the CICUMANView class
//
#include "stdafx.h"
#include "mainfrm.h"
#include "NoteBuilderFrame.h"

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "ImageTestDlg.h"
#include "BuildLog.h"
#include "ScrollWnd.h"

#include "Setting.h"

#include <stdio.h>
#include <math.h>

struct timeval start_time;
struct timeval end_time;
double elapsed_time;
float fps;
double total_fps = 0.0,average_fps = 0.0;
long cnt=1;
int cx,cy,radius,ccnt;

using namespace std;

IplImage* img;
IplImage* GrayImg=0;
IplImage* Edge=0;
IplImage* HSVImg;
IplImage* HImg = 0;
IplImage* SImg = 0;
IplImage* VImg = 0;

int key;
int i,k;
double dp = 1;
double min_dist = 300;
double t,s;
double angle12,angle23,angle34,angle41;
int px1,py1,px2,py2,sx1,sy1,sx2,sy2,sx3,sy3,sx4,sy4,spx1,spy1,spx2,spy2;
double len12,len23,len34,len41;
double longlen;
double abase, aheight;
char buffer[25];
CvFont font;
CvSeq* result;
CvPoint* pt1;
CvPoint* pt2;
CvPoint* pt3;
CvPoint* pt4;

extern Mat croppedImage, cropped_gray;
extern Mat cropped_edges;

/*
double pow(double x, int n)
{
	double r = 1.;
	while(n) {
		r *= x;
		n--;
	}

	return r;
}
*/

#ifndef _pow
#define	_pow(x,n) ((float)(x)*(float)(x))
#endif

extern int edgeThresh ;//= 1;
extern int kernel_size ;//= 3;
extern int canny_th_min[];
extern int canny_th_max[];
extern int canny_th_cnt;

void lpr2()
{
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,0.5,0.5);

	cvtColor( croppedImage, cropped_gray, CV_BGR2GRAY );
	/// Reduce noise with a kernel 3x3
	blur( cropped_gray, cropped_edges, Size(3,3) );

	/// Canny detector
	Canny( cropped_edges, cropped_edges, canny_th_min[0], canny_th_max[0], kernel_size );

	img = &IplImage(croppedImage);
	//Edge = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	//cvCvtColor(img, GrayImg, CV_BGR2GRAY);
	//GrayImg = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	GrayImg = &IplImage(cropped_gray);

//	cvSmooth(GrayImg,GrayImg,CV_GAUSSIAN,9,9,2,2);
	cvSmooth(GrayImg,GrayImg, CV_BLUR, 3, 3, 1, 1);
//	cvThreshold(GrayImg,GrayImg,200,255,CV_THRESH_BINARY);
//	cvCanny(GrayImg, Edge,25,200,3);
	Edge = &IplImage(cropped_edges);
	imshow("cropped_edges", cropped_edges);
	CvMemStorage* storage = cvCreateMemStorage(0);
	int channels = cropped_edges.channels();
	CvSeq* Contours;
	cvFindContours(Edge, storage, &Contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	cvShowImage( "Edge-00", Edge );
	while(Contours)
	{
		result = cvApproxPoly(Contours,sizeof(CvContour),storage,CV_POLY_APPROX_DP,cvContourPerimeter(Contours)*0.02,0);
		float farea = fabs(cvContourArea(result,CV_WHOLE_SEQ));
		if(result->total == 4 && farea > 100 && cvCheckContourConvexity(result))
		{
			pt1 = (CvPoint*)cvGetSeqElem(result,0);
			pt2 = (CvPoint*)cvGetSeqElem(result,1);
			pt3 = (CvPoint*)cvGetSeqElem(result,2);
			pt4 = (CvPoint*)cvGetSeqElem(result,3);
			int x1 = pt1->x;
			int y1 = pt1->y;
			int x2 = pt2->x;
			int y2 = pt2->y;
			int x3 = pt3->x;
			int y3 = pt3->y;
			int x4 = pt4->x;
			int y4 = pt4->y;

			cvCircle(img,cvPoint(x1,y1),3,CV_RGB(255,255,255),3,8,0);
			cvCircle(img,cvPoint(x2,y2),3,CV_RGB(255,255,255),3,8,0);
			cvCircle(img,cvPoint(x3,y3),3,CV_RGB(255,255,255),3,8,0);
			cvCircle(img,cvPoint(x4,y4),3,CV_RGB(255,255,255),3,8,0);

			cvLine(img,cvPoint(x1,y1),cvPoint(x2,y2),CV_RGB(0,0,255),2,8,0);
			cvLine(img,cvPoint(x2,y2),cvPoint(x3,y3),CV_RGB(0,0,255),2,8,0);
			cvLine(img,cvPoint(x3,y3),cvPoint(x4,y4),CV_RGB(0,0,255),2,8,0);
			cvLine(img,cvPoint(x4,y4),cvPoint(x1,y1),CV_RGB(0,0,255),2,8,0);

			//calculate angle
				//calculate length of two point
			len12 = sqrt(_pow(x1-x2,2) + _pow(y1-y2,2));
			len23 = sqrt(_pow(x2-x3,2) + _pow(y2-y3,2));
			len34 = sqrt(_pow(x3-x4,2) + _pow(y3-y4,2));
			len41 = sqrt(_pow(x4-x1,2) + _pow(y4-y1,2));

			//sort len
			longlen = len12;
			px1 = x1;
			py1 = y1;
			px2 = x2;
			py2 = y2;

		sx1 = x2;
			sy1 = y2;
			sx2 = x3;
			sy2 = y3;
			sx3 = x4;
			sy3 = y4;
			sx4 = x1;
			sy4 = y1;

			if(len23 > longlen)
			{
				longlen = len23;
				px1 = x2;
				py1 = y2;
				px2 = x3;
				py2 = y3;
				sx1 = x3;
				sy1 = y3;
				sx2 = x4;
				sy2 = y4;
				sx3 = x1;
				sy3 = y1;
				sx4 = x2;
				sy4 = y2;
			}
			if(len34 > longlen)
			{
				longlen = len34;
				px1 = x3;
				py1 = y3;
				px2 = x4;
				py2 = y4;
				sx1 = x4;
				sy1 = y4;
				sx2 = x1;
				sy2 = y1;
				sx3 = x2;
				sy3 = y2;
				sx4 = x3;
				sy4 = y3;
			}

			if(len41 > longlen)
			{
				longlen = len41;
				px1 = x1;
				py1 = y1;
				px2 = x4;
				py2 = y4;

		sx1 = x1;
				sy1 = y1;
				sx2 = x2;
				sy2 = y2;
				sx3 = x3;
				sy3 = y3;
				sx4 = x4;
				sy4 = y4;
			}

			spx1 = (sx1 + sx2) / 2;
			spy1 = (sy1 + sy2) / 2;
			spx2 = (sx3 + sx4) / 2;
			spy2 = (sy3 + sy4) / 2;
			cvLine(img,cvPoint(spx1,spy1),cvPoint(spx2,spy2),CV_RGB(0,255,0),2,8,0);
			px1 = spx1;
			py1 = spy1;
			px2 = spx2;
			py2 = spy2;
			//find min x -> point2
			//point 1 - point2
			abase = px2 - px1;
			aheight = py2 - py1;
			angle23 = atan(aheight/abase);
			angle12 = angle23*180.0/3.141592;
			sprintf_s(buffer,sizeof(buffer), "ang= %02.2f",angle12);
			cvPutText(img,buffer,cvPoint(abs((px2+px1)/2),abs((py2+py1)/2)),&font,CV_RGB(255,0,0));
		}
		Contours = Contours->h_next;
	}
	cvNamedWindow( "PreviewImage", 1 );
	cvShowImage( "PreviewImage", img );
	cvMoveWindow("PreviewImage",200,100);
	cvNamedWindow( "GrayImage", 1 );
	cvShowImage( "GrayImage", GrayImg );
	cvMoveWindow("GrayImage",850,600);
	cvNamedWindow( "Edge", 1 );
	cvShowImage( "Edge", Edge );
	cvMoveWindow("Edge",850,100);

	elapsed_time = (double)(end_time.tv_sec) + (double)(end_time.tv_usec)/1000000.0 - (double)(start_time.tv_sec) - (double)(start_time.tv_usec)/1000000.0;
	fps = 1.0 / elapsed_time;
	total_fps += fps;
	average_fps = total_fps / (double)(cnt);
	elapsed_time *= 1000.0;
	printf("Elapsed T = %2.3f ms, Frame = %2.3f (fps), Avrg Frame Rate = %2.3f count = %d\n",elapsed_time,fps,average_fps,result->total);
	//Circles->total = 0;
	cvReleaseImage(&GrayImg);
	cvReleaseImage(&Edge);
	cvReleaseMemStorage(&storage);
} //end of while(1)



