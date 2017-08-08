#include "Global.h"

class WaterFlow
{
private:
	int T;	//梯度阈值
	int g;  //geometric threshold 几何阈值
	IplImage* inputImage;
	IplImage* inputRoughness;
	IplImage* L;	//线响应输出

public:
	WaterFlow(IplImage* giudedFilterImage,IplImage* imageRoughness);
	~WaterFlow();
	void Calculate_t();//计算坡阈值T
	int colorhist(float *data,int i0);
	void Caculate_g();  //计算几何阈值g
	void Caculate_L();  //计算L
	void pixelWaterFlow(int x,int y);  //水流模拟
	void processFlow();
	IplImage* GetL();
};
