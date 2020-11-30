#pragma once
#include <fstream>
#include "dna.hpp"


struct DnaLoader
{
	static DNA loadDnaFrom(const std::string& filename, uint64_t bytes_count, uint64_t offset, bool from_end = false)
	{
		std::ifstream infile(filename, std::ios::binary);

		DNA dna(bytes_count * 8);
		if (!infile) {
			std::cout << "Error when trying to open file." << std::endl;
			return dna;
		}

		if (!from_end) {
			infile.seekg(offset * bytes_count, std::ios::beg);
		}
		else {
			infile.seekg(offset * bytes_count, std::ios::end);
		}

		std::vector<uint8_t> right_order = dna.code;
		for (uint64_t i(0); i < right_order.size(); ++i) {
			right_order[i] = dna.code[right_order.size() - 1 - i];
		}
		dna.code = right_order;
		
		if (!infile.read((char*)dna.code.data(), bytes_count)) {
			std::cout << "Error while reading file." << std::endl;
		}

		return dna;
	}

	static void writeDnaToFile(const std::string& filename, const DNA& dna)
	{
		std::ofstream outfile(filename, std::ios::ate);
		const uint64_t element_count = dna.getElementsCount<float>();
		for (uint64_t i(0); i<element_count; ++i) {
			float value = dna.get<float>(i);
			outfile.write((char*)&value, sizeof(float));
		}
		outfile.close();
	}
};
