#include "stdafx.h"
#include<opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void face_detection(Mat &source);

void color_adjust(Mat &source);

int main()
{
	VideoCapture cap(0);
	
	//camera_image表示摄像头捕获的原图
	Mat camera_image;

	//blur模糊化图像
	Mat blur;

	//轮廓图像
	Mat edge;

	int mode = 0;

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
		cv::resize(camera_image, reduced_image, reduced_size);

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

		//将图像放大至原大小，存在magnified_image里，此时magnified_image里存放的是无边缘卡通图
		Mat magnified_image;
		cv::resize(reduced_image, magnified_image, size);
		//imshow("magnified_image", magnified_image);


		//因为提取边缘对噪声敏感，所以先用高斯模糊消除原图camera_image中噪声的影响
		cv::GaussianBlur(camera_image, blur, Size(5, 5), 0, 0);

		//素描边缘图生成

		if (mode == 0)
		{
			//使用laplace算法提取边缘线
			const int LAPLACIAN_FILTER_SIZE = 5;
			printf("using laplace\n");
			cv::Laplacian(blur, edge, CV_8U, LAPLACIAN_FILTER_SIZE);
		}

		if (mode == 1)
		{
			//使用Canny算法提取边缘线
			Canny(blur, edge, 30, 75);
			printf("using Canny\n");
		}

		const int EDGES_THRESHOLD = 100;
		cv::threshold(edge, edge, EDGES_THRESHOLD, 255, THRESH_BINARY_INV);
		//imshow("edge", edge);

		//素描边缘图和卡通无边缘图叠加
		Mat cartoon_image = Mat(size, CV_8UC3);
		cartoon_image.setTo(0);
		magnified_image.copyTo(cartoon_image, edge);

		color_adjust(cartoon_image);

		cv::imshow("按空格键拍照，ESC键退出，a(小写)切换至Canny模式，s(小写)换回laplace", cartoon_image);

		//按键检测
		char key = (char)cv::waitKey(10);
		//printf("key is:*%d*, mode is:*%d*\n", key,mode);

		//如果按下空格键就"拍照"（保存图片）
		if (key == ' ')
		{
			imwrite("photo.jpg", cartoon_image);
		}
		//如果esc键按下就退出，不拍照
		if (key == 27)
			break;
		//按下a（小写）转换成Canny模式
		if (key == 97 && mode == 0)
		{
			mode = 1;
			printf("now switch to Canny method\n");
		}
		//按下s（小写）转换成Laplacian模式
		if (key == 115 && mode == 1)
		{
			mode = 0;
			printf("now switch to Laplacian method\n");
		}

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


void color_adjust(Mat &source)
{

	//调亮度
	double alpha = 1.5, beta = 10;
	for (int i = 0; i < source.rows; i++)
	{
		for (int j = 0; j < source.cols; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				int tmp = (uchar)source.at<Vec3b>(i, j)[k] * alpha + beta;
				if (tmp > 255)
					source.at<Vec3b>(i, j)[k] = 2 * 255 - tmp;
				else
					source.at<Vec3b>(i, j)[k] = tmp;
			}
		}
	}

	int width = source.cols;
	int heigh = source.rows;

	//一行一行横向遍历图片上的所有像素点，通过原图的RGB计算新图片的RGB值
	for (int y = 0; y<heigh; y++)
	{
		uchar* P = source.ptr<uchar>(y);
		for (int x = 0; x<width; x++)
		{
			double B = P[3 * x];
			double G = P[3 * x + 1];
			double R = P[3 * x + 2];

			double newB = B;
			double newG = G;
			double newR = R;

			double times = 1.1;

			if (B > G&&B > R)
				newB = times * B;
			if (G > B&&G > R)
				newG = times * G;
			if (R > G&&R > B)
				newR = times * R;
			
			if (newB > 255)
				newB = 2 * 255 - newB;
			if (newG > 255)
				newG = 2 * 255 - newG;
			if (newR > 255)
				newR = 2 * 255 - newR;

			P[3 * x] = (uchar)newB;
			P[3 * x + 1] = (uchar)newG;
			P[3 * x + 2] = (uchar)newR;
		}
	}

}