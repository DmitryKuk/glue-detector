// Author: Dmitry Kukovinets (d1021976@gmail.com), 27.02.2016, 14:16

#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>


int
main()
{
	cv::VideoCapture cap("examples/18732_Spurgin-S8mm_23-28-01_SQ.mp4");
	
	cv::Mat mat;
	cap >> mat;
	
	cv::namedWindow("Test window");
	
	cv::imshow("Test window", mat);
	cv::waitKey();
	
	return 0;
}
