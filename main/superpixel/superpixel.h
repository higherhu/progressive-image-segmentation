#pragma once

#include <vector>
#include <string>
#include <fstream>
using namespace std;

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#include "helper.h"
#include "seeds2.h"
#include "SLIC.h"
#include "segment-image.h"

#include "../basicfuncs.h"


class Superpixel
{
public:
	Superpixel(const Mat&);
	~Superpixel(void);

	void GetSeeds(int NR_SUPERPIXELS);
	void GetSlic(int step);

	void GetGraphBased(double sigma=0.5, double k=30, int min_size=30);

	void ShowSlic(const cv::Vec3b _color=cv::Vec3b(0,0,255));
	void ShowSeeds(const cv::Vec3b _color=cv::Vec3b(0,0,255));

	void ShowGraphBased(const cv::Vec3b _color=cv::Vec3b(0,0,255));

	Mat m_Image;
	Mat m_Seeds;
	Mat m_Slic;

	Mat m_GraphBased;

	int n_Seeds;	// ³¬ÏñËØ¸öÊý
	int n_Slic;

	int n_GraphBased;
};

