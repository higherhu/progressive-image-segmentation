#pragma once

#include<iostream>
using namespace std;

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

class MSRCFunc
{
public:
	MSRCFunc(void);
	~MSRCFunc(void);

	Mat getImageLagels(const string _imgName);

	Mat CompareLabel(const Mat _inLabel, const string _imgName);

	string m_Dir;
	string m_ImgExt;
};

