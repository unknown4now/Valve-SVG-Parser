#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct svgheader
{
    uint16_t hash;
    uint8_t pad[30];
    uint16_t svg_data;
};

struct SVGData
{
    uint8_t pad[38];
    char svg_string[];
};

namespace svg
{
    std::string parse(const std::vector<char>& file_bytes);
}