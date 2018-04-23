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

// 1*LBP，3*ETF，1*Saliency，
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

	static int s_Dim;		// 特征维度
	static int s_textureDim;//纹理特征维度

private:
	void CalLBP();
	void CalETF();
	void CalSaliency();
	void CalTexture();

	void CalLocation();

private:
	Mat image;

	//features 
	Mat LBPFeature;	// 1维
	bool lbpCaled;
	ETF e;	// 方向，模长，2维
	bool etfCaled;
	bool salCaled;
	bool textureCaled;
	bool locationCaled;

	int m_pixelNum;	// 像素个数
	int m_height;
	int m_width;

	vector<Mat> allFeatureVec;;
};

