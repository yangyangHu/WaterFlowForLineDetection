#include "Global.h"

class Roughness
{
private:
	int r;   //扫描窗口的大小
	IplImage* win_mat;  //扫描窗口
	IplImage* inputImage; 
	IplImage* image_sdv;
	IplImage* image_roughness;
	int threshold_roughness;

public:
	Roughness(IplImage* src);
	~Roughness();
	void CaculateNeighbourPixel(int x0,int y0);
	void CaculateRoughness();
	IplImage* GetImageRoughness();
};
