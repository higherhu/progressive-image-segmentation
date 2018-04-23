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
	double distance;	// �ϸ���������˵�����Ǿ��롣
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

	/*��ȡ����+��������*/
	void GetNeighFeaturesFromPixel(const vector<Mat>&);
	void GetRegionPoints(vector<vector<Point>>& rp);

	FeatureVector GetFeatures();

	void GetRegionDis(vector<RegionDis>&);
	void CalSegNeighbors();
	double CalRegionAveLab(vector<Scalar>&);
	void ShowDis(const vector<RegionDis>& _Dis);

public:
	Mat _image;
	Mat _superpixel;	// �����أ��о�����������0��ʼ������
	int n_Region;
	//set<int> regionId;

	vector<set<int>> m_Neighbors;	// ��¼ÿ��������ڽ�������,vector�е�i��λ�ö���idΪi�����������

	int _width, _height;
};

