#include "msrcfunc.h"


MSRCFunc::MSRCFunc(void)
{
	m_Dir = "G:/ImageDataSet/MSRC_ObjCategImageDatabase_v2/";
	m_ImgExt = ".bmp";
}


MSRCFunc::~MSRCFunc(void)
{
}


// 以Mat的形式返回_imgName每个像素的ground-truth标签
Mat MSRCFunc::getImageLagels(const string _imgName)
{
	Mat groundTruth = imread(m_Dir+"GroundTruth/"+_imgName+"_GT"+m_ImgExt);
	Mat labels(groundTruth.size(), CV_32SC1);

	return labels;
}

// 对比输入的_inLabel和_imgName的GroundTruth，返回混淆矩阵
Mat MSRCFunc::CompareLabel(const Mat _inLabel, const string _imgName)
{
	Mat groundTruth = imread(m_Dir+"GroundTruth/"+_imgName+"_GT"+m_ImgExt);
	Mat confusion;
	
	return confusion;
}