// Author: Dmitry Kukovinets (d1021976@gmail.com), 03.03.2016, 20:15

#include "process.hpp"

#include <iostream>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


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



std::vector<frame>
process(cv::VideoCapture &cap, size_t frame_count)
{
	std::vector<frame> frames{frame_count};
	
	cv::Mat mat;
	cv::Mat edge_mat;
	cv::Mat avg_row, avg_elem;
	frame prev;
	
	
	size_t i = 0;
	for (auto &curr: frames) {
		++i;
		if (i % 100 == 0) {	// Print every 100 frames
			std::cout << '\r' << i << " / " << frame_count;
			std::cout.flush();
		}
		
		cap >> mat;
		
		
		// White pixels metric (found edges)
		cv::cvtColor(mat, edge_mat, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(edge_mat, edge_mat, cv::Size(9, 9), 2, 2);
		cv::Canny(edge_mat, edge_mat, 10, 10);
		
		curr.wp = white_pixels(edge_mat);
		
		
		// Avarage frame color
		cv::reduce(mat, avg_row, 0, cv::REDUCE_AVG);
		cv::reduce(avg_row, avg_elem, 1, cv::REDUCE_AVG);
		
		cv::Vec3b avg_color = avg_elem.at<cv::Vec3b>(0, 0);
		
		curr.ar = avg_color[2];
		curr.ag = avg_color[1];
		curr.ab = avg_color[0];
		
		// Green and blue colors domination
		curr.gb_dom = 0;
		if (curr.ag + curr.ar != 0 && curr.ab + curr.ar != 0)
			curr.gb_dom = (  static_cast<double>(curr.ag - curr.ar) / (curr.ag + curr.ar)
						   + static_cast<double>(curr.ab - curr.ar) / (curr.ab + curr.ar)) * 100;
		
		
		// Delta from previous frame
		double wp_delta = curr.wp - prev.wp;
		
		double color_delta = (  static_cast<double>(curr.ar - prev.ar) / 255
							  + static_cast<double>(curr.ag - prev.ag) / 255
							  + static_cast<double>(curr.ab - prev.ab) / 255) * 100 / 3;
		
		double gb_dom_delta = curr.gb_dom - prev.gb_dom;
		
		
		curr.is_cut = false;
		if (curr.wp > 10) {
			curr.is_cut = false;
		} else {
			if (   (curr.wp < 4.5 && (curr.ar > 100                                  ) && curr.gb_dom < 20)
				|| (curr.wp < 3   && (curr.ar >  30 || curr.ag >  30 || curr.ab >  30) && curr.gb_dom < 20)
				|| (                 (curr.ar <   3 && curr.ag <   3 && curr.ab <   3)                    )
				|| (curr.wp < 1                                                                           ))
				curr.is_cut = true;
			
			
			if (   !prev.is_cut && curr.is_cut
				&& color_delta < 0 && curr.gb_dom < 0
				&& (curr.wp > 1 || (curr.ar < 5 && curr.ag < 5 && curr.ab < 5))) {
				curr.is_cut = false;
			} else if (   prev.is_cut && !curr.is_cut
					   && (wp_delta < 2 || curr.wp < 3 || curr.gb_dom < -50)) {
				curr.is_cut = true;
			} else if ((prev.is_cut ^ curr.is_cut)
					   && (std::abs(wp_delta) < 2.5) && (std::abs(color_delta) < 10)) {
				curr.is_cut = prev.is_cut;
			}
		}
		
		prev = curr;
	}
	
	
	cap.set(cv::CAP_PROP_POS_FRAMES, 0);
	
	std::cout << '\r' << frame_count << " / " << frame_count << std::endl;
	
	return frames;
}
