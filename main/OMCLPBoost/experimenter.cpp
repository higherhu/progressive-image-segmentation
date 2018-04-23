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

#include <fstream>
//#include <sys/time.h>

#include "experimenter.h"

int train(Classifier* model, DataSet& dataset, Hyperparameters& hp) {
    clock_t startTime;
	startTime = clock();

    vector<int> randIndex;
    int sampRatio = dataset.m_numSamples/10 + 1;		// +1
    vector<double> trainError(hp.numEpochs, 0.0);
    for (int nEpoch = 0; nEpoch < hp.numEpochs; nEpoch++) {
        randPerm(dataset.m_numSamples, randIndex);
        for (int nSamp = 0; nSamp < dataset.m_numSamples; nSamp++) {

			model->update(dataset.m_samples[randIndex[nSamp]]);

            if (hp.findTrainError) {
                Result result(dataset.m_numClasses);
                model->eval(dataset.m_samples[randIndex[nSamp]], result);
                if (result.prediction != dataset.m_samples[randIndex[nSamp]].y) {
                    trainError[nEpoch]++;
                }
            }
           
            if (hp.verbose && (nSamp % sampRatio) == 0) {
                cout << "--- " << model->name() << " training --- Epoch: " << nEpoch + 1 << " --- ";
                cout << (10 * nSamp) / sampRatio << "%";
                cout << " --- Training error = " << trainError[nEpoch] << "/" << nSamp << endl;
            }
        }
    }

    //timeval endTime;
    //gettimeofday(&endTime, NULL);
	clock_t endTime;
	endTime = clock();
    cout << "--- " << model->name() << " training time = ";
    //cout << (endTime.tv_sec - startTime.tv_sec + (endTime.tv_usec - startTime.tv_usec) / 1e6) << " seconds." << endl;
	cout << (endTime - startTime)/1000.0 << " seconds." << endl;

	return trainError[hp.numEpochs-1];
}

vector<Result> test(Classifier* model, DataSet& dataset, Hyperparameters& hp) {
   clock_t startTime;
	startTime = clock();

    vector<Result> results;
	//results.reserve(dataset.m_numSamples);
	results.resize(dataset.m_numSamples);
#pragma omp parallel for
    for (int nSamp = 0; nSamp < dataset.m_numSamples; nSamp++) {
		//printf("nSamp = %d, m_numSamples = %d\n", nSamp, dataset.m_numSamples);
        Result result(dataset.m_numClasses);
        model->eval(dataset.m_samples[nSamp], result);
        //results.push_back(result);
		results[nSamp] = result;
    }

	if (hp.verbose) {
		//printf("error\n");
		double error = compError(results, dataset);   
        cout << "--- " << model->name() << " test error: " << error << endl;
    }

    //timeval endTime;
    //gettimeofday(&endTime, NULL);
	clock_t endTime;
	endTime = clock();
    cout << "--- " << model->name() << " testing time = ";
    //cout << (endTime.tv_sec - startTime.tv_sec + (endTime.tv_usec - startTime.tv_usec) / 1e6) << " seconds." << endl;
	cout << (endTime - startTime)/1000.0 << " seconds." << endl;

    return results;
}

vector<Result> trainAndTest(Classifier* model, DataSet& dataset_tr, DataSet& dataset_ts, Hyperparameters& hp) {
    clock_t startTime;
	startTime = clock();

    vector<Result> results;
    vector<int> randIndex;
    int sampRatio = dataset_tr.m_numSamples / 10;
    vector<double> trainError(hp.numEpochs, 0.0);
    vector<double> testError;
    for (int nEpoch = 0; nEpoch < hp.numEpochs; nEpoch++) {
        randPerm(dataset_tr.m_numSamples, randIndex);
        for (int nSamp = 0; nSamp < dataset_tr.m_numSamples; nSamp++) {
            if (hp.findTrainError) {
                Result result(dataset_tr.m_numClasses);
                model->eval(dataset_tr.m_samples[randIndex[nSamp]], result);
                if (result.prediction != dataset_tr.m_samples[randIndex[nSamp]].y) {
                    trainError[nEpoch]++;
                }
            }

            model->update(dataset_tr.m_samples[randIndex[nSamp]]);
            if (hp.verbose && (nSamp % sampRatio) == 0) {
                cout << "--- " << model->name() << " training --- Epoch: " << nEpoch + 1 << " --- ";
                cout << (10 * nSamp) / sampRatio << "%";
                cout << " --- Training error = " << trainError[nEpoch] << "/" << nSamp << endl;
            }
        }

        testError.push_back(compError(results, dataset_ts));
    }

    //timeval endTime;
    //gettimeofday(&endTime, NULL);
	clock_t endTime;
	endTime = clock();
    cout << "--- Total training and testing time = ";
    //cout << (endTime.tv_sec - startTime.tv_sec + (endTime.tv_usec - startTime.tv_usec) / 1e6) << " seconds." << endl;
	cout << (endTime - startTime)/1000.0 << " seconds." << endl;

    if (hp.verbose) {
        cout << endl << "--- " << model->name() << " test error over epochs: ";
        dispErrors(testError);
    }

    // Write the results
   /* string saveFile = hp.savePath + ".errors";
    ofstream file(saveFile.c_str(), ios::binary);
    if (!file) {
        cout << "Could not access " << saveFile << endl;
        exit(EXIT_FAILURE);
    }
    file << hp.numEpochs << " 1" << endl;
    for (int nEpoch = 0; nEpoch < hp.numEpochs; nEpoch++) {
        file << testError[nEpoch] << endl;
    }
    file.close();*/

    return results;
}

double compError(const vector<Result>& results, const DataSet& dataset) {
    double error = 0.0;
    for (int nSamp = 0; nSamp < dataset.m_numSamples; nSamp++) {
        if (results[nSamp].prediction != dataset.m_samples[nSamp].y) {
            error++;
        }
    }

    return error / dataset.m_numSamples;
}

void dispErrors(const vector<double>& errors) {
    for (int nSamp = 0; nSamp < (int) errors.size(); nSamp++) {
        cout << nSamp + 1 << ": " << errors[nSamp] << " --- ";
    }
    cout << endl;
}
