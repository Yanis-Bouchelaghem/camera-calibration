#include "Camera.h"
#include <stdexcept>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <string>

Camera::Camera(cv::Size checkerboardSize, float squareSize, float distanceBetweenCameras, int cameraIndex, cv::TermCriteria criteria)
	:
	checkerboardSize(checkerboardSize),
	squareSize(squareSize),
	distanceBetweenCameras(distanceBetweenCameras),
	criteria(criteria),
	videoCapture(cameraIndex),
	cameraIndex(cameraIndex)
{
	if (!videoCapture.isOpened())
	{
		throw std::runtime_error("Error: Could not open camera.");
	}
}

std::vector<cv::Point2f> Camera::Tick()
{
	//Capture an image from the camera
	if (!videoCapture.read(frame))
	{
		throw std::runtime_error("Error: Could not get frame from camera.");
	}
	cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
	std::vector<cv::Point2f> corners;
	if (cv::findChessboardCorners(gray, checkerboardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE))
	{
		// Raffiner les coins 
		cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);
	}
	return corners;
}

void Camera::Draw(const std::vector<cv::Point2f>& corners)
{
	// Afficher les coins détectés
	cv::drawChessboardCorners(frame, checkerboardSize, corners, corners.size() != 0);
	cv::imshow(std::string("Camera ") + std::to_string(cameraIndex), frame);
}
