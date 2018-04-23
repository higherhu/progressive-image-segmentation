#pragma once

#include "featureVector.h"
#include "basicfuncs.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#include <set>
using namespace std;

struct RegionDis{
	int regionId1;
	int regionId2;
	double distance;	// 严格意义上来说并不是距离。
};

class RegionFeatures
{
public:
	RegionFeatures(void);
	~RegionFeatures(void);
	RegionFeatures(const Mat& _img, const Mat& _sp);

	int GetRegionCount();

	void SetImage(const Mat&);
	void SetSuperPixel(const Mat&);

	FeatureVector fv;
	void GetFeaturesFromPixel(const vector<Mat>&);

	/*提取区域+邻域特征*/
	void GetNeighFeaturesFromPixel(const vector<Mat>&);
	void GetRegionPoints(vector<vector<Point>>& rp);

	FeatureVector GetFeatures();

	void GetRegionDis(vector<RegionDis>&);
	void CalSegNeighbors();
	double CalRegionAveLab(vector<Scalar>&);
	void ShowDis(const vector<RegionDis>& _Dis);

public:
	Mat _image;
	Mat _superpixel;	// 超像素，有经过修正，从0开始，连续
	int n_Region;
	//set<int> regionId;

	vector<set<int>> m_Neighbors;	// 记录每个区域的邻接区域编号,vector中第i个位置对于id为i的区域的邻域

	int _width, _height;
};

