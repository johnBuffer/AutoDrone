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

	static uint64_t getDnaCount(const std::string& filename, uint64_t bytes_count)
	{
		return filesize(filename.c_str()) / bytes_count;
	}

	static DNA loadDnaFrom(const std::string& filename, uint64_t bytes_count, uint64_t offset, bool from_end = false)
	{
		std::ifstream infile(filename, std::ios_base::binary);

		DNA dna(bytes_count * 8);
		if (!infile) {
			std::cout << "Error when trying to open file." << std::endl;
			return dna;
		}

		if (!from_end) {
			infile.seekg(offset * bytes_count, std::ios_base::beg);
		}
		else {
			infile.seekg(offset * bytes_count, std::ios_base::end);
		}

		std::vector<float> test_vec(202);
		if (!infile.read((char*)test_vec.data(), bytes_count)) {
			std::cout << "Error while reading file." << std::endl;
		}

		infile.seekg(offset * bytes_count, std::ios_base::beg);
		if (!infile.read((char*)dna.code.data(), bytes_count)) {
			std::cout << "Error while reading file." << std::endl;
		}

		return dna;
	}

	static void writeDnaToFile(const std::string& filename, const DNA& dna)
	{
		std::ofstream outfile(filename, std::istream::out | std::ios::binary | std::ios::app);
		const uint64_t element_count = dna.code.size();
		outfile.write((char*)dna.code.data(), element_count);
		outfile.close();
	}
};

