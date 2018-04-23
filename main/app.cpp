#include "app.h"
#include "OMCLPBoost/utilities.h"

App::App(void)
{
	m_ChooseType = -1;
	m_isDrawing = false;
	drawLineWidth = 6;

	weight = 0.1;
	slic_step = 5;	////slic步长稍大一些

	gc = NULL;

	_getDistance = true;
}


App::~App(void)
{
	delete model;
	delete gc;
}

void App::ReleaseMat()
{
	cout<<"Release previous data...";
	m_SrcImage.release();
	m_Superpixel.release();
	m_OverSeg.release();
	m_Label.release();
	m_DisImage.release();
	m_ResultImg.release();
	m_InterMat.release();
	m_GtLabel.release();
	if (gc)
	{
		delete gc;
		gc = NULL;
	}
	m_ChooseType = -1;
	m_isDrawing = false;
	m_SpLabel.clear();
	m_UserInter.clear();
	m_SpGtLabel.clear();
	cout<<"Finished!"<<endl;
}

void App::reset()
{
	m_DisImage = m_SrcImage.clone();
	m_ResultImg = m_SrcImage;
	m_ChooseType = -1;
	m_UserInter.clear();
	m_InterMat.setTo(0);
	m_isDrawing = false;
	m_SpLabel.assign(m_SpLabel.size(), 0);

	UpdateInterWindow();
}

bool App::SetImageName(const string& _inName)
{
	cout<<">>Image:"<<gImgDir+_inName+gImgExt<<endl;
	m_ImageName = _inName;
	m_SrcImage = imread(gImgDir+_inName+gImgExt);
	if (m_SrcImage.empty())
	{
		cout<<"read image "<<gImgDir+_inName+gImgExt<<" failed!"<<endl;
		return false;
	}
	m_DisImage = m_SrcImage.clone();
	m_ResultImg = m_SrcImage.clone();
	//m_ResultImg.create(m_SrcImage.size(), m_SrcImage.type());
	//m_ResultImg.setTo(cv::Scalar(0,0,0));
	m_InterMat.create(m_SrcImage.size(), CV_32SC1);
	m_InterMat.setTo(0);
	return true;
}

void App::PreProcessing()
{
	Superpixel sp(m_SrcImage);

	cout<<"Pre-Processing ... "<<endl;
	//cout<<"get superpixels... "<<endl;
	if (_access((gDataDir+m_ImageName+gSpExt).c_str(), 0) == 0)	// 超像素文件存在
	{
		Mat tmp;
		readMat<int>(tmp, (gDataDir+m_ImageName+gSpExt).c_str());
		m_SpCount = fixSuperpixel(tmp, m_Superpixel);
	}
	else {
		sp.GetSlic(slic_step);		// slic step
		//sp.ShowSlic();
		m_SpCount = fixSuperpixel(sp.m_Slic, m_Superpixel);
		saveMat<int>(m_Superpixel, (gDataDir+m_ImageName+gSpExt).c_str());
	}
	//cout<<"get over-segmentation... "<<endl;
	if (_access((gDataDir+m_ImageName+gOversegExt).c_str(), 0) == 0)	// 过分割文件存在
	{
		readMat<int>(m_OverSeg, (gDataDir+m_ImageName+gOversegExt).c_str());
		//showSp(m_SrcImage, m_OverSeg, "OverSeg ");
	}
	else {
		/*sp.GetSeeds(266);
		sp.ShowSeeds();
		m_OverSeg = sp.m_Seeds.clone(); */
		sp.GetGraphBased();
		//sp.ShowGraphBased();
		m_OverSeg = sp.m_GraphBased.clone();
		saveMat<int>(m_OverSeg, (gDataDir+m_ImageName+gOversegExt).c_str());
	}
	InBoundary();	// 得到超像素是否跨过过分割边界的信息
	//cout<<"get features... "<<endl;
	if (_access((gDataDir+m_ImageName+gFeatureExt).c_str(), 0) == 0)	// 特征文件存在
	{
		m_Features.readFromFile(gDataDir+m_ImageName+gFeatureExt);
	}
	else {
		cout<<"Calculating features: "<<m_ImageName<<endl;
		PixelFeatures pf(m_SrcImage);
		vector<Mat> f = pf.GetAllFeatureVec();
		RegionFeatures rf(m_SrcImage, m_Superpixel);
		vector<Mat> texton;
		for (int i = 5; i < 5+17+2; i++)	// 仅保留Texton特征和Location特征
		{
			texton.push_back(f[i]);
		}
		rf.GetFeaturesFromPixel(texton);
		m_Features = rf.fv;
		m_Features.writeToFile(gDataDir+m_ImageName+gFeatureExt);
	}

	ReadColorTable();	//////////////////////////////////////////////////////////
	//assert(m_SpGtLabel.size() == m_Features._nSample);

	cout<<"features to DataSet... "<<endl;
	featuresVecToDataSet(m_Features, m_FeatureSet, m_SpGtLabel);
	//cout<<"calculating regionid..."<<endl;
	//RegionFeatures rf(m_SrcImage, m_Superpixel);
	//rf.GetRegionDis(m_RegionDis);	// 严格意义上来说并不是距离。越相似的区域值越大。
	//rf.ShowDis(m_RegionDis);
	if (_getDistance)
	{
		cout<<"Get Pixel Distance... "<<endl;
		GetPixelDis();
	}
	//m_SpId = rf.regionId;
	//set<int>::iterator it = m_SpId.end();
	//it--;
	//m_SpLabel.resize(*(it)+1);	// 记录每个sp的label，第i个为id为i的sp的label
	m_SpLabel.resize(m_SpCount,0);
	resultProb.resize(m_SpCount);
}

void App::ReadGroundTruth()
{
	cout<<"get pixel labels... "<<endl;
	if (_access((gDataDir+m_ImageName+gLblExt).c_str(), 0) == 0)	// 像素Label文件存在
	{
		readMat<int>(m_GtLabel, (gDataDir+m_ImageName+gLblExt).c_str());
	}
	else {
		cout<<"Calculating pixel label: "<<m_ImageName<<endl;
		GetGTPixelLabel();	
		SaveColorTable();	// 存储colortable
		saveMat<int>(m_GtLabel, (gDataDir+m_ImageName+gLblExt).c_str());
	}
	cout<<"get groundtruth... "<<endl;
	if (_access((gDataDir+m_ImageName+gSpLblExt).c_str(), 0) == 0)	// 超像素Label文件存在
	{
		readVector<int>(m_SpGtLabel, (gDataDir+m_ImageName+gSpLblExt).c_str());
	}
	else {
		cout<<"Calculating superpixel label: "<<m_ImageName<<endl;
		GetGTRegionLabel();
		saveVector<int>(m_SpGtLabel, (gDataDir+m_ImageName+gSpLblExt).c_str());
	}
}

void App::InBoundary()
{
	m_SpInBound.clear();
	m_SpInBound.resize(m_SpCount, false);
	int label, top, bot, left, right;
	int h = m_SrcImage.rows, w = m_SrcImage.cols;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = m_OverSeg.at<int>(i, j);
			top = m_OverSeg.at<int>( (i-1 > -1 ? i-1 : 0), j );
			bot = m_OverSeg.at<int>( (i+1 < h ? i+1 : h-1), j );
			left = m_OverSeg.at<int>( i, (j-1>-1 ? j-1 : 0) );
			right = m_OverSeg.at<int>( i, (j+1<w ? j+1 : w-1) );
			if (label!=top || label!=left || label!=right || label!=bot)
				m_SpInBound[m_Superpixel.at<int>(i, j)] = true;
		}
	}
}

void App::GetPixelDis()
{
	Timer t("getpixeldis");
	t.Start();
	Mat lab(m_SrcImage.rows, m_SrcImage.cols, CV_32F);
	m_SrcImage.convertTo(lab, CV_32F, 1.0/255);		// 对32位图像，RGB需转换为0~1
	cvtColor(lab, lab, COLOR_BGR2Lab);
	double varianceSquared = 0;

	// caluating varianceSquared
	int counter = 0;
	int NEIGHBORHOOD = 1;
#pragma omp parallel for
	for(int i=0; i<lab.rows; i++)
	{
		//#pragma omp parallel for
		for(int j=0; j<lab.cols; j++) 
		{
			float b = (float)lab.at<Vec3f>(i,j)[0];
			float g = (float)lab.at<Vec3f>(i,j)[1];
			float r = (float)lab.at<Vec3f>(i,j)[2];
			for (int si = -NEIGHBORHOOD; si <= NEIGHBORHOOD && si + i < lab.rows && si + i >= 0 ; si++)
			{
				
				for (int sj = 0; sj <= NEIGHBORHOOD && sj + j < lab.cols && sj+j>=0; sj++)
				{
					if ((si == 0 && sj == 0) /*||
						(si == 1 && sj == 0) || 
						(si == NEIGHBORHOOD && sj == 0)*/)
						continue;

					float nb = (float)lab.at<Vec3f>(i+si,j+sj)[0];
					float ng = (float)lab.at<Vec3f>(i+si,j+sj)[1];
					float nr = (float)lab.at<Vec3f>(i+si,j+sj)[2];
#pragma omp critical
					{
						varianceSquared+= (b-nb)*(b-nb) + (g-ng)*(g-ng) + (r-nr)*(r-nr); 
						counter ++;
					}
				}
			}
		}
	}
	varianceSquared/=counter;
	//cout<<varianceSquared<<endl;

	// get pixel distance 
	int nLabels = gClassNum;
	int numSample = m_SrcImage.rows * m_SrcImage.cols;
	gc = new GCoptimizationGeneralGraph(numSample, nLabels);	//当前图像的gc，换图像后需更换gc。

#pragma omp parallel for
	for(int i=0; i<lab.rows; i++)
	{
		//#pragma omp parallel for
		for(int j=0; j<lab.cols; j++) 
		{
			//RegionDis dis;
			// this is the node id for the current pixel
			int id1 = i * lab.cols + j;
			//dis.regionId1 = i * lab.cols + j;

			// You can now access the pixel value with cv::Vec3b
			float b = (float)lab.at<Vec3f>(i,j)[0];
			float g = (float)lab.at<Vec3f>(i,j)[1];
			float r = (float)lab.at<Vec3f>(i,j)[2];

			// go over the neighbors
			for (int si = -NEIGHBORHOOD; si <= NEIGHBORHOOD && si + i < lab.rows && si + i >= 0 ; si++)
			{
				for (int sj = 0; sj <= NEIGHBORHOOD && sj + j < lab.cols && sj+j>=0; sj++)
				{
					if ((si == 0 && sj == 0) /*||
						(si == 1 && sj == 0) || 
						(si == NEIGHBORHOOD && sj == 0)*/)
						continue;

					// this is the node id for the neighbor
					int id2 = (i+si) * lab.cols + (j + sj);
					//dis.regionId2 = (i+si) * lab.cols + (j + sj);

					float nb = (float)lab.at<Vec3f>(i+si,j+sj)[0];
					float ng = (float)lab.at<Vec3f>(i+si,j+sj)[1];
					float nr = (float)lab.at<Vec3f>(i+si,j+sj)[2];

					//   ||I_p - I_q||^2  /   2 * sigma^2
					double currEdgeStrength = exp(-((b-nb)*(b-nb) + (g-ng)*(g-ng) + (r-nr)*(r-nr))/(2*varianceSquared));
					double currDist = sqrt((float)si*(float)si + (float)sj*(float)sj);

					// this is the edge between the current two pixels (i,j) and (i+si, j+sj)
					currEdgeStrength = ((float)0.95 * currEdgeStrength + (float)0.05) /currDist;

					//dis.distance = currEdgeStrength;
					//m_RegionDis.push_back(dis);

					//gc->setNeighbors(dis.regionId1, dis.regionId2, currEdgeStrength);
#pragma omp critical
					{
						gc->setNeighbors(id1, id2, currEdgeStrength);
					}
				}
			}
		}
	}

	gc->setVerbosity(1);

	t.Report();
}

void App::UpdateInterWindow()
{
	//imshow(gWinInter, m_DisImage);
	Mat tmpImg = m_DisImage.clone();
	addWeighted(tmpImg, 0.65, m_ResultImg, 0.35, 0, tmpImg);
	//showSp(tmpImg, m_OverSeg, gWinInter, Vec3b(255,255,128));
	imshow(gWinInter, tmpImg);
}

void App::ShowResult()
{
	imshow(gWinResult, m_ResultImg);

	// show interactive map
	//Mat interMap(m_SrcImage.size(), CV_8UC3);
	//for (int i = 0; i < m_SrcImage.rows; i++)
	//{
	//	const int* ptrLabel = m_InterMat.ptr<int>(i);	// 用户交互图
	//	for (int j = 0; j < m_SrcImage.cols; j++)
	//	{
	//		Scalar c = ColorList[ptrLabel[j]];
	//		interMap.at<Vec3b>(i, j) = Vec3b(c[0],c[1],c[2]);
	//	}
	//}
	//imshow("Interaction", interMap);
} 

void App::featuresVecToDataSet(const FeatureVector &_fVector, DataSet &ds, const vector<int>& label)
{
	bool hasLabel=false;
	ds.m_numSamples = _fVector._nSample;
	ds.m_numFeatures = _fVector._nDimension;

	if (label.size() == ds.m_numSamples)
		hasLabel = true;

	ds.m_samples.clear();
	ds.m_samples.resize(ds.m_numSamples);
#pragma omp parallel for
	for (int nSamp = 0; nSamp < ds.m_numSamples; nSamp++) {
		Sample sample;
		sample.x = VectorXd(ds.m_numFeatures);
		sample.id = nSamp;
		sample.w = 1.0;
		//yfp >> sample.i;
		//labels.insert(sample.i);
		for (int nFeat = 0; nFeat < ds.m_numFeatures; nFeat++) {
			sample.x(nFeat) = _fVector._features[nSamp][nFeat];
		}
		if (hasLabel)
			sample.y = label[nSamp];
		//ds.m_samples.push_back(sample); // push sample into dataset
		ds.m_samples[nSamp] = sample;
	}
	ds.m_numClasses = gClassNum;

	// Find the data range
	ds.findFeatRange();
}

void App::showSp(const cv::Mat& _img, const cv::Mat& _sp, const string _winName, const cv::Vec3b _color)
{
	int w = _img.cols, h = _img.rows;
	int label, top, bot, left, right;//, topl, topr, botl, botr;
	cv::Mat showImg = _img.clone();

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = _sp.at<int>(i, j);
			top = _sp.at<int>( (i-1 > -1 ? i-1 : 0), j );
			bot = _sp.at<int>( (i+1 < h ? i+1 : h-1), j );
			left = _sp.at<int>( i, (j-1>-1 ? j-1 : 0) );
			right = _sp.at<int>( i, (j+1<w ? j+1 : w-1) );
			if (label!=top || label!=left || label!=right || label!=bot)
				showImg.at<cv::Vec3b>(i, j) = _color;
		}
	}
	imshow(_winName, showImg);
}

void App::onMouseCallback( int event, int x, int y, int flags, void* param )	
{
	if (m_ChooseType > 0)
	{
		putPointInImg(x, y, m_SrcImage.size());
		switch(event)
		{
		case CV_EVENT_LBUTTONDOWN:
			{
				m_isDrawing = true;				
				m_PrevPoint.x = x;
				m_PrevPoint.y = y;
				//drawPoint(m_DisImage, m_PrevPoint, ColorList[m_ChooseType]);
				cv::line(m_DisImage, m_PrevPoint, cv::Point(x,y), ColorList[m_ChooseType], drawLineWidth);
				cv::line(m_ResultImg, m_PrevPoint, cv::Point(x,y), ColorList[m_ChooseType], drawLineWidth);	///////////////////////// added//////////////
				//m_InterMat.at<int>(m_PrevPoint) = m_ChooseType;
				cv::line(m_InterMat, m_PrevPoint, Point(x,y), Scalar(m_ChooseType,0,0), drawLineWidth);
			}
			break;
		case CV_EVENT_MOUSEMOVE:
			{
				if(m_isDrawing)
				{
					cv::line(m_DisImage, m_PrevPoint, cv::Point(x,y), ColorList[m_ChooseType], drawLineWidth);
					cv::line(m_ResultImg, m_PrevPoint, cv::Point(x,y), ColorList[m_ChooseType], drawLineWidth); 	///////////////////////// added//////////////
					cv::line(m_InterMat, m_PrevPoint, Point(x,y), Scalar(m_ChooseType,0,0), drawLineWidth);
					m_PrevPoint.x=x;
					m_PrevPoint.y=y;
				}
			}
			break;
		case CV_EVENT_LBUTTONUP:
			{			
				m_isDrawing=false;
				drawPoint(m_DisImage,cv::Point(x,y),ColorList[m_ChooseType]);
				drawPoint(m_ResultImg,cv::Point(x,y),ColorList[m_ChooseType]);	///////////////////////// added//////////////
				m_InterMat.at<int>(Point(x, y)) = m_ChooseType;
			}
			break;
		}
		UpdateInterWindow();
	}
}

void App::InitModel()
{
	if (model == NULL) {
		model = new OnlineMCLPBoost(hp, gClassNum, m_FeatureSet.m_numFeatures, 
			m_FeatureSet.m_minFeatRange, m_FeatureSet.m_maxFeatRange);
	}
}

// 利用GroundTruth数据运行算法，得到模型后对未知图像进行测试
void App::Run()
{
	fstream filelist(gImgSet+string("allImgNames.txt"), ios::in);
	string imgName;
	vector<string> imgList;

	while (filelist>>imgName)
	{
		imgList.push_back(imgName);
	}
	filelist.close();

	int imgNum = imgList.size();
	float trainPercent = 0.5;
	vector<int> randIndex;
	randPerm(imgNum, randIndex);

	// training...
	_getDistance = false;
	int index = 0;
	for (; index < imgNum*trainPercent; index++)
	{
		cout<<"---------training with: "<<imgList[randIndex[index]]<<"------"<<(float)(index)/(imgNum*trainPercent) * 100<<"%"<<"----------"<<endl;
		SetImageName(imgList[randIndex[index]]);

		//m_SpLabel.clear();
		//m_SpLabel.resize(m_SpCount);
		for (int i = 0; i < m_SpCount; i++)
		{
			m_SpLabel[i] = m_SpGtLabel[i];
		}

		TrainAll();
		ReleaseMat();
	}
	//SaveModel();
	//SaveColorTable();
	// predicting...
	//int index = imgNum*trainPercent;
	//LoadModel();
	ReadColorTable();
	_getDistance = true;
	vector<vector<int>> confusion(gtColorTable.size()+1);	// 混淆矩阵
	for (int i = 0; i < gtColorTable.size() + 1; i++)
	{
		confusion[i].resize(gtColorTable.size()+1, 0);
	}

	int totalPixel=0, totalError = 0;
	for (; index < imgNum; index++)
	{
		cout<<"predicting image:"<<imgList[randIndex[index]]<<"------"<<index<<" in "<<imgNum<<"----------"<<endl;
		SetImageName(imgList[randIndex[index]]);
		_name = imgList[randIndex[index]];

		doPredict();

		int total = m_SrcImage.rows*m_SrcImage.cols;
		/*Mat err = m_GtLabel!=m_Label;
		err.convertTo(err, CV_32SC1, 1/255.0); 
		saveMat<int>(err, "err.txt");
		Scalar e = sum(err);
		int error = e[0];*/
		int error = 0;
		for (int y = 0; y < m_SrcImage.rows; y++)
		{
			for (int x = 0; x < m_SrcImage.cols; x++)
			{
				int gtlabel = m_GtLabel.at<int>(Point(x,y));
				int preLabel = m_Label.at<int>(Point(x,y));
				confusion[gtlabel][preLabel] ++;
				if (gtlabel != preLabel)
				{
					error ++;
				}
			}
		}

		totalPixel += total;
		totalError += error;
		cout<<"   Error:"<<(float)(error)/total<<endl;
		imwrite(gOutputDir+imgList[randIndex[index]]+".bmp", m_ResultImg);
		ReleaseMat();
	}
	cout<<"Average Error:"<<(float)(totalError)/totalPixel<<endl;

	ofstream con("confusion.txt");
	for (int i = 0; i < confusion.size(); i++)
	{
		for (int j = 0; j < confusion[i].size(); j++)
		{
			con<<confusion[i][j]<<" ";
			cout<<confusion[i][j]<<"\t";
		}
		con<<endl;
		cout<<endl;
	}
	cout<<"finished!"<<endl;
}

// 计算分割的GroundTruth数据，像素label
void App::GetGTPixelLabel()
{
	Mat gtImg = imread(gGTDir+m_ImageName+gGtExt);
	cout<<"Reading image:"<<gGTDir+m_ImageName+gGtExt<<endl;
	if (m_GtLabel.empty())
	{
		m_GtLabel = Mat(gtImg.size(), CV_32SC1);
	}
	assert(m_GtLabel.size() == m_SrcImage.size());
	for (int y = 0; y < gtImg.rows; y++)
	{
		for (int x = 0; x < gtImg.cols; x++)
		{
			Vec3b c = gtImg.at<Vec3b>(Point(x,y));
			//if (c[0]==255 && c[1]==255 && c[2]==255)	// 去除null区域
			//{
			//	m_GtLabel.at<int>(Point(x,y)) = 0;		// null区域label为0
			//	continue; 
			//}
			Scalar color(c[0],c[1],c[2]);
			vector<Scalar>::iterator iter = find(gtColorTable.begin(), gtColorTable.end(), color);
			if (iter != gtColorTable.end())	// find
			{
				int l = iter - gtColorTable.begin() + 1;	// Label从1开始
				m_GtLabel.at<int>(Point(x,y)) = l;
			} else {	// not find
				gtColorTable.push_back(color);
				m_GtLabel.at<int>(Point(x,y)) = gtColorTable.size();
			}
		}
	}
}

// 计算超像素区域的GroundTruth数据，超像素label
void App::GetGTRegionLabel()
{
	m_SpGtLabel.clear();
	m_SpGtLabel.resize(m_SpCount);
	vector<vector<int>> count(m_SpCount);
	for (int i = 0; i < m_SrcImage.rows; i++)
	{
		const int* ptrLabel = m_GtLabel.ptr<int>(i);
		const int* ptrSp = m_Superpixel.ptr<int>(i);
		for (int j = 0; j < m_SrcImage.cols; j++)
		{
			int spId = ptrSp[j];
			int label = ptrLabel[j];
			if (label < 0) label = 0;	// MSRC中，-1为NULL，置为0
			if (count[spId].size() == 0)
			{
				count[spId].resize(gClassNum, 0);
			}
			if (label >= gClassNum)
			{
				printf("Error: Label >= gClassNum");
				exit(0);
			}
			count[spId][label] ++;
		}
	}
	for (int i = 0; i < m_SpCount; i++)		// 取该超像素内出现最多的类别
	{
		if (count[i][0] > 0)	// 超像素内包含label为0的像素，则将该超像素label设置为0
		{
			m_SpGtLabel[i] = 0;
			continue;
		}
		int pos = (int) (max_element(count[i].begin(),count[i].end()) - count[i].begin());
		m_SpGtLabel[i] = pos;
	}
}

void App::SaveColorTable()
{
	ofstream fp(gImgSet+"colortable.txt");
	fp<<gtColorTable.size()<<endl;
	for (int i = 0; i < gtColorTable.size(); i++)
	{
		fp<<gtColorTable[i][0]<<" "<<gtColorTable[i][1]<<" "<<gtColorTable[i][2]<<endl;
	}
	fp.close();
}

void App::ReadColorTable()
{
	ifstream fp(gImgSet+"colortable.txt");
	if(!fp)
	{
		gtColorTable.clear();
		gtColorTable.resize(0);	// 第0位不用，设置为0,0,0
		return;
	}

	int size;
	fp>>size;
	gtColorTable.clear();
	gtColorTable.resize(size);	// 第0位不用，设置为0,0,0
	for (int i = 0; i < gtColorTable.size(); i++)
	{
		fp>>gtColorTable[i][0]>>gtColorTable[i][1]>>gtColorTable[i][2];
	}
	fp.close();
	/*gtColorTable.resize(10);
	for (int i = 0; i < gtColorTable.size(); i++)
	{
		gtColorTable[i] = ColorList[i+1];
	}*/
}


void App::showOversegLabel()
{
	Mat tmpImg = m_SrcImage.clone();
	Label2Result();
	//Mat tmpImg = m_ResultImg;
	addWeighted(tmpImg, 0.65, m_ResultImg, 0.35, 0, tmpImg);
	showSp(tmpImg, m_OverSeg, "OverSeg", cv::Vec3b(0,255,0));
	//imshow("Broadened Labels", tmpImg);
}
// 从用户交互m_InterMat中读取超像素样本标签信息
// 同一个
void App::GetLabelFromUser()
{
	map<int,int> OverSegLabel;	// 记录有标记的过分割区域
	m_UserInter.clear();
	m_SpLabel.assign(m_SpLabel.size(), 0);
	// 根据用户标记，得到样本标签
	vector<vector<int>> count(m_SpCount);	
	for (int i = 0; i < m_SrcImage.rows; i++)
	{
		const int* ptrLabel = m_InterMat.ptr<int>(i);	// 用户交互图
		const int* ptrSp = m_Superpixel.ptr<int>(i);	// 超像素编号图
		const int* ptrOSeg = m_OverSeg.ptr<int>(i);		// 过分割图
		for (int j = 0; j < m_SrcImage.cols; j++)
		{
			int spId = ptrSp[j];
			int label = ptrLabel[j];
			if (label > 0)
			{
				m_UserInter.insert(make_pair(spId, label));
				OverSegLabel.insert(make_pair(ptrOSeg[j], label));
				if (count[spId].size() == 0)
				{
					count[spId].resize(gClassNum, 0);
				}
				count[spId][label] ++;
				//m_SpLabel[ptrSp[j]] = ptrLabel[j];		// 应该取最多的类别
			}
		}
	}

	int num = 0;
	for (int i = 0; i < m_SpCount; i++)		// 取该超像素内出现最多的类别
	{
		if (count[i].size() > 0)
		{
			int pos = (int) (max_element(count[i].begin(),count[i].end()) - count[i].begin());
			m_SpLabel[i] = pos;
			num ++;
		}
	}

	//// 根据过分割区域，扩充样本数量
	for (int i = 0; i < m_SrcImage.rows; i++)
	{
		const int* ptrLabel = m_InterMat.ptr<int>(i);
		const int* ptrSp = m_Superpixel.ptr<int>(i);
		const int* ptrOSeg = m_OverSeg.ptr<int>(i);
		for (int j = 0; j < m_SrcImage.cols; j++)
		{
			if (ptrLabel[j] < 1)
			{
				int oSegId = ptrOSeg[j];
				map<int, int>::iterator iter = OverSegLabel.find(oSegId);
				if (iter != OverSegLabel.end() && !m_SpInBound[ptrSp[j]])			// 同一个过分割区域内存在已标记的超像素，且当前超像素未跨过过分割边界。
				{
					m_SpLabel[ptrSp[j]] = iter->second;	// 则将本超像素标记设置为已标记的那个, 未考虑同一过分割区域存在两个不同标记的情况
				}
			}
		}
	}

	cout<<"Sample number:"<<num<<endl;

	/*Mat tmp(m_SrcImage);
	for (int y = 0; y < tmp.rows; y++)
	{
	for (int x = 0; x < tmp.cols; x++)
	{
	int id = m_Superpixel.at<int>(Point(x,y));
	Vec3b& result = tmp.at<Vec3b>(Point(x,y));
	int label = m_SpLabel[id];
	result[0] = ColorList[label][0];
	result[1] = ColorList[label][1];
	result[2] = ColorList[label][2];
	}
	}
	imwrite("tmp.png", tmp);*/

	//showOversegLabel();
}

void App::doTrain()
{
	GetLabelFromUser();
	// add region to trainSet
	DataSet trainSet;
	//int i=0;
	//for (set<int>::iterator iter=m_SpId.begin(); iter != m_SpId.end(); iter++, i++)	// iter,i避免超像素编号不连续的情况，即m_SpLabel.size()!=m_FeatureSet.m_samples.size()
	//{
	//	if (m_SpLabel[*iter] > 0)
	//	{
	//		Sample s = m_FeatureSet.m_samples[i];
	//		s.y = m_SpLabel[*iter];
	//		trainSet.m_samples.push_back(s);
	//	}
	//}
	trainSet.m_samples.reserve(m_FeatureSet.m_numSamples);
	for (int i = 0; i < m_SpCount; i++)
	{
		if (m_SpLabel[i] > 0)
		{
			Sample s = m_FeatureSet.m_samples[i];
			s.y = m_SpLabel[i];
			trainSet.m_samples.push_back(s);
		}
	}

	trainSet.m_numSamples = trainSet.m_samples.size();
	trainSet.m_numFeatures = m_FeatureSet.m_numFeatures;
	trainSet.m_numClasses = gClassNum;
	trainSet.findFeatRange();
	cout<<"Trainset samples number:"<<trainSet.m_numSamples<<endl;
	if (trainSet.m_numSamples == 0)
	{
		return;
	}
	double trainErr = 1.0;
	int trainTime = 0;
	while (trainErr > 0.10)		// SLIC步长增加后，容错需要增大。
	{
		int err = train(model, trainSet, hp);
		trainErr = (double)err / trainSet.m_numSamples;
		trainTime ++;
		if (trainTime > 8)		// ..............................
		{
			break;
		}
	}

	cout<<"Training Finished! Training Error:"<<trainErr<<endl;

	//m_InterMat.setTo(0);	// 去掉上阶段的交互内容。

	doSegment();
}

void App::doSegment()
{
	doPredict();

	RefinePixelBoundary();
}

void App::doPredict()
{
	//std::cout<<"now predicting ... "<<endl;
	vector<Result> resultPredict;
	resultPredict = test(model, m_FeatureSet, hp);
	//std::cout<<"finish predicting ! "<<endl;

	assert(m_SpCount == resultPredict.size());
#pragma omp parallel for
	for (int i = 0; i < m_SpCount; i++)
	{
		map<int, int>::iterator it = m_UserInter.find(i);
		if (it == m_UserInter.end())						// 用户未标记该区域，
		{
			m_SpLabel[i] = resultPredict[i].prediction;	// 则将该区域标签设置为预测结果
		}
	}

	// 显示预测结果 
	Label2Result();
	//imshow("Prediction", m_ResultImg);
	//imwrite(gOutputDir+"predict_"+m_ImageName+".png", m_ResultImg);		

	#pragma omp parallel for
	for (int i = 0; i < m_SpCount; i++)
	{
		vector<double> tmp(gClassNum, 0);		// 第0个不用，第1...gClassNum-1个分别记录属于第1...gClassNum-1类的概率
		double allsum=0.0;
		map<int, int>::iterator it = m_UserInter.find(i);
		if (it != m_UserInter.end())	// 用户已标记该区域，
		{
			allsum = 1;
			tmp[it->second] = 1.0;		// 则将该区域属于该标记的概率设置为1
		} else {
			for (int j = 1; j < gClassNum; j++)	// 否则按预测概率
			{
				allsum += resultPredict[i].confidence[j];
				tmp[j] = resultPredict[i].confidence[j];
			}
		}
		for (int j = 1; j < gClassNum; j++)
		{
			tmp[j] /= allsum;
		}
		resultProb[i] = tmp;
	}
}

// 利用超像素标签，得到最终的结果图像
void App::Label2Result()
{
	for (int y = 0; y < m_SrcImage.rows; y++)
	{
		for (int x = 0; x < m_SrcImage.cols; x++)
		{
			Vec3b& result = m_ResultImg.at<Vec3b>(Point(x,y));
			int label = m_SpLabel[m_Superpixel.at<int>(Point(x,y))];
			result[0] = ColorList[label][0];										/////////////////////////////////
			result[1] = ColorList[label][1];
			result[2] = ColorList[label][2];
			/*result[0] = gtColorTable[label-1][0];
			result[1] = gtColorTable[label-1][1];
			result[2] = gtColorTable[label-1][2];*/
		}
	}
}

void App::RefineBoundary(const vector<vector<double>> prob, vector<int>& segLabels)
{
	float *data, *smooth;
	int nLabels = gClassNum;
	int numSample = prob.size();
	//writeVector<double>(prob, "prob.txt");

	double maxv;
	double refineWeight = 0.04;		// ***** user setting ...
	data = new float[numSample * nLabels];
	smooth = new float[nLabels * nLabels];
	maxv = -FLT_MAX;
	for (int i = 0; i < numSample; ++ i)
	{
		for (int j = 0; j < nLabels; ++ j)
		{
			assert(prob[i][j] <=1 && prob[i][j] >= 0);
			double d = -(log(prob[i][j]+1E-20)*refineWeight);
			data[i*nLabels + j] = d;
			if (data[i*nLabels + j] > maxv)
			{
				maxv = data[i*nLabels + j];
			}
		}
	} 
	printf("data maxv: %lf\n", maxv);

	for (int i = 0; i < nLabels; ++ i)
	{
		smooth[i + i*nLabels] = 0;
		for (int j = i+1; j < nLabels; ++ j)
		{
			smooth[i + j*nLabels] = 1;
			smooth[j + i*nLabels] = 1;
		}
	}

	int node1, node2;
	double w;

	try{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(numSample, nLabels);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);


		for (int i = 0; i < m_RegionDis.size(); ++ i)
		{
			node1 = m_RegionDis[i].regionId1;
			node2 = m_RegionDis[i].regionId2;
			w = m_RegionDis[i].distance;
			/*assert(w <= 1);
			if (w > 1)
			{
			w = 1;
			}*/

			gc->setNeighbors(node1,node2,w);	
		}		
		gc->setVerbosity(1);
		//gc->swap(-1);
		gc->expansion(5);

		for ( int  i = 0; i < numSample; i++ )
		{
			segLabels[i] = gc->whatLabel(i);
		}

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete []data;
	delete []smooth;
}


void App::RefinePixelBoundary()
{
	float *data, *smooth;
	int nLabels = gClassNum;
	int numSample = m_SrcImage.rows * m_SrcImage.cols;
	//writeVector<double>(prob, "prob.txt");

	double maxv;
	double refineWeight = weight;		// ***** user setting ...
	data = new float[numSample * nLabels];
	smooth = new float[nLabels * nLabels];
	maxv = -FLT_MAX;
	for (int y = 0; y < m_SrcImage.rows; y++)
	{
		for (int x = 0; x < m_SrcImage.cols; x++)
		{
			int id = y*m_SrcImage.cols + x;		// 像素编号
			int rid = m_Superpixel.at<int>(Point(x, y));	// 所在超像素编号
			for (int j = 0; j < nLabels; j++)
			{
				assert(resultProb[rid][j] <=1 && resultProb[rid][j] >= 0);
				double d = -(log(resultProb[rid][j]+1E-20)*refineWeight);
				data[id*nLabels + j] = d;
				if (d > maxv)
				{
					maxv = d;
				}
			}
		}

	}
	//printf("data maxv: %lf\n", maxv);

	for (int i = 0; i < nLabels; ++ i)
	{
		smooth[i + i*nLabels] = 0;
		for (int j = i+1; j < nLabels; ++ j)
		{
			smooth[i + j*nLabels] = 1;
			smooth[j + i*nLabels] = 1;
		}
	}

	if (m_Label.empty())
	{
		m_Label.create(m_SrcImage.size(), CV_32SC1);
	}
	try{
		gc->setDataCost(data);

		gc->expansion(5);

		for ( int  i = 0; i < numSample; i++ )
		{
			m_Label.at<int>(i/m_SrcImage.cols, i%m_SrcImage.cols) = gc->whatLabel(i);
		}

		//delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete []data;
	delete []smooth;

	// 根据平滑结果，计算超像素样本标签，同时修改m_ResultImg
	vector<vector<int>> count(m_SpCount);
	for (int i = 0; i < m_SrcImage.rows; i++)
	{
		const int* ptrLabel = m_Label.ptr<int>(i);
		const int* ptrSp = m_Superpixel.ptr<int>(i);
		for (int j = 0; j < m_SrcImage.cols; j++)
		{
			int spId = ptrSp[j];
			int label = ptrLabel[j];
			if (count[spId].size() == 0)
			{
				count[spId].resize(nLabels, 0);
			}
			count[spId][label] ++;

			Vec3b& result = m_ResultImg.at<Vec3b>(Point(j,i));
			result[0] = gtColorTable[label-1][0];
			result[1] = gtColorTable[label-1][1];
			result[2] = gtColorTable[label-1][2];

		}
	}
	for (int i = 0; i < m_SpCount; i++)		// 取该超像素内出现最多的类别
	{
		int pos = (int) (max_element(count[i].begin(),count[i].end()) - count[i].begin());
		m_SpLabel[i] = pos;
	}

	//UpdateInterWindow();
	ShowResult();
	cout<<"Finished!"<<endl;
}


void App::TrainAll()
{
	// add region to trainSet
	DataSet trainSet;
	int i=0;
	//for (set<int>::iterator iter=m_SpId.begin(); iter != m_SpId.end(); iter++, i++)	// iter,i避免超像素编号不连续的情况，即m_SpLabel.size()!=m_FeatureSet.m_samples.size()
	//{
	//	if (m_SpLabel[*iter] > 0)
	//	{
	//		Sample s = m_FeatureSet.m_samples[i];
	//		s.y = m_SpLabel[*iter];
	//		trainSet.m_samples.push_back(s);
	//	}
	//}
	vector<int> count(gClassNum, 0);
	trainSet.m_samples.reserve(m_FeatureSet.m_numSamples);
	for (int i = 0; i < m_SpCount; i++)
	{
		if (m_SpLabel[i] > 0)
		{
			Sample s = m_FeatureSet.m_samples[i];
			s.y = m_SpLabel[i];
			count[s.y] ++;
			trainSet.m_samples.push_back(s);
		}
		else {
			continue;
			//printf("Error:Label<=0");
		}
	}

	/*ofstream ofcount(gImgSet+"trainingdata.csv", ios::out|ios::app);
	for (int i = 0; i < gClassNum; i++)
	{
		ofcount<<count[i]<<",";
	}
	ofcount<<endl;
	ofcount.close();*/

	/*Mat tmp(m_SrcImage);
	for (int y = 0; y < tmp.rows; y++)
	{
	for (int x = 0; x < tmp.cols; x++)
	{
	int id = m_Superpixel.at<int>(Point(x,y));
	Vec3b& result = tmp.at<Vec3b>(Point(x,y));
	int label = m_SpLabel[id];
	result[0] = ColorList[label][0];
	result[1] = ColorList[label][1];
	result[2] = ColorList[label][2];
	}
	}
	imwrite("tmp.png", tmp);*/

	trainSet.m_numSamples = trainSet.m_samples.size();
	trainSet.m_numFeatures = m_FeatureSet.m_numFeatures;
	trainSet.m_numClasses = gClassNum;
	trainSet.findFeatRange();

	double trainErr = 1.0;
	int trainTime = 0;
	while (trainErr > 0.1)		// SLIC步长增加后，容错需要增大。
	{
		int err = train(model, trainSet, hp);
		trainErr = (double)err / trainSet.m_numSamples;
		trainTime ++;
		if (trainTime > 1)	//......................................................
		{
			break;
		}
	}

	cout<<"Training Finished! Training Error:"<<trainErr<<endl;
}

void App::SaveModel()
{
	FILE* fp;
	fp = fopen("model.dat", "w");
	model->RecordModel(fp);
	fclose(fp);
	cout<<"Model Saved!"<<endl;
}

void App::LoadModel()
{
	FILE* fp;
	fp = fopen("model.dat", "r");
	if (!fp)
	{
		cout<<"Failed to open 'model.dat'"<<endl;
	}
	model->LoadModel(fp);
	fclose(fp);
	cout<<"Model Loaded!"<<endl;
}
