#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include "bitwriter.hpp"
#include "bitreader.hpp"

void huffman(std::string filename) {
	std::map<char, int> freq;
	std::ifstream file(filename + ".qd");
	char tmp;
	int x, y;
	file >> x >> y;
	while (file >> tmp) {
		if (freq.find(tmp) == freq.end())
			freq[tmp] = 0;
		freq[tmp]++;
	}

	std::vector<std::pair<std::string, int>> tree;
	std::map<char, std::string> sym;
	for (auto it = freq.begin(); it != freq.end(); it++) {
		sym[it->first] = "";
		tree.push_back(std::make_pair(std::string(1, it->first), it->second));
	}

	while (tree.size() > 1) {
		std::sort(tree.begin(), tree.end(), [](std::pair<std::string, int> a, std::pair<std::string, int> b) {return a.second > b.second;});
		for (unsigned int i = 0; i < tree[tree.size() - 1].first.size(); i++)
			sym[tree[tree.size() - 1].first[i]] = "0" + sym[tree[tree.size() - 1].first[i]];
		for (unsigned int i = 0; i < tree[tree.size() - 2].first.size(); i++)
			sym[tree[tree.size() - 2].first[i]] = "1" + sym[tree[tree.size() - 2].first[i]];
		tree[tree.size() - 2].first += tree[tree.size() - 1].first;
		tree[tree.size() - 2].second += tree[tree.size() - 1].second;
		tree.pop_back();
	}
	file.close();

	int badbits = (13 - std::accumulate(sym.begin(), sym.end(), 0, [&freq](int sum, const std::map<char, std::string>::value_type& p){return sum + freq[p.first] * p.second.size();})) % 8;
	auto nth_badbit = [&badbits](int i) -> std::string {return (badbits >> 7 - (i & 7)) & 1 ? "1" : "0";};
	auto nth_sizebit = [](int size, int i) -> char {return (size >> 15 - (i & 15)) & 1 ? '1' : '0';};

	std::string sx = "", sy = "";
	std::generate_n(std::back_insert_iterator<std::string>(sx), 16, [x, nth_sizebit, i = 0]() mutable {return nth_sizebit(x, i++);});
	std::generate_n(std::back_insert_iterator<std::string>(sy), 16, [y, nth_sizebit, i = 0]() mutable {return nth_sizebit(y, i++);});

	file.open(filename + ".qd");
	BitWriter out(filename + ".wb");
	int tmpi;
	file >> tmpi >> tmpi;
	out.write(sx);
	out.write(sy);
	out.write(nth_badbit(5) + nth_badbit(6) + nth_badbit(7));
	while (file >> tmp)
		out.write(sym[tmp]);
	file.close();

	std::ofstream symfile(filename + ".sym");
	for (auto it = sym.begin(); it != sym.end(); it++)
		symfile << it->first << " " << it->second << "\n";
	symfile.close();
}

void dehuffman(std::string filename) {
	std::map<std::string, char> sym;
	std::ifstream symfile(filename + ".sym");
	char tmpc;
	std::string tmps;
	while (symfile >> tmpc >> tmps) 
		sym[tmps] = tmpc;
	symfile.close();

	auto get_bit = [](bool b, int i, int size){return ((int)b & 0x1) << (size - 1) - (i & (size - 1));};

	BitReader in(filename + ".wb");
	std::ofstream file(filename + ".qd");
	std::vector<bool> bits = in.read();	
	int x = 0, y = 0, badbits = 0;
	for (int i = 0; i < 16; i++)
		x |= get_bit(bits[i], i, 16);		
	for (int i = 0; i < 16; i++)
		y |= get_bit(bits[16 + i], i, 16);
	for (int i = 0; i < 3; i++)
		badbits |= get_bit(bits[32 + i], 5 + i, 8);
	bits.erase(bits.begin(), bits.begin() + 35);
	file << x << " " << y;
	tmps = "";
	while (bits.size() != 0) {
		for (int i = 0; i < bits.size(); i++) {
			if (i == bits.size() - badbits && !in.ready())
				break;
			tmps.push_back(bits[i] ? '1' : '0');
			if (sym.find(tmps) != sym.end()) {
				file << sym[tmps];
				tmps = "";
			}
		}
		bits = in.read();
	}
	file.close();
}
