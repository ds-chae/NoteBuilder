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

	// OpenCV Mat ���� (�޸� ����)
	void Attach(const cv::Mat& m);
	void Attach(const IplImage& m);
	TypedMat(const cv::Mat& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	TypedMat(const IplImage& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	const TypedMat & operator =(const cv::Mat& m){ Attach(m); return *this;}
	const TypedMat & operator =(const IplImage& m){ Attach(m); return *this;}

	// ��(row) ��ȯ
	T* GetPtr(int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// ������ ��ø (��������) -- 2D
	T * operator [](int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	const T * operator [](int r) const
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// ������ ��ø (��������) -- 3D
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
3. TypedMat ����

�޸𸮸� �����ϱ� ������ �߰����� ���� �ε�� ���� �����ϴ�. IplImage, cv::Mat�� �����մϴ�. ������ ������ �����ϴ�.

* �̹����� ���
#include "use_opencv.h"

Mat image;

TypedMat<unsigned char> tm = image;    // ������ 1
TypedMat<unsigned char> tm; tm = image;    // ������ 2
TypedMat<unsigned char> tm; tm.Attach(image);    // ������ 3

// image�� 1ä�� grayscale �̹����� ���
tm[y][x] = 100;    // (x,y)�� �ȼ����� 100���� ����

// image�� 3ä�� color �̹����� ���
tm(y,x,0) = 100;    // (x,y)�� �ȼ��� blue���� 100���� ����
tm(y,x,1) = 200;    // (x,y)�� �ȼ��� green���� 200���� ����
tm(y,x,2) = 50;    // (x,y)�� �ȼ��� red���� 50���� ����

* 2D float Ÿ�� ����� ���
#include "use_opencv.h"
Mat f_image;
TypedMat<float> fm = f_image;
fm[y][x] = 3.12f;
**/