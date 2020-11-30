#pragma once
#include <fstream>
#include "dna.hpp"


struct DnaLoader
{
	static std::ifstream::pos_type filesize(const char* filename)
	{
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	}

	static DNA loadDnaFrom(const std::string& filename, uint64_t bytes_count, uint64_t offset, bool from_end = false)
	{
		std::ifstream infile(filename, std::ios::binary);

		std::cout << "Number of DNAs in file: " << filesize(filename.c_str()) / bytes_count << std::endl;

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
		
		if (!infile.read((char*)dna.code.data(), bytes_count)) {
			std::cout << "Error while reading file." << std::endl;
		}

		std::cout << "First value: " << dna.get<float>(0) << std::endl;

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
