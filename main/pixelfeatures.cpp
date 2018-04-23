#include "stdafx.h"
#include "pixelfeatures.h"
#include "features\fdog.h"

int PixelFeatures::s_Dim = 5+17+2;
int PixelFeatures::s_textureDim = 17;

PixelFeatures::PixelFeatures(void)
{
	lbpCaled = false;
	etfCaled = false;
	salCaled = false;
	textureCaled = false;
	locationCaled = false;
}


PixelFeatures::~PixelFeatures(void)
{
	allFeatureVec.clear();
}

PixelFeatures::PixelFeatures(const Mat& _image)
{
	SetImage(_image);
	lbpCaled = false;
	etfCaled = false;
	salCaled = false;
	textureCaled = false;
	locationCaled = false;
}

//http://blog.csdn.net/dujian996099665/article/details/8886576
void PixelFeatures::CalLBP()
{
	//cout<<"caluating LBP..."<<endl;
	LBPFeature.create(image.size(), CV_8UC1); //////////************
    // zero the result matrix  
    LBPFeature.setTo(0);  
	Mat src(image.size(), CV_8UC1);
	cvtColor(image, src, CV_BGR2GRAY);
  
    // calculate patterns  
//#pragma omp parallel for
    for(int i=0;i<m_height;i++) {  
        //cout<<endl;  
        for(int j=0;j<m_width;j++) 
		{  
			uchar center = src.at<uchar>(i,j);  
			uchar ll = (i-1)>=0 ? src.at<uchar>(i-1,j  ) : center;	//复制中央像素到边界外的像素
			uchar lt = (i-1>=0 && j-1>=0) ? src.at<uchar>(i-1,j-1) : center;
			uchar tt = (j-1)>=0 ? src.at<uchar>(i ,j-1) : center;
			uchar rt = (j-1>=0 && i+1<m_height) ? src.at<uchar>(i+1,j-1) : center;
			uchar rr = (i+1)<m_height ? src.at<uchar>(i+1,j) : center;
			uchar rd = (i+1<m_height && j+1<m_width) ? src.at<uchar>(i+1,j+1) : center;
			uchar dd = (j+1<m_width) ? src.at<uchar>(i  ,j+1) : center;
			uchar ld = (i-1>=0 && j+1<m_width) ? src.at<uchar>(i-1,j+1) : center;
            unsigned char code = 0;  
            code |= (lt >= center) << 7;  
            code |= (ll >= center) << 6;  
            code |= (ld >= center) << 5;  
            code |= (dd >= center) << 4;  
            code |= (rd >= center) << 3;  
            code |= (rr >= center) << 2;  
            code |= (rt >= center) << 1;  
            code |= (tt >= center) << 0;  
  
            LBPFeature.at<uchar>(i,j) = code;  
            //cout<<(int)code<<" ";  
            //cout<<(int)code<<endl;  
        }  
	}
	//imwrite	("lbp.jpg", LBPFeature);
	LBPFeature.convertTo(LBPFeature, CV_32FC1, 1.0/255);
	allFeatureVec[0] = LBPFeature;
	lbpCaled = true;
}

void PixelFeatures::CalETF()
{
	//cout<<"caluating ETF..."<<endl;
	imatrix img(m_width, m_height);
	Mat src(image.size(), CV_8UC1);
	cvtColor(image, src, CV_BGR2GRAY);
	for(int i=0;i<m_width;i++)
	{
		for(int j=0;j<m_height;j++)
		{
			img[i][j] = (int)src.at<uchar>(j,i);
			//cout<<img[i][j]<<"    ";
		}
	}
	int image_x = img.getRow();
	int image_y = img.getCol();
	e.init(image_x, image_y);
	e.set2(img); // get gradients from gradient map
	e.Smooth(4, 2);

	double tao = 0.999;
	double thres = 0.9;
	GetFDoG(img, e, 1.0, 3.0, tao); 	
	GrayThresholding(img, thres); 

	Mat mag(image.size(), CV_32FC1), ang(image.size(), CV_32FC1), cld(image.size(), CV_32FC1);
//#pragma omp parallel for
	for(int i=0; i<m_pixelNum; i++)
	{
		int x=i%m_width, y=i/m_width;
		Vect tmp = e.get(x, y);
		mag.at<float>(y, x) = tmp.mag;
		double angle = atan2(tmp.ty, tmp.tx+0.000000001);//  -pi~+pi
		ang.at<float>(y, x) = angle;
		cld.at<float>(y, x) = img[i%m_width][i/m_width] / 255.0;

	}
	allFeatureVec[1] = mag;
	allFeatureVec[2] = ang;
	allFeatureVec[3] = cld;

	etfCaled = true;
}

void PixelFeatures::CalSaliency()
{
	//cout<<"caluating Saliency..."<<endl;
	Saliency sal;
	Mat saliency(image.size(), CV_32FC1);

	IplImage* displayImage = sal.Get(&IplImage(image));

//#pragma omp parallel for
	for(int row=0;row<m_height;row++)
	{
		for(int col=0;col<m_width;col++)
		{
			saliency.at<float>(row, col) = cvGetReal2D(displayImage,row,col);
		}
	}
	//cvSaveImage("saliency.jpg",displayImage);
	cvReleaseImage(&displayImage);

	saliency.convertTo(saliency, CV_32F, 1.0/255);
	allFeatureVec[4] = saliency;
	salCaled = true;
}

// 计算滤波器响应，在CIE Lab通道计算
//   L、a、b通道上分别计算三个高斯响应（sigma=1、2、4），共9个
//   L通道上的四个拉普拉斯高斯响应（sigma=1、2、4、8），共4个
//   L通道上，x、y方向上分别计算两个高斯响应（sigma=2、4），共4个
// responNum列，regionNum行，每一列为一个滤波器响应。
void PixelFeatures::CalTexture()
{
	//cout<<"caluating Texture..."<<endl;
	assert(image.channels() == 3);		// 要求是3个通道

	Mat Lab, imgf32;
	image.convertTo(imgf32, CV_32F, 1.0/255);
	cvtColor(imgf32, Lab, CV_BGR2Lab);
	std::vector<cv::Mat> LabVec(Lab.channels());
	split(Lab, LabVec);
	//cout<<mean(Lab)<<endl;

	int winSize = 0;	// 滤波窗口大小为0 ，由sigma计算得到核大小
	std::vector<cv::Mat> responseMat;
	double sigma1[3] = {1,2,4};
//#pragma omp parallel for			// 此处不可用并行，responseMat冲突
	for (int i = 0; i < 3; i++)
	{
		Mat res;
		GaussianBlur(Lab, res, Size(winSize, winSize), sigma1[i]);		// L、a、b通道的Gaussian(sigma=1,2,4)
		//res.convertTo(res, CV_32F, 1.0/255);
		std::vector<cv::Mat> resLab(Lab.channels());
		split(res, resLab);
		//imwrite("guassianBlur_L_0.jpg", resLab[0]);
		//cout<<mean(res)<<endl;
		responseMat.insert(responseMat.end(), resLab.begin(), resLab.end());

		resLab.clear();	// 重要！否则下一次split将修改resLab的内容，而insert到responseMat中的内容实际上并未新建Mat，将导致responseMat中数据被修改
						// 这一步强行使insert到responseMat中的数据保留。
		Laplacian(res, res, res.depth());	// L、a、b通道的Laplacian of Gaussian(sigma=1,2,4)
		split(res, resLab);
		//imwrite("LoG_L_0.jpg", resLab[0]);
		//cout<<mean(res).val[0]<<endl;
		responseMat.push_back(resLab[0]);	// 只保存L通道的LoG
	}

	Mat res;
	GaussianBlur(LabVec[0], res, Size(winSize, winSize), 8);	// L通道的Gaussian，sigma=8
	Laplacian(res, res, CV_32FC1);	// L通道的LoG，sigma=8
	//res.convertTo(res, CV_32F, 1.0/255);
	responseMat.push_back(res);
	//cout<<mean(res).val[0]<<endl;

	double sigma2[2] = {2, 4};
//#pragma omp parallel for
	// x-aligned and y-aligned first order derivative
	for (int i = 0; i < 2; i++)
	{
		Mat gauRes;
		GaussianBlur(LabVec[0], gauRes, Size(winSize, winSize), sigma2[i]);

		Mat rx, ry;
		int scale = 1;
		int delta = 0;
		int ddepth = CV_32FC1;
		int sobelSize = 3;	//sobel核，默认为3

		Sobel( gauRes, rx, ddepth, 1, 0, sobelSize, scale, delta, BORDER_DEFAULT );
		Sobel( gauRes, ry, ddepth, 0, 1, sobelSize, scale, delta, BORDER_DEFAULT );
		//imwrite("4.png", rx);
		//rx.convertTo(rx, CV_32F, 1.0/255);
		//ry.convertTo(ry, CV_32F, 1.0/255);
		responseMat.push_back(rx);
		responseMat.push_back(ry);
	}
	//for (int i = 0; i < 2; i++)
	//{
	//	Mat rx, ry;
	//	int scale = 1;
	//	int delta = 0;
	//	int ddepth = CV_32FC1;
	//	int sobelSize = 3;	//sobel核，默认为3

	//	Sobel( LabVec[0], rx, ddepth, 1, 0, sobelSize, scale, delta, BORDER_DEFAULT );
	//	GaussianBlur(rx, rx, Size(winSize, winSize), sigma2[i]);

	//	Sobel( LabVec[0], ry, ddepth, 0, 1, sobelSize, scale, delta, BORDER_DEFAULT );
	//	GaussianBlur(ry, ry, Size(winSize, winSize), sigma2[i]);
	//	imwrite("3.png", rx);

	//	rx.convertTo(rx, CV_32F, 1.0/255);
	//	ry.convertTo(ry, CV_32F, 1.0/255);
	//	responseMat.push_back(rx);
	//	responseMat.push_back(ry);
	//}

//#pragma omp parallel for
	for (int i = 0; i < s_textureDim; i++)
	{
		allFeatureVec[5+i] = responseMat[i];
	}

	textureCaled = true;
}


void PixelFeatures::CalLocation()
{
	Mat lx(image.size(), CV_32FC1), ly(image.size(), CV_32FC1);

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			lx.at<float>(Point(x,y)) = (double)x / m_width;
			ly.at<float>(Point(x,y)) = (double)y / m_height;
		}
	}

	allFeatureVec[22] = lx;
	allFeatureVec[23] = ly;
	locationCaled = true;
}

void PixelFeatures::SetImage(const Mat& _image)
{
	image = _image.clone();
	m_height = image.rows;
	m_width = image.cols;
	m_pixelNum = m_height*m_width;

	allFeatureVec.clear();
	allFeatureVec.resize(s_Dim);
}

int PixelFeatures::GetSampleNum()
{
	return m_pixelNum;
}

vector<Mat> PixelFeatures::GetAllFeatureVec()
{
	if (image.empty())
		return allFeatureVec;
	if (!lbpCaled)
		CalLBP();
	if (!etfCaled)
		CalETF();	
	if (!salCaled)
		CalSaliency();
	if (!textureCaled)
		CalTexture();
	if (!locationCaled)
		CalLocation();

	return allFeatureVec;
}

FeatureVector PixelFeatures::GetAllFeatures()
{
	if (fv._nSample == m_pixelNum)
	{
		return fv;
	}

	fv.clearAll();
	GetAllFeatureVec();
	vector<vector<double>> tmp;
	tmp.resize(m_pixelNum, vector<double>(s_Dim, 0));

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			int id = x + y*m_width;
			for (int i = 0; i < allFeatureVec.size(); i++)
			{
				tmp[id][i] += allFeatureVec[i].at<float>(Point(x, y));
			}
		}
	}

	fv.addColumn(tmp);
	//fv.writeToFile("features2.txt");
	return fv;
}