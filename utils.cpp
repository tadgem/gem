#include "utils.h"
#include <fstream>
#include <sstream>

std::string utils::load_string_from_path(const std::string& path)
{
    std::ifstream in(path);
    std::stringstream stream;
    if (!in.is_open())
    {
        return "";
    }

    stream << in.rdbuf();
    return stream.str();
}
