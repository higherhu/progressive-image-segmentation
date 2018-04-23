#pragma once

#include <vector>
//#include <fstream>
#include <iostream>
using namespace std;
#include <stdio.h>

template<typename T>
class FeatureVector_
{
public:
	int _nDimension;	// ����ά��
	int _nSample;		// ������
	vector< vector<T> > _features;	// ����ֵ

private:


public:
	FeatureVector_(void) { _nDimension = 0; _nSample = 0; };
	FeatureVector_(int d) : _nDimension(d) { _nSample = 0; };
	FeatureVector_(int d, int s) : _nDimension(d), _nSample(s) { _features.reserve(s); };
	~FeatureVector_(void) { clearAll();};

	// ��������������������ӵ���������ά��Ӧ��������ά����ͬ
	void add(const vector<T> &f) {
		if (_nDimension == 0) _nDimension = f.size();
		_ASSERT(f.size() == _nDimension);
		_features.push_back(f);
		_nSample ++;
	}

	// ��������ά�ȣ�����������ĳһ��������������һ����
	void addColumn(const vector< vector<T> > &f) {
		if (_nSample == 0)
		{
			_nSample = f.size();
			_features.resize(_nSample);
		}
		_ASSERT(f.size() == _nSample);	// ȷ������������ͬ
		int addSize = f[0].size();
		for (int i=0; i<_nSample; i++)	
		{
			_ASSERT(f[i].size() == addSize); // ȷ�����ӵ�ÿһ��������ά����ͬ
			_features[i].insert(_features[i].end(), f[i].begin(), f[i].end());
		}
		_nDimension += addSize;
	};

	// ��������ά�ȣ�����������ĳһ��������������һ����
	void addColumn(const FeatureVector_<T> &f) {
		////////
		return;
	};

	// ����+�����
	friend FeatureVector_<T> operator+(FeatureVector_<T> f1, FeatureVector_<T> f2) {
		//////////////
		return f1;
	};

	void clearAll() {
		vector< vector<T> >::iterator it; 
		for (it = _features.begin(); it != _features.end(); it++)
		{
			it->clear();
		}
		_features.clear();
		_nSample = 0;
		_nDimension = 0;
	}

	void writeToFile(const string &_filename) {
		//ofstream ofs(_filename, ios::out|ios::trunc);
		//
		////cout<<"writing superpixel to file: "<<_filename<<endl;
		//ofs<<_nDimension<<" "<<_nSample<<endl;
		//for (int i = 0; i < _nSample; i++)
		//{
		//	for (int j = 0; j < _nDimension; j++)
		//	{
		//		ofs<<_features[i][j]<<" ";
		//	}
		//	ofs<<endl;
		//}
		//ofs.close();

		FILE* f = fopen(_filename.c_str(), "w");
		fprintf(f, "%d %d\n", _nDimension, _nSample);
		for (int i = 0; i < _nSample; i++)
		{
			for (int j = 0; j < _nDimension; j++)
			{
				fprintf(f, "%-12f", _features[i][j]);
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}

	void readFromFile(const string &_filename) {
		//ifstream ifs(_filename, ios::in);
		//
		////cout<<"reading features from file: "<<_filename<<endl;
		//ifs>>_nDimension>>_nSample;
		//_features.resize(_nSample);
		//for (int i = 0; i < _nSample; i++)
		//{
		//	_features[i].resize(_nDimension);
		//	for (int j = 0; j < _nDimension; j++)
		//	{
		//		ifs>>_features[i][j];
		//	}
		//}
		//ifs.close();

		FILE* f = fopen(_filename.c_str(), "r");
		fscanf(f, "%d %d", &_nDimension, &_nSample);
		_features.resize(_nSample);
		for (int i = 0; i < _nSample; i++)
		{
			_features[i].resize(_nDimension);
			for (int j = 0; j < _nDimension; j++)
			{
				float t;
				fscanf(f, "%f", &t);
				_features[i][j] = t;
			}
		}
		fclose(f);
	}
};

typedef FeatureVector_<double> FeatureVector;