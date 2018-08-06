#include "config_parser.h"

#include <regex>
#include <algorithm>

void config_parser::ConfigParser::parse_file(const std::string &filename) {
    std::ifstream file(filename);
    if (file.bad()) {
        std::string msg = "Unable to read " + filename;
        throw ConfigParserException(msg.c_str());
    }

    parse(file);
    file.close();
}

void config_parser::ConfigParser::parse_string(const std::string &content) {
    std::stringstream input_stream(std::ios::in | std::ios::out);
    input_stream.str(content);
    parse(input_stream);
}

void config_parser::ConfigParser::write_file(const std::string &filename) const {
    std::ofstream file(filename);
    if (file.bad()) {
        std::string msg = "Unable to read " + filename;
        throw ConfigParserException(msg.c_str());
    }
    write(file);
    file.close();
}

const std::string config_parser::ConfigParser::write_string() const {
    std::stringstream out;
    write(out);
    return out.str();
}

std::vector<config_parser::IniParser::KeyType> config_parser::IniParser::sections() const {
    std::vector<KeyType> keys;
    keys.reserve(m_map.size());
    for (const auto &kv : m_map) {
        keys.push_back(kv.first);
    }
    return keys;
}

std::vector<config_parser::IniParser::KeyType>
config_parser::IniParser::options(const config_parser::IniParser::KeyType &section) const {
    auto section_iter = m_map.find(normalize_key(section));
    if (section_iter == m_map.end()) {
        std::string msg = "Section ‘" + section + "’ not present";
        throw ConfigParserException(msg.c_str());
    }
    std::vector<KeyType> options;
    options.reserve(section_iter->second.size());
    for (const auto &okv : section_iter->second) {
        options.push_back(okv.first);
    }
    return options;
}

const std::unordered_map<config_parser::IniParser::KeyType, config_parser::IniParser::ValueType> &
config_parser::IniParser::items(const config_parser::IniParser::KeyType &section) const {
    auto section_iter = m_map.find(normalize_key(section));
    if (section_iter == m_map.end()) {
        std::string msg = "Section ‘" + section + "’ not present";
        throw ConfigParserException(msg.c_str());
    }
    return section_iter->second;
}

bool config_parser::IniParser::has(const config_parser::IniParser::KeyType &section) const {
    auto section_iter = m_map.find(normalize_key(section));
    return (section_iter != m_map.end());
}

bool config_parser::IniParser::has(const config_parser::IniParser::KeyType &section,
                                       const config_parser::IniParser::KeyType &option) const {
    auto section_iter = m_map.find(normalize_key(section));
    if (section_iter == m_map.end())
        return false;
    auto option_iter = section_iter->second.find(normalize_key(option));
    return (option_iter != section_iter->second.end());
}

void config_parser::IniParser::remove(const config_parser::IniParser::KeyType &section) {
    auto section_iter = m_map.find(normalize_key(section));
    if (section_iter == m_map.end()) {
        std::string msg = "Section ‘" + section + "’ not present";
        throw ConfigParserException(msg.c_str());
    }
    m_map.erase(section_iter);
}

void config_parser::IniParser::remove(const config_parser::IniParser::KeyType &section,
                                          const config_parser::IniParser::KeyType &option) {
    auto section_iter = m_map.find(normalize_key(section));
    if (section_iter == m_map.end()) {
        std::string msg = "Section ‘" + section + "’ not present";
        throw ConfigParserException(msg.c_str());
    }
    auto option_iter = section_iter->second.find(normalize_key(option));
    if (option_iter == section_iter->second.end()) {
        std::string msg = "Option ‘" + option + "’ not present";
        throw ConfigParserException(msg.c_str());
    }
    section_iter->second.erase(option_iter);
}

void config_parser::IniParser::parse(std::istream &in) {
    static const std::regex empty_regex{R"x(\s*)x"};
    static const std::regex comment_regex{R"x(\s*[;#])x"};
    static const std::regex section_regex{R"x(\s*\[([^\]]+)\])x"};
    static const std::regex value_regex{R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*$)x"};

    std::string current_section;
    std::smatch match;
    std::size_t line_number = 0;
    for (std::string line; std::getline(in, line);) {
        ++line_number;
        if (line.empty()
            || std::regex_match(line, match, empty_regex)
            || std::regex_match(line, match, comment_regex)) {
            continue;
        } else if (std::regex_match(line, match, section_regex)) {
            if (match.size() == 2) { // exactly one match
                current_section = match[1].str();
            }
        } else if (std::regex_match(line, match, value_regex)) {
            if (match.size() == 4) { // exactly enough matches
                m_map[current_section][match[1].str()] = match[2].str();
            }
        } else {
            std::string msg = "Failed to parse line " + std::to_string(line_number) + ": '" + line + "'";
            throw ConfigParserException(msg.c_str());
        }
    }
}

void config_parser::IniParser::write(std::ostream &os) const {
    for (const auto &section : m_map) {
        os << "[" << section.first << "]" << std::endl;
        for (const auto &option: section.second) {
            os << option.first << " = " << option.second << std::endl;
        }
        os << std::endl;
    }
}

std::string config_parser::IniParser::normalize_key(config_parser::IniParser::KeyType key) const {
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key;
}

void config_parser::IniParser::parse_value(const config_parser::IniParser::ValueType &text, bool &value) const {
    auto value_str(text);
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(value_str.begin(), value_str.end(), value_str.begin(), ::tolower);
    if (value_str == "true" || value_str == "yes" || value_str == "on" || value_str == "1")
        value = true;
    else if (value_str == "false" || value_str == "no" || value_str == "off" || value_str == "0")
        value = false;
    else {
        std::string msg = "Value ‘" + text + "’ failed to parse as boolean";
        throw ConfigParserException(msg.c_str());
    }
}

void
config_parser::IniParser::parse_value(const config_parser::IniParser::ValueType &text, std::string &value) const {
    value = text;
}
