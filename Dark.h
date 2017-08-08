#include "Global.h"

class Dark
{
private:
	int r; //扫描窗口的大小
	IplConvKernel* patch;
	IplImage* inputImage;
	IplImage* darkImage;
	IplImage* darkImageBi;

public:
	Dark(IplImage* src);
	~Dark();
	void CaculateDark();
	IplImage* GetDarkImageBi();
};
