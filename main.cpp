#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>


int main(int argc, char** argv) {

	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

	cv::VideoCapture cap;
	if (argc >= 2) cap.open(argv[1], cv::CAP_ANY);
	else           cap.open(0, cv::CAP_ANY);

	if (!cap.isOpened()) {
		std::cerr << "ERROR: Could not open source.\n";
		return 1;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 1200);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

	const std::string win = "Preview";
	cv::namedWindow(win, cv::WINDOW_AUTOSIZE);
	std::cout << "Previw runnig. Press 'q' or ESC to quit, 's' to save a frame.\n";

	using clock = std::chrono::steady_clock;
	auto t0 = clock::now();
	int frames = 0;
	double fps = 0.0;
	int saved = 0;

	cv::Mat frame;
	for (;;) {
		if (!cap.read(frame)) {
			std::cerr << "WARN: Faild to grab frame. \n";
			break;
		}

		frames++;
		auto t1 = clock::now();
		double sec = std::chrono::duration<double>(t1 - t0).count();
		if (sec >= 1.0) {
			fps = frames / sec;
			frames = 0;
			t0 = t1;
		}

		
		std::string text =	"FPS: " + std::to_string(fps).substr(0, 5) +
							" | Press 's' to save (" + std::to_string(saved) + ")";
		cv::putText	(frame, text, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.8,
					cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
		
		
		cv::imshow(win, frame);

		
		if (cv::getWindowProperty(win, cv::WND_PROP_VISIBLE) < 1)break;

		int key = cv::waitKey(1);
		if (key == 27 || key == 'q') break;
		if (key == 's') {
			bool ok = cv::imwrite("frame.png", frame);
			std::cout << (ok ? "Saved: frame.png\n" : "Faild to save frame.\n");
		}
	}

	cap.release();
	cv::destroyAllWindows();
	return 0;
}