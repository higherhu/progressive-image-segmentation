// main.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "app.h"
#include <time.h>


// Random numbers
double getRand() {
	static bool seedFlag = false;

	if (!seedFlag) {
		srand(time(0));
		seedFlag = true;
	}
	return ((double) rand()) / (RAND_MAX);
}

template<typename T>
void randVector(vector<T>& outVect) {
	int randIndex, size = outVect.size();
	T tempIndex;

	for (int nFeat = 0; nFeat < size; nFeat++) {
		randIndex = (int) floor(((double) size) * getRand());// + nFeat;
		if (randIndex == size) {
			randIndex--;
		}
		tempIndex = outVect[nFeat];
		outVect[nFeat] = outVect[randIndex];
		outVect[randIndex] = tempIndex;
	}
}

const Scalar ColorList[] = {Scalar(255-0,255-0,255-0),
	Scalar(0,0,255), 
	Scalar(255,0,0),
	Scalar(0,255,128),
	Scalar(128,0,128),
	Scalar(128,128,0),
	Scalar(64,0,128),
	Scalar(128,192,128),
	Scalar(0,64,64),
	Scalar(0,64,192)};

const string gExperDir = "./expr/";
const string gOutputDir = "./out/";
string gImgSet;
string gImgDir;
string gGTDir;
string gDataDir;		// slic step
const string gImgExt = ".jpg";					// ͼ���׺
const string gGtExt = ".png";					// GrouhdTurthͼ���׺
const string gSpExt = gImgExt+".txt";				// �������ļ���׺
const string gSpLblExt = gImgExt+".labels";			// ������label�ļ���׺
const string gFeatureExt = gImgExt+".features";		// �����ļ���׺
const string gOversegExt = ".graphbased.txt";		// ���ָ��ļ���׺
const string gLblExt = ".txt";					// ����label�ļ���׺
const string gWinInter = "Interaction Window"; //��������
const string gWinResult = "Result";				//�������
const int gClassNum = 10;		//�����+1�� 1,2,3,4,5,6,7,8,9,����ǩ>0 

App* app;

static void on_mouse( int event, int x, int y, int flags, void* param )
{
	app->onMouseCallback( event, x, y, flags, param );
}

void calFeatures()
{
	fstream fp(gImgDir+"../bintestlist.txt", ios::in);
	string filename;
	while (fp>>filename)
	{
		cout<<"--dealing with:"<<filename<<endl;
		app->SetImageName(filename);
	}
	fp.close();

	fp.open(gImgDir+"../bintrainlist.txt", ios::in);
	while (fp>>filename)
	{
		cout<<"--dealing with:"<<filename<<endl;
		app->SetImageName(filename);
	}
	fp.close();
}

void InteractiveSegmenataion(int argc, char* argv[])
{

	ifstream file(gImgSet+"imglist.txt", ios::in);	// ͼ���ļ�����������׺
	string imgName;
	vector<string> imgList;

	while (file>>imgName)
	{
		imgList.push_back(imgName);
	}
	file.close();

	app = new App();

	int imgIndex = 0;
	imgName = imgList[imgIndex];

	cv::namedWindow(gWinInter);
	setMouseCallback(gWinInter, on_mouse);

	app->SetImageName(imgName);
	app->PreProcessing();
	app->UpdateInterWindow();
	app->InitModel();	
	app->ShowResult();

	std::cout<<"input your operation: ";	
	for(;;)
	{
		int c = cv::waitKey(0);
		switch( (char) c )
		{
		case '\x1b':	//�˳�
			std::cout << "Finished!" << endl;
			goto exit;
			break;
		case 'g':	// �����޽���ʽ��ѧϰ��Ԥ��
			app->ReleaseMat();
			app->Run();
			break;
		case 'b':	// ���� model
			app->SaveModel();
			break;
		case 'l':
			app->LoadModel();
			break;
		case 'f':	// ��������
			calFeatures();
			break;
		case 'r':	// reset
			app->reset();
			break;
		case 'p':	//predict
			app->doSegment();
			app->ShowResult();
			break;
		case 't':	//train
			app->doTrain();
			break;
		case 'i':	//input another image
			std::cout<<"input the new image name:";
			std::cin>>imgName;
			//app->reset();
			app->ReleaseMat();
			if (app->SetImageName(imgName))
			{
				app->PreProcessing();
				app->UpdateInterWindow();
				std::cout<<"predicting ... "<<endl;
				app->doSegment();
				app->ShowResult();
			}
			break;
		case 'n':	//next image
			imgIndex ++;
			if (imgIndex >=imgList.size())
			{
				std::cout<<"All images segmented!"<<endl;
				goto exit;
			}
			imgName = imgList[imgIndex];
			std::cout<<"the next image name:"<<imgName<<endl;
			//app->reset();
			app->ReleaseMat();
			if (app->SetImageName(imgName))
			{
				app->PreProcessing();

				std::cout<<"predicting ... ";
				app->doSegment();
				app->ShowResult();
				std::cout<<"Finished!"<<endl;
			}
			break;
		case 'w':	// re-weight������Ȩ��
			std::cout<<"input the new weight:";
			double weight;
			std::cin>>weight;
			app->weight = weight;
			std::cout<<"ReSegment..."<<endl;
			app->RefinePixelBoundary();
			break;
		case 's':	// �ύ��ͼ��ķָ���
			std::cout<<"submitting the image ... "<<endl;
			app->TrainAll();
			break;
		case '-':	// �߿�-1
			if (app->drawLineWidth > 2)
				app->drawLineWidth --;
			cout<<"Current line width:"<<app->drawLineWidth<<endl;
			break;
		case '+':	// �߿�+1
			if (app->drawLineWidth < 50)
				app->drawLineWidth ++;
			cout<<"Current line width:"<<app->drawLineWidth<<endl;
			break;
		default:
			if (c > '0' && c <= '9')	// 1~9
			{
				int type = (int)(c-'0');
				app->m_ChooseType = type;
				std::cout<<"current type:"<<type<<endl;
			}
		}
	}

exit:
	return;
	//imwrite(imgName+".bmp", app->m_ResultImg);
	delete app;
}

int main(int argc, char* argv[])
{
	
	if (argc >= 2)
	{
		gImgSet = argv[1];
		cout<<"gImgSet: "<<gImgSet<<endl;
	}
	else
	{
		cout<<"Input the gImgSet:";
		cin>>gImgSet;
	}

	gImgSet = gImgSet + "/";
	gImgDir = gImgSet+"Images/";
	gGTDir = gImgSet+"GroundTruth/";
	gDataDir = gImgSet+"slic_7/";

	InteractiveSegmenataion(argc, argv);

	return 0;
}

