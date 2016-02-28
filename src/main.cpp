// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <iomanip>
#include <string>
#include <atomic>

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


unsigned long long
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
	
	return n;
}


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
	
	
#ifdef INTERACTIVE
	const std::string
		original_window	= "original_window",
		// edge_window		= "edge_window",
		trackbar		= "Frame";
	
	int trackbar_frame = 0;
	
	
	cv::namedWindow(original_window);
	cv::createTrackbar(trackbar, original_window,
					   &trackbar_frame, frame_count);
	
	// cv::namedWindow(edge_window);
	
	std::cout << "Press space to play/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	int delay = 0;
	bool prev_was_cut = false;
#endif	// ifdef INTERACTIVE
	
	
	std::cout << "Frames count: " << frame_count << std::endl;
	
	
	cv::Mat mat;
	// cv::Mat edge_mat;
	cv::Mat gauss_edge_mat;
	cv::Mat avg_row, avg_elem;
	
	while (true) {
		cap >> mat;
		
		if (mat.size[0] <= 0) {	// End of file reached
			cap.set(cv::CAP_PROP_POS_FRAMES, 0);
#ifdef INTERACTIVE
			cv::setTrackbarPos(trackbar, original_window, 0);
			continue;	// Start again in interactive mode
#else	// ifdef INTERACTIVE
			break;		// Stop in non-interactive mode
#endif	// ifdef INTERACTIVE
		}
		
		
		int frame = cap.get(cv::CAP_PROP_POS_FRAMES);
		
#ifdef INTERACTIVE
		if (trackbar_frame + 1 != frame) {	// Trackbar navigation
			cap.set(cv::CAP_PROP_POS_FRAMES, trackbar_frame);
			continue;
		}
#endif	// ifdef INTERACTIVE
		
		
#ifdef INTERACTIVE
		cv::setTrackbarPos(trackbar, original_window, frame);	// Keep trackbar actual
		
		double time = cap.get(cv::CAP_PROP_POS_MSEC);
		cv::setWindowTitle(original_window,
						   "Frame: " + std::to_string(frame) + ",  Time: " + std::to_string(time) + "ms");
		
		cv::reduce(mat, avg_row, 0, cv::REDUCE_AVG);
		cv::reduce(avg_row, avg_elem, 1, cv::REDUCE_AVG);
		// // cv::Mat tmp{avg_row.size[0], avg_row.size[1], avg_elem.type(), avg_elem.at<double>(0, 0)};
		
		// for (int i = 0; i < 50; ++i)
		// 	mat.push_back(avg_row);
		
		cv::imshow(original_window, mat);
#endif	// ifdef INTERACTIVE
		
		
		// cv::Canny(mat, edge_mat, 10, 10);
		
		cv::cvtColor(mat, gauss_edge_mat, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(gauss_edge_mat, gauss_edge_mat, cv::Size(9, 9), 2, 2);
		cv::Canny(gauss_edge_mat, gauss_edge_mat, 10, 10);
		
		
		double k = static_cast<double>(100) / mat.total();
		// auto wp = white_pixels(edge_mat) * k;
		auto gauss_wp = white_pixels(gauss_edge_mat) * k;
		cv::Vec3b avg_color = avg_elem.at<cv::Vec3b>(0, 0);
		unsigned int ar = avg_color[2],
					 ag = avg_color[1],
					 ab = avg_color[0];
		
		
		bool is_cut =    (gauss_wp < 5 && ar > 100 && ag >   0 && ab >   0)
					  || (gauss_wp < 2 && ar >  10 && ag >  10 && ab >  10)
					  || (                ar <   5 && ag <   5 && ab <   5);
		
		
		std::cout
			// << type2str(mat.type()) << " "
			<< std::setw(6) << frame << ":    "
			// << std::setw(9) << std::fixed << wp << "%;    "
			<< std::setw(9) << std::fixed << gauss_wp << "%;  "
			<< "  R: " << std::setw(3) << ar
			<< "  G: " << std::setw(3) << ag
			<< "  B: " << std::setw(3) << ab
			<< ((is_cut)? "  CUT DETECTED!": "")
			<< std::endl;
		
// #ifdef INTERACTIVE
// 		cv::setWindowTitle(edge_window,
// 						   "White pixels: "/* + std::to_string(wp) + "%; Gauss: "*/ + std::to_string(gauss_wp) + '%');
// 		cv::imshow(edge_window, edge_mat);
// #endif	// ifdef INTERACTIVE
		
		
#ifdef INTERACTIVE
		if (prev_was_cut ^ is_cut)	// Pause, if something changed
			delay = 0;
		prev_was_cut = is_cut;
		
		// Primitive player
		auto key = cv::waitKey(delay);
		if (key >= 0) {
			switch (key) {
				case ' ':	// Play/pause
					delay = ((delay == 0)? 1: 0);
					continue;	// Continue cycle
				case 'n':	// Next
					break;
				case 'b':	// Previous
					cap.set(cv::CAP_PROP_POS_FRAMES, frame - 2);
					cv::setTrackbarPos(trackbar, original_window, frame - 2);
					break;
			}
		}
#endif	// ifdef INTERACTIVE
	}
	
	
#ifdef INTERACTIVE
	cv::destroyAllWindows();
#endif	// ifdef INTERACTIVE
	
	return 0;
}
