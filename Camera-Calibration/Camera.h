#pragma once
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <vector>

class Camera
{
public:
	Camera(cv::Size checkerboardSize,
			float squareSize,
			float distanceBetweenCameras,
			int cameraIndex,
		    cv::TermCriteria criteria);
	Camera(const Camera& other) = delete;
	Camera& operator=(const Camera& other) = delete;
	~Camera();

	std::vector<cv::Point2f> Tick();
	void Draw(const std::vector<cv::Point2f>& corners, float distanceToBoard);
private:
	const cv::Size checkerboardSize;
	const float squareSize;
	const float distanceBetweenCameras;
	const cv::TermCriteria criteria;
	std::vector<std::vector<cv::Point3f>> objPoints;
	std::vector<std::vector<cv::Point3f>> imgPoints;
	const int cameraIndex;
	cv::VideoCapture videoCapture;
	cv::Mat frame;
	cv::Mat gray;
}; 