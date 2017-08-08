#include "GuidedFilter.h"
#include "roughness.h"
#include "WaterFlow.h"
#include "Dark.h"
#include "imagePro.h"


int main()
{
	IplImage* inputImage = cvLoadImage("E:\\上海中医药大学合作项目\\projects\\舌像特征提取\\裂纹\\data\\17.jpg",CV_LOAD_IMAGE_COLOR);
	IplImage* grayImage = cvCreateImage(cvGetSize(inputImage),8,1);
	cvCvtColor(inputImage,grayImage,CV_BGR2GRAY);

	int areaMin = 15;	//噪声的最大面积

	/*IplImage* lab = cvCreateImage(cvGetSize(inputImage),8,3);
	IplImage* l = cvCreateImage(cvGetSize(inputImage),8,1);
	IplImage* a = cvCreateImage(cvGetSize(inputImage),8,1);
	IplImage* b = cvCreateImage(cvGetSize(inputImage),8,1);
	cvCvtColor(inputImage,lab,CV_BGR2Lab);
	cvSplit(lab,l,a,b,NULL);*/

	/*IplImage* r = cvCreateImage(cvGetSize(inputImage),8,1);
	IplImage* g = cvCreateImage(cvGetSize(inputImage),8,1);
	IplImage* b = cvCreateImage(cvGetSize(inputImage),8,1);
	cvSplit(inputImage,b,g,r,NULL);*/


	GuidedFilter guidedfilter = GuidedFilter(grayImage,grayImage,5,0.001);
	IplImage* q = guidedfilter.Getq();

	IplImage* qq = cvCreateImage(cvGetSize(q),8,1);
	for (int y=0;y<q->height;y++)
	{
		for (int x=0;x<q->width;x++)
		{
			int value = cvRound(255*cvGetReal2D(q,y,x));
			cvSetReal2D(qq,y,x,value);
		}
	}

	//计算Roughness
	Roughness roughness = Roughness(qq);
	roughness.CaculateRoughness();
	IplImage* imageRoughness = cvCloneImage(roughness.GetImageRoughness());

	//计算暗度
	Dark dark = Dark(qq);
	dark.CaculateDark();
	IplImage* DarkImageBI = cvCloneImage(dark.GetDarkImageBi());
	cvShowImage("darkBi",DarkImageBI);

	//计算线响应
	WaterFlow waterFlow = WaterFlow(qq,imageRoughness);
	waterFlow.processFlow();
	IplImage* image_L = cvCloneImage(waterFlow.GetL());



	cvShowImage("in",inputImage);
	cvShowImage("gray",grayImage);
	cvShowImage("qq",qq);
	cvShowImage("LBi",image_L);
	cvShowImage("mrough",imageRoughness);

	//结合暗通道消除亮噪声
	for (int y=0;y<image_L->height;y++)
	{
		for (int x=0;x<image_L->width;x++)
		{
			if (cvGetReal2D(DarkImageBI,y,x)<100)
			{
				cvSetReal2D(image_L,y,x,0);
			}
		}
	}
	cvShowImage("L+D",image_L);

	//中值滤波去除椒盐噪声
	cvSmooth(image_L,image_L,CV_MEDIAN);
	cvShowImage("LS",image_L);

	//区域增长法去除噪声
	ImagePro imagepro;
	imagepro.areaGrow(image_L,areaMin);
	cvShowImage("LAregrow",image_L);

	//裂纹最终检测效果
	for (int y=0;y<inputImage->height;y++)
	{
		for (int x=0;x<inputImage->width;x++)
		{
			if (cvGetReal2D(image_L,y,x)>100)
			{
				cvSet2D(inputImage,y,x,CV_RGB(0,255,0));
			}
		}
	}
	cvShowImage("result",inputImage);

	cvWaitKey(0);
	return 0;
}
