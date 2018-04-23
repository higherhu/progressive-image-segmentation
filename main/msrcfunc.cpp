#include "msrcfunc.h"


MSRCFunc::MSRCFunc(void)
{
	m_Dir = "G:/ImageDataSet/MSRC_ObjCategImageDatabase_v2/";
	m_ImgExt = ".bmp";
}


MSRCFunc::~MSRCFunc(void)
{
}


// ��Mat����ʽ����_imgNameÿ�����ص�ground-truth��ǩ
Mat MSRCFunc::getImageLagels(const string _imgName)
{
	Mat groundTruth = imread(m_Dir+"GroundTruth/"+_imgName+"_GT"+m_ImgExt);
	Mat labels(groundTruth.size(), CV_32SC1);

	return labels;
}

// �Ա������_inLabel��_imgName��GroundTruth�����ػ�������
Mat MSRCFunc::CompareLabel(const Mat _inLabel, const string _imgName)
{
	Mat groundTruth = imread(m_Dir+"GroundTruth/"+_imgName+"_GT"+m_ImgExt);
	Mat confusion;
	
	return confusion;
}