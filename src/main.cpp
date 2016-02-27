// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


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
	
	int
		frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT),
		trackbar_frame = 0;
	
	
	const std::string
		original_window	= "original_window",
		edge_window		= "edge_window",
		trackbar		= "Frame";
	
	
	cv::namedWindow(original_window);
	cv::createTrackbar(trackbar, original_window,
					   &trackbar_frame, frame_count);
	
	cv::namedWindow(edge_window);
	
	
	std::cout
		<< "Frames count: " << frame_count << std::endl
		<< "Press space to plau/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	
	cv::Mat mat, edge_mat;
	int delay = 0;
	
	while (true) {
		cap >> mat;
		
		if (mat.size[0] <= 0) {	// Start again, if end of file reached
			cap.set(cv::CAP_PROP_POS_FRAMES, 0);
			cv::setTrackbarPos(trackbar, original_window, 0);
			continue;
		}
		
		
		int frame = cap.get(cv::CAP_PROP_POS_FRAMES);
		if (trackbar_frame + 1 != frame) {	// Trackbar navigation
			cap.set(cv::CAP_PROP_POS_FRAMES, trackbar_frame);
			continue;
		}
		
		
		cv::setTrackbarPos(trackbar, original_window, frame);	// Keep trackbar actual
		
		
		double time = cap.get(cv::CAP_PROP_POS_MSEC);
		
		
		cv::setWindowTitle(original_window,
						   "Frame: " + std::to_string(frame) + ",  Time: " + std::to_string(time) + "ms");
		cv::imshow(original_window, mat);
		
		
		cv::Canny(mat, edge_mat, 50, 50);
		
		cv::imshow(edge_window, edge_mat);
		
		
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
	}
	
	
	cv::destroyAllWindows();
	return 0;
}
