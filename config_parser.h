#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <exception>

namespace config_parser {


class ConfigParserException : public std::exception {
public:
    explicit ConfigParserException(const char *msg) noexcept : m_message(msg) {}

    ~ConfigParserException() noexcept final = default;

    const char *what() const noexcept final { return m_message; }

private:
    const char *m_message;
};

class ConfigParser {
public:
    ConfigParser() = default;

    ~ConfigParser() = default;

    void parse_file(const std::string &filename);

    void parse_string(const std::string &content);

    virtual void parse(std::istream &in) = 0;

    void write_file(const std::string &filename) const;;

    const std::string write_string() const;

    virtual void write(std::ostream &os) const = 0;
};

class IniParser : public ConfigParser {
public:
    using KeyType = std::string;
    using ValueType = std::string;
    using SectionType = std::unordered_map<KeyType, ValueType>;

    IniParser() = default;

    ~IniParser() = default;

    explicit IniParser(std::istream &stream) { parse(stream); }

    explicit IniParser(const std::string &filename) { parse_file(filename); }


    std::vector<KeyType> sections() const;

    std::vector<KeyType> options(const KeyType &section) const;

    const std::unordered_map<KeyType, ValueType> &items(const KeyType &section) const;


    bool has(const KeyType &section) const;

    bool has(const KeyType &section, const KeyType &option) const;

    void remove(const KeyType &section);

    void remove(const KeyType &section, const KeyType &option);

    template<typename T>
    void set(const KeyType &section,
             const KeyType &option,
             const T &value) {
        static_assert(std::is_fundamental<T>::value ||
                      std::is_same<T, std::string>::value, "Use fundamental type to get option");

        std::ostringstream os;
        os << value;
        set(section, option, os.str());
    }

    void set(const KeyType &section,
             const KeyType &option,
             const bool &value) {
        set(section, option, ValueType(value ? "true" : "false"));
    }

    void set(const KeyType &section,
             const KeyType &option,
             const char *value) {
        m_map[normalize_key(section)][normalize_key(option)] = std::string(value);
    }

    void set(const KeyType &section,
             const KeyType &option,
             const ValueType &value) {
        m_map[normalize_key(section)][normalize_key(option)] = value;
    }

    template<typename T>
    const T get(const KeyType &section,
                const KeyType &option) const {
        static_assert(std::is_fundamental<T>::value ||
                      std::is_same<T, std::string>::value, "Use fundamental type to get option");

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
        T store;
        parse_value(option_iter->second, store);
        return store;
    }

    template<typename T>
    const T get(const KeyType &section,
                const KeyType &option,
                const T &default_value) const {
        static_assert(std::is_fundamental<T>::value ||
                      std::is_same<T, std::string>::value, "Use fundamental type to get option");

        auto section_iter = m_map.find(normalize_key(section));
        if (section_iter == m_map.end())
            return default_value;
        auto option_iter = section_iter->second.find(normalize_key(option));
        if (option_iter == section_iter->second.end())
            return default_value;

        T store;
        parse_value(option_iter->second, store);
        return store;
    }

    void parse(std::istream &in) final;

    void write(std::ostream &os) const final;

private:
    std::unordered_map<KeyType, SectionType> m_map;

    std::string normalize_key(KeyType key) const;

    template<typename T>
    void parse_value(const ValueType &text, T &value) const {
        std::istringstream is(text);
        if (!(is >> value) || (is.rdbuf()->in_avail() != 0)) {
            std::string msg = "Value ‘" + text + "’ failed to parse";
            throw ConfigParserException(msg.c_str());
        }
    }

    void parse_value(const ValueType &text, bool &value) const;

    void parse_value(const ValueType &text, std::string &value) const;

};
}
