// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <iomanip>
#include <string>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


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
		edge_window		= "edge_window",
		trackbar		= "Frame";
	
	int trackbar_frame = 0;
	
	
	cv::namedWindow(original_window);
	cv::createTrackbar(trackbar, original_window,
					   &trackbar_frame, frame_count);
	
	cv::namedWindow(edge_window);
	
	std::cout << "Press space to play/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	int delay = 0;
#endif	// ifdef INTERACTIVE
	
	
	std::cout << "Frames count: " << frame_count << std::endl;
	
	
	cv::Mat mat, edge_mat, gauss_edge_mat;
	
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
		
		cv::imshow(original_window, mat);
#endif	// ifdef INTERACTIVE
		
		
		cv::Canny(mat, edge_mat, 50, 50);
		
		cv::cvtColor(mat, gauss_edge_mat, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(gauss_edge_mat, gauss_edge_mat, cv::Size(9, 9), 2, 2);
		cv::Canny(gauss_edge_mat, gauss_edge_mat, 50, 50);
		
		
		double k = static_cast<double>(100) / (mat.size[0] * mat.size[1]);
		auto wp = white_pixels(edge_mat),
			 gauss_wp = white_pixels(gauss_edge_mat);
		
		std::cout
			<< std::setw(6) << frame << ":    "
			<< std::setw(6) << wp << ":  "
			<< std::setw(8) << std::fixed << wp * k << "%;    Gauss:  "
			<< std::setw(6) << gauss_wp << ":  "
			<< std::setw(8) << std::fixed << gauss_wp * k << '%'
			<< std::endl;
		
		
#ifdef INTERACTIVE
		cv::setWindowTitle(edge_window,
						   "White pixels: " + std::to_string(wp * k) + "; Gauss: " + std::to_string(gauss_wp * k));
		cv::imshow(edge_window, edge_mat);
#endif	// ifdef INTERACTIVE
		
		
#ifdef INTERACTIVE
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
