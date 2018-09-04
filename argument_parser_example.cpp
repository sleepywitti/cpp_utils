#include <iostream>
#include <sstream>
#include "argument_parser.h"

using ArgumentParser = utils::argument_parser;
using ParsingException = utils::ParsingException;
using UsageException = utils::UsageException;

int main(int argc, char** argv) {
    ArgumentParser parser;
    parser.set_program_info("app", "1.0");
    parser.set_positional_help("some additional arguments");
    parser.set_help_info("pretext", "some epilog");
    parser.add_option({"-h", "--help"}, "show help");
    parser.add_flag({"-s"}, "short option as flag");
    parser.add_flag({"-f", "--flag", "--other-name"}, "arbitrary number of names");
    parser.add_option({"--long"}, "long option with argument");
    parser.add_option({"--hidden"}, "hidden option");
    parser.set_hidden({"hidden"});
    parser.add_option({"--default"}, "option with default", "META", "DEFAULT");
    parser.add_option({"--appending"}, "option that appends values, like -I file1 -I file2");
    parser.add_option({"--required"}, "required option");
    parser.add_option({"-c", "--choice"}, 1, "choice desc", {"ARG"}, {}, {{"a", "b"}});
    parser.add_option({"--all"}, 2, "multiple arguments", {"ARG1", "ARG2"}, {"a", "d"}, {{"a", "b"}, {"c", "d"}});
    parser.set_appending_arguments({"appending", "all"});
    parser.set_required({"required", "all"});
    parser.set_allowed_positionals(2);

    std::vector<std::string> arguments{"help", "s", "f", "long", "hidden", "default", "appending", "required",
                                       "choice", "all"};

    try {
        parser.parse(argc, argv);
        if (parser.is_parsed("help")) {
            std::cout << parser.help();
            return 0;
        }
        for (const auto& arg : arguments) {
            std::cout << arg << ": ";
            if (parser.is_parsed(arg)) {
                auto values = parser.get_n<std::string>(arg);
                for (const auto& value : values) {
                    std::cout << "'" << value << "',";
                }
            }
            if (!parser.is_parsed(arg)) {
                std::cout << "(not set)";
            }
            std::cout << std::endl;
        }
    } catch (const ParsingException& e) {
        std::cerr << "Error:" << e.what() << std::endl;
        std::cout << parser.help();
        return -1;
    }
    return 0;
}
