// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <iomanip>
#include <string>
#include <atomic>
// #include <cmath>
// #include <future>
// #include <array>
// #include <queue>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


std::string
type2str(int type)
{
	std::string res;
	
	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);
	
	switch ( depth ) {
		case CV_8U:  res = "8U"; break;
		case CV_8S:  res = "8S"; break;
		case CV_16U: res = "16U"; break;
		case CV_16S: res = "16S"; break;
		case CV_32S: res = "32S"; break;
		case CV_32F: res = "32F"; break;
		case CV_64F: res = "64F"; break;
		default:     res = "User"; break;
	}
	
	res += "C";
	res += (chans + '0');
	
	return res;
}


double
white_pixels(const cv::Mat &mat)
{
	std::atomic<unsigned long long> n{0};
	
	mat.forEach<uint8_t>(
		[&](uint8_t pixel, const int * /* unused */)
		{
			if (pixel)
				++n;
		}
	);
	
	return static_cast<double>(n) * 100 / mat.total();
}


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
	
	
	const std::string
		original_window	= "original_window",
		trackbar		= "Frame";
	
	int trackbar_frame = 0;
	
	
	cv::namedWindow(original_window);
	cv::createTrackbar(trackbar, original_window,
					   &trackbar_frame, frame_count);
	
	std::cout << "Press space to play/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	int delay = 0;
	bool prev_was_cut = false;
	
	
	std::cout << "Frames count: " << frame_count << std::endl;
	
	std::cout << std::setprecision(3) << std::fixed;
	
	
	cv::Mat mat;
	cv::Mat edge_mat;
	cv::Mat avg_row, avg_elem;
	
	double prev_wp = 0, prev_ar = 0, prev_ag = 0, prev_ab = 0, prev_gb_dom = 0;
	
	
	while (true) {
		cap >> mat;
		
		if (mat.size[0] <= 0) {	// End of file reached
			cap.set(cv::CAP_PROP_POS_FRAMES, 0);
			cv::setTrackbarPos(trackbar, original_window, 0);
			continue;	// Start again
		}
		
		
		int frame = cap.get(cv::CAP_PROP_POS_FRAMES);
		
		if (trackbar_frame + 1 != frame) {	// Trackbar navigation
			cap.set(cv::CAP_PROP_POS_FRAMES, trackbar_frame);
			continue;
		}
		
		
		unsigned long long time = cap.get(cv::CAP_PROP_POS_MSEC);
		
		cv::setTrackbarPos(trackbar, original_window, frame);	// Keep trackbar actual
		
		cv::setWindowTitle(original_window,
						   "Frame: " + std::to_string(frame) + ",  Time: " + std::to_string(time) + "ms");
		
		cv::imshow(original_window, mat);
		
		
		// White pixels metric (found edges)
		cv::cvtColor(mat, edge_mat, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(edge_mat, edge_mat, cv::Size(9, 9), 2, 2);
		cv::Canny(edge_mat, edge_mat, 10, 10);
		
		auto wp = white_pixels(edge_mat);
		
		
		// Avarage frame color
		cv::reduce(mat, avg_row, 0, cv::REDUCE_AVG);
		cv::reduce(avg_row, avg_elem, 1, cv::REDUCE_AVG);
		
		cv::Vec3b avg_color = avg_elem.at<cv::Vec3b>(0, 0);
		
		int ar = avg_color[2],
				 ag = avg_color[1],
				 ab = avg_color[0];
		
		
		double gb_dom = 0;
		if (ag + ar != 0 && ab + ar != 0)
			gb_dom = (static_cast<double>(ag - ar) / (ag + ar) + static_cast<double>(ab - ar) / (ab + ar)) * 100;
		
		
		// Delta from previous frame
		double wp_delta = wp - prev_wp;
		
		double color_delta = (  static_cast<double>(ar - prev_ar) / 255
							  + static_cast<double>(ag - prev_ag) / 255
							  + static_cast<double>(ab - prev_ab) / 255) * 100 / 3;
		
		double gb_dom_delta = gb_dom - prev_gb_dom;
		
		
		bool is_cut = false;
		if (wp > 10) {
			is_cut = false;
		} else {
			if (   (wp < 4.5 && (ar > 100                        ) && gb_dom < 20)
				|| (wp < 3   && (ar >  30 || ag >  30 || ab >  30) && gb_dom < 20)
				|| (            (ar <   3 && ag <   3 && ab <   3)               )
				|| (wp < 1                                                       ))
				is_cut = true;
			
			
			if (!prev_was_cut && is_cut && color_delta < 0 && gb_dom < 0 && (wp > 1 || (ar < 5 && ag < 5 && ab < 5)))
				is_cut = false;
			else if (prev_was_cut && !is_cut && (wp_delta < 2 || wp < 3 || gb_dom < -50))
				is_cut = true;
			else if ((prev_was_cut ^ is_cut) && (std::abs(wp_delta) < 2.5) && (std::abs(color_delta) < 10))
				is_cut = prev_was_cut;
		}
		
		
		if (prev_was_cut ^ is_cut)	// Pause, if something changed
			delay = 0;
		
		
		prev_was_cut = is_cut;
		prev_wp = wp;
		prev_ar = ar;
		prev_ag = ag;
		prev_ab = ab;
		prev_gb_dom = gb_dom;
		
		
		// if (prev_was_cut ^ is_cut) {	// Current frame is different type than previous
		// 	if (prev_was_cut) {
				
		// 	} else {
				
		// 	}
		// } else {
		// 	if (prev_was_cut)
		// 		++cut_len;
		// 	else
		// 		++content_len;
		// }
		
		
		std::cout
			// << type2str(mat.type()) << " "
			<< std::setw(6) << frame << "  -  " << std::setw(6) << time << "ms:    "
			<< "WP:" << std::setw(8) << std::noshowpos << wp
			<< std::setw(9) << std::showpos << wp_delta << "%;  "
			<< "  R:" << std::setw(4) << std::noshowpos << ar
			<< "  G:" << std::setw(4) << std::noshowpos << ag
			<< "  B:" << std::setw(4) << std::noshowpos << ab
			<< std::setw(10) << std::showpos << color_delta << "%;"
			<< "    GB_DOM:" << std::setw(9) << std::showpos << gb_dom
			<< std::setw(9) << std::showpos << gb_dom_delta << "%;"
			<< ((is_cut)? "    CUT DETECTED!": "")
			<< std::endl;
		
		
		// Primitive player
		auto key = cv::waitKey(delay);
		switch (key) {
			case -1:	// Key not pressed
				break;
			case ' ':	// Play/pause
				delay = ((delay == 0)? 1: 0);
				break;
			case 'n':	// Next
				break;
			case 'b':	// Previous
				cap.set(cv::CAP_PROP_POS_FRAMES, frame - 2);
				cv::setTrackbarPos(trackbar, original_window, frame - 2);
				break;
		}
	}
	
	
	cv::destroyAllWindows();
	
	return 0;
}
