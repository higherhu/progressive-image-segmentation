#pragma once

#include <string>
using namespace std;
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#include "features/ETF.h"
#include "features/Saliency.h"

#include "featureVector.h"

// 1*LBP��3*ETF��1*Saliency��
// 17*Texture
// 2*Location
class PixelFeatures
{
public:
	PixelFeatures(void);
	~PixelFeatures(void);
	PixelFeatures(const Mat&);

	vector<Mat> GetAllFeatureVec();
	void SetImage(const Mat& _image);
	int GetSampleNum();

	FeatureVector fv;
	FeatureVector GetAllFeatures();

	static int s_Dim;		// ����ά��
	static int s_textureDim;//��������ά��

private:
	void CalLBP();
	void CalETF();
	void CalSaliency();
	void CalTexture();

	void CalLocation();

private:
	Mat image;

	//features 
	Mat LBPFeature;	// 1ά
	bool lbpCaled;
	ETF e;	// ����ģ����2ά
	bool etfCaled;
	bool salCaled;
	bool textureCaled;
	bool locationCaled;

	int m_pixelNum;	// ���ظ���
	int m_height;
	int m_width;

	vector<Mat> allFeatureVec;;
};

