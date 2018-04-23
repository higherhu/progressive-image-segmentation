#include "regionfeatures.h"


RegionFeatures::RegionFeatures(void)
{
	n_Region = -1;
}


RegionFeatures::~RegionFeatures(void)
{
}

RegionFeatures::RegionFeatures(const Mat& _img, const Mat& _sp)
{
	SetImage(_img);
	SetSuperPixel(_sp);
}

void RegionFeatures::SetImage(const Mat& _img)
{
	_image = _img.clone();
	_width = _img.cols;
	_height = _img.rows;
}

void RegionFeatures::SetSuperPixel(const Mat& _sp)
{
	assert(_sp.size() == _image.size());
	n_Region = fixSuperpixel(_sp, _superpixel);

	//saveMat<int>(_superpixel, "superpixel.txt");
}

int RegionFeatures::GetRegionCount()
{
	return n_Region;
}

FeatureVector RegionFeatures::GetFeatures()
{
	return fv;
}

// 将像素特征转换为区域特征，输入为矩阵vector，vector中每一个矩阵表示一种像素特征
void RegionFeatures::GetFeaturesFromPixel(const vector<Mat>& _pixelFeatures)
{
	cout<<"Get Features from pixel ... "<<endl;
	vector<vector<double>> tmp;
	tmp.resize(n_Region, vector<double>(_pixelFeatures.size()+3, 0));	// 像素特征维度+3维颜色特征

	Mat lab(_height, _width, CV_32F);
	_image.convertTo(lab, CV_32F, 1.0/255);		// 对32位图像，RGB需转换为0~1
	cvtColor(lab, lab, COLOR_BGR2Lab);
	vector<int> pixelCount(n_Region, 0);

	for (int y = 0; y < _height; y++)
	{
		for (int x = 0; x < _width; x++)
		{
			int spId = _superpixel.at<int>(Point(x, y));
			int i = 0;
			for (; i < _pixelFeatures.size(); i++)
			{
				tmp[spId][i] += _pixelFeatures[i].at<float>(Point(x, y));
			}
			Vec3f color = lab.at<Vec3f>(Point(x, y));
			tmp[spId][i++] += color[0]/100.0;			//L
			tmp[spId][i++] += (color[1]+127.0)/254.0;	//a
			tmp[spId][i++] += (color[2]+127.0)/254.0;	//b
			pixelCount[spId] ++;
		}
	}
	for (int i = 0; i < n_Region; i++)
	{
		for (int j = 0; j  < _pixelFeatures.size()+3; j ++)
		{
			tmp[i][j] /= pixelCount[i];
		}
	}

	fv.addColumn(tmp);

	//vector<vector<double>> tmp2;
	//tmp2.resize(n_Region);
	//for (int i = 0; i < n_Region; i++)
	//{
	//	tmp2[i].resize(_pixelFeatures.size());
	//	Mat mask = _superpixel==i;
	//	for (int j = 0; j < _pixelFeatures.size(); j++)
	//	{
	//		tmp2[i][j] = mean(_pixelFeatures[j], mask)[0];
	//	}
	//}
	//fv.addColumn(tmp2);

	//// 增加CIE Lab颜色特征
	////Mat lab(_height, _width, CV_32F);
	////_image.convertTo(lab, CV_32F, 1.0/255);		// 对32位图像，RGB需转换为0~1
	////cvtColor(lab, lab, COLOR_BGR2Lab);
	//vector<vector<double>> regionLab;
	//regionLab.resize(n_Region);

	//for (int rId = 0; rId < n_Region; rId++)
	//{
	//	regionLab[rId].resize(3);
	//	Mat mask = _superpixel==rId;
	//	Scalar aveLab = mean(lab, mask);

	//	regionLab[rId][0] = aveLab[0]/100.0;	//Lab在正确的取值范围，L、a、b：0~1
	//	regionLab[rId][1] = (aveLab[1]+127.0)/254.0;
	//	regionLab[rId][2] = (aveLab[2]+127.0)/254.0;
	//}
	//fv.addColumn(regionLab);
}

void RegionFeatures::GetRegionPoints(vector<vector<Point>>& rp)
{
	rp.resize(n_Region);
	for (int i = 0; i < _height; i++)
	{
		const int* ptr = _superpixel.ptr<int>(i);
		for (int j = 0; j < _width; j++)
		{
			rp[ptr[j]].push_back(Point(j, i));
		}
	}
}

// 提取区域+邻域区域特征，邻域为8个方向的3*3的邻域，输入为矩阵vector，vector中每一个矩阵表示一种像素特征
void RegionFeatures::GetNeighFeaturesFromPixel(const vector<Mat>& _pixelFeatures)
{
	cout<<"Get Neighbor Regions Features ... "<<endl;
	vector<Mat> pf;
	for (int i = 0; i < _pixelFeatures.size(); i++)
	{
		Mat t = _pixelFeatures[i].clone();
		pf.push_back(t);
		copyMakeBorder(pf[i], pf[i], 3, 3, 3, 3, BORDER_REFLECT);
	}
	vector<vector<Point>> rp;
	GetRegionPoints(rp);

	vector<vector<double>> tmp;
	tmp.resize(n_Region);
	//int index = 0;
	/*for (set<int>::iterator iter = regionId.begin(); iter != regionId.end(); iter++, index++)
	{*/
	for (int i = 0; i < n_Region; i++)
	{
		//int id = *iter;
		int id = i;
		int index = i;
		tmp[index].resize(_pixelFeatures.size()*9);		// 当前区域+8邻域
		Mat mask = _superpixel==id;
		int total = 0;
		for ( ; total < _pixelFeatures.size(); total++)
		{
			tmp[index][total] = mean(_pixelFeatures[total], mask)[0];
		}
		Rect rt = boundingRect(rp[index]);
		rt = Rect(rt.x+3, rt.y+3, rt.width, rt.height);
		// left
		Rect r = Rect(rt.x-3, rt.y, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// right
		r = Rect(rt.x+3, rt.y, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// top
		r = Rect(rt.x, rt.y-3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// bottom
		r = Rect(rt.x, rt.y+3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// top-left
		r = Rect(rt.x-3, rt.y-3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// top-right
		r = Rect(rt.x+3, rt.y-3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// bottom-left
		r = Rect(rt.x-3, rt.y+3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
		// bottom-right
		r = Rect(rt.x+3, rt.y+3, rt.width, rt.height);
		for (int j = 0; j < pf.size(); j++)
		{
			Mat tmpMat = pf[j](r);
			tmp[index][total++] = mean(tmpMat)[0];
		}
	}
	fv.addColumn(tmp);
}

void RegionFeatures::ShowDis(const vector<RegionDis>& _Dis)
{
	map<int, double> edge;
	for (int i = 0; i < _Dis.size(); i++)
	{
		map<int, double>::iterator it = edge.find(_Dis[i].regionId1);
		if (it != edge.end())
		{
			it->second = min(it->second, _Dis[i].distance);
		}
		else
		{
			edge.insert(make_pair(_Dis[i].regionId1, _Dis[i].distance));
		}
	}
	Mat disMat(_image.size(), CV_32FC1);
	for (int y = 0; y < _image.rows; y++)
	{
		for (int x = 0; x < _image.cols; x++)
		{
			int id = _superpixel.at<int>(Point(x,y));
			disMat.at<float>(Point(x,y)) = edge.find(id)->second;
		}
	}

	double maxEdge;
	minMaxLoc(disMat,NULL,&maxEdge);
	//cout << showEdgesImg.at<float>(maxPoint) << endl;
	imshow("Edges", disMat/maxEdge);
}

// 计算区域之间的距离, 利用区域平均Lab颜色之差
void RegionFeatures::GetRegionDis(vector<RegionDis>& _Dis)
{
	//std::cout<<"caluating the region distance ... "<<endl;
	_Dis.clear();
	CalSegNeighbors();

	vector<Scalar> regionLab;
	double varianceSquared = CalRegionAveLab(regionLab);

	//fstream f("dis.txt", ios::out);
	/*for (set<int>::iterator iter = regionId.begin(); iter != regionId.end(); iter++)
	{*/
	for (int index = 0; index < n_Region; index++)
	{
		//int rId = *iter;
		int rId = index;
		for (set<int>::iterator i = m_Neighbors[rId].begin(); i != m_Neighbors[rId].end(); i++)
		{
			int nId = *i;
			RegionDis dis;
			dis.regionId1 = rId; dis.regionId2 = nId;
			double d =  ((regionLab[rId][0]-regionLab[nId][0])*(regionLab[rId][0]-regionLab[nId][0]) +
								(regionLab[rId][1]-regionLab[nId][1])*(regionLab[rId][1]-regionLab[nId][1]) +
								(regionLab[rId][2]-regionLab[nId][2])*(regionLab[rId][2]-regionLab[nId][2])) ; // 越相似，值越小。
			dis.distance = exp(-d / (2*varianceSquared))/3.0;	// 除几何距离，越相似，值越大，故此严格意义上来说并不是距离。
			_Dis.push_back(dis);
			//f<<rId<<" "<<nId<<" "<<dis.distance<<endl;
		}
	}
	//f.close();
}
//struct regionPair{
//	int id1;
//	int id2;
//};
//struct cmp_key
//{
//	bool operator()(const regionPair &k1, const regionPair &k2)const
//	{
//		if ( (k1.id1 < k2.id1)
//			|| (k1.id1==k2.id1 && k1.id2<k2.id2) )
//			return true;
//		else
//			return false;
//	}
//};
//void RegionFeatures::GetRegionDis(vector<RegionDis>& _Dis)
//{
//	//std::cout<<"caluating the region distance ... "<<endl;
//	_Dis.clear();
//
//	double varianceSquared = 0.0;
//	int counter = 0;
//	for(int i=0; i<_image.rows; i++)
//	{
//		for(int j=0; j<_image.cols; j++) 
//		{
//			
//			// You can now access the pixel value with cv::Vec3b
//			float b = (float)_image.at<Vec3b>(i,j)[0];
//			float g = (float)_image.at<Vec3b>(i,j)[1];
//			float r = (float)_image.at<Vec3b>(i,j)[2];
//			for (int si = -1; si <= 1 && si + i < _image.rows && si + i >= 0; si++)
//			{
//				for (int sj = -1; sj <= 1 && sj + j < _image.cols && sj + j >= 0; sj++)
//
//				{
//					if ((si == 0 && sj == 0))
//						continue;
//
//					float nb = (float)_image.at<Vec3b>(i+si,j+sj)[0];
//					float ng = (float)_image.at<Vec3b>(i+si,j+sj)[1];
//					float nr = (float)_image.at<Vec3b>(i+si,j+sj)[2];
//
//					varianceSquared+= (b-nb)*(b-nb) + (g-ng)*(g-ng) + (r-nr)*(r-nr); 
//					counter ++;
//				}
//			}
//		}
//	}
//	varianceSquared/=counter;
//
//	map<regionPair, double, cmp_key> dis;
//	map<regionPair, int, cmp_key> num;
//	for(int i=0; i<_image.rows; i++)
//	{
//		for(int j=0; j<_image.cols; j++) 
//		{
//			//float edgeStrength = 0;
//			int id1 = _superpixel.at<int>(i,j);
//			/*set<int> idList;
//			set<double> edgeStrength;*/
//			map<int, pair<int, double>> neighStrength;
//			// You can now access the pixel value with cv::Vec3b
//			float b = (float)_image.at<Vec3b>(i,j)[0];
//			float g = (float)_image.at<Vec3b>(i,j)[1];
//			float r = (float)_image.at<Vec3b>(i,j)[2];
//			for (int si = -1; si <= 1 && si + i < _image.rows && si + i >= 0; si++)
//			{
//				for (int sj = -1; sj <= 1 && sj + j < _image.cols && sj + j >= 0; sj++)
//				{
//					if ((si == 0 && sj == 0))
//						continue;
//
//					float nb = (float)_image.at<Vec3b>(i+si,j+sj)[0];
//					float ng = (float)_image.at<Vec3b>(i+si,j+sj)[1];
//					float nr = (float)_image.at<Vec3b>(i+si,j+sj)[2];
//
//					//   ||I_p - I_q||^2  /   2 * sigma^2
//					float currEdgeStrength = exp(-((b-nb)*(b-nb) + (g-ng)*(g-ng) + (r-nr)*(r-nr))/(2*varianceSquared));
//					float currDist = sqrt((float)si*(float)si + (float)sj * (float)sj);
//
//					
//					// this is the edge between the current two pixels (i,j) and (i+si, j+sj)
//					double es = ((double)0.95 * currEdgeStrength + (double)0.05) /currDist;		
//
//					//idList.insert(_superpixel.at<int>(i+si,j+sj));
//					int id2 = _superpixel.at<int>(i+si,j+sj);
//					map<int, pair<int, double>>::iterator it = neighStrength.find(id2);
//					if (it != neighStrength.end())
//					{
//						it->second.first += 1;		// 个数+1
//						it->second.second += es;	// 强度+es
//					}
//					else
//					{
//						neighStrength.insert(make_pair(id2, make_pair(1,0)));
//					}
//
//							
//				}
//			}
//			regionPair p;
//			for (map<int, pair<int, double>>::iterator iter = neighStrength.begin(); iter != neighStrength.end(); iter++)
//			{
//				int id2 = iter->first;
//				if (id1 == id2)
//					continue;
//				p.id1 = id1>id2 ? id2 : id1;
//				p.id2 = id1>id2 ? id1 : id2;
//				map<regionPair, double, cmp_key>::iterator it = dis.find(p);
//				map<regionPair, int, cmp_key>::iterator it2 = num.find(p);
//				if (it != dis.end())
//				{
//					it->second += iter->second.second;
//					it2->second += iter->second.first;
//				}
//				else
//				{
//					dis.insert(make_pair(p,iter->second.second));
//					num.insert(make_pair(p, iter->second.first));
//				}
//			}
//		}
//	}
//
//	map<regionPair, double, cmp_key>::iterator it = dis.begin();
//	map<regionPair, int, cmp_key>::iterator it2 = num.begin();
//	for ( ; it != dis.end(); it++, it2++)
//	{
//		RegionDis d1,d2;
//		d1.regionId1 = it->first.id1;
//		d1.regionId2 = it->first.id2;
//		d1.distance = it->second / it2->second;
//		d2.regionId1 = it->first.id2;
//		d2.regionId2 = it->first.id1;
//		d2.distance = it->second / it2->second;
//		_Dis.push_back(d1);
//		_Dis.push_back(d2);
//	}
//}

void RegionFeatures::CalSegNeighbors()
{
	m_Neighbors.clear();
	//set<int>::iterator it = regionId.end();
	//it--;
 //   m_Neighbors.resize(*(it)+1);	// 最大的Id值
	m_Neighbors.resize(n_Region);
    for (int y = 0; y < _height; y++) 
	{
		for (int x = 0; x < _width; x++) 
		{
            int segId = _superpixel.at<int>(y, x);
            int nbrId;
            if (x > 0) 
			{
                nbrId = _superpixel.at<int>(y, x - 1);
                if (nbrId != segId) 
				{
                    m_Neighbors[segId].insert(nbrId);
                    m_Neighbors[nbrId].insert(segId);
                }
            }
            if (y > 0) 
			{
                nbrId = _superpixel.at<int>(y - 1, x);
                if (nbrId != segId) 
				{
                    m_Neighbors[segId].insert(nbrId);
                    m_Neighbors[nbrId].insert(segId);
                }
            }
		}
    }
}

double RegionFeatures::CalRegionAveLab(vector<Scalar>& _aveLab)
{
	Mat lab(_height, _width, CV_32F);
	_image.convertTo(lab, CV_32F, 1.0/255);		// 对32位图像，RGB需转换为0~1
	cvtColor(lab, lab, COLOR_BGR2Lab);
	//set<int>::iterator it = regionId.end();
	//it--;
	_aveLab.clear();
	//_aveLab.resize(*(it)+1);	// 最大的Id值
	_aveLab.resize(n_Region);

	//for (set<int>::iterator iter = regionId.begin(); iter != regionId.end(); iter++)
	//{
	for (int rId = 0; rId < n_Region; rId++)
	{

		//int rId = *iter;
		Mat mask = _superpixel==rId;
		Scalar aveLab = mean(lab, mask);	//Lab在正确的取值范围，L：0~100，a、b:-127~127
		_aveLab[rId] = aveLab;
	}

	double varianceSquared = 0.0;
	int counter = 0;
	//for (set<int>::iterator iter = regionId.begin(); iter != regionId.end(); iter++)
	//{
	for (int rId = 0; rId < n_Region; rId++)
	{
		//int rId = *iter;
		for (set<int>::iterator i = m_Neighbors[rId].begin(); i != m_Neighbors[rId].end(); i++)
		{
			int nId = *i;
			RegionDis dis;
			dis.regionId1 = rId; dis.regionId2 = nId;
			varianceSquared += (_aveLab[rId][0]-_aveLab[nId][0])*(_aveLab[rId][0]-_aveLab[nId][0]) +
							  (_aveLab[rId][1]-_aveLab[nId][1])*(_aveLab[rId][1]-_aveLab[nId][1]) +
							  (_aveLab[rId][2]-_aveLab[nId][2])*(_aveLab[rId][2]-_aveLab[nId][2]);	// 0,1,2分别为Lab
			counter ++;
		}
	}
	varianceSquared/=counter;
	return varianceSquared;
}