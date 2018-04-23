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

//struct InterInfo{	// �û�������Ϣ�����û�ѡ����������꼰����
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
	Mat m_DisImage;		// ����������ʾ��ͼ��
	Mat m_ResultImg;	// ���ͼ��

	Mat m_GtLabel;		// ���ر�ǩ,GroundTruth
	vector<int> m_SpGtLabel;	//������LabelGroundTruth

	Mat m_Label;		// Ԥ������ر�ǩ
	vector<int>  m_SpLabel;		// ��¼���µĳ����ط����ǩ����i��Ϊ��i����sp��label

	FeatureVector m_Features;	// ��������������������������װm_SpId��������
	int m_SpCount;	// �����ظ������������ر�����ֵ+1

	Classifier* model;
	Hyperparameters hp;
	DataSet m_FeatureSet;	// �����������������������������Ŵ�С�������С�

	vector<RegionDis> m_RegionDis;

	//vector<InterInfo> m_UserInter;	// ��¼�û�������Ϣ����ÿ��ѵ��ǰ��m_InterMat�ж�ȡ
	map<int, int> m_UserInter;		// �û������õ���������+����
	vector<bool> m_SpInBound;	// ��¼ÿ���������Ƿ������ָ�߽硣
	int m_ChooseType;
	Point m_PrevPoint;
	bool m_isDrawing;
	Mat m_InterMat;		// ��¼�û������ľ���,��ʼΪ0���û������󽫶�Ӧ��λ�ø�Ϊ��Ӧ�ı�ǩ

	int drawLineWidth;

	GCoptimizationGeneralGraph *gc;

	double weight;
	int slic_step;

	vector<vector<double>> resultProb; // �����ط������
};

