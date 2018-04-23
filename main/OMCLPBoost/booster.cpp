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

#include "booster.h"
#include "online_rf.h"
#include "linear_larank.h"

Booster::Booster(const Hyperparameters& hp, const int& numClasses, const int& numFeatures,
                 const VectorXd& minFeatRange, const VectorXd& maxFeatRange) :
    Classifier(hp, numClasses), m_w(VectorXd::Constant(hp.numBases, hp.shrinkage)) {
    switch (hp.weakLearner) {
    case WEAK_ORF: {
        OnlineRF *weakLearner;
        for (int nBase = 0; nBase < hp.numBases; nBase++) {
            weakLearner = new OnlineRF(hp, numClasses, numFeatures, minFeatRange, maxFeatRange);
            m_bases.push_back(weakLearner);
        }
        break;
    }
    case WEAK_LARANK: {
        LinearLaRank *weakLearner;
        for (int nBase = 0; nBase < hp.numBases; nBase++) {
            weakLearner = new LinearLaRank(hp, numClasses, numFeatures, minFeatRange, maxFeatRange);
            m_bases.push_back(weakLearner);
        }
        break;
    }
    }

}

Booster::~Booster() {
    for (int nBase = 0; nBase < m_hp->numBases; nBase++) {
        delete m_bases[nBase];
    }
}
    
void Booster::eval(Sample& sample, Result& result) {
    for (int nBase = 0; nBase < m_hp->numBases; nBase++) {
		//printf("\tnBase = %d, m_hp->numBases = %d\n", nBase, m_hp->numBases);
        Result baseResult(*m_numClasses);
        m_bases[nBase]->eval(sample, baseResult);
        baseResult.confidence *= m_w(nBase);
        result.confidence += baseResult.confidence;
    }
    result.confidence.maxCoeff(&result.prediction);
}

//////////////////////////////////////////////////////////////////////////
void Booster::RecordModel(FILE *fp)
{	
	printf("*****Booster::RecordModel*****\n");
	 for (int i = 0; i < m_hp->numBases; i++) 
	{
		m_bases[i]->RecordModel(fp);
	}

	//int d = m_w.rows();
	//fprintf(fp, "%d\t", d);			//////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < m_hp->numBases; ++ i)
	{
		 fprintf(fp, "%lf\t", m_w(i));
	}
	fprintf(fp, "\n");

	int nCache = m_cache.size();
	fprintf(fp, "%d\t", nCache);			//////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < nCache; i++) 
	{
		m_cache[i].RecordModel(fp);
	}
}

void Booster::LoadModel(FILE *fp)
{	
	printf("*****Booster::LoadModel*****\n");
	for (int i = 0; i < m_hp->numBases; i++) 
	{
		m_bases[i]->LoadModel(fp);
	}

	//int d = m_w.rows();
	//fscanf(fp, "%d", &d);		//////////////////////////////////////////////////////////////////////////	
	for (int i = 0; i < m_hp->numBases; ++ i)
	{
		double w;
		fscanf(fp, "%lf", &w);
		m_w(i) = w;
	}

	int nCache;
	fscanf(fp, "%d", &nCache);		//////////////////////////////////////////////////////////////////////////	
	//nCache = m_hp->cacheSize;
	m_cache.resize(nCache);
	for (int i = 0; i < nCache; i++) 
	{
		m_cache[i].LoadModel(fp);
	}
}
//////////////////////////////////////////////////////////////////////////