#include "superpixel.h"


Superpixel::Superpixel(const Mat& _inImg)
{
	m_Image = _inImg.clone();
}


Superpixel::~Superpixel(void)
{
}

void Superpixel::GetSeeds(int NR_SUPERPIXELS=266)
{
	IplImage* img = new IplImage(m_Image);

	int width = img->width;
	int height = img->height;
	int sz = height*width;

	//printf("Image loaded %d\n",img->nChannels);

	UINT* ubuff = new UINT[sz];
	UINT* ubuff2 = new UINT[sz];
	UINT* dbuff = new UINT[sz];
	UINT pValue;
	UINT pdValue;
	char c;
	UINT r,g,b,d,dx,dy;
	int idx = 0;
	for(int j=0;j<img->height;j++)
	{
		for(int i=0;i<img->width;i++)
		{
			if(img->nChannels == 3)
			{
				// image is assumed to have data in BGR order
				b = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
				g = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels+1];
				r = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels+2];
				//if (d < 128) d = 0;
				pValue = b | (g << 8) | (r << 16);
			}
			else if(img->nChannels == 1)
			{
				c = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
				pValue = c | (c << 8) | (c << 16);
			}
			else
			{
				printf("Unknown number of channels %d\n", img->nChannels);
				return ;
			}          
			ubuff[idx] = pValue;
			ubuff2[idx] = pValue;
			idx++;
		}
	}

	/********************************************
	* SEEDS SUPERPIXELS                       *
	*******************************************/
	int NR_BINS = 5; // Number of bins in each histogram channel

	//printf("Generating SEEDS with %d superpixels\n", NR_SUPERPIXELS);
	SEEDS seeds(width, height, 3, NR_BINS);

	// SEEDS INITIALIZE
	int nr_superpixels = NR_SUPERPIXELS;

	// NOTE: the following values are defined for images from the BSD300 or BSD500 data set.
	// If the input image size differs from 480x320, the following values might no longer be 
	// accurate.
	// For more info on how to select the superpixel sizes, please refer to README.TXT.

	int seed_width = 3; int seed_height = 4; int nr_levels = 4;
	if (width >= height)
	{
		if (nr_superpixels == 600) {seed_width = 2; seed_height = 2; nr_levels = 4;}
		if (nr_superpixels == 400) {seed_width = 3; seed_height = 2; nr_levels = 4;}
		if (nr_superpixels == 266) {seed_width = 3; seed_height = 3; nr_levels = 4;}
		if (nr_superpixels == 200) {seed_width = 3; seed_height = 4; nr_levels = 4;}
		if (nr_superpixels == 150) {seed_width = 2; seed_height = 2; nr_levels = 5;}
		if (nr_superpixels == 100) {seed_width = 3; seed_height = 2; nr_levels = 5;}
		if (nr_superpixels == 50)  {seed_width = 3; seed_height = 4; nr_levels = 5;}
		if (nr_superpixels == 25)  {seed_width = 3; seed_height = 2; nr_levels = 6;}
		if (nr_superpixels == 17)  {seed_width = 3; seed_height = 3; nr_levels = 6;}
		if (nr_superpixels == 12)  {seed_width = 3; seed_height = 4; nr_levels = 6;}
		if (nr_superpixels == 9)  {seed_width = 2; seed_height = 2; nr_levels = 7;}
		if (nr_superpixels == 6)  {seed_width = 3; seed_height = 2; nr_levels = 7;}
	}
	else
	{
		if (nr_superpixels == 600) {seed_width = 2; seed_height = 2; nr_levels = 4;}
		if (nr_superpixels == 400) {seed_width = 2; seed_height = 3; nr_levels = 4;}
		if (nr_superpixels == 266) {seed_width = 3; seed_height = 3; nr_levels = 4;}
		if (nr_superpixels == 200) {seed_width = 4; seed_height = 3; nr_levels = 4;}
		if (nr_superpixels == 150) {seed_width = 2; seed_height = 2; nr_levels = 5;}
		if (nr_superpixels == 100) {seed_width = 2; seed_height = 3; nr_levels = 5;}
		if (nr_superpixels == 50)  {seed_width = 4; seed_height = 3; nr_levels = 5;}
		if (nr_superpixels == 25)  {seed_width = 2; seed_height = 3; nr_levels = 6;}
		if (nr_superpixels == 17)  {seed_width = 3; seed_height = 3; nr_levels = 6;}
		if (nr_superpixels == 12)  {seed_width = 4; seed_height = 3; nr_levels = 6;}
		if (nr_superpixels == 9)  {seed_width = 2; seed_height = 2; nr_levels = 7;}
		if (nr_superpixels == 6)  {seed_width = 2; seed_height = 3; nr_levels = 7;}
	}
	seeds.initialize(seed_width, seed_height, nr_levels);

	//clock_t begin = clock();

	seeds.update_image_ycbcr(ubuff);

	seeds.iterate();

	/*clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	printf("    elapsed time=%lf sec\n", elapsed_secs);*/

	//printf("SEEDS produced %d labels\n", seeds.count_superpixels());
	n_Seeds = seeds.count_superpixels();

	int* labels = (int*)seeds.get_labels();
	int2Mat(labels, width, height, m_Seeds);

	delete[] ubuff;
	//printf("Done!\n");
}

void Superpixel::GetSlic(int step=3)
{
	SLIC slicsp;
	int h=m_Image.rows, w=m_Image.cols;
	UINT* imgData = new UINT[h*w];
	bgr2Int(m_Image, imgData);
		
	int numLabels;
	int *klabels = new int[h*w];
	slicsp.PerformSLICO_ForGivenStepSize(imgData, w, h, klabels, numLabels, step, NULL);
	int2Mat(klabels, w, h, m_Slic);
	n_Slic = numLabels;

	delete[] klabels;
	delete[] imgData;
}

void Superpixel::GetGraphBased(double sigma, double k, int min_size)
{
	Mat img3f;
	m_Image.convertTo(img3f, CV_32FC3, 1.0/255);
	cvtColor(img3f, img3f, COLOR_BGR2Lab);
	n_GraphBased = SegmentImage(img3f, m_GraphBased, sigma, k, min_size);

}

void Superpixel::ShowSlic(const cv::Vec3b _color)
{
	int w = m_Image.cols, h = m_Image.rows;
	int label, top, bot, left, right;//, topl, topr, botl, botr;
	Mat showImg = m_Image.clone();
	Mat mask(m_Image.size(), CV_8UC1);
	mask = 0;

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = m_Slic.at<int>(i, j);
			top = m_Slic.at<int>( (i-1 > -1 ? i-1 : 0), j );
			bot = m_Slic.at<int>( (i+1 < h ? i+1 : h-1), j );
			left = m_Slic.at<int>( i, (j-1>-1 ? j-1 : 0) );
			right = m_Slic.at<int>( i, (j+1<w ? j+1 : w-1) );
			if (label!=top || label!=left || label!=right || label!=bot)
				mask.at<uchar>(i, j) = 255;
				//showImg.at<cv::Vec3b>(i, j) = _color;
		}
	}
	showImg.setTo(_color, mask);
	cv::imshow("slic", showImg);
}

void Superpixel::ShowSeeds(const cv::Vec3b _color)
{
	int w = m_Image.cols, h = m_Image.rows;
	int label, top, bot, left, right;//, topl, topr, botl, botr;
	cv::Mat showImg = m_Image.clone();

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = m_Seeds.at<int>(i, j);
			top = m_Seeds.at<int>( (i-1 > -1 ? i-1 : 0), j );
			bot = m_Seeds.at<int>( (i+1 < h ? i+1 : h-1), j );
			left = m_Seeds.at<int>( i, (j-1>-1 ? j-1 : 0) );
			right = m_Seeds.at<int>( i, (j+1<w ? j+1 : w-1) );
			if (label!=top || label!=left || label!=right || label!=bot)
				showImg.at<cv::Vec3b>(i, j) = _color;
		}
	}
	cv::imshow("seeds", showImg);
}

void Superpixel::ShowGraphBased(const cv::Vec3b _color)
{
	int w = m_Image.cols, h = m_Image.rows;
	int label, top, bot, left, right;//, topl, topr, botl, botr;
	cv::Mat showImg = m_Image.clone();

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = m_GraphBased.at<int>(i, j);
			top = m_GraphBased.at<int>( (i-1 > -1 ? i-1 : 0), j );
			bot = m_GraphBased.at<int>( (i+1 < h ? i+1 : h-1), j );
			left = m_GraphBased.at<int>( i, (j-1>-1 ? j-1 : 0) );
			right = m_GraphBased.at<int>( i, (j+1<w ? j+1 : w-1) );
			if (label!=top || label!=left || label!=right || label!=bot)
				showImg.at<cv::Vec3b>(i, j) = _color;
		}
	}
	cv::imshow("GraphBased", showImg);
}