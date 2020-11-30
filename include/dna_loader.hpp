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
		
		if (!infile.read((char*)dna.code.data(), bytes_count)) {
			std::cout << "Error while reading file." << std::endl;
		}

		return dna;
	}

	static void writeDnaToFile(const std::string& filename, const DNA& dna)
	{
		std::ofstream outfile(filename, std::ios_base::app);
		const uint64_t element_count = dna.code.size();
		outfile.write((char*)dna.code.data(), element_count);
		outfile.close();
		
		const uint64_t float_count = dna.getElementsCount<float>();
		for (uint64_t i(0); i < float_count; ++i) {
			std::cout << dna.get<float>(i) << std::endl;
		}
	}
};
