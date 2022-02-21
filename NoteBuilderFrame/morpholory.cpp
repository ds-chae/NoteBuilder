#include "stdafx.h"
#include "opencv2/opencv.hpp"
//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "opencv_use.h"

using namespace cv;


typedef unsigned char BYTE;

void monopoly(BYTE *pImage, int step, int width, int height, BYTE *pEdge);

void monopoly(BYTE *pImage, int step, int width, int height, BYTE *pEdge)
{
	// 최대값으로 치환하기
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			// 5*5 공간에서 제일 큰 거 구하기
			int x1 = x - 2;
			int x2 = x + 2;
			int y1 = y - 2;
			int y2 = y + 2;

			// 경계선 고려한다
			if(x1 < 0) x1 = 0;
			if(x2 > width) x2 = width;
			if(y1 < 0) y1 = 0;
			if(y2 > height) y2 = height;

			// 5 5 공간에서 뺑뺑이를 돈다
			int tempmax = 0;
			for(int cx = x1; cx < x2; cx++ ) {
				for(int cy = y1; cy < y2; cy++) {
					int v = pImage[y * width + x]; // 원본 값 가져오기
					if(v > tempmax)
						tempmax = v;
				}
			}

			// 타겟에 골라온 값 넣기
			pEdge[y * width + x] = tempmax;
		}
	}
}

// 호출 사례
void monopoly_caller()
{
	Mat blur_src;  // Original Image...
	Mat detected_edges;  // Edge Image....
	Mat new_edges;
	// blur_src에 원본이 들어있다고 가정한다.
	// pedge는 임시 이미지 저장소
	unsigned char* pedge = (unsigned char*)malloc(blur_src.step * blur_src.rows);

	monopoly(blur_src.data, blur_src.step, blur_src.cols, blur_src.rows, pedge);
	// new_edges가 새 이미지가 들어갈 cv::Mat 구조체
	new_edges.create(blur_src.rows, blur_src.cols, CV_8U);
	// 임시 이미지에 들어있는 것을 new_edges에 옮긴다.
	for(int y = 0; y < blur_src.rows; y++) {
		unsigned char *ucp = new_edges.data + blur_src.step * y;
		unsigned char *src = pedge + blur_src.cols * y;
		for(int x = 0; x < blur_src.cols; x++)
			*ucp++ = *src++;
	}
	free(pedge);

	// 화면에 보여준다.
	imshow("new edge", new_edges);
}
