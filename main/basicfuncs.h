#pragma once 

#include <fstream>
#include <set>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


template<typename T>
inline void saveMat(const cv::Mat& _inMat, const char* _fname)
{
	std::fstream outfile(_fname, std::ios::out);
	assert(outfile);
	outfile<<_inMat.rows<<' '<<_inMat.cols<<endl;
	for( int i = 0; i < _inMat.rows; i++ )
	{
		const T* ptr = _inMat.ptr<T>(i);
		for (int j = 0; j < _inMat.cols; j++)
		{
			outfile<<ptr[j]<<' ';
		}
		outfile<<'\n';
	}
	outfile.close();
}

template<typename T>
inline void readMat(cv::Mat& _inMat, const char* _fname)
{
	std::fstream infile(_fname, std::ios::in);
	assert(infile);

	int row, col;
	infile>>row>>col;
	if (_inMat.empty())
	{
		_inMat.create(row, col, CV_32SC1);
	}
	assert((_inMat.rows==row) && (_inMat.cols==col));

	for( int i = 0; i < _inMat.rows; i++ )
	{
		T* ptr = _inMat.ptr<T>(i);
		for (int j = 0; j < _inMat.cols; j++)
		{
			infile>>ptr[j];
		}
	}
	infile.close();
}

template<typename T>
inline void saveVector(std::vector<T>& _v, const char* _fname)
{
	std::fstream outfile(_fname, std::ios::out);
	assert(outfile);

	for (int i = 0; i < _v.size(); i++)
	{
		outfile<<_v[i]<<endl;
	}
	outfile.close();
}

template<typename T>
inline void readVector(std::vector<T>& _v, const char* _fname)
{
	std::fstream infile(_fname, std::ios::in);
	assert(infile);
	_v.clear();

	T t;
	while(infile>>t)
	{
		_v.push_back(t);
	}
	infile.close();
}

// ��������ת��ΪMat
inline void int2Mat(const int* _klabels, const int& _width, const int& _height, cv::Mat& _spMat)
{
	if (_spMat.empty())
	{
		_spMat = cv::Mat(_height, _width, CV_32SC1);
	}
	for (int i = 0; i < _height; i++)
	{
		int* ptr = _spMat.ptr<int>(i);
		for (int j = 0; j < _width; j++)
		{
			ptr[j] = _klabels[i*_width+j];
		}
	}
	return;
}

// BGRͼ��ת��Ϊ��ͨ����������
inline void bgr2Int(const cv::Mat& _img, unsigned int* _intMat)
{
	for (int i = 0; i < _img.rows; i++)
	{
		for (int j = 0; j < _img.cols; j++)
		{
			cv::Vec3b bgr = _img.at<cv::Vec3b>(i, j);
			_intMat[i*_img.cols+j] = (bgr[2]<<16) | (bgr[1]<<8) | (bgr[0]);
		}
	}
}

// ������(_x,_y)�淶��ͼ��ߴ���
inline void putPointInImg(int &_x, int &_y, const cv::Size &_s)
{
	if (_x < 0 || _x > 32767)
		_x = 0;
	if (_y < 0 || _y > 32767)
		_y = 0;

	if (_x >= _s.width)
		_x = _s.width - 1;
	if (_y >= _s.height)
		_y = _s.height - 1;
}

// _canvas�ϻ��㣬
inline void drawPoint(cv::Mat &_canvas, const cv::Point &_p, const cv::Scalar &_color=cv::Scalar(255,0,0))
{
	assert(!_canvas.empty());
	assert(_p.x>=0 && _p.x<_canvas.cols);
	assert(_p.y>=0 && _p.y<_canvas.rows);

	_canvas.at<cv::Vec3b>(_p) = cv::Vec3b(_color[0], _color[1], _color[2]);
}

// ���������ر�ţ�ʹ�ñ�Ŵ�0��ʼ�������������س����ظ���(�����+1��
// ������֤���һ������С��������
// �����������Ϊͬһ����
inline int fixSuperpixel(const cv::Mat& _sp, cv::Mat& _fixedSp)
{
	if (_fixedSp.empty())
	{
		_fixedSp.create(_sp.size(), CV_32SC1);
	}
	assert(_fixedSp.size() == _sp.size());

	int regionNum=0;
	//set<int> regionId;
	int* lookUpTable = new int[_sp.cols*_sp.rows];	// look up table,��ʼΪ-1,
	memset(lookUpTable, -1, _sp.cols*_sp.rows);
	for (int i = 0; i < _sp.rows; i++)
	{
		const int* ptr = _sp.ptr<int>(i);
		for (int j = 0; j < _sp.cols; j++)
		{
			int spId = ptr[j];
			if (lookUpTable[spId] < 0)	// ��ǰId�ڲ��ұ��в����ڣ��������ұ�
			{
				lookUpTable[spId] = regionNum;
				regionNum ++;
			}
			//regionId.insert(ptr[j]);
		}
		
	}
	//regionNum = regionId.size();

	for (int i = 0; i < _sp.rows; i++)
	{
		const int* ptrSp = _sp.ptr<int>(i);
		int* ptrFix = _fixedSp.ptr<int>(i);
		for (int j = 0; j < _sp.cols; j++)
		{
			int spId = ptrSp[j];
			if (lookUpTable[spId] >= 0)
			{
				ptrFix[j] = lookUpTable[spId];
			}
		}
		
	}

	/*int i=0;
	for (set<int>::iterator iter = regionId.begin(); iter != regionId.end(); iter++, i++)
	{
		int id = *iter;
		cv::Mat mask = _sp==id;
		_fixedSp.setTo(i, mask);
	}*/
	return regionNum;
}