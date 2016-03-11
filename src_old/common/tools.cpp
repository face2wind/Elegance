#include "common/tools.hpp"
#include <sstream>

namespace face2wind
{
	std::string BytesToSizeStr(unsigned long long size)
	{
		unsigned int KB = 1024;
		unsigned int MB = KB * 1024;
		unsigned int GB = MB * 1024;
		std::string unit;
		long double file_size = (long double)size;
		if (file_size > GB)
		{
			file_size /= GB;
			unit = "GB";
		}
		else if (file_size > MB)
		{
			file_size /= MB;
			unit = "MB";
		}
		else if (file_size > KB)
		{
			file_size /= KB;
			unit = "KB";
		}
		else
		{
			unit = "B";
		}
		file_size = int(file_size * 10 + 0.5) / 10.0; // Keep 1 decimal places
		
		std::stringstream strstream;
		strstream << file_size << unit;
		return strstream.str();
	}
}