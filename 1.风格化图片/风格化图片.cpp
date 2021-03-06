// image_processing_project.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>    
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/core/core.hpp>   
#include <iostream>
using namespace cv;
using namespace std;

int main()
{
	//读取原图片文件
	Mat src = imread("C://Users//Administrator//Desktop//images//o_3.jpg", 1);

	//如果图像没能成功打开，控制台窗口弹出错误信息
	if(src.empty())
	{
		cout << "图像加载失败！"<< endl;
		return -1;
	}

	//获取图片行列大小信息, col表示横行像素数，row表示纵行像素数
	int width = src.cols;
	int heigh = src.rows;
	//printf("width:%d\n", width);
	//printf("heigh:%d\n", heigh);
	
	//new一个新图片对象，和原图大小一致
	Mat img(src.size(), CV_8UC3);

	//一行一行横向遍历图片上的所有像素点，通过原图的RGB计算新图片的RGB值
	for (int y = 0; y<heigh; y++)
	{
		uchar* P0 = src.ptr<uchar>(y);
		uchar* P1 = img.ptr<uchar>(y);
		for (int x = 0; x<width; x++)
		{
			double B = P0[3 * x];
			double G = P0[3 * x + 1];
			double R = P0[3 * x + 2];

			//怀旧
			/*double newB = 0.272*R + 0.534*G + 0.131*B;
			double newG = 0.349*R + 0.686*G + 0.168*B;
			double newR = 0.393*R + 0.769*G + 0.189*B;*/

			//蓝调
			/*double newR = (G * B / 255);
			double newG = (B * R / 255);
			double newB = (R * G / 255);*/

			//阴沉
			double newR = (R * R / 255)*0.7;
			double newG = (G * G / 255)*0.7;
			double newB = (B * B / 255)*0.7;

			if (newB<0)newB = 0;
			if (newB>255)newB = 255;
			if (newG<0)newG = 0;
			if (newG>255)newG = 255;
			if (newR<0)newR = 0;
			if (newR>255)newR = 255;
			P1[3 * x] = (uchar)newB;
			P1[3 * x + 1] = (uchar)newG;
			P1[3 * x + 2] = (uchar)newR;
		}
	}
	imshow("阴沉", img);
	//waitKey(3000);
	imwrite("C://Users//Administrator//Desktop//images//n_33.jpg", img);
	
	//system("pause");
	return 0;
}