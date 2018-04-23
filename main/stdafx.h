// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
using namespace std;


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#define print_d(fmt, args,...) ({\
		if (enable_debug && log_file) \
			printf("Worker[%lu] %s:%d\t" fmt, syscall(__NR_gettid),__FUNCTION__ , __LINE__, ## args); \
	})


//const string gOutputDir = "./out/";
//const string gImgSet = "G:/ImageDataSet/tmp/";
////const string gImgSet = "G:/ImageDataSet/MSRC_ObjCategImageDatabase_v2/";
//const string gImgDir = gImgSet+"Images/";
//const string gGTDir = gImgSet+"GroundTruth/";
//const string gDataDir = gImgSet+"slic_5/";		// slic step
//const string gImgExt = ".jpg";					// ͼ���׺
//const string gGtExt = ".bmp";					// GrouhdTurthͼ���׺
//const string gSpExt = gImgExt+".txt";				// �������ļ���׺
//const string gSpLblExt = gImgExt+".labels";			// ������label�ļ���׺
//const string gFeatureExt = gImgExt+".features";		// �����ļ���׺
//const string gOversegExt = ".graphbased.txt";		// ���ָ��ļ���׺
//const string gLblExt = ".txt";					// ����label�ļ���׺
//const string gWinInter = "Interaction Window"; //��������
//const string gWinResult = "Result";				//�������
//const int gClassNum = 10;		//�����+1�� 1,2,3,4,5,6,7,8,9,����ǩ>0 

extern const Scalar ColorList[];
extern const string gExperDir;
extern const string gOutputDir;
extern string gImgSet;
extern /*const*/ string gImgDir;
extern /*const*/ string gGTDir;
extern /*const*/ string gDataDir;	
extern const string gImgExt;
extern const string gGtExt;
extern const string gSpExt;
extern const string gSpLblExt;
extern const string gFeatureExt;
extern const string gOversegExt;
extern const string gLblExt;
extern const string gWinInter;
extern const string gWinResult;
extern const int gClassNum;