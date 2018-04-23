#pragma once 

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
#include <queue>
#include <iostream>
#include <xfunctional>
#include <cv.h>
#include <cvaux.h>
using namespace cv;
using namespace std;

typedef vector<double> vecD;
typedef vector<float> vecF;
typedef pair<float, int> CostfIdx;
template<typename T> inline T sqr(T x) { return x * x;}
template<class T> inline T vecDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqrt(sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]));}
template<class T> inline T vecSqrDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]);}
//template<class T1, class T2> inline void operator /= (Vec<T1, 3> &v1, const T2 v2) { v1[0] /= v2; v1[1] /= v2; v1[2] /= v2; }
template<class T1> inline void operator /= (Vec<T1, 3> &v1, const int v2) { v1[0] /= v2; v1[1] /= v2; v1[2] /= v2; }
template<class T> inline T pntSqrDist(const Point_<T> &p1, const Point_<T> &p2) {return sqr(p1.x - p2.x) + sqr(p1.y - p2.y);}