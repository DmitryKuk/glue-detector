// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>


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
	
	cv::namedWindow("Test window");
	
	std::cout
		<< "Frames count: " << cap.get(cv::CAP_PROP_FRAME_COUNT) << std::endl
		<< "Press space to plau/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	cv::Mat mat;
	cap >> mat;
	
	int delay = 0;
	while (mat.size[0] > 0) {
		unsigned long frame = cap.get(cv::CAP_PROP_POS_FRAMES);
		double time = cap.get(cv::CAP_PROP_POS_MSEC);
		
		cv::setWindowTitle("Test window",
						   "Frame: " + std::to_string(frame) + ",  Time: " + std::to_string(time) + "ms");
		
		cv::imshow("Test window", mat);
		
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
					break;
			}
		}
		
		cap >> mat;
	}
	
	
	cv::destroyAllWindows();
	return 0;
}
