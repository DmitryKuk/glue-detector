// Author: Dmitry Kukovinets (d1021976@gmail.com), 03.03.2016, 20:15

#ifndef PLAY_HPP
#define PLAY_HPP

#include <vector>

#include <opencv2/videoio.hpp>

#include "frame.hpp"


void
play(cv::VideoCapture &cap, const std::vector<frame> &frames, int frame_count);


#endif	// PLAY_HPP
