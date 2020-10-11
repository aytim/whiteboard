#include "bitwriter.hpp"

BitWriter::BitWriter(std::string file) : size(8), counter(0) {
	out.open(file, std::ios::binary);
}

BitWriter::~BitWriter() {
	flush();
	out.close();
}

void BitWriter::flush() {
	out.write((const char*)&(buffer[0]), buffer.size());
	buffer.clear();
	counter = 0;
}

void BitWriter::setBit(int index, int value) {
	buffer[index / sizeof(char) / 8] |= (value & 1) << 7 - (index & 7);
}

void BitWriter::writeBit(int value) {	
	if (counter == buffer.size() * 8)
		buffer.push_back(0);
	setBit(counter, value);
	counter++;
	if (counter == size * sizeof(char) * 8)
		flush();
}

void BitWriter::write(std::string s) {
	for (unsigned int i = 0; i < s.size(); i++)
		writeBit(std::stoi(std::string(1, s[i])));
}
