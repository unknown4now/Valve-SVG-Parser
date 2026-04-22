#include "parser.h"
#include <algorithm>
#include <cctype>


namespace svg
{
	std::string parse(const std::vector<char>& file_bytes)
	{
		if (file_bytes.size() < sizeof(svgheader))
		{
			return "";
		}

		const auto header = reinterpret_cast<const svgheader*>(file_bytes.data());
		size_t xml_start = (size_t)header->svg_data + sizeof(svgheader) - sizeof(char*);

		if (xml_start >= file_bytes.size())
		{
			return "";
		}

		std::string str(file_bytes.begin() + xml_start, file_bytes.end());

		if (str.empty())
		{
			return "";
		}

		while (!str.empty() && str.back() == '\0')
		{
			str.pop_back();
		}

		size_t f = str.find("<?xml");
		if (f != std::string::npos)
		{
			size_t e = str.find("?>", f);
			if (e != std::string::npos)
			{
				str.erase(f, (e + 2) - f);
			}
		}

		f = str.find("<!DOCTYPE"); //this crap
		if (f != std::string::npos)
		{
			size_t e = str.find(">", f);
			if (e != std::string::npos)
			{
				str.erase(f, (e + 1) - f);
			}
		}

		f = 0;
		while ((f = str.find("<!--", f)) != std::string::npos)
		{
			size_t e = str.find("-->", f);
			if (e == std::string::npos)
			{
				break;
			}
			str.erase(f, (e + 3) - f);
		}

		size_t sOpen = str.find("<svg");
		if (sOpen != std::string::npos)
		{
			size_t sEnd = str.find(">", sOpen);
			if (sEnd != std::string::npos)
			{
				str.erase(0, sEnd + 1);
			}
		}

		size_t sClose = str.rfind("</svg>");
		if (sClose != std::string::npos)
		{
			str.erase(sClose);
		}

		while (!str.empty() && std::isspace((unsigned char)str.front()))
		{
			str.erase(0, 1);
		}

		while (!str.empty() && std::isspace((unsigned char)str.back()))
		{
			str.pop_back();
		}

		return str;


	}
}