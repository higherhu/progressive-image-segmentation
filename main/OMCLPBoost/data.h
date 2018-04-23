// -*- C++ -*-
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2010 Amir Saffari, amir@ymer.org
 * Copyright (C) 2010 Amir Saffari, 
 *                    Institute for Computer Graphics and Vision, 
 *                    Graz University of Technology, Austria
 */

#ifndef DATA_H_
#define DATA_H_

#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <Eigen/Core>
//#include <Eigen/Array>
//#include "MyVectorXd.h"

using namespace std;
using namespace Eigen;

// DATA CLASSES
class Sample {
public:
    VectorXd x;
    int y;
    double w;
    int id;
	//////////////////////////////////////////////////////////////////////////
public:
	void RecordModel(FILE *fp)
	{
		int d = x.rows();
		fprintf(fp, "%d\t", d);
		for (int i = 0; i < d; ++ i)
		{
			fprintf(fp, "%lf\t", x(i));
		}
		fprintf(fp, "%d\t%lf\t%d\n", y, w, id);
	};
	void LoadModel(FILE *fp)
	{
		int d;
		fscanf(fp, "%d", &d);
		x = VectorXd(d);
		for (int i = 0; i < d; ++ i)
		{
			double v;
			fscanf(fp, "%lf", &v);
			x(i) = v;
		}
		fscanf(fp, "%d%lf%d", &y, &w, &id);
	};
	//////////////////////////////////////////////////////////////////////////
};

class DataSet {
 public:
    void findFeatRange();

    void load(const string& x_filename, const string& y_filename);

    vector<Sample> m_samples;
    int m_numSamples;
    int m_numFeatures;
    int m_numClasses;

    VectorXd m_minFeatRange;
    VectorXd m_maxFeatRange;
};

class Result {
 public:
    Result();
    Result(const int& numClasses);

    VectorXd confidence;
    int prediction;
};

class Cache {
public:
    Cache();
    Cache(const Sample& sample, const int& numBases, const int& numClasses);

    Sample cacheSample;
    double margin;
    int yPrime; // Class with closest margin to the sample
	//////////////////////////////////////////////////////////////////////////
public:
	void RecordModel(FILE *fp);
	void LoadModel(FILE *fp);
	//////////////////////////////////////////////////////////////////////////
};

#endif /* DATA_H_ */
