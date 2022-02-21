#include "stdafx.h"

using namespace cv;
using namespace std;

static void help()
{
    cout << "\nThis program demonstrates line finding with the Hough transform.\n"
            "Usage:\n"
            "./houghlines <image_name>, Default is pic1.png\n" << endl;
}

double hough_rho = 1;
double hough_theta = CV_PI / 180;
int hough_threshold = 20;
double hough_minLineLength = 5;
double hough_maxLineGap = 1;

vector<Vec4i> lines;

int houghlines(Mat& edge_mat, Mat& cdst_src)
{
	Mat cdst;
	cdst_src.copyTo(cdst);

	lines.clear();
	lines.resize(100000);
//    Canny(src, dst, 50, 200, 3);
//    cvtColor(edge_mat, cdst, CV_GRAY2BGR);

#if 0
    vector<Vec2f> lines;
    HoughLines(dst, lines, 1, CV_PI/180, 100, 0, 0 );

    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( cdst, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
#else
//    vector<Vec4i> lines;
//! finds line segments in the black-n-white image using probabalistic Hough transform
//CV_EXPORTS_W void HoughLinesP( InputArray image, OutputArray lines,
//                               double rho, double theta, int threshold,
//                               double minLineLength=0, double maxLineGap=0 );
//    HoughLinesP(edge_mat, lines, 1, CV_PI/180, 50, 50, 10 );
//    HoughLinesP(edge_mat, lines, 2, CV_PI/180, 5, 5, 1 );

	HoughLinesP(edge_mat, lines, hough_rho, hough_theta, hough_threshold, hough_minLineLength, hough_maxLineGap);

	for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,255,255), 1, CV_AA);
    }
#endif
//    imshow("source", src);
    imshow("detected lines", cdst);

	return 0;
}

