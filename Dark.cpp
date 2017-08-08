#include "Dark.h"

Dark::Dark(IplImage* src)
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

	patch = cvCreateStructuringElementEx(2*r+1,2*r+1,r,r,CV_SHAPE_RECT);

	darkImage = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);

	darkImageBi = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);

}

Dark::~Dark()
{
	cvReleaseImage(&inputImage);
	cvReleaseImage(&darkImage);
	cvReleaseImage(&darkImageBi);
	cvReleaseStructuringElement(&patch);
}

void Dark::CaculateDark()
{
	cvErode(inputImage,darkImage,patch,1);

	cvThreshold(darkImage,darkImageBi,0,255,CV_THRESH_BINARY_INV|CV_THRESH_OTSU);

	cvShowImage("dark",darkImage);
	//cvShowImage("darkBi",darkImageBi);
}

IplImage* Dark::GetDarkImageBi()
{
	return darkImageBi;
}
