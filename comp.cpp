#include "quad.hpp"
#include "huff.hpp"
#include "proc.hpp"

/*! \mainpage Algorithm outline
 * The compression algorithm can be broken down to the folllowing steps:
 * 	- locate the whiteboard and crop image,
 * 	- image decomposition using a quadtree,
 * 	- write quadtree to file
 * 	- compress file containing the quadtree (Huffman code).
 *
 * Decompression is just the above steps backwards. The lossy part of the algorithm is the image decompoosition, i.e., if the user compresses an image multiple times they won't lose data after the first compression.
 *
 * Below is an example of compressing an image. The original file size is around 2.6 MB and the compressed file size is 88 KB.
 *
 * \image html img.jpg width=640px
 * \image html img_comp.jpg width=640px
 */

int main(int argc, char** argv) {

	if (argc < 2) {
		printf("USAGE: wb [-d] FILENAME\n");
		return -1;
	}

	bool demo = false;

	for (int i = 1; i < argc; i++)
		if (argv[i][0] == '-')
			if (argv[i][1] == 'd')
				demo = true;

	cv::Mat image = cv::imread(argv[argc - 1]);

	if (demo) {
		demo_draw(image);

		cv::Mat canny = apply_canny(image);
		demo_draw(canny);

		cv::Mat canny_lines;
		cv::cvtColor(canny, canny_lines, CV_GRAY2BGR);
		std::vector<cv::Vec4f> lines = apply_hough(image);
		for (unsigned int i = 0; i < lines.size(); i++) {
			cv::Vec4f line = lines[i];
			cv::line(canny_lines, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
		}
		demo_draw(canny_lines);

		cv::Mat canny_circles;
		cv::cvtColor(canny, canny_circles, CV_GRAY2BGR);
		std::vector<cv::Point2f> points;
		for (unsigned int i = 0; i < lines.size(); i++)
			for (unsigned int j = 0; j < lines.size(); j++)
				if (i != j) {
					cv::Point2f tmp = intersect(lines[i], lines[j]);
					if (0 <= tmp.x && tmp.x < canny_circles.cols && 0 <= tmp.y && tmp.y < canny_circles.rows)
						points.push_back(tmp);
				}
		for (unsigned int i = 0; i < points.size(); i++)
			cv::circle(canny_circles, points[i], 15, cv::Scalar(255, 0, 0), 8, cv::LINE_AA);
		demo_draw(canny_circles);

		cv::Mat canny_corners;
		cv::cvtColor(canny, canny_corners, CV_GRAY2BGR);
		std::vector<cv::Point2f> corners = find_corners(lines, canny_corners.rows, canny_corners.cols);
		for (unsigned int i = 0; i < corners.size(); i++)
			cv::circle(canny_corners, corners[i], 15, cv::Scalar(255, 0, 0), 8, cv::LINE_AA);
		demo_draw(canny_corners);

		cv::Mat cropped = crop(image);
		demo_draw(cropped);
		double alpha = 1.0, beta = 20, gamma = 0.9;
		for (int y = 0; y < cropped.rows; y++)
			for (int x = 0; x < cropped.cols; x++)
				for (int c = 0; c < cropped.channels(); c++)
					cropped.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(alpha * cropped.at<cv::Vec3b>(y, x)[c] + beta);
		cv::Mat lut(1, 256, CV_8U);
		uchar* p = lut.ptr();
		for (int i = 0; i < 256; i++)
			p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
		cv::LUT(cropped, lut, cropped);

		demo_draw(cropped);
	}

	image = crop(image);

	double alpha = 1.0, beta = 20, gamma = 0.9;
	for (int y = 0; y < image.rows; y++)
		for (int x = 0; x < image.cols; x++)
			for (int c = 0; c < image.channels(); c++)
				image.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(alpha * image.at<cv::Vec3b>(y, x)[c] + beta);

	cv::Mat lut(1, 256, CV_8U);
	uchar* p = lut.ptr();
	for (int i = 0; i < 256; i++)
		p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
	cv::LUT(image, lut, image);

	QuadTree q(image);
	std::string filename = argv[argc - 1];
	filename = filename.substr(0, filename.find_last_of("."));
	q.print(filename);
	huffman(filename);

	if (demo) {
		dehuffman(filename);
		QuadTree qq = QuadTree(filename);
		cv::Mat comb = cv::Mat(image.rows, 2 * image.cols, CV_8UC3);
		image.copyTo(comb(cv::Range(0, image.rows), cv::Range(0, image.cols)));
		cv::Mat decomp = qq.getImage(true);
		decomp.copyTo(comb(cv::Range(0, image.rows), cv::Range(image.cols, 2 * image.cols)));

		cv::namedWindow("Demo", cv::WINDOW_NORMAL);
		cv::resizeWindow("Demo", 1600, 600);
		cv::imshow("Demo", comb);

		while (cv::waitKey() != 27) {
		}
	}

	return 0;
}
