#include <iostream>
#include <sstream>
#include "config_parser.h"

int main() {
    std::stringstream ss{"[protocol]\nversion = 6     \n\n[user]\nname = Bob Smith       \n"
                         "email = bob@smith.com \nactive = true\n\npi = 3.14159"};
    config_parser::IniParser cfg(ss);

    for (const auto& section : cfg.sections()) {
        std::cout << "[" << section << "]" << std::endl;
        for (const auto& option: cfg.options(section)) {
            std::cout << option << "=" << cfg.get<std::string>(section, option) << std::endl;
        }
    }
    std::cout << cfg.write_string() << std::endl;

    std::cout << "6 = " << cfg.get<std::string>("protocol", "version") << std::endl;
    std::cout << "True = " << cfg.has("protocol", "version") << std::endl;
    std::cout << "False = " << cfg.has("protocol", "versioadsn") << std::endl;
    std::cout << "True = " << cfg.has("protocol") << std::endl;
    std::cout << "False = " << cfg.has("protocolasd") << std::endl;
}
