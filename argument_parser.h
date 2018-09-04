/**
 *   - short options start with a hyphen `-` followed by a alpha character
 *   - long options start with two hyphens `--` followed by a alpha character and
 *     at least another alphanumeric character, dash or underscore
 *   - multiple options can be grouped: `-abc` is equivalent to `-a -b -c`
 *   - options may require one or multiple value
 *   - values can be set in the following ways: `-n/value`, `-n value` `-n=value`, `-n=`
 *   - `--` will treat all following arguments as positional arguments
 *   - options and positional arguments can not be interleaved
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <sstream>
#include <limits>

namespace utils {
class UsageException : public std::runtime_error {
public:
    explicit UsageException(const std::string& msg) noexcept : std::runtime_error(msg) {}
};

class ParsingException : public std::runtime_error {
public:
    explicit ParsingException(const std::string& msg) noexcept : std::runtime_error(msg) {}
};

class argument_parser {
public:
    using ArgumentType = std::string;
    using ArgumentListType = std::vector<ArgumentType>;
    using OptionNameType = ArgumentType;
    using OptionNameSetType = std::unordered_set<OptionNameType>;
    using StorageValueType = ArgumentType;
    using StorageType = std::vector<StorageValueType>;
    using ChoiceStorageType = std::vector<std::unordered_set<StorageValueType>>;
    using HelpTextType = std::string;
    using MetaVarListType = std::vector<HelpTextType>;
    using ArgumentCountType = unsigned int;
    static const ArgumentCountType unlimited_positionals{std::numeric_limits<ArgumentCountType>::max()};

    argument_parser() = default;

    argument_parser(const argument_parser&) = delete;

    ~argument_parser() = default;

    void set_help_info(const HelpTextType& preamble, const HelpTextType& epilog);

    inline const HelpTextType& help_epilog() const { return m_help_epilog; }

    inline const HelpTextType& help_preamble() const { return m_help_preamble; }

    void set_program_info(const HelpTextType& program_name, const HelpTextType& version = "");

    inline const HelpTextType& program_name() const { return m_program_name; }

    inline const HelpTextType& program_version() const { return m_program_version; }

    void add_flag(const OptionNameSetType& names, const HelpTextType& help = "");

    void add_option(const OptionNameSetType& names,
                    const HelpTextType& help = "");

    void add_option(const OptionNameSetType& names,
                    const HelpTextType& help, const HelpTextType& meta_var);

    void add_option(const OptionNameSetType& names,
                    const HelpTextType& help, const HelpTextType& meta_var,
                    const StorageValueType& default_value);

    void add_option(const OptionNameSetType& names, ArgumentCountType number_of_arguments,
                    const HelpTextType& help = "",
                    const std::vector<HelpTextType>& meta_vars = {},
                    const StorageType& default_values = {},
                    const ChoiceStorageType& choices = {});

    bool has_option(const OptionNameType& name) const;

    void parse(int argc, char** argv);

    void parse(int argc, const char** argv);

    void parse(const ArgumentListType& argv);

    bool is_parsed(const OptionNameType& name) const;

    void reset_storage();

    template<typename T>
    T get(const OptionNameType& name) const {
        const auto& option = find_option(name);
        if (option.number_of_arguments() != 0 && option.number_of_arguments() != 1) {
            throw UsageException("Invalid number of arguments used for getting the option");
        }
        auto storage = option.get<T>();
        return storage[0];
    }

    template<typename T>
    std::vector<T> get_n(const OptionNameType& name) const {
        const auto& option = find_option(name);
        return option.get<T>();
    }

    void set_required(const OptionNameSetType& names);

    void set_hidden(const OptionNameSetType& names);

    void set_appending_arguments(const OptionNameSetType& names);

    void add_xor(const OptionNameSetType& names);

    void set_required_positionals(ArgumentCountType minimum_positionals);

    void set_allowed_positionals(ArgumentCountType maximum_positionals);

    void set_positional_help(const HelpTextType& help, const HelpTextType& meta_var = "POSITIONALS");

    bool has_positionals() const;

    const StorageType& get_positionals() const;

    std::string help() const;

private:
    class Option {
    public:
        explicit Option(ArgumentCountType number_of_arguments, ChoiceStorageType choices = {});

        void parse(const StorageType& values);

        void set_appending(bool is_appending);

        inline bool is_appending() const { return m_is_appending; }

        inline void set_hidden(bool is_hidden) { m_is_hidden = is_hidden; }

        inline bool is_hidden() const { return m_is_hidden; }

        inline void set_required(bool is_required) { m_is_required = is_required; }

        inline bool is_required() const { return m_is_required; }

        void set_default(const StorageType& values);

        void set_meta_vars(const MetaVarListType& meta_vars);

        inline void set_help(const HelpTextType& help) { m_help = help; }

        void set_names(const OptionNameSetType& names);

        inline bool has_name(const OptionNameType& name) const;

        inline void reset_storage() { m_storage.clear(); }

        inline bool has_default() const { return !m_default_storage.empty(); }

        inline ArgumentCountType number_of_arguments() const { return m_number_of_arguments; }

        inline bool is_parsed() const { return (!m_storage.empty()); }

        inline std::string get_name() const;

        template<typename T>
        const std::vector<T> get() const {
            static_assert(std::is_fundamental<T>::value ||
                          std::is_same<T, std::string>::value, "Use fundamental type to get option");

            std::vector<T> storage;
            const StorageType* ref_storage;
            if (is_parsed()) {
                ref_storage = &m_storage;
            } else if (has_default()) {
                ref_storage = &m_default_storage;
            } else {
                throw ParsingException("Failed to get unparsed error");
            }

            for (auto& ref_value : (*ref_storage)) {
                T value;
                parse_value(ref_value, value);
                storage.emplace_back(value);
            }
            return storage;
        }

        std::pair<std::string, std::string> generate_help_text() const;

    protected:
        template<typename T>
        static void parse_value(const StorageValueType& text, T& value) {
            std::istringstream is(text);
            if (!(is >> value) || (is.rdbuf()->in_avail() != 0)) {
                throw ParsingException("Argument ‘" + text + "’ failed to parse");
            }
        }

        static void parse_value(const StorageValueType& text, bool& value);

        static void parse_value(const StorageValueType& text, std::string& value);

        OptionNameSetType m_names;
        HelpTextType m_help;
        MetaVarListType m_meta_vars;
        ArgumentCountType m_number_of_arguments{0};
        StorageType m_storage{};
        StorageType m_default_storage{};
        ChoiceStorageType m_choices;
        bool m_is_appending{false};
        bool m_is_hidden{false};
        bool m_is_required{false};
    };

    using OptionStorage = std::vector<Option>;

    void ensure_valid_option_list(const OptionNameSetType& names) const;

    OptionStorage::iterator find_option_iter(const OptionNameType& name);

    OptionStorage::const_iterator find_option_iter(const OptionNameType& name) const;

    Option& find_option_to_parse(const OptionNameType& name);

    Option& find_option(const OptionNameType& name);

    const Option& find_option(const OptionNameType& name) const;

    bool is_short_option_name(const ArgumentType& arg) const;

    bool is_short_option_group_name(const ArgumentType& arg) const;

    bool is_long_option(const ArgumentType& arg) const;

    OptionNameType strictly_normalize_option_name(const OptionNameType& name) const;

    OptionNameType normalize_option_name(const OptionNameType& name) const;

    std::pair<OptionNameType, StorageValueType> split_argument_text(const ArgumentType& text) const;

    void check_required_arguments() const;

    void check_xor_arguments() const;

    void set_program_name_from_argv(const ArgumentType& argv0);

    HelpTextType m_program_name{};
    HelpTextType m_program_version{};
    HelpTextType m_help_preamble{};
    HelpTextType m_help_epilog{};

    ArgumentCountType m_number_of_minimum_positionals{0};
    ArgumentCountType m_number_of_maximum_positionals{0};
    HelpTextType m_positional_help{};
    HelpTextType m_positional_meta_var{};

    StorageType m_positionals;
    std::vector<OptionNameSetType> m_xor_lists;

    OptionStorage m_options;
};

} // namespace utils
