#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>

#include "trie.hpp"

#define DOMAIN_MAX_LENGTH	15

int main(int argc, char* argv[]) {
	Trie trie;

	std::ifstream fin(argv[1], std::ios_base::in);
	std::string line;

	// build a trie tree
	auto start = std::chrono::system_clock::now();
	while (std::getline(fin, line)) {
		std::reverse(line.begin(), line.end());
		trie.insert(line);
	}
	auto end = std::chrono::system_clock::now();
	auto duration = duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Build Trie Tree: " << duration.count() << "(ms)" << std::endl;

	// get search domains
	std::ifstream fin(argv[2], std::ios_base::in);
	std::vector<std::string> domains;
	while (std::getline(fin, line)) {
		if (line.length() > DOMAIN_MAX_LENGTH) {
			std::reverse(line.begin(), line.end());
			domains.push_back(line);
		}
	}

	// search domain
	start = std::chrono::system_clock::now();
	int count = 0;
	for (int i = 0; i < 1000; i++) {
		for (auto domain: domains) {
			count += trie.search(domain);
		}
	}
	end = std::chrono::system_clock::now();
	duration = duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Match: " << count << std::endl;
	std::cout << "Search Trie Tree: " << duration.count() << "(ms)" << std::endl;
	return 0;
}
