#include "gtest/gtest.h"
#include "config_parser.h"

using ConfigParser = config_parser::IniParser;

TEST(ConfigParser, Has) {
    ConfigParser cfg;
    cfg.set("foo", "bar", "value");

    EXPECT_TRUE(cfg.has("foo"));
    EXPECT_TRUE(cfg.has("foo", "bar"));
    EXPECT_FALSE(cfg.has("bar"));
    EXPECT_FALSE(cfg.has("foo", "foo"));
    EXPECT_FALSE(cfg.has(""));
    EXPECT_FALSE(cfg.has("foo", ""));
    EXPECT_FALSE(cfg.has("bar", "foo"));
    EXPECT_FALSE(cfg.has("", ""));
}

TEST(ConfigParser, Set) {
    ConfigParser cfg;
    cfg.set("foo", "bar", "value");

    EXPECT_TRUE(cfg.has("foo"));
    EXPECT_TRUE(cfg.has("foo", "bar"));
    EXPECT_EQ("value", cfg.get<std::string>("foo", "bar"));
    
    cfg.set("foo", "int", 2);
    EXPECT_EQ(2, cfg.get<int>("foo", "int"));
    cfg.set("foo", "float", 2.f);
    EXPECT_FLOAT_EQ(2.f, cfg.get<float>("foo", "float"));
    cfg.set("foo", "bool", true);
    EXPECT_TRUE(cfg.get<bool>("foo", "bool"));
    cfg.set("foo", "bool", false);
    EXPECT_FALSE(cfg.get<bool>("foo", "bool"));
}

TEST(ConfigParser, Items) {
    ConfigParser cfg;
    EXPECT_EQ(std::vector<std::string>(), cfg.sections());
    std::stringstream ss{"[foo]\nbar=value\nbar2=value\n[bar]\nfoo=value"};
    cfg.parse(ss);
    
    auto sections = cfg.sections();
    std::sort(sections.begin(), sections.end());
    std::vector<std::string> section_test({"bar", "foo"});
    EXPECT_EQ(section_test, sections);
    auto options = cfg.options("foo");
    std::sort(options.begin(), options.end());
    std::vector<std::string> option_test({"bar", "bar2"});
    EXPECT_EQ(option_test, options);
    EXPECT_THROW(cfg.options("abc"), config_parser::ConfigParserException);

    std::unordered_map<std::string, std::string> item_test({{"bar", "value"}, {"bar2", "value"}});
    EXPECT_EQ(item_test, cfg.items("foo"));
}

TEST(ConfigParser, Remove) {
    std::stringstream ss{"[foo]\nbar=value\nbar2=value\n[bar]\nfoo=value"};
    ConfigParser cfg(ss);

    EXPECT_TRUE(cfg.has("foo"));
    EXPECT_TRUE(cfg.has("foo", "bar"));
    cfg.remove("foo", "bar");
    EXPECT_FALSE(cfg.has("foo", "bar"));
    EXPECT_TRUE(cfg.has("foo"));
    cfg.remove("foo");
    EXPECT_FALSE(cfg.has("foo"));
}


TEST(ConfigParser, Get) {
    ConfigParser cfg;
    cfg.set("foo", "bar", "value");

    EXPECT_TRUE(cfg.has("foo"));
    EXPECT_TRUE(cfg.has("foo", "bar"));
    EXPECT_EQ("value", cfg.get<std::string>("foo", "bar"));

    EXPECT_THROW(cfg.get<int>("foo", "notfound"), config_parser::ConfigParserException);
    EXPECT_THROW(cfg.get<int>("notfound", "bar"), config_parser::ConfigParserException);
    cfg.set("foo", "int", 2);
    EXPECT_EQ(2, cfg.get<int>("foo", "int"));
    EXPECT_THROW(cfg.get<int>("foo", "bar"), config_parser::ConfigParserException);
    cfg.set("foo", "float", 2.f);
    EXPECT_FLOAT_EQ(2.f, cfg.get<float>("foo", "int"));
    EXPECT_FLOAT_EQ(2.f, cfg.get<float>("foo", "float"));
    EXPECT_FLOAT_EQ(2.f, cfg.get<float>("foo", "nofloat", 2.0f));
    cfg.set("foo", "bool", true);
    EXPECT_THROW(cfg.get<int>("foo", "bool"), config_parser::ConfigParserException);
    EXPECT_TRUE(cfg.get<bool>("foo", "bool"));
    EXPECT_TRUE(cfg.get<bool>("foo", "nobool", true));
    cfg.set("foo", "bool", false);
    EXPECT_FALSE(cfg.get<bool>("foo", "bool"));
    EXPECT_FALSE(cfg.get<bool>("foo", "nobool", false));
}
