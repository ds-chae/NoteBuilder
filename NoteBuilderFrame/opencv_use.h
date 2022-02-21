#pragma once

template<class T>
class TypedMat
{
	T** m_pData;
	int m_nChannels;
	int m_nRows, m_nCols;

public:
	TypedMat():m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0){}
	~TypedMat(){if(m_pData) delete [] m_pData;}

	// OpenCV Mat 연동 (메모리 공유)
	void Attach(const cv::Mat& m);
	void Attach(const IplImage& m);
	TypedMat(const cv::Mat& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	TypedMat(const IplImage& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	const TypedMat & operator =(const cv::Mat& m){ Attach(m); return *this;}
	const TypedMat & operator =(const IplImage& m){ Attach(m); return *this;}

	// 행(row) 반환
	T* GetPtr(int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// 연산자 중첩 (원소접근) -- 2D
	T * operator [](int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	const T * operator [](int r) const
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// 연산자 중첩 (원소접근) -- 3D
	T & operator ()(int r, int c, int k)
	{ assert(r>=0 && r<m_nRows && c>=0 && c<m_nCols && k>=0 && k<m_nChannels); return m_pData[r][c*m_nChannels+k];}

	const T operator ()(int r, int c, int k) const
	{ assert(r>=0 && r<m_nRows && c>=0 && c<m_nCols && k>=0 && k<m_nChannels); return m_pData[r][c*m_nChannels+k];}
};

template<class T>
void TypedMat<T>::Attach(const cv::Mat& m)
{
	assert(sizeof(T)==m.elemSize1());

	m_nChannels = m.channels();
	m_nRows = m.rows;
	m_nCols = m.cols;
	
	if(m_pData) delete [] m_pData;
	m_pData = new T * [m_nRows];
	for(int r=0; r<m_nRows; r++)
	{
		m_pData[r] = (T *)(m.data + r*m.step);
	}
}

template<class T>
void TypedMat<T>::Attach(const IplImage& m)
{
	assert(sizeof(T)==m.elemSize1());

	m_nChannels = m.nChannels;
	m_nRows = m.height;
	m_nCols = m.width;
	
	if(m_pData) delete [] m_pData;
	m_pData = new T * [m_nRows];
	for(int r=0; r<m_nRows; r++)
	{
		m_pData[r] = (T *)(m.imageData + r*m.widthStep);
	}
}

/**
3. TypedMat 사용법

메모리를 공유하기 때문에 추가적인 연산 로드는 거의 없습니다. IplImage, cv::Mat을 지원합니다. 사용법은 다음과 같습니다.

* 이미지의 경우
#include "use_opencv.h"

Mat image;

TypedMat<unsigned char> tm = image;    // 연결방법 1
TypedMat<unsigned char> tm; tm = image;    // 연결방법 2
TypedMat<unsigned char> tm; tm.Attach(image);    // 연결방법 3

// image가 1채널 grayscale 이미지일 경우
tm[y][x] = 100;    // (x,y)의 픽셀값을 100으로 설정

// image가 3채널 color 이미지일 경우
tm(y,x,0) = 100;    // (x,y)의 픽셀의 blue값을 100으로 설정
tm(y,x,1) = 200;    // (x,y)의 픽셀의 green값을 200으로 설정
tm(y,x,2) = 50;    // (x,y)의 픽셀의 red값을 50으로 설정

* 2D float 타입 행렬의 경우
#include "use_opencv.h"
Mat f_image;
TypedMat<float> fm = f_image;
fm[y][x] = 3.12f;
**/