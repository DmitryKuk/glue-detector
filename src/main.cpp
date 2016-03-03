// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <iomanip>

#include <opencv2/videoio.hpp>

#include "frame.hpp"
#include "process.hpp"
#include "play.hpp"

// #include <cmath>
// #include <future>
// #include <array>
// #include <queue>



// std::string
// type2str(int type)
// {
// 	std::string res;
	
// 	uchar depth = type & CV_MAT_DEPTH_MASK;
// 	uchar chans = 1 + (type >> CV_CN_SHIFT);
	
// 	switch ( depth ) {
// 		case CV_8U:  res = "8U"; break;
// 		case CV_8S:  res = "8S"; break;
// 		case CV_16U: res = "16U"; break;
// 		case CV_16S: res = "16S"; break;
// 		case CV_32S: res = "32S"; break;
// 		case CV_32F: res = "32F"; break;
// 		case CV_64F: res = "64F"; break;
// 		default:     res = "User"; break;
// 	}
	
// 	res += "C";
// 	res += (chans + '0');
	
// 	return res;
// }


// double
// enthropy(const cv::Mat &mat)
// {
// 	std::array<unsigned int, 256> red, green, blue;
// 	// unsigned int total = 0;
	
// 	for (unsigned int i = 0, i_max = mat.size[0]; i < i_max; ++i)
// 		for (unsigned int j = 0, j_max = mat.size[1]; j < j_max; ++j) {
// 			auto v = mat.at<cv::Vec3b>(i, j);
			
// 			++red[v[2]];
// 			++green[v[1]];
// 			++blue[v[0]];
// 		}
	
	
// 	auto total = mat.total();
	
// 	const auto color_entropy =
// 		[&](const auto &color) -> double
// 		{
// 			std::priority_queue<double, std::vector<double>, std::function<bool (double, double)>> queue{
// 				[](double a, double b) -> bool { return std::abs(a) < std::abs(b); }
// 			};
			
// 			for (double x: color) {
// 				auto w = x / total;
// 				if (w > 0)
// 					queue.push(w * std::log2(w));
// 			}
			
// 			while (queue.size() > 1) {
// 				auto x1 = queue.top();
// 				queue.pop();
				
// 				auto x2 = queue.top();
// 				queue.pop();
				
// 				queue.push(x1 + x2);
// 			}
			
// 			return queue.top();
// 		};
	
// 	std::future<double> green_entropy = std::async(color_entropy, green),
// 						blue_entropy  = std::async(color_entropy, blue);
	
// 	double sum = color_entropy(red) + green_entropy.get() + blue_entropy.get();
	
// 	return -sum;
// }


// double
// gray_enthropy(const cv::Mat &mat)
// {
// 	std::array<unsigned int, 256> gray;
// 	// unsigned int total = 0;
	
// 	for (unsigned int i = 0, i_max = mat.size[0]; i < i_max; ++i)
// 		for (unsigned int j = 0, j_max = mat.size[1]; j < j_max; ++j) {
// 			auto pixel = mat.at<uint8_t>(i, j);
// 			++gray[pixel];
// 		}
	
	
// 	auto total = mat.total();
	
// 	const auto color_entropy =
// 		[&](const auto &color) -> double
// 		{
// 			std::priority_queue<double, std::vector<double>, std::function<bool (double, double)>> queue{
// 				[](double a, double b) -> bool { return std::abs(a) < std::abs(b); }
// 			};
			
// 			for (double x: color) {
// 				auto w = x / total;
// 				if (w > 0)
// 					queue.push(w * std::log2(w));
// 			}
			
// 			while (queue.size() > 1) {
// 				auto x1 = queue.top();
// 				queue.pop();
				
// 				auto x2 = queue.top();
// 				queue.pop();
				
// 				queue.push(x1 + x2);
// 			}
			
// 			return queue.top();
// 		};
	
// 	return -color_entropy(gray);
// }



int
main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage:" << std::endl << "    " << argv[0] << " VIDEO_FILE" << std::endl;
		return 1;
	}
	
	cv::VideoCapture cap(argv[1]);
	
	if (!cap.isOpened()) {
		std::cerr << "Can\'t open file: \"" << argv[1] << "\"." << std::endl;
		return 2;
	}
	
	
	int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
	if (frame_count <= 0) {
		std::cerr << "Frame count = " << frame_count << " (expected > 0)." << std::endl;
		return 3;
	}
	
	
	std::cout << "Frames count: " << frame_count << std::endl;
	std::cout << std::setprecision(3) << std::fixed;
	
	
	std::vector<frame> frames = process(cap, frame_count);
	
	play(cap, frames, frame_count);
	
	
	return 0;
}
