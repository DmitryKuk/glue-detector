// Author: Dmitry Kukovinets (d1021976@gmail.com), 03.03.2016, 20:15

#include "play.hpp"

#include <iostream>
#include <iomanip>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


void
play(cv::VideoCapture &cap, const std::vector<frame> &frames, int frame_count)
{
	const std::string
		original_window	= "original_window",
		trackbar		= "Frame";
	
	int trackbar_i = 0, delay = 0;
	
	cv::namedWindow(original_window);
	cv::createTrackbar(trackbar, original_window, &trackbar_i, frame_count);
	
	std::cout << "Press space to play/pause, 'n' to next frame, 'b' to previous frame." << std::endl;
	
	
	cv::Mat mat;
	
	frame prev_;
	
	while (true) {
		int i = cap.get(cv::CAP_PROP_POS_FRAMES);
		unsigned long long time = cap.get(cv::CAP_PROP_POS_MSEC);
		
		cap >> mat;
		
		
		if (mat.size[0] <= 0) {	// End of file reached
			cap.set(cv::CAP_PROP_POS_FRAMES, 0);
			cv::setTrackbarPos(trackbar, original_window, 0);
			continue;	// Start again
		}
		
		
		if (trackbar_i != i && trackbar_i + 1 != i) {	// Trackbar navigation
			cap.set(cv::CAP_PROP_POS_FRAMES, trackbar_i);
			continue;
		}
		
		
		
		cv::setTrackbarPos(trackbar, original_window, i);	// Keep trackbar actual
		
		cv::setWindowTitle(original_window, "Frame: " + std::to_string(i) + ",  Time: " + std::to_string(time) + "ms");
		
		cv::imshow(original_window, mat);
		
		
		const auto &curr = frames[i];
		const auto &prev = (i == 0)? prev_: frames[i - 1];
		
		
		double wp_delta = curr.wp - prev.wp;
		
		double color_delta = (  static_cast<double>(curr.ar - prev.ar) / 255
							  + static_cast<double>(curr.ag - prev.ag) / 255
							  + static_cast<double>(curr.ab - prev.ab) / 255) * 100 / 3;
		
		double gb_dom_delta = curr.gb_dom - prev.gb_dom;
		
		
		std::cout
			<< std::setw(6) << i << "  -  " << std::setw(6) << time << "ms:    "
			<< "WP:" << std::setw(8) << std::noshowpos << curr.wp
			<< std::setw(9) << std::showpos << wp_delta << "%;  "
			<< "  R:" << std::setw(4) << std::noshowpos << curr.ar
			<< "  G:" << std::setw(4) << std::noshowpos << curr.ag
			<< "  B:" << std::setw(4) << std::noshowpos << curr.ab
			<< std::setw(10) << std::showpos << color_delta << "%;"
			<< "    GB_DOM:" << std::setw(9) << std::showpos << curr.gb_dom
			<< std::setw(9) << std::showpos << gb_dom_delta << "%;"
			<< ((curr.is_cut)? "    CUT DETECTED!": "")
			<< std::endl;
		
		
		if (prev.is_cut ^ curr.is_cut)	// Pause, if something changed
			delay = 0;
		
		
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
				cap.set(cv::CAP_PROP_POS_FRAMES, i - 2);
				cv::setTrackbarPos(trackbar, original_window, i - 2);
				break;
		}
	}
	
	
	cv::destroyAllWindows();
}
