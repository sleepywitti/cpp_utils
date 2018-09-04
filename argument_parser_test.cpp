#include <vector>
#include <string>
#include <gtest/gtest.h>

#include "argument_parser.h"

using ArgumentParser = utils::argument_parser;
using ParsingException = utils::ParsingException;
using UsageException = utils::UsageException;

TEST(ArgumentParser, SimpleConstructor) {
    ArgumentParser namedParser;
    EXPECT_THROW(namedParser.parse({}), UsageException);
}

TEST(ArgumentParser, ParseEmpty) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.parse({"app"}));
}

TEST(ArgumentParser, ParseIllegalArgument) {
    ArgumentParser parser;
    EXPECT_THROW(parser.parse({"app", "positional"}), ParsingException);
}

TEST(ArgumentParser, SetProgramHelp) {
    ArgumentParser parser;
    parser.set_help_info("pre", "post");
    EXPECT_STREQ("pre", parser.help_preamble().c_str());
    EXPECT_STREQ("post", parser.help_epilog().c_str());
}

TEST(ArgumentParser, SetProgramNameAndVersion) {

    ArgumentParser namedParser;
    namedParser.set_program_info("app", "1.1");
    EXPECT_STREQ("app", namedParser.program_name().c_str());
    EXPECT_STREQ("1.1", namedParser.program_version().c_str());
}

TEST(ArgumentParser, SetProgramFromArgvSimple) {
    ArgumentParser unnamedParser;
    ASSERT_NO_THROW(unnamedParser.parse({"app"}));
    EXPECT_STREQ("app", unnamedParser.program_name().c_str());
}

TEST(ArgumentParser, SetProgramFromArgvOsSpecific) {
    ArgumentParser unnamedParserOs;
    ArgumentParser unnamedParserOsDir;
#ifdef _WIN32
    EXPECT_NO_THROW(unnamedParserOs.parse({"C:\\Program Files\\myprogram.exe"}));
    EXPECT_NO_THROW(unnamedParserOsDir.parse({"C:\\Program Files\\"}));
    EXPECT_STREQ("myprogram.exe", unnamedParserOs.program_name().c_str());
    EXPECT_STREQ("C:\\Program Files\\", unnamedParserOsDir.program_name().c_str());
#else
    EXPECT_NO_THROW(unnamedParserOs.parse({"/usr/bin/myprogram"}));
    EXPECT_NO_THROW(unnamedParserOsDir.parse({"/usr/bin/"}));
    EXPECT_STREQ("myprogram", unnamedParserOs.program_name().c_str());
    EXPECT_STREQ("/usr/bin/", unnamedParserOsDir.program_name().c_str());
#endif
}

TEST(ArgumentParser, NoPositionalsAsDefault) {
    ArgumentParser parser;
    EXPECT_THROW(parser.parse({"app", "p1"}), ParsingException);
}

TEST(ArgumentParser, SetPositionalHelp) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.set_positional_help("abc", "def"));
}

TEST(ArgumentParser, SetPositionalsAllowed) {
    ArgumentParser parser;
    parser.set_allowed_positionals(1);
    EXPECT_NO_THROW(parser.parse({"app", "p"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "p1", "p2"}), ParsingException);
}

TEST(ArgumentParser, SetPositionalsRequiredHigherThanAllowed) {
    ArgumentParser parser;
    EXPECT_THROW(parser.set_required_positionals(1), UsageException);
}

TEST(ArgumentParser, SetPositionalsAllowedLowerThanRequired) {
    ArgumentParser parser;
    parser.set_allowed_positionals(2);
    parser.set_required_positionals(2);
    EXPECT_THROW(parser.set_allowed_positionals(1), UsageException);
}

TEST(ArgumentParser, SetPositionalsRequired) {
    ArgumentParser parser;
    parser.set_allowed_positionals(1);
    EXPECT_NO_THROW(parser.set_required_positionals(1));
    EXPECT_NO_THROW(parser.parse({"a", "p1"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"a", "p1", "p2"}), ParsingException);
}

TEST(ArgumentParser, SetPositionalsAllowedToInfinity) {
    ArgumentParser parser;
    parser.add_flag({"-s"});
    parser.set_allowed_positionals(ArgumentParser::unlimited_positionals);
    EXPECT_NO_THROW(parser.parse({"app", "p1", "p1", "p1", "p1", "p1", "p1", "p1", "p1", "p1", "p1", "p1"}));
}
TEST(ArgumentParser, ParsePositionals) {
    ArgumentParser parser;
    parser.add_flag({"-s"});
    parser.set_allowed_positionals(2);
    EXPECT_NO_THROW(parser.parse({"app", "p1"}));
    EXPECT_TRUE(parser.has_positionals());
    auto positionals = parser.get_positionals();
    ASSERT_EQ(1, positionals.size());
    EXPECT_STREQ("p1", positionals[0].c_str());
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-s", "p1"}));
    EXPECT_TRUE(parser.has_positionals());
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "p1", "p2"}));
    EXPECT_TRUE(parser.has_positionals());
    parser.reset_storage();
}

TEST(ArgumentParser, ParsePositionalIndicator) {
    ArgumentParser parser;
    parser.add_flag({"-s"});
    parser.set_allowed_positionals(ArgumentParser::unlimited_positionals);
    EXPECT_NO_THROW(parser.parse({"app", "p1", "--", "-p"}));
    auto positionals = parser.get_positionals();
    ASSERT_EQ(2, positionals.size());
    EXPECT_STREQ("p1", positionals[0].c_str());
    EXPECT_STREQ("-p", positionals[1].c_str());
}

TEST(ArgumentParser, ParseSecondPositionalIndicator) {
    ArgumentParser parser;
    parser.set_allowed_positionals(3);
    EXPECT_NO_THROW(parser.parse({"app", "p1", "--", "-p", "--"}));
}

TEST(ArgumentParser, SetRequired) {
    ArgumentParser parser;
    parser.add_flag({"-v"});
    EXPECT_NO_THROW(parser.set_required({}));
    EXPECT_NO_THROW(parser.parse({"apps", "-v"}));
}

TEST(ArgumentParser, ParseNonExistingRequiredOption) {
    ArgumentParser parser;
    parser.add_flag({"-v"});
    EXPECT_NO_THROW(parser.parse({"apps"}));
    EXPECT_NO_THROW(parser.set_required({"v"}));
    EXPECT_THROW(parser.parse({"apps"}), ParsingException);
}

TEST(ArgumentParser, SetNonExistingOptionAsRequired) {
    ArgumentParser parser;
    EXPECT_THROW(parser.set_required({"v"}), UsageException);
}

TEST(ArgumentParser, addXOR) {
    ArgumentParser parser;
    parser.add_flag({"-a"});
    parser.add_flag({"-b"});
    EXPECT_NO_THROW(parser.add_xor({"a", "-b"}));
    EXPECT_THROW(parser.add_xor({"a", "b", "c"}), UsageException);
    EXPECT_THROW(parser.add_xor({"a"}), UsageException);
    EXPECT_THROW(parser.add_xor({}), UsageException);
    EXPECT_NO_THROW(parser.parse({"app", "-a"}));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-a", "-b"}), ParsingException);
    parser.reset_storage();
}

TEST(ArgumentParser, optionNaming) {
    ArgumentParser parser;
    EXPECT_THROW(parser.add_flag({""}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"0"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"#"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"a"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"---a"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"-0"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"-#"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"--a#"}, "illegal"), UsageException);
    EXPECT_NO_THROW(parser.add_flag({"-x", "-y", "-z"}, "multiple names"));
    EXPECT_THROW(parser.add_flag({"--g", "--h", "--i"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"--_"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"-a,-b"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"-aa"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"a1"}, "illegal"), UsageException);
    EXPECT_NO_THROW(parser.add_flag({"-c", "-c"}, "illegal")); // possible .. names are saved in a set
    EXPECT_THROW(parser.add_flag({"-d", "--d"}, "illegal"), UsageException);
    EXPECT_THROW(parser.add_flag({"-e,--d"}, "illegal"), UsageException);
    EXPECT_NO_THROW(parser.add_flag({"-a", "--a1"}, "legal"));
    EXPECT_NO_THROW(parser.add_flag({"--a_"}, "legal"));
    EXPECT_NO_THROW(parser.add_flag({"-b", "--b-b"}, "legal"));
}

TEST(ArgumentParser, AddFlag) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.add_flag({"-b"}, "desc"));
    EXPECT_THROW(parser.add_flag({"--c"}, "desc"), UsageException);
}

TEST(ArgumentParser, ParseSimpleFlag) {
    ArgumentParser parser;
    parser.add_flag({"--true"}, "desc");
    parser.add_flag({"--false"}, "desc");

    parser.parse({"app", "--true"});
    EXPECT_TRUE(parser.is_parsed("true"));
    EXPECT_FALSE(parser.is_parsed("false"));
    EXPECT_TRUE(parser.get<bool>("true"));
    EXPECT_FALSE(parser.get<bool>("false"));
}

TEST(ArgumentParser, ParseMultiFlags) {
    ArgumentParser parser;
    parser.add_flag({"--true"}, "desc");
    parser.add_flag({"--false"}, "desc");
    parser.add_flag({"-a"}, "desc");
    parser.add_flag({"-b"}, "desc");
    parser.add_flag({"-c"}, "desc");

    parser.parse({"app", "-abc", "--true"});
    EXPECT_TRUE(parser.get<bool>("a"));
    EXPECT_TRUE(parser.get<bool>("b"));
    EXPECT_TRUE(parser.get<bool>("c"));
    EXPECT_TRUE(parser.get<bool>("true"));
    EXPECT_FALSE(parser.get<bool>("false"));
    parser.reset_storage();

    parser.parse({"app", "-cab"});
    EXPECT_TRUE(parser.get<bool>("a"));
    EXPECT_TRUE(parser.get<bool>("b"));
    EXPECT_TRUE(parser.get<bool>("c"));
    EXPECT_FALSE(parser.get<bool>("true"));
    EXPECT_FALSE(parser.get<bool>("false"));
}

TEST(ArgumentParser, AddOption) {
    ArgumentParser parser;
    EXPECT_THROW(parser.add_option({""}), UsageException);
    EXPECT_NO_THROW(parser.add_option({"--aa"}, "help"));
    EXPECT_NO_THROW(parser.add_option({"--bb"}, "help"));
    EXPECT_NO_THROW(parser.add_option({"--cc"}, "help", "META"));
    EXPECT_NO_THROW(parser.add_option({"--dd"}, "help", "META", "DEFAULT"));

    parser.add_option({"--true"}, "desc", "ARG", "false");
    parser.add_option({"--false"}, "desc", "ARG", "false");
    parser.add_option({"--int1"}, "desc", "ARG", "1");
    parser.add_option({"--int2"}, "desc", "ARG", "");
    parser.add_option({"--float1"}, "desc", "ARG", "3.1");
    parser.add_option({"--float2"}, "desc", "ARG", "");
    parser.add_option({"--double"}, "desc", "ARG", "");
    parser.add_option({"--string"}, "desc", "ARG", "mydef");

    EXPECT_EQ(1, parser.get<int>("int1"));
    EXPECT_FLOAT_EQ(3.1, parser.get<float>("float1"));
    EXPECT_EQ("mydef", parser.get<std::string>("string"));
    EXPECT_NO_THROW(parser.parse({"app", "--int1=8", "--int2=1.9", "--float1", "8", "--float2=2.9", "--double=8.9",
                                  "--string=hallo", "--true=on"}));
    EXPECT_TRUE(parser.is_parsed("int1"));
    EXPECT_TRUE(parser.is_parsed("int2"));
    EXPECT_TRUE(parser.is_parsed("float1"));
    EXPECT_TRUE(parser.is_parsed("float2"));
    EXPECT_TRUE(parser.is_parsed("double"));
    EXPECT_TRUE(parser.is_parsed("string"));
    EXPECT_TRUE(parser.is_parsed("true"));
    EXPECT_FALSE(parser.is_parsed("false"));
    EXPECT_EQ(8, parser.get<int>("int1"));
    EXPECT_THROW(parser.get<int>("int2"), ParsingException);
    EXPECT_FLOAT_EQ(8, parser.get<float>("float1"));
    EXPECT_FLOAT_EQ(2.9, parser.get<float>("float2"));
    EXPECT_DOUBLE_EQ(8.9, parser.get<double>("double"));
    EXPECT_EQ("hallo", parser.get<std::string>("string"));
    EXPECT_TRUE(parser.get<bool>("true"));
    EXPECT_FALSE(parser.get<bool>("false"));
    parser.reset_storage();
}

TEST(ArgumentParser, ParseBooleanTrueArguments) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.add_option({"-b"}));
    EXPECT_NO_THROW(parser.parse({"app", "-b", "on"}));
    EXPECT_TRUE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "1"}));
    EXPECT_TRUE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "trUe"}));
    EXPECT_TRUE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "YEs"}));
    EXPECT_TRUE(parser.get<bool>("b"));
}

TEST(ArgumentParser, ParseBooleanFalseArguments) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.add_option({"-b"}));
    EXPECT_NO_THROW(parser.parse({"app", "-b", "off"}));
    EXPECT_FALSE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "0"}));
    EXPECT_FALSE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "faLSE"}));
    EXPECT_FALSE(parser.get<bool>("b"));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "No"}));
    EXPECT_FALSE(parser.get<bool>("b"));
}

TEST(ArgumentParser, ParseBooleanInvalidArguments) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.add_option({"-b"}));
    EXPECT_NO_THROW(parser.parse({"app", "-b", "falsch"}));
    EXPECT_THROW(parser.get<bool>("b"), ParsingException);
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "2"}));
    EXPECT_THROW(parser.get<bool>("b"), ParsingException);
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", "onn"}));
    EXPECT_THROW(parser.get<bool>("b"), ParsingException);
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-b", ""}));
    EXPECT_THROW(parser.get<bool>("b"), ParsingException);
}


TEST(ArgumentParser, AddChoiceOption) {
    ArgumentParser parser;

    EXPECT_NO_THROW(parser.add_option({"-c", "--choice"}, 1, "", {}, {}, {{"a", "b"}}));
    EXPECT_THROW(parser.add_option({"-d", "--def"}, 1, "", {}, {}, {{"a", "b"},
                                                                    {"a", "b"}}), UsageException);
    EXPECT_NO_THROW(parser.add_option({"--twos"}, 2, "", {}, {}, {{"a", "b"},
                                                                  {"a", "b"}}));

    EXPECT_NO_THROW(parser.parse({"app", "--choice", "a"}));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "--choice", "b"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "--choice", "c"}), ParsingException);
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-c", "b"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-c", "c"}), ParsingException);
    parser.reset_storage();
}

TEST(ArgumentParser, AddMultiOption) {
    ArgumentParser parser;
    EXPECT_THROW(parser.add_option({""}, 2), UsageException);
    EXPECT_NO_THROW(parser.add_option({"--aa"}, 2));
    EXPECT_NO_THROW(parser.add_option({"--bb"}, 2, "help"));
    EXPECT_NO_THROW(parser.add_option({"--cc"}, 2, "help", {"X", "Y"}));
    EXPECT_THROW(parser.add_option({"--ee"}, 2, "help", {"X"}), UsageException);
    EXPECT_THROW(parser.add_option({"--ff"}, 2, "help", {"X", "Y", "Z"}), UsageException);
    EXPECT_NO_THROW(parser.add_option({"--dd"}, 2, "help", {"X", "Y"}, {"2", "3"}));
    EXPECT_NO_THROW(parser.add_option({"--ii"}, 2, "help", {}, {"2", "3"}));
    EXPECT_THROW(parser.add_option({"--gg"}, 2, "help", {"X", "Y"}, {"X"}), UsageException);
    EXPECT_THROW(parser.add_option({"--hh"}, 2, "help", {"X", "Y"}, {"X", "Y", "Z"}), UsageException);
    EXPECT_THROW(parser.add_option({"--jj"}, 2, "help", {}, {"a", "c"}, {{"a", "b"},
                                                                         {"a", "b"}}), UsageException);

    EXPECT_THROW(parser.add_option({""}, 2), UsageException);
    parser.add_option({"--multi"}, 3);
    parser.add_option({"--int"}, 2, "help", {}, {"5", "6"});
    parser.add_option({"--float"}, 2);

    // test without parsing
    EXPECT_FALSE(parser.is_parsed("multi"));
    EXPECT_THROW(parser.get<std::string>("int"), UsageException);
    EXPECT_THROW(parser.get_n<std::string>("multi"), ParsingException);
    auto str_def = parser.get_n<std::string>("int");
    ASSERT_EQ(2, str_def.size());
    EXPECT_EQ("5", str_def[0]);
    EXPECT_EQ("6", str_def[1]);
    auto int_def = parser.get_n<int>("int");
    ASSERT_EQ(2, str_def.size());
    EXPECT_EQ(5, int_def[0]);
    EXPECT_EQ(6, int_def[1]);

    // test after parsing
    EXPECT_NO_THROW(parser.parse({"app", "--multi", "a", "b", "c", "--int", "1", "2", "--float", "1.1", "2.2"}));
    EXPECT_TRUE(parser.is_parsed("multi"));
    EXPECT_TRUE(parser.is_parsed("int"));
    EXPECT_TRUE(parser.is_parsed("float"));
    EXPECT_THROW(parser.get<std::string>("multi"), UsageException);
    auto multi = parser.get_n<std::string>("multi");
    ASSERT_EQ(3, multi.size());
    EXPECT_STREQ("a", multi[0].c_str());
    EXPECT_STREQ("b", multi[1].c_str());
    EXPECT_STREQ("c", multi[2].c_str());
    auto int_opt = parser.get_n<int>("int");
    EXPECT_EQ(1, int_opt[0]);
    EXPECT_EQ(2, int_opt[1]);
    auto float_opt = parser.get_n<float>("float");
    EXPECT_FLOAT_EQ(1.1, float_opt[0]);
    EXPECT_FLOAT_EQ(2.2, float_opt[1]);
}

TEST(ArgumentParser, AddAppendingOption) {
    ArgumentParser parser;
    parser.add_option({"--int"}, "help");
    parser.add_option({"--float"}, "help");
    parser.add_option({"--string"}, "help");
    ASSERT_NO_THROW(parser.set_appending_arguments({"int", "--float", "string"}));

    EXPECT_FALSE(parser.is_parsed("string"));
    EXPECT_THROW(parser.get<std::string>("string"), ParsingException);
    EXPECT_THROW(parser.get_n<std::string>("string"), ParsingException);

    ASSERT_NO_THROW(parser.parse({"app", "--int", "1", "--int=2", "--float", "8", "--float=2.9",
                                  "--string=abc", "--string=def"}));
    EXPECT_TRUE(parser.is_parsed("int"));
    EXPECT_TRUE(parser.is_parsed("float"));
    EXPECT_TRUE(parser.is_parsed("string"));
    auto int_opt = parser.get_n<int>("int");
    EXPECT_EQ(2, int_opt.size());
    EXPECT_EQ(1, int_opt[0]);
    EXPECT_EQ(2, int_opt[1]);
    auto float_opt = parser.get_n<float>("float");
    EXPECT_FLOAT_EQ(8, float_opt[0]);
    EXPECT_FLOAT_EQ(2.9, float_opt[1]);
    auto string_opt = parser.get_n<std::string>("string");
    EXPECT_STREQ("abc", string_opt[0].c_str());
    EXPECT_STREQ("def", string_opt[1].c_str());
}

TEST(ArgumentParser, SetAppendingAfterParsing) {
    ArgumentParser parser;
    parser.add_option({"-a"});
    ASSERT_NO_THROW(parser.set_appending_arguments({"-a"}));
    ASSERT_NO_THROW(parser.parse({"app", "-a=1", "-a=2"}));
    ASSERT_THROW(parser.set_appending_arguments({}), UsageException);
}

TEST(ArgumentParser, IllegalNameParsing) {
    ArgumentParser parser;
    EXPECT_NO_THROW(parser.add_flag({"-a", "--a1"}, "legal"));
    EXPECT_NO_THROW(parser.add_flag({"--a_"}, "legal"));

    EXPECT_NO_THROW(parser.parse({"app"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "positional", "0"}), ParsingException);
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-0"}), ParsingException);
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-_a"}), ParsingException);
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-_"}), ParsingException);
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-"}), ParsingException);
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "---"}), ParsingException);
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "-a"}));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "--a1"}));
    parser.reset_storage();
    EXPECT_NO_THROW(parser.parse({"app", "--a_"}));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-u"}), ParsingException);
}

TEST(ArgumentParser, Parse) {
    ArgumentParser parser;
    parser.add_flag({"-s"}, "short");
    parser.add_flag({"-b", "--both"}, "both");
    parser.add_flag({"--long"}, "long");
    parser.add_option({"-d", "--def"}, "def", "DEF", "-1");
    parser.add_option({"-n", "--nodef"}, "nodef", "NODEF", "ad");
    parser.add_option({"--no_default"}, "def", "DEF");

    ASSERT_NO_THROW(parser.parse({"app", "-s", "--both", "--long"}));
    EXPECT_TRUE(parser.is_parsed("both"));
    EXPECT_TRUE(parser.is_parsed("b"));
    EXPECT_TRUE(parser.is_parsed("long"));
    EXPECT_TRUE(parser.is_parsed("s"));
    EXPECT_TRUE(parser.is_parsed("--long"));
    EXPECT_TRUE(parser.is_parsed("-s"));
    EXPECT_FALSE(parser.is_parsed("no_default"));
    EXPECT_THROW(parser.get<std::string>("no_default"), ParsingException);
    parser.reset_storage();
    ASSERT_NO_THROW(parser.parse({"app", "--def=11"}));
    EXPECT_TRUE(parser.is_parsed("def"));
    EXPECT_EQ(11, parser.get<int>("def"));
    EXPECT_EQ(11, parser.get<int>("d"));
    EXPECT_EQ(11, parser.get<int>("--def"));
    EXPECT_EQ(11, parser.get<int>("-d"));
    parser.reset_storage();
    ASSERT_NO_THROW(parser.parse({"app", "--nodef="}));
    EXPECT_EQ("", parser.get<std::string>("nodef"));
    parser.reset_storage();
    ASSERT_NO_THROW(parser.parse({"app", "-d/ad"}));
    EXPECT_TRUE(parser.is_parsed("d"));
    EXPECT_EQ("/ad", parser.get<std::string>("d"));
    parser.reset_storage();
    EXPECT_THROW(parser.parse({"app", "-d", "1", "-d", "2"}), ParsingException);
    EXPECT_THROW(parser.parse({"app", "--def", "1", "-d", "2"}), ParsingException);
}


TEST(ArgumentParser, ParseMainLikeInput) {
    ArgumentParser parser;
    parser.add_flag({"-f"}, "false");
    parser.add_flag({"-s"}, "short");
    parser.add_flag({"-b", "--both"}, "both");
    parser.add_option({"--long"}, "long");
    parser.set_allowed_positionals(2);

    std::vector<const char*> arg_source{"app", "-s", "--long", "value", "--both", "positional"};
    auto argv = arg_source.data();
    auto argc = static_cast<int>(arg_source.size());

    ASSERT_NO_THROW(parser.parse(argc, argv));
    EXPECT_TRUE(parser.is_parsed("both"));
    EXPECT_TRUE(parser.is_parsed("b"));
    EXPECT_TRUE(parser.is_parsed("long"));
    EXPECT_TRUE(parser.is_parsed("s"));
    EXPECT_TRUE(parser.is_parsed("--long"));
    EXPECT_TRUE(parser.is_parsed("-s"));
    EXPECT_FALSE(parser.is_parsed("-f"));
    EXPECT_STREQ("value", parser.get<std::string>("long").c_str());
    EXPECT_TRUE(parser.has_positionals());
    auto pos = parser.get_positionals();
    ASSERT_EQ(1, pos.size());
    EXPECT_STREQ("positional", pos[0].c_str());
}

TEST(ArgumentParser, GenerateHelp) {
    ArgumentParser parser;
    parser.set_program_info("app", "1.0");
    parser.set_help_info("pretext", "some epilog");
    parser.add_flag({"-s"}, "short option as flag");
    parser.add_flag({"-f", "--flag", "--other-name"}, "arbitrary number of names");
    parser.add_option({"--long"}, "long option with argument");
    parser.add_option({"--hidden"}, "hidden option");
    parser.set_hidden({"hidden"});
    parser.add_option({"--default"}, "option with default", "META", "DEFAULT");
    parser.add_option({"--appending"}, "option that appends values, like -I file1 -I file2");
    parser.add_option({"--required"}, "required option");
    parser.add_option({"-c", "--choice"}, 1, "choice desc", {"ARG"}, {}, {{"a", "b"}});
    parser.add_option({"--all"}, 2, "multiple arguments", {"ARG1", "ARG2"}, {"a", "d"}, {{"a", "b"},
                                                                                         {"c", "d"}});
    parser.set_appending_arguments({"appending", "all"});
    parser.set_required({"required", "all"});
    parser.set_allowed_positionals(2);

    std::string expected_help =
    "Usage of app 1.0:\n"
    "  app [OPTION...] <0-2 POSITIONALS>\n"
    "\n"
    "pretext\n"
    "\n"
    "Options:\n"
    " -s                      short option as flag\n"
    " -f --other-name --flag  arbitrary number of names\n"
    " --long <ARG>            long option with argument\n"
    " --default <META>        option with default (default: DEFAULT)\n"
    " --appending <ARG>       option that appends values, like -I file1 -I file2 (appending)\n"
    " --required <ARG>        required option (required)\n"
    " -c --choice <ARG>       choice desc (choices: [b|a])\n"
    " --all <ARG1> <ARG2>     multiple arguments (required, appending, choices: [b|a] [d|c], default: a d)\n"
    "\n"
    "some epilog";

    auto help = parser.help();
    EXPECT_STREQ(expected_help.c_str(), help.c_str());
}
