#include "quad.hpp"
#include "huff.hpp"
#include "proc.hpp"

int main(int argc, char** argv) {

	if (argc < 2) {
		printf("USAGE: unwb FILENAME [OUT_FILENAME]\n");
		return -1;
	}

	std::string filename = argv[1];
	filename = filename.substr(0, filename.find_last_of("."));
	dehuffman(filename);
	QuadTree q(filename);
	cv::Mat decomp = q.getImage(false);
	if (argc == 3)
		cv::imwrite(argv[2], decomp);
	else
		cv::imwrite(std::string(filename) + "_comp.jpg", decomp);

	return 0;
}
