// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
using namespace std;


// TODO: 在此处引用程序需要的其他头文件
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
//const string gImgExt = ".jpg";					// 图像后缀
//const string gGtExt = ".bmp";					// GrouhdTurth图像后缀
//const string gSpExt = gImgExt+".txt";				// 超像素文件后缀
//const string gSpLblExt = gImgExt+".labels";			// 超像素label文件后缀
//const string gFeatureExt = gImgExt+".features";		// 特征文件后缀
//const string gOversegExt = ".graphbased.txt";		// 过分割文件后缀
//const string gLblExt = ".txt";					// 像素label文件后缀
//const string gWinInter = "Interaction Window"; //交互窗口
//const string gWinResult = "Result";				//结果窗口
//const int gClassNum = 10;		//类别数+1， 1,2,3,4,5,6,7,8,9,类别标签>0 

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