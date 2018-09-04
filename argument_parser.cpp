#include "argument_parser.h"

#include <algorithm>
#include <utility>

using ap = utils::argument_parser;

void ap::set_help_info(
        const ap::HelpTextType& preamble,
        const ap::HelpTextType& epilog) {
    m_help_epilog = epilog;
    m_help_preamble = preamble;
}

void ap::set_program_info(
        const ap::HelpTextType& program_name,
        const ap::HelpTextType& version) {
    m_program_name = program_name;
    m_program_version = version;
}

void ap::set_required_positionals(ap::ArgumentCountType minimum_positionals) {
    if (minimum_positionals > m_number_of_maximum_positionals) {
        throw UsageException(
                "Number of minimum positionals must not be less than the number of maximum positionals");
    }
    m_number_of_minimum_positionals = minimum_positionals;
}

void ap::set_allowed_positionals(ap::ArgumentCountType maximum_positionals) {
    if (m_number_of_minimum_positionals > maximum_positionals) {
        throw UsageException(
                "Number of minimum positionals must not be less than the number of maximum positionals");
    }
    m_number_of_maximum_positionals = maximum_positionals;
}

void ap::set_positional_help(const ap::HelpTextType& help,
                                                 const ap::HelpTextType& meta_var) {
    m_positional_help = help;
    m_positional_meta_var = meta_var;
}

void ap::reset_storage() {
    for (auto& option : m_options) {
        option.reset_storage();
    }
    m_positionals.clear();
}

void ap::parse(int argc, char** argv) {
    ArgumentListType arguments(argv, argv + argc);
    parse(arguments);
}

void ap::parse(int argc, const char** argv) {
    ArgumentListType arguments(argv, argv + argc);
    parse(arguments);
}

void ap::set_program_name_from_argv(const ap::ArgumentType& argv0) {
    if (m_program_name.empty()) {
        m_program_name = argv0;
#ifdef _WIN32
        auto name_start = std::find( m_program_name.rbegin(), m_program_name.rend(), '\\' );
#else
        auto name_start = std::find(m_program_name.rbegin(), m_program_name.rend(), '/');
#endif
        if (name_start != m_program_name.rend() && name_start != m_program_name.rbegin()) {
            m_program_name.erase(m_program_name.begin(), name_start.base());
        }
    }
}

void ap::parse(const ap::ArgumentListType& argv) {
    if (argv.empty()) {
        throw UsageException("argument parser was called with zero arguments.");
    }
    set_program_name_from_argv(argv[0]);
    bool positional_indicator_set{false};

    for (ArgumentListType::size_type current = 1; current < argv.size(); ++current) {
        ArgumentType arg = argv[current];
        if ((!positional_indicator_set) && (arg == "--")) {
            positional_indicator_set = true;
        } else if (positional_indicator_set || (!arg.empty() && (arg[0] != '-'))) {
            if (m_positionals.size() >= m_number_of_maximum_positionals) {
                throw ParsingException("Found an additional positional argument '" + arg +
                                       "', although maximum number of positional arguments is already reached.");
            }
            m_positionals.emplace_back(arg);
        } else if (is_short_option_name(arg) || is_long_option(arg)) {
            if (!m_positionals.empty()) {
                throw ParsingException("Found an option after a positional was given '" + arg + "'");
            }

            if (is_short_option_group_name(arg)) {
                for (ArgumentType::size_type i = 1; i < arg.size(); ++i) {
                    if (!std::isalpha(arg[i])) {
                        throw ParsingException("only alpha chars are allowed for option sequences, not: " + arg);
                    }
                    auto& option = find_option_to_parse(std::string(1, arg[i]));
                    option.parse({});
                }
            } else {
                auto name_value_pair = split_argument_text(arg);
                StorageType values;
                if (!name_value_pair.second.empty()) {
                    if (name_value_pair.second[0] == '=') {
                        name_value_pair.second.erase(0, 1);
                    }
                    values.emplace_back(name_value_pair.second);
                }
                auto& option = find_option_to_parse(name_value_pair.first);
                const auto missing_arguments = option.number_of_arguments() - values.size();
                if ((missing_arguments > 0) && ((current + missing_arguments) < argv.size())) {
                    std::copy_n(argv.begin() + (current + 1), missing_arguments, std::back_inserter(values));
                    current += missing_arguments;
                }
                option.parse(values);
            }
        } else {
            throw ParsingException("Unrecognized argument found: " + argv[current]);
        }
    }

    check_required_arguments();
    check_xor_arguments();
}

void ap::add_flag(const ap::OptionNameSetType& names,
                                      const ap::HelpTextType& help) {
    add_option(names, 0, help, {}, {});
}

void ap::add_option(const ap::OptionNameSetType& names,
                                        const ap::HelpTextType& help) {
    add_option(names, 1, help, {}, {});
}

void ap::add_option(const ap::OptionNameSetType& names,
                                        const ap::HelpTextType& help,
                                        const ap::HelpTextType& meta_var) {
    add_option(names, 1, help, {meta_var}, {});
}

void ap::add_option(const ap::OptionNameSetType& names,
                                        const ap::HelpTextType& help,
                                        const ap::HelpTextType& meta_var,
                                        const ap::StorageValueType& default_value) {
    add_option(names, 1, help, {meta_var}, {default_value});
}

void ap::add_option(const ap::OptionNameSetType& names,
                                        ap::ArgumentCountType number_of_arguments,
                                        const ap::HelpTextType& help,
                                        const std::vector<ap::HelpTextType>& meta_vars,
                                        const ap::StorageType& default_values,
                                        const ap::ChoiceStorageType& choices) {
    if (names.empty()) {
        throw UsageException("No option name was given");
    }

    OptionNameSetType option_names;
    for (auto& name: names) {
        auto option_name = strictly_normalize_option_name(name);
        if (has_option(option_name)) {
            throw UsageException("Option '" + option_name + "' already exists");
        }
        option_names.emplace(option_name);
    }

    auto option = Option(number_of_arguments, choices);
    option.set_names(option_names);
    option.set_help(help);
    option.set_meta_vars(meta_vars);
    option.set_default(default_values);
    m_options.emplace_back(option);
}

bool ap::has_option(const ap::OptionNameType& name) const {
    const auto& iter = find_option_iter(normalize_option_name(name));
    return iter != m_options.end();
}

bool ap::is_parsed(const ap::OptionNameType& name) const {
    const auto& iter = find_option_iter(normalize_option_name(name));
    return (iter != m_options.end()) && iter->is_parsed();
}

bool ap::has_positionals() const {
    return (!m_positionals.empty());
}

const ap::StorageType& ap::get_positionals() const {
    return m_positionals;
}

std::string ap::help() const {
    std::vector<std::pair<std::string, std::string> > formatted_options;

    size_t longest = 0;

    for (const auto& option : m_options) {
        if (!option.is_hidden()) {
            auto option_names_help_pair = option.generate_help_text();
            longest = std::max(longest, option_names_help_pair.first.length());
            formatted_options.emplace_back(option_names_help_pair);
        }
    }
    if (!m_positional_help.empty() && !m_positional_meta_var.empty()) {
        formatted_options.emplace_back(" " + m_positional_meta_var, m_positional_help);
    }

    std::string text = "Usage of " + m_program_name;
    if (!m_program_version.empty()) {
        text += " " + m_program_version;
    }
    text += ":\n";
    text += "  " + m_program_name + " [OPTION...]";
    if (m_number_of_maximum_positionals > 0) {
        if (m_positional_meta_var.empty()) {
            if (m_number_of_maximum_positionals == 1) {
                text += " <POSITIONAL>";
            } else if (m_number_of_maximum_positionals < unlimited_positionals) {
                text += " <" + std::to_string(m_number_of_minimum_positionals) + "-" +
                        std::to_string(m_number_of_maximum_positionals) + " POSITIONALS>";
            } else {
                text += " <POSITIONALS>";
            }
        } else {
            text += " " + m_positional_meta_var;
        }
    }
    text += "\n\n";
    if (!m_help_preamble.empty()) {
        text += m_help_preamble + "\n\n";
    }
    text += "Options:\n";
    for (const auto& o : formatted_options) {
        text += o.first + std::string(longest + 2 - o.first.length(), ' ') + o.second + "\n";
    }
    if (!m_help_epilog.empty()) {
        text += "\n" + m_help_epilog;
    }

    return text;
}

void ap::set_required(const ap::OptionNameSetType& names) {
    for (auto& option: m_options) {
        option.set_required(false);
    }
    for (auto& name : names) {
        auto& existing_option = find_option(name);
        existing_option.set_required(true);
    }
}

void ap::set_hidden(const ap::OptionNameSetType& names) {
    for (auto& option: m_options) {
        option.set_hidden(false);
    }
    for (auto& name : names) {
        auto& existing_option = find_option(name);
        existing_option.set_hidden(true);
    }
}

void ap::set_appending_arguments(const ap::OptionNameSetType& names) {
    for (auto& option: m_options) {
        option.set_appending(false);
    }
    for (auto& name : names) {
        auto& existing_option = find_option(name);
        existing_option.set_appending(true);
    }
}

void ap::add_xor(const ap::OptionNameSetType& names) {
    if (names.size() < 2) {
        throw UsageException("too less arguments for XOR");
    }
    ensure_valid_option_list(names);
    m_xor_lists.emplace_back(names);
}

void ap::ensure_valid_option_list(const ap::OptionNameSetType& names) const {
    for (const auto& name : names) {
        if (!has_option(name)) {
            throw UsageException("Option '" + name + "' does not exist.");
        }
    }
}

std::vector<ap::Option, std::allocator<ap::Option>>::const_iterator
ap::find_option_iter(const ap::OptionNameType& name) const {
    auto iter = std::find_if(m_options.begin(), m_options.end(), [name](const Option& o) {
        return o.has_name(name);
    });
    return iter;
}

ap::Option&
ap::find_option_to_parse(const ap::OptionNameType& name) {
    auto existing_option_iter = find_option_iter(normalize_option_name(name));

    if (existing_option_iter == m_options.end()) {
        throw ParsingException("Option '" + name + "' does not exist.");
    } else {
        return *existing_option_iter;
    }
}

ap::Option&
ap::find_option(const ap::OptionNameType& name) {
    auto existing_option_iter = find_option_iter(normalize_option_name(name));

    if (existing_option_iter == m_options.end()) {
        throw UsageException("Option '" + name + "' does not exist.");
    } else {
        return *existing_option_iter;
    }
}

const ap::Option&
ap::find_option(const ap::OptionNameType& name) const {
    auto existing_option_iter = find_option_iter(normalize_option_name(name));

    if (existing_option_iter == m_options.end()) {
        throw ParsingException("Option '" + name + "' does not exist.");
    } else {
        return *existing_option_iter;
    }
}

bool ap::is_short_option_name(const ap::ArgumentType& arg) const {
    return ((arg.size() > 1) && (arg[0] == '-') && std::isalpha(arg[1]));
}

bool ap::is_short_option_group_name(const ap::ArgumentType& arg) const {
    return ((arg.size() > 2) && (arg[0] == '-') && std::isalpha(arg[1]) && std::isalpha(arg[2]));
}

bool ap::is_long_option(const ap::ArgumentType& arg) const {
    return ((arg.size() > 3) && (arg[0] == '-') && (arg[1] == '-') && std::isalpha(arg[2]) &&
            (std::isalnum(arg[3]) || arg[3] == '-' || arg[3] == '_'));
}

ap::OptionNameType
ap::strictly_normalize_option_name(const ap::OptionNameType& name) const {
    if ((name.size() == 2) && (name[0] == '-')) { // short option
        if (!std::isalpha(name[1])) {
            throw UsageException("Illegal name, letter of  '" + name + "' is a non alpha-character");
        }
        return name.substr(1);
    } else if ((name.size() >= 4) && (name[0] == '-') && (name[1] == '-')) { // long option
        if (!std::isalpha(name[2])) {
            throw UsageException("Illegal name, first letter of '" + name + "' is a non alpha-character");
        }
        for (const auto c : name) {
            if ((!std::isalnum(c)) && c != '-' && c != '_') {
                throw UsageException("'" + name + "' contains other characters than a-zA-Z0-9_-");
            }
        }
        return name.substr(2);
    } else {
        throw UsageException("Given option name has to be '-[a-zA-Z0-9]' or '--[a-zA-Z0-9][a-zA-Z0-9_-]*'");
    }
}

ap::OptionNameType
ap::normalize_option_name(const ap::OptionNameType& name) const {
    if ((name.size() == 1) && name[0] != '-') { // short option
        return strictly_normalize_option_name("-" + name);
    } else if ((name.size() >= 2) && name[0] != '-') { // short option
        return strictly_normalize_option_name("--" + name);
    } else {
        return strictly_normalize_option_name(name);
    }
}

std::pair<ap::OptionNameType, ap::StorageValueType>
ap::split_argument_text(const ap::ArgumentType& text) const {
    if (is_short_option_name(text)) {
        return {std::string(1, text[1]), text.substr(2)};
    } else {
        ArgumentType::size_type cur = 2;
        while (std::isalnum(text[cur]) || text[cur] == '-' || text[cur] == '_') {
            cur++;
        }
        return {text.substr(2, cur - 2), text.substr(cur)};
    }
}

void ap::check_required_arguments() const {
    std::string missing_requireds;
    for (auto& option : m_options) {
        if (option.is_required() && (!option.is_parsed())) {
            missing_requireds += option.get_name() + ", ";
        }
    }
    if (!missing_requireds.empty()) {
        throw ParsingException("The following arguments are required, but were not set: " + missing_requireds);
    }
    if (m_positionals.size() < m_number_of_minimum_positionals) {
        std::string error_msg = "There are " + std::to_string(m_number_of_minimum_positionals) +
                                " positional arguments required, but only " + std::to_string(m_positionals.size()) +
                                " were given.";
        throw ParsingException(error_msg);
    }
}

void ap::check_xor_arguments() const {
    for (auto& xor_list: m_xor_lists) {
        OptionNameType xor_option{};
        for (auto& option: xor_list) {
            if (is_parsed(option)) {
                if (xor_option.empty()) {
                    xor_option = option;
                } else {
                    std::string msg = "Option '" + xor_option + "' and '";
                    msg += option + "' must not be used together.";
                    throw ParsingException(msg);
                }
            }
        }
    }
}

std::vector<ap::Option, std::allocator<ap::Option>>::iterator
ap::find_option_iter(const ap::OptionNameType& name) {
    auto iter = std::find_if(m_options.begin(), m_options.end(), [name](const Option& o) {
        return o.has_name(name);
    });
    return iter;
}

ap::Option::Option(ap::ArgumentCountType number_of_arguments, ap::ChoiceStorageType choices)
        : m_number_of_arguments(number_of_arguments), m_choices{std::move(choices)} {
    if ((!m_choices.empty()) && (m_number_of_arguments != m_choices.size())) {
        throw UsageException("Number of arguments does not match number of choices");
    }
    if (number_of_arguments == 0) { m_default_storage = {"false"}; }  // special handling for flags
}

void ap::Option::parse(const ap::StorageType& values) {
    if (values.size() != m_number_of_arguments) {
        std::string msg = "Option '" + get_name() + "' expects " + std::to_string(m_number_of_arguments);
        msg += " argument, but " + std::to_string(values.size()) + " were given.";
        throw ParsingException(msg);
    }
    if (!m_storage.empty() && !m_is_appending) {
        throw ParsingException("Option '" + get_name() + "' already parsed");
    }
    if (m_number_of_arguments == 0) {  // special handling for flags
        m_storage.emplace_back("true");
    } else {
        if (!m_choices.empty()) {  // special options with values for flags
            for (StorageType::size_type i = 0; i < m_number_of_arguments; ++i) {
                if (std::find(m_choices[i].begin(), m_choices[i].end(), values[i]) != m_choices[i].end()) {
                    m_storage.emplace_back(values[i]);
                } else {
                    std::string msg = "'" + values[i] + "' does not match possible choices for " + get_name();
                    throw ParsingException(msg);
                }
            }
        } else {
            for (auto& value : values) {
                m_storage.emplace_back(value);
            }
        }
    }
}

void ap::Option::set_appending(bool is_appending) {
    m_is_appending = is_appending;
    if ((!is_appending) && (!m_storage.empty()) && (m_storage.size() != m_number_of_arguments)) {
        if (m_storage.size() != m_number_of_arguments) {
            std::string msg = "'" + get_name() + "' expects " + std::to_string(m_number_of_arguments);
            msg += " argument, but got " + std::to_string(m_storage.size());
            throw UsageException(msg);
        }
    }
}

void ap::Option::set_default(const ap::StorageType& values) {
    if ((!values.empty()) && (values.size() != m_number_of_arguments)) {
        throw UsageException("number of default arguments does not match number of arguments");
    }
    if (m_number_of_arguments != 0) { //  ignore for option flags
        m_default_storage.clear();

        if ((!values.empty()) && (!m_choices.empty())) {  // special options with values for flags
            for (StorageType::size_type i = 0; i < m_number_of_arguments; ++i) {
                if (std::find(m_choices[i].begin(), m_choices[i].end(), values[i]) !=
                    m_choices[i].end()) {
                    m_default_storage.emplace_back(values[i]);
                } else {
                    throw UsageException("Value does not match any possible choice");
                }
            }
        } else {
            m_default_storage = values;
        }
    }
}

void ap::Option::set_meta_vars(const ap::MetaVarListType& meta_vars) {
    if ((!meta_vars.empty()) && (meta_vars.size() != m_number_of_arguments)) {
        throw UsageException("number of meta vars does not match number of arguments");
    }
    m_meta_vars = meta_vars;
}

void ap::Option::set_names(const ap::OptionNameSetType& names) {
    if (names.empty()) {
        throw UsageException("No name given for option.");
    }
    m_names = names;
}

bool ap::Option::has_name(const ap::OptionNameType& name) const {
    return m_names.find(name) != m_names.end();
}

std::string ap::Option::get_name() const { return m_names.empty() ? "" : *m_names.begin(); }

std::pair<std::string, std::string> ap::Option::generate_help_text() const {
    std::string name_text;
    for (const auto& name : m_names) {
        if (name.size() == 1) {
            name_text += " -" + name;
        } else {
            name_text += " --" + name;
        }
    }

    if (!m_meta_vars.empty()) {
        for (auto& meta_var : m_meta_vars) {
            name_text += " <" + meta_var + ">";
        }
    } else {
        for (ArgumentCountType i = 0; i < m_number_of_arguments; ++i) {
            name_text += " <ARG>";
        }
    }

    std::string help_text;
    // add required
    if (is_required()) {
        help_text += "required";
    }
    // add required
    if (is_appending()) {
        if (!help_text.empty()) {
            help_text += ", ";
        }
        help_text += "appending";
    }
    // add choices
    if ((m_number_of_arguments > 0) && (!m_choices.empty())) {
        if (!help_text.empty()) {
            help_text += ", ";
        }
        help_text += "choices:";
        for (auto& argument_choices : m_choices) {
            help_text += " [";
            for (auto& choice : argument_choices) {
                help_text += choice + "|";
            }
            help_text.erase(help_text.end() - 1, help_text.end());  // remove last whitespace
            help_text += "]";
        }
    }

    // add default
    if ((m_number_of_arguments > 0) && (!m_default_storage.empty())) {
        if (!help_text.empty()) {
            help_text += ", ";
        }
        help_text += "default: ";
        for (auto& default_value : m_default_storage) {
            help_text += default_value + " ";
        }
        help_text.erase(help_text.end() - 1, help_text.end());  // remove last whitespace
    }

    if (!help_text.empty()) {
        help_text = " (" + help_text + ")";
    }
    help_text = m_help + help_text;

    return {name_text, help_text};
}

void ap::Option::parse_value(const ap::StorageValueType& text, bool& value) {
    StorageValueType lower_text(text);
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);

    if ((lower_text == "on") ||
        (lower_text == "true") ||
        (lower_text == "1") ||
        (lower_text == "yes")) {
        value = true;
    } else if ((lower_text == "off") ||
               (lower_text == "false") ||
               (lower_text == "0") ||
               (lower_text == "no")) {
        value = false;
    } else {
        throw ParsingException("Argument ‘" + text + "’ failed to parse");
    }
}

void
ap::Option::parse_value(const ap::StorageValueType& text, std::string& value) {
    value = text;
}
