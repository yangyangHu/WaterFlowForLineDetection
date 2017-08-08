#include "roughness.h"

Roughness::Roughness(IplImage* src)
{
	inputImage = cvCreateImage(cvGetSize(src),src->depth,1);
	if (src->nChannels>1)
	{
		cvCvtColor(src,inputImage,CV_BGR2GRAY);
	}
	else
	{
		inputImage = cvCloneImage(src);
	}

	r = 7;

	threshold_roughness = 10;

	win_mat = cvCreateImage(cvSize(2*r+1,2*r+1),IPL_DEPTH_8U,1);

	image_sdv = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvZero(image_sdv);

	image_roughness = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvZero(image_roughness);
}

Roughness::~Roughness()
{
	cvReleaseImage(&inputImage);
	cvReleaseImage(&image_sdv);
	cvReleaseImage(&image_roughness);
	cvReleaseImage(&win_mat);
}

void Roughness::CaculateRoughness()
{
	for (int y=r;y<inputImage->height-r;y++)
	{
		for (int x=r;x<inputImage->width-r;x++)
		{
			//求出(x,y)领域像素
			CaculateNeighbourPixel(x,y);

			CvScalar mean = cvScalar(0);
			CvScalar sdv = cvScalar(0);
			cvAvgSdv(win_mat,&mean,&sdv);

			cvSetReal2D(image_sdv,y,x,sdv.val[0]);

		/*	if (sdv.val[0]<threshold_roughness)
			{
				cvSetReal2D(image_roughness,y,x,0);
			}
			else
			{
				cvSetReal2D(image_roughness,y,x,255);
			}*/

		}
	}

	cvThreshold(image_sdv,image_roughness,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);

	cvShowImage("sdv",image_sdv);
	//cvShowImage("Rough",image_roughness);

}

void Roughness::CaculateNeighbourPixel(int x0,int y0)
{
	for (int y=y0-r;y-y0+r<2*r+1;y++)
	{
		for (int x=x0-r;x-x0+r<2*r+1;x++)
		{
			cvSetReal2D(win_mat,y-y0+r,x-x0+r,cvGetReal2D(inputImage,y,x));
		}
	}
}

IplImage* Roughness::GetImageRoughness()
{
	return image_roughness;
}
