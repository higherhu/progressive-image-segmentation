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

 Data:
{
    trainData = "./data/dna-train.data";
    trainLabels = "./data/dna-train.labels";
    testData = "./data/dna-test.data";
    testLabels = "./data/dna-test.labels";
};
Forest:
{
    maxDepth = 20;
    numRandomTests = 20;
    counterThreshold = 200;
    numTrees = 10;
};
LaRank:
{
    larankC = 1.0;
};
Boosting:
{
    numBases = 10;
    weakLearner = 0; // 0: ORF, 1: LaRank
    shrinkage = 0.5;
    lossFunction = 0; // 0 = Exponential Loss, 1 = Logit Loss
    C = 5.0;
    cacheSize = 7;
    nuD = 2.0;
    nuP = 1e-6;
    annealingRate = 0.9999999;
    theta = 1.0;
    numIterations = 1;
};
Experimenter:
{
    findTrainError = 0;
    numEpochs = 1;
};
Output:
{
    savePath = "/tmp/online-mcboost-";
    verbose = 1;
};

 */

#include <iostream>
//#include <libconfig.h++>

#include "hyperparameters.h"

using namespace std;
//using namespace libconfig;

Hyperparameters::Hyperparameters() {
    // Forest
    maxDepth = 20;
    numRandomTests = 20;
    counterThreshold = 200;
    numTrees = 10;

    // LaRank
    larankC = 1.0;

    // Boosting
    numBases = 10;
    weakLearner = (WEAK_LEARNER) 0;

    // Online MCBoost
    shrinkage = 0.5;
    lossFunction = (LOSS_FUNCTION) 0;

    // Online MCLPBoost
    C = 5.0;
    cacheSize = 7;
    nuD = 2.0;
    nuP = 1e-6;
    theta = 1.0;
    annealingRate = 0.9999999;
    numIterations = 1;

    // Experimenter
    findTrainError = 1;
    numEpochs = 1;

    // Data
    trainData = (const char *) "./data/dna-train.data";
    trainLabels = (const char *) "./data/dna-train.labels";
    testData = (const char *) "./data/dna-test.data";
    testLabels = (const char *) "./data/dna-test.labels";

    // Output
    savePath = (const char *) "./tmp/online-mcboost-";
    verbose = 1;

    //cout << "Done." << endl;
}

Hyperparameters::Hyperparameters(const string& confFile) {
	Hyperparameters();
//    cout << "Loading config file: " << confFile << " ... ";
//
//    Config configFile;
//    configFile.readFile(confFile.c_str());
//
//    int tmp;
//
//    // Forest
//    maxDepth = configFile.lookup("Forest.maxDepth");
//    numRandomTests = configFile.lookup("Forest.numRandomTests");
//    counterThreshold = configFile.lookup("Forest.counterThreshold");
//    numTrees = configFile.lookup("Forest.numTrees");
//
//    // LaRank
//    larankC = configFile.lookup("LaRank.larankC");
//
//    // Boosting
//    numBases = configFile.lookup("Boosting.numBases");
//    tmp = configFile.lookup("Boosting.weakLearner");
//    weakLearner = (WEAK_LEARNER) tmp;
//
//    // Online MCBoost
//    shrinkage = configFile.lookup("Boosting.shrinkage");
//    tmp = configFile.lookup("Boosting.lossFunction");
//    lossFunction = (LOSS_FUNCTION) tmp;
//
//    // Online MCLPBoost
//    C = configFile.lookup("Boosting.C");
//    cacheSize = configFile.lookup("Boosting.cacheSize");
//    nuD = configFile.lookup("Boosting.nuD");
//    nuP = configFile.lookup("Boosting.nuP");
//    theta = configFile.lookup("Boosting.theta");
//    annealingRate = configFile.lookup("Boosting.annealingRate");
//    numIterations = configFile.lookup("Boosting.numIterations");
//
//    // Experimenter
//    findTrainError = configFile.lookup("Experimenter.findTrainError");
//    numEpochs = configFile.lookup("Experimenter.numEpochs");
//
//    // Data
//    trainData = (const char *) configFile.lookup("Data.trainData");
//    trainLabels = (const char *) configFile.lookup("Data.trainLabels");
//    testData = (const char *) configFile.lookup("Data.testData");
//    testLabels = (const char *) configFile.lookup("Data.testLabels");
//
//    // Output
//    savePath = (const char *) configFile.lookup("Output.savePath");
//    verbose = configFile.lookup("Output.verbose");
//
//    cout << "Done." << endl;
}
