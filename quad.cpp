#include <string>
#include "quad.hpp"

double QuadTree::Node::diffThreshold = 45.0;

std::map<char, Color> QuadTree::Node::colorMap = QuadTree::Node::initializeColorMap();

std::map<char, Color> QuadTree::Node::initializeColorMap() {
	std::map<char, Color> res;
	res['k'] = Color::BLACK;
	res['b'] = Color::BLUE;
	res['g'] = Color::GREEN;
	res['r'] = Color::RED;
	res['w'] = Color::WHITE;
	return res;
}

QuadTree::Node::Node() : nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr) {}

QuadTree::Node::Node(cv::Mat image) : Node() {
	cv::Scalar mean, dev;
	cv::meanStdDev(image, mean, dev);
	int r = image.rows / 2, c = image.cols / 2;
	double min, max;
	cv::minMaxLoc(image, &min, &max);
	if (r != 0 && c != 0 && (max - min) > diffThreshold) {
		nw = new Node(image(cv::Range(0, r), cv::Range(0, c)));
		ne = new Node(image(cv::Range(0, r), cv::Range(c, image.cols)));
		sw = new Node(image(cv::Range(r, image.rows), cv::Range(0, c)));
		se = new Node(image(cv::Range(r, image.rows), cv::Range(c, image.cols)));
	} else
		color = scalar2Color(mean);
}

QuadTree::Node::Node(std::ifstream& file) : Node() {
	char tmp;
	file >> tmp;
	if (tmp == '|') {
		nw = new Node(file);
		ne = new Node(file);
		sw = new Node(file);
		se = new Node(file);
	} else
		setColor(tmp);
}

void QuadTree::Node::setColor(char _color) {
	color = QuadTree::Node::colorMap[_color];
}

Color QuadTree::Node::scalar2Color(cv::Scalar s) {
	std::vector<int> v;
	v.push_back(cv::norm(s - cv::Scalar(0, 0, 0), cv::NORM_INF));
	cv::Scalar c;
	for (int i = 0; i < 3; i++) {
		c = cv::Scalar(0, 0, 0);
		c[i] = 255;
		v.push_back(cv::norm(s - c, cv::NORM_INF));
	}
	v.push_back(cv::norm(s - cv::Scalar(255, 255, 255), cv::NORM_INF));
	return static_cast<Color>(std::distance(v.begin(), std::min_element(v.begin(), v.end())));
}

cv::Scalar QuadTree::Node::color2Scalar(Color c) {
	cv::Scalar s;
	switch (c) {
		case Color::BLACK:
			s = cv::Scalar(0, 0, 0);
			break;
		case Color::BLUE:
			s = cv::Scalar(255, 0, 0);
			break;
		case Color::GREEN:
			s = cv::Scalar(0, 255, 0);
			break;
		case Color::RED:
			s = cv::Scalar(0, 0, 255);
			break;
		case Color::WHITE:
			s = cv::Scalar(255, 255, 255);
			break;
		default:
			s = cv::Scalar(255, 255, 255);
			break;
	}
	return s;
}

char QuadTree::Node::color2String(Color c) {
	char s;
	switch(c) {
		case Color::BLACK:
			s = 'k';
			break;
		case Color::BLUE:
			s = 'b';
			break;
		case Color::GREEN:
			s = 'g';
			break;
		case Color::RED:
			s = 'r';
			break;
		case Color::WHITE:
			s = 'w';
			break;
	}
	return s;
}

void QuadTree::Node::destroy() {
	if (nw != nullptr) {
		nw->destroy();
		ne->destroy();
		sw->destroy();
		se->destroy();
	}
	delete this;
	(void*)0;
}

void QuadTree::Node::print(std::ofstream& file) {
	if (nw != nullptr) {
		file << "|";
		nw->print(file);
		ne->print(file);
		sw->print(file);
		se->print(file);
	} else
		file << color2String(color);
}

QuadTree::QuadTree(std::string filename) {
	std::ifstream file(filename + std::string(".qd"));
	file >> size_x >> size_y;
	root = new Node(file);
	file.close();
}

QuadTree::QuadTree(cv::Mat image) {
	size_x = image.cols;
	size_y = image.rows;
	root = new Node(image);
}

void QuadTree::print(std::string filename) {
	std::ofstream file(filename + std::string(".qd"));
	file << size_x << " " << size_y;
	root->print(file);
	file.close();
}

QuadTree::~QuadTree() {
	root->destroy();
}

cv::Mat QuadTree::Node::compose(int x, int y, bool grid) {
	cv::Mat image(y, x, CV_8UC3);
	if (nw != nullptr) {
		int r = image.rows / 2, c = image.cols / 2;	
		nw->compose(c, r, grid).copyTo(image(cv::Range(0, r), cv::Range(0, c)));
		ne->compose(image.cols - c, r, grid).copyTo(image(cv::Range(0, r), cv::Range(c, image.cols)));
		sw->compose(c, image.rows - r, grid).copyTo(image(cv::Range(r, image.rows), cv::Range(0, c)));
		se->compose(image.cols - c, image.rows - r, grid).copyTo(image(cv::Range(r, image.rows), cv::Range(c, image.cols)));
		if (grid) {
			cv::line(image, cv::Point(0, r), cv::Point(image.cols, r), cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
			cv::line(image, cv::Point(c, 0), cv::Point(c, image.rows), cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
		}
	} else
		image = color2Scalar(color);
	return image;
}

cv::Mat QuadTree::getImage(bool grid) {
	return root->compose(size_x, size_y, grid);
}
