#include "stdafx.h"
#include<opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void face_detection(Mat &source);

int main()
{
	VideoCapture cap(0);
	
	//camera_image表示摄像头捕获的原图
	Mat camera_image;

	//blur模糊化图像
	Mat blur;

	//轮廓图像
	Mat edge;

	while (1)
	{
		//从摄像头获取图像
		cap >> camera_image;

		//增加先验检测人脸
		face_detection(camera_image);
		//imshow("加先验", camera_image);

		//为提高双边滤波平滑算法执行效率，将原图像camera_image原来1/4缩减图存入reduced_image
		Size size = camera_image.size();
		Size reduced_size;
		reduced_size.width = size.width / 2;
		reduced_size.height = size.height / 2;
		Mat reduced_image = Mat(reduced_size, CV_8UC3);
		resize(camera_image, reduced_image, reduced_size);

		//对缩小后的图像进行双边滤波处理
		Mat tmp = Mat(reduced_size, CV_8UC3);
		int repetitions = 7;
		for (int i = 0; i < repetitions; i++)
		{
			int kernelSize = 9;
			double sigmaColor = 9;
			double sigmaSpace = 7;
			bilateralFilter(reduced_image, tmp, kernelSize, sigmaColor, sigmaSpace);
			bilateralFilter(tmp, reduced_image, kernelSize, sigmaColor, sigmaSpace);
		}

		//将图像放大至原大小，存在magnified_image里
		Mat magnified_image;
		resize(reduced_image, magnified_image, size);
		//imshow("magnified_image", magnified_image);


		//因为提取边缘对噪声敏感，所以先用高斯模糊消除原图camera_image中噪声的影响
		GaussianBlur(camera_image, blur, Size(5, 5), 0, 0);

		//素描边缘图生成
		//使用laplace算法提取边缘线
		const int LAPLACIAN_FILTER_SIZE = 5;
		Laplacian(blur, edge, CV_8U, LAPLACIAN_FILTER_SIZE);

		//Canny只有轮廓线
		//Canny(blur, edge, 30, 75);

		const int EDGES_THRESHOLD = 100;
		threshold(edge, edge, EDGES_THRESHOLD, 255, THRESH_BINARY_INV);
		//imshow("edge", edge);

		//素描边缘图和卡通无边缘图叠加
		Mat cartoon_image = Mat(size, CV_8UC3);
		cartoon_image.setTo(0);
		magnified_image.copyTo(cartoon_image, edge);

		imshow("按空格键拍照，ESC键退出", cartoon_image);

		//按键检测
		char key = (char)cv::waitKey(30);
		//按下空格键就"拍照"（保存图片）
		if (key == ' ')
		{
			imwrite("photo.jpg", cartoon_image);
		}
		//esc键按下就退出，不拍照
		if (key == 27)
			break;
	}

	//system("pause");
	return 0;
}

void face_detection(Mat &source)
{
	CascadeClassifier faceCascade;
	//加载分类器
	faceCascade.load("D:/OpenCV/opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml");

	if (source.empty())
	{
		printf("摄像头打开失败！\n");
	}

	Mat imgGray;
	//如果原图是RGB三通道，则转化为灰度图像
	if (source.channels() == 3)
		cvtColor(source, imgGray, cv::COLOR_RGB2GRAY);
	else
	{
		imgGray = source;
	}

	vector<Rect> faces;
	// 检测人脸，最小检测范围是10×10像素
	faceCascade.detectMultiScale(imgGray, faces, 1.2, 6, 0, Size(10, 10));

	//检测到了人脸，就在脸上加绿框
	if (faces.size()>0)
	{
		for (int i = 0; i<faces.size(); i++)
		{
			rectangle(source, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 255, 0), 1, 8);    // 框出人脸
		}
	}
}