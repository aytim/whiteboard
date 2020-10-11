#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

//! Computes the intersection of two lines.
/*!
	\param cv::Vec4f containing two points of line 1.
	\param cv::Vec4f containing two points of line 2.
	\return cv::Point2f containing the intersection.
	*/
cv::Point2f intersect(cv::Vec4f l1, cv::Vec4f l2) {
	return cv::Point2f((double)((l1[0] * l1[3] - l1[1] * l1[2]) * (l2[0] - l2[2]) - (l1[0] - l1[2]) * (l2[0] * l2[3] - l2[1] * l2[2])) / ((l1[0] - l1[2]) * (l2[1] - l2[3]) - (l1[1] - l1[3]) * (l2[0] - l2[2])), (double)((l1[0] * l1[3] - l1[1] * l1[2]) * (l2[1] - l2[3]) - (l1[1] - l1[3]) * (l2[0] * l2[3] - l2[1] * l2[2])) / ((l1[0] - l1[2]) * (l2[1] - l2[3]) - (l1[1] - l1[3]) * (l2[0] - l2[2])));
}

//! Find the corners of the shape defined by the given lines.
/*!
	The procedure computes the intersection of each two lines and filters out those lying outside of the image. Then using k-means algorithm it computes the approximate corners of the shape.
	\param Vector containing the lines.
	\param Height of image.
	\param Width of image.
	\return Vector containing the corners.
	*/
std::vector<cv::Point2f> find_corners(std::vector<cv::Vec4f> lines, int row_max, int col_max) {
	std::vector<cv::Point2f> points;
	for (unsigned int i = 0; i < lines.size(); i++)
		for (unsigned int j = 0; j < lines.size(); j++)
			if (i != j) {
				cv::Point2f tmp = intersect(lines[i], lines[j]);
				if (0 <= tmp.x && tmp.x < col_max && 0 <= tmp.y && tmp.y < row_max)
					points.push_back(tmp);
			}
	cv::Mat labels;
	cv::Mat centers;
	cv::kmeans(points, 4, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centers);
	std::vector<cv::Point2f> corners;
	std::generate_n(std::back_insert_iterator<std::vector<cv::Point2f>>(corners), 4, [&centers, i = 0]() mutable {return centers.at<cv::Point2f>(i++);});
	return corners;
}

//! Applies Contrast Limited Adaptive Histogram Equalization to the image.
/*!
	\param cv::Mat object containing the image.
	\return cv::Mat object containing the transformed image.
	*/
cv::Mat apply_clahe(cv::Mat image) {
	cv::Mat image_yuv;
	cv::Mat image_clahe;
	cv::cvtColor(image, image_yuv, CV_BGR2YUV);
	std::vector<cv::Mat> yuv_channels;
	cv::split(image_yuv, yuv_channels);
	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe -> setClipLimit(4);
	clahe->apply(yuv_channels[0], yuv_channels[0]);
	cv::merge(yuv_channels, image_yuv);
	cv::cvtColor(image_yuv, image_clahe, CV_YUV2BGR);
	cv::Mat image_comb(std::max(image.rows, image_clahe.rows), image.cols + image_clahe.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	image.copyTo(image_comb(cv::Rect(0, 0, image.cols, image.rows)));
	image_clahe.copyTo(image_comb(cv::Rect(image.cols, 0,  image_clahe.cols, image_clahe.rows)));
	return image_comb;
}

//! Applies Canny edge detection to the image.
/*!
	\param cv::Mat object containing the image.
	\return cv::Mat object containing the transformed image.
	*/
cv::Mat apply_canny(cv::Mat image) {
	cv::Mat image_gray;
	cv::cvtColor(image, image_gray, CV_BGR2GRAY);
	cv::blur(image_gray, image_gray, cv::Size(3, 3));
	cv::Canny(image_gray, image_gray, 10, 10 * 3, 3);
	return image_gray;
}

//! Applies Hough transform to the image.
/*!
	\param cv::Mat object containing the image.
	\return Vector of detected lines.
	*/
std::vector<cv::Vec4f> apply_hough(cv::Mat image) {
	cv::Mat image_gray;
	image_gray = apply_canny(image);
	std::vector<cv::Vec4f> lines;
	cv::HoughLinesP(image_gray, lines, 1, CV_PI / 180, 10, 200, 10);
	double angle_threshold = 10.0;
	for (unsigned int i = 0; i < lines.size(); i++) {
		double angle = fabs(atan2(lines[i][0] - lines[i][2], lines[i][1] - lines[i][3])) * 180 / CV_PI;
		if (!(fabs(angle - 90) < angle_threshold || fabs(angle - 180) < angle_threshold || fabs(angle) < angle_threshold)) {
			lines.erase(lines.begin() + i);
			i--;
		}
	}
	return lines;
}

//! Crops image.
/*!
	With the computed approximate corners if the whiteboard a perspective transform is applied so the whiteboard is faced squarely.
	\param cv::Mat object containing the image.
	\return cv::Mat object containing the cropped image.
	*/
cv::Mat crop(cv::Mat image) {
	std::vector<cv::Vec4f> lines = apply_hough(image);
	std::vector<cv::Point2f> corners = find_corners(lines, image.rows, image.cols);
	std::sort(corners.begin(), corners.end(), [](cv::Point2i a, cv::Point2i b){return a.x * a.x + a.y *a.y < b.x * b.x + b.y * b.y;});
	int height = std::max(std::sqrt(std::pow(corners[0].x - corners[1].x, 2) + std::pow(corners[0].y - corners[1].y, 2)), std::sqrt(std::pow(corners[2].x - corners[3].x, 2) + std::pow(corners[2].y - corners[3].y, 2))); 
	int width = std::max(std::sqrt(std::pow(corners[0].x - corners[2].x, 2) + std::pow(corners[0].y - corners[2].y, 2)), std::sqrt(std::pow(corners[1].x - corners[3].x, 2) + std::pow(corners[1].y - corners[3].y, 2))); 
	std::vector<cv::Point2f> new_corners = {cv::Point2f(0, 0), cv::Point2f(width - 1, 0), cv::Point2f(width - 1, height - 1), cv::Point2f(0, height - 1)};
	corners.push_back(corners[1]);
	corners.erase(corners.begin() + 1);
	cv::Mat image_crop(height, width, image.type());
	cv::warpPerspective(image, image_crop, cv::getPerspectiveTransform(corners, new_corners), image_crop.size());
	return image_crop;
}

//! Displays given image.
/*!
	\param cv::Mat object containing the image.
	*/
void demo_draw(cv::Mat image) {
	cv::namedWindow("Demo", cv::WINDOW_NORMAL);
	cv::resizeWindow("Demo", 800, 600);
	cv::imshow("Demo", image);

	while (cv::waitKey() != 27) {
	}
}
