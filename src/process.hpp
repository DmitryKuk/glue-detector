// Author: Dmitry Kukovinets (d1021976@gmail.com), 03.03.2016, 20:15

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>

#include <opencv2/videoio.hpp>

#include "frame.hpp"


std::vector<frame>
process(cv::VideoCapture &cap, size_t frame_count);


#endif	// PROCESS_HPP
