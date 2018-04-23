#pragma once

#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
using namespace std;

#include "pixelfeatures.h"
#include "featureVector.h"
#include "basicfuncs.h"
#include "regionfeatures.h"
#include "stdafx.h"

#include "superpixel\superpixel.h"

#include "OMCLPBoost\experimenter.h"
#include "OMCLPBoost\online_mclpboost.h"
#include "multi-labelGraphCut\GCoptimization.h"
#include "multi-labelGraphCut\energy.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#include "Timer.h"

//struct InterInfo{	// 用户交互信息，即用户选择的像素坐标及类型
//	Point p;
//	int type;
//};

class App
{
public:
	App(void);
	~App(void);

	void showSp(const cv::Mat& _img, const cv::Mat& _sp, const string _winName="result", const cv::Vec3b _color=cv::Vec3b(255,0,0));

	void onMouseCallback( int event, int x, int y, int flags, void* param );
	bool SetImageName(const string&);
	void InitModel();
	void featuresVecToDataSet(const FeatureVector &_fVector, DataSet &ds, const vector<int>& label=vector<int>());
	void doTrain();
	void GetLabelFromUser();
	void doPredict();
	void doSegment();

	void UpdateInterWindow();
	void ShowResult();
	void Label2Result();
	void RefineBoundary(const vector<vector<double>> prob, vector<int>& segLabels);

	void GetPixelDis();
	void RefinePixelBoundary();

	void TrainAll();

	void reset();

	void ReleaseMat();

	void showOversegLabel();

	void SaveModel();
	void LoadModel();
	
	void InBoundary();

	void PreProcessing();
	void ReadGroundTruth();

	void Run();
	void GetGTPixelLabel();
	void GetGTRegionLabel();
	void SaveColorTable();
	void ReadColorTable();
	vector<Scalar> gtColorTable;
	string _name;
	bool _getDistance;

	string m_ImageName;
	Mat m_SrcImage;
	Mat m_Superpixel;
	Mat m_OverSeg;
	Mat m_DisImage;		// 交互窗口显示的图像
	Mat m_ResultImg;	// 结果图像

	Mat m_GtLabel;		// 像素标签,GroundTruth
	vector<int> m_SpGtLabel;	//超像素LabelGroundTruth

	Mat m_Label;		// 预测的像素标签
	vector<int>  m_SpLabel;		// 记录最新的超像素分类标签，第i个为第i个的sp的label

	FeatureVector m_Features;	// 超像素区域特征。超像素区域安装m_SpId排列索引
	int m_SpCount;	// 超像素个数，即超像素编号最大值+1

	Classifier* model;
	Hyperparameters hp;
	DataSet m_FeatureSet;	// 超像素区域特征。按超像素区域编号从小到大排列。

	vector<RegionDis> m_RegionDis;

	//vector<InterInfo> m_UserInter;	// 记录用户交互信息，在每次训练前从m_InterMat中读取
	map<int, int> m_UserInter;		// 用户交互得到的区域编号+类别号
	vector<bool> m_SpInBound;	// 记录每个超像素是否跨过过分割边界。
	int m_ChooseType;
	Point m_PrevPoint;
	bool m_isDrawing;
	Mat m_InterMat;		// 记录用户交互的矩阵,初始为0，用户交互后将对应的位置改为对应的标签

	int drawLineWidth;

	GCoptimizationGeneralGraph *gc;

	double weight;
	int slic_step;

	vector<vector<double>> resultProb; // 超像素分类概率
};

