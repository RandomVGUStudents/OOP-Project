#ifndef CLI_HPP
#define CLI_HPP

#include <codecvt>
#include <iomanip>
#include <iostream>
#include <locale>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

constexpr string CLEAR_SCREEN = "\e[1;1H\e[2J";
constexpr string BOLD = "\e[1m";
constexpr string RESET = "\e[0m";
constexpr string CLI_GREEN = "\e[32m";
constexpr string CLI_BLUE = "\e[34m";
constexpr string CLI_YELLOW = "\e[33m";
constexpr string CLI_RED = "\e[31m";

string createProgressBar(double percentage, int width = 30);
string createBox(const std::string& title, const std::string& content);

#endif /* CLI_HPP */
