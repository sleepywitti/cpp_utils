#pragma once

#include <string>
#include <vector>
#include <locale>

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>

namespace utils
{
namespace string
{
// removes leading and trailing white spaces
// credit goes to: http://www.cplusplus.com/forum/general/115700/
template< typename C, typename T, typename A >
std::basic_string<C,T,A> trim( const std::basic_string<C,T,A>& str,
                               const std::locale& loc = std::locale::classic() )
{
    auto begin = str.begin() ; // first non ws from left
    while( begin != str.end() && std::isspace( *begin, loc ) ) ++begin ;
    if( begin == str.end() ) return {};

    auto rbegin = str.rbegin() ; // first non ws from right
    while( rbegin != str.rend() && std::isspace( *rbegin, loc ) ) ++rbegin ;

    return { begin, rbegin.base() } ;
}
template std::basic_string<char> trim( const std::basic_string<char>& str,
                               const std::locale& loc = std::locale::classic());
template std::basic_string<wchar_t> trim( const std::basic_string<wchar_t>& str,
                                       const std::locale& loc = std::locale::classic());

template< typename C, typename T, typename A >
std::basic_string<C,T,A> ltrim( const std::basic_string<C,T,A>& str,
                               const std::locale& loc = std::locale::classic() )
{
    auto begin = str.begin() ; // first non ws from left
    while( begin != str.end() && std::isspace( *begin, loc ) ) ++begin ;
    return { begin, str.rbegin().base() } ;
}

template< typename C, typename T, typename A >
std::basic_string<C,T,A> rtrim( const std::basic_string<C,T,A>& str,
                                const std::locale& loc = std::locale::classic() )
{
    auto rbegin = str.rbegin() ; // first non ws from right
    while( rbegin != str.rend() && std::isspace( *rbegin, loc ) ) ++rbegin ;
    return { str.begin(), rbegin.base() } ;
}

inline bool ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string to_lower(const std::string& str);

// split string at every delimiter and return vector with cut strings
std::vector<std::string> split(const std::string &s, char delim);

std::string replace(const std::string& str, const std::string& from, const std::string& to);
std::string replace_all(const std::string& str, const std::string& from, const std::string& to);

std::string string_format(const std::string fmt_str, ...);

}
}

