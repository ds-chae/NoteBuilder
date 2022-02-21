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
	// �ִ밪���� ġȯ�ϱ�
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			// 5*5 �������� ���� ū �� ���ϱ�
			int x1 = x - 2;
			int x2 = x + 2;
			int y1 = y - 2;
			int y2 = y + 2;

			// ��輱 ����Ѵ�
			if(x1 < 0) x1 = 0;
			if(x2 > width) x2 = width;
			if(y1 < 0) y1 = 0;
			if(y2 > height) y2 = height;

			// 5 5 �������� �����̸� ����
			int tempmax = 0;
			for(int cx = x1; cx < x2; cx++ ) {
				for(int cy = y1; cy < y2; cy++) {
					int v = pImage[y * width + x]; // ���� �� ��������
					if(v > tempmax)
						tempmax = v;
				}
			}

			// Ÿ�ٿ� ���� �� �ֱ�
			pEdge[y * width + x] = tempmax;
		}
	}
}

// ȣ�� ���
void monopoly_caller()
{
	Mat blur_src;  // Original Image...
	Mat detected_edges;  // Edge Image....
	Mat new_edges;
	// blur_src�� ������ ����ִٰ� �����Ѵ�.
	// pedge�� �ӽ� �̹��� �����
	unsigned char* pedge = (unsigned char*)malloc(blur_src.step * blur_src.rows);

	monopoly(blur_src.data, blur_src.step, blur_src.cols, blur_src.rows, pedge);
	// new_edges�� �� �̹����� �� cv::Mat ����ü
	new_edges.create(blur_src.rows, blur_src.cols, CV_8U);
	// �ӽ� �̹����� ����ִ� ���� new_edges�� �ű��.
	for(int y = 0; y < blur_src.rows; y++) {
		unsigned char *ucp = new_edges.data + blur_src.step * y;
		unsigned char *src = pedge + blur_src.cols * y;
		for(int x = 0; x < blur_src.cols; x++)
			*ucp++ = *src++;
	}
	free(pedge);

	// ȭ�鿡 �����ش�.
	imshow("new edge", new_edges);
}
