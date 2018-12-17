#include "string_utils.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <cstdarg> 

namespace utils {
namespace string {
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        item = trim(item);
        if (!item.empty()) { elems.push_back(item); }
    }
    return elems;
}

std::string to_lower(const std::string& str) {
    std::string ret(str);
    std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
    return ret;
}

// https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
std::string replace(const std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return std::string();
    }
    std::string ret(str);
    ret.replace(start_pos, from.length(), to);
    return ret;
}

std::string replace_all(const std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return std::string();
    }
    size_t start_pos = 0;
    std::string ret(str);
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        ret.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return ret;
}

std::string appendNumber(std::string base, size_t number, long length) {
    std::ostringstream number_osst;
    number_osst.setf(std::ios::right, std::ios::adjustfield); // padd from left
    number_osst.setf(std::ios::dec, std::ios::basefield);
    number_osst.width(length);
    number_osst << number;

    return (base + number_osst.str());
}

// credit goes to https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
std::string string_format(const std::string fmt_str, ...) {
    // Reserve two times as much as the length of the fmt_str
    int final_n, n = ((int) fmt_str.size()) * 2;
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1) {
        // Wrap the plain char array into the unique_pt
        formatted.reset(new char[n]);
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], (size_t) n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n) {
            n += abs(final_n - n + 1);
        }
        else {
            break;
        }
    }
    return std::string(formatted.get());
}
}
}
