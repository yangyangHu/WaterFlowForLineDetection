#include "WaterFlow.h"

WaterFlow::WaterFlow(IplImage* giudedFilterImage,IplImage* imageRoughness)
{
	//T = -6;

	inputImage = cvCreateImage(cvGetSize(giudedFilterImage),giudedFilterImage->depth,1);
	inputImage = cvCloneImage(giudedFilterImage);

	inputRoughness = cvCreateImage(cvGetSize(imageRoughness),imageRoughness->depth,1);
	inputRoughness = cvCloneImage(imageRoughness);

	L = cvCreateImage(cvGetSize(giudedFilterImage),8,1);
	cvZero(L);
}

WaterFlow::~WaterFlow()
{
	cvReleaseImage(&inputImage);
	cvReleaseImage(&inputRoughness);
	cvReleaseImage(&L);
}

void WaterFlow::processFlow()
{
	//自适应计算坡阈值T
	Calculate_t();

	//计算线响应图L
	Caculate_L();
}

void WaterFlow::Caculate_L()
{
	//单核处理
	/*for (int y0=0;y0<inputImage->height;y0++)
	{
		for (int x0=0;x0<inputImage->width;x0++)
		{
			if (cvRound(cvGetReal2D(inputRoughness,y0,x0))>200)
			{
				pixelWaterFlow(x0,y0);
			}
		}
	}*/

	//多核并行处理
	parallel_for (int(0), inputImage->height, [&](int y0)
	{
		parallel_for (int(0), inputImage->width, [&](int x0)
		{
			if (cvRound(cvGetReal2D(inputRoughness,y0,x0))>200)
			{
				pixelWaterFlow(x0,y0);
			}

		});
	});

	cvShowImage("fill",inputImage);

	//线响应图（深度图）
	cvShowImage("L",L);
	//cvNormalize(L,L,255,0.0,CV_C);//归一化到255
	//cvShowImage("LNorm",L);

	//计算几何阈值g
	Caculate_g();

	//消除较暗的噪声点和线外围的阴影区域
	for (int y=0;y<L->height;y++)
	{
		for (int x=0;x<L->width;x++)
		{
			double value = cvGetReal2D(L,y,x);
			if (value>g)
			{
				//cvSetReal2D(L,y,x,value-g);
			}
			else
			{
				cvSetReal2D(L,y,x,0);
			}
		}
	}

	cvShowImage("L-DarkNoise",L);

	cvThreshold(L, L, 0, 255, CV_THRESH_BINARY| CV_THRESH_OTSU);//可考虑采用大津阈值
	cvShowImage("LBi",L);
}

void WaterFlow::Caculate_g()
{
	//确定阈值g--方法一
	/*double deep_max;
	cvMinMaxLoc(L,NULL,&deep_max,NULL,NULL,NULL);
	g = deep_max/20;*/

	//确定阈值g--方法二
	float data[256] = {0};
	int total = 0;
	for (int y=0;y<L->height;y++)
	{
		for (int x=0;x<L->width;x++)
		{
			if (cvGetReal2D(L,y,x)>0)
			{
				int intensity = cvRound(cvGetReal2D(L,y,x));
				data[intensity]++;
				total++;
			}
		}
	}

	//舍去较暗的20%
	float sum_dark =0;
	double rate_dark = 0.2;
	for (int i=0;i<256;i++)
	{
		sum_dark += data[i];
		if ((sum_dark)/((double)total)>rate_dark)
		{
			g = i;
			break;
		}
	}
	printf("g=%d\n",g);

	colorhist(data,g);//直方图显示
}

void WaterFlow::Calculate_t()
{
	IplImage* image_difference = cvCreateImage(cvGetSize(inputRoughness),8,1);
	cvZero(image_difference);
	float data[256] = {0};
	int total = 0;

	for (int y=0;y<inputImage->height;y++)
	{
		for (int x=0;x<inputImage->width;x++)
		{
			if (cvRound(cvGetReal2D(inputRoughness,y,x))>200)	//粗糙度较大的候选水分子
			{
				int currentPixel = cvRound(cvGetReal2D(inputImage,y,x));
				int d = 1;
				int maxDifference = -300;
				int maxabsDifference = 0;

				if((x-d>=0)&&(y>=0)&&(x-d<inputImage->width)&&(y<inputImage->height))//西
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y,x-d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x-d>=0)&&(y-d>=0)&&(x-d<inputImage->width)&&(y-d<inputImage->height))//西北
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x-d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x>=0)&&(y-d>=0)&&(x<inputImage ->width)&&(y-d<inputImage->height))//北
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x+d>=0)&&(y-d>=0)&&(x+d<inputImage->width)&&(y-d<inputImage->height))//东北
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x+d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x+d>=0)&&(y>=0)&&(x+d<inputImage->width)&&(y<inputImage->height))//东
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y,x+d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x+d>=0)&&(y+d>=0)&&(x+d<inputImage->width)&&(y+d<inputImage->height))//东南
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y+d,x+d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x>=0)&&(y+d>=0)&&(x<inputImage->width)&&(y+d<inputImage->height))//南
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y+d,x));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}

				if((x-d>=0)&&(y+d>=0)&&(x-d<inputImage->width)&&(y+d<inputImage->height))//西南
				{
					int nextPixel = cvRound(cvGetReal2D(inputImage,y,x-d));
					int difference = currentPixel - nextPixel;
					if (difference>maxDifference)
					{
						maxDifference = difference;
					}
					if (maxabsDifference<abs(difference))
					{
						maxabsDifference = abs(difference);
					}
				}
				//if (maxDifference<0)
				//{
					cvSetReal2D(image_difference,y,x,abs(maxDifference));
					if (maxabsDifference!=0)
					{
						data[maxabsDifference]++;
						total++;
					}
				//}
			}
		}
	}

	float sum =0;
	double rate = 0.15;
	for (int i=0;i<256;i++)
	{
		sum += data[i];
		if ((sum)/((double)total)>rate)
		{
			T = -(i-1);
			if (T>=-1)
			{
				T=-2 /*-3*/;
			}
			if (T<=-9)
			{
				T = -9;
			}

			//colorhist(data,i);//直方图显示
			break;
		}
	}
	//T = -3;
	printf("T=%d\n",T);
	
}

int WaterFlow::colorhist(float *data,int i0)
{
	int hist_size = 256;
	int hist_height = 200;
	float range[] = {0,255};
	float *ranges[] = {range};
	//创建一维直方图
	CvHistogram* hist = cvCreateHist(1,&hist_size,CV_HIST_ARRAY,ranges,1);

	//根据已给定的数据创建直方图
	cvMakeHistHeaderForArray(1,&hist_size,hist,data,ranges,1);
	//归一化直方图
	cvNormalizeHist(hist,1.0);

	//创建一张一维直方图的“图”，横坐标为灰度级，纵坐标为像素个数
	int scale = 2;
	IplImage* hist_image = cvCreateImage(cvSize(hist_size*scale,hist_height),8,3);
	cvZero(hist_image);

	//统计直方图中的最大bin
	float max_value = 0;
	cvGetMinMaxHistValue(hist,0,&max_value,0,0);

	//分别将每个bin的值绘制在图中
	for (int i=0;i<hist_size;i++)
	{
		float bin_val = cvQueryHistValue_1D(hist,i);
		int intensity = cvRound(bin_val*hist_height/max_value);//要绘制的高度
		if (i==i0)
		{
			cvRectangle(hist_image,cvPoint(i*scale,hist_height-1),cvPoint((i+1)*scale-1,hist_height-intensity),CV_RGB(255,0,0));
		}
		else
		{
			cvRectangle(hist_image,cvPoint(i*scale,hist_height-1),cvPoint((i+1)*scale-1,hist_height-intensity),CV_RGB(0,0,255));
		}
	}

	cvShowImage("hist",hist_image);

	return 0;
}

void WaterFlow::pixelWaterFlow(int x,int y)
{
	IplImage* flag  = cvCreateImage(cvSize(inputImage->width ,inputImage->height ),IPL_DEPTH_8U,1);
	cvZero(flag);

	while(true) 
	{
		int currentPixel = cvRound(cvGetReal2D(inputImage,y,x));
		cvSetReal2D(flag,y,x,1);

		int difference[8] = {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
		CvPoint nextpoint[8];
		/*for (int i=0;i<8;i++)
		{
			nextpoint[i].x = x;
			nextpoint[i].y = y;
		}*/

		int d = 1;

		//确定当前水分子流向（八领域方向）
		if((x-d>=0)&&(y>=0)&&(x-d<inputImage->width)&&(y<inputImage->height))//西
		{
			nextpoint[0].x = x-d;
			nextpoint[0].y = y;
			if (!cvGetReal2D(flag,y,x-d))
			{
				cvSetReal2D(flag,y,x-d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y,x-d));
				difference[0] = currentPixel - nextPixel;
			}
			else
			{
				//difference[0] = -300;
			}
		}

		if((x-d>=0)&&(y-d>=0)&&(x-d<inputImage->width)&&(y-d<inputImage->height))//西北
		{
			nextpoint[1].x = x-d;
			nextpoint[1].y = y-d;
			if (!cvGetReal2D(flag,y-d,x-d))
			{
				cvSetReal2D(flag,y-d,x-d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x-d));
				difference[1] = currentPixel - nextPixel;
			}
			else
			{
				//difference[1] = -300;
			}
		}

		if((x>=0)&&(y-d>=0)&&(x<inputImage ->width)&&(y-d<inputImage->height))//北
		{
			nextpoint[2].x = x;
			nextpoint[2].y = y-d;
			if (!cvGetReal2D(flag,y-d,x))
			{
				cvSetReal2D(flag,y-d,x,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x));
				difference[2] = currentPixel - nextPixel;
			}
			else
			{
				//difference[2] = -300;
			}
		}

		if((x+d>=0)&&(y-d>=0)&&(x+d<inputImage->width)&&(y-d<inputImage->height))//东北
		{
			nextpoint[3].x = x+d;
			nextpoint[3].y = y-d;
			if (!cvGetReal2D(flag,y-d,x+d))
			{
				cvSetReal2D(flag,y-d,x+d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y-d,x+d));
				difference[3] = currentPixel - nextPixel;
			}
			else
			{
				//difference[3] = -300;
			}
		}

		if((x+d>=0)&&(y>=0)&&(x+d<inputImage->width)&&(y<inputImage->height))//东
		{
			nextpoint[4].x = x+d;
			nextpoint[4].y = y;
			if (!cvGetReal2D(flag,y,x+d))
			{
				cvSetReal2D(flag,y,x+d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y,x+d));
				difference[4] = currentPixel - nextPixel;
			}
			else
			{
				//difference[4] = -300;
			}
		}

		if((x+d>=0)&&(y+d>=0)&&(x+d<inputImage->width)&&(y+d<inputImage->height))//东南
		{
			nextpoint[5].x = x+d;
			nextpoint[5].y = y+d;
			if (!cvGetReal2D(flag,y+d,x+d))
			{
				cvSetReal2D(flag,y+d,x+d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y+d,x+d));
				difference[5] = currentPixel - nextPixel;
			}
			else
			{
				//difference[5] = -300;
			}
		}

		if((x>=0)&&(y+d>=0)&&(x<inputImage->width)&&(y+d<inputImage->height))//南
		{
			nextpoint[6].x = x;
			nextpoint[6].y = y+d;
			if (!cvGetReal2D(flag,y+d,x))
			{
				cvSetReal2D(flag,y+d,x,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y+d,x));
				difference[6] = currentPixel - nextPixel;
			}
			else
			{
				//difference[6] = -300;
			}
		}

		if((x-d>=0)&&(y+d>=0)&&(x-d<inputImage->width)&&(y+d<inputImage->height))//西南
		{
			nextpoint[7].x = x-d;
			nextpoint[7].y = y+d;
			if (!cvGetReal2D(flag,y+d,x-d))
			{
				cvSetReal2D(flag,y+d,x-d,1);
				int nextPixel = cvRound(cvGetReal2D(inputImage,y+d,x-d));
				difference[7] = currentPixel - nextPixel;
			}
			else
			{
				//difference[7] = -300;
			}
		}
		//difference 最大的方向为水分子流动的方向
		int maxDifference = -300;
		int maxAbsDifference = 0;
		CvPoint flowDirection;
		for (int i=0;i<8;i++)
		{
			if (maxDifference<difference[i])
			{
				maxDifference = difference[i];
				flowDirection.x = nextpoint[i].x;
				flowDirection.y = nextpoint[i].y;
			}
			if (maxAbsDifference<abs(difference[i])&&abs(difference[i])<255)
			{
				maxAbsDifference = abs(difference[i]);
			}
		}

		//更新flag矩阵
		int flagy=y-d;
		if (flagy<0)
		{
			flagy = 0;
		}
		int flagx=x-d;
		if (flagx<0)
		{
			flagx = 0;
		}

		for (;flagy-y+d<2*d+1&&flagy<flag->height;flagy++)
		{
			for (;flagx-x+d<2*d+1&&flagx<flag->width;flagx++)
			{
				cvSetReal2D(flag,flagy,flagx,1);
			}
		}

		if ((x>0)&&(y>0)&&(x<inputImage->width-1)&&(y<inputImage->height-1)&&(maxDifference<T))//水分子遇到暗沟，停止流动,填补暗沟
		{
			//更新线响应图
			cvSetReal2D(L,y,x,cvGetReal2D(L,y,x)+maxAbsDifference);
			//填补暗沟
			cvSetReal2D(inputImage,y,x,cvGetReal2D(inputImage,y,x)+maxAbsDifference);
			/*if (maxDifference==-300)
			{
				maxDifference = T;
			}
			cvSetReal2D(inputImage,y,x,cvGetReal2D(inputImage,y,x)+abs(maxDifference));
			*/

			break;
		}
		else if ((x==0)||(y==0)||(x==inputImage->width-1)||(y==inputImage->height-1))//水分子流动到了图像边缘，直接流出图像，循环停止
		{
			break;
		}
		else //水分子继续流动
		{
			//cvSetReal2D(resultImage,y,x,255);
			//朝下个方向流动
			//prevousPoint.x = x;
			//prevousPoint.y = y;
			x = flowDirection.x;
			y = flowDirection.y;
			
		}
	}

	cvReleaseImage(&flag);
}

IplImage* WaterFlow::GetL()
{
	return L;
}
