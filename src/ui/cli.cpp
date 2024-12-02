#include "cli.hpp"

string createProgressBar(double percentage, int width)
{
    double filled = percentage * width / 100;
    stringstream ss;
    ss << "[";
    for (int i = 0; i < width; ++i)
    {
        double t = i - filled;

        if (t < 0)          ss << "█";
        else if (t < 0.125) ss << "▉";
        else if (t < 0.250) ss << "▊";
        else if (t < 0.375) ss << "▋";
        else if (t < 0.500) ss << "▌";
        else if (t < 0.625) ss << "▍";
        else if (t < 0.750) ss << "▎";
        else if (t < 0.825) ss << "▏";
        else                ss << " ";
    }
    ss << "] " << fixed << setprecision(1) << percentage << "%";
    return ss.str();
}

vector<string> splitLines(const string& str) {
    vector<string> lines;
    stringstream ss(str);
    string line;
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    return lines;
}

string stripAnsiCodes(const string& str)
{
    static const regex ansiRegex("\x1B\\[[0-9;]*m");
    return regex_replace(str, ansiRegex, "");
}

int calculateDisplayWidth(const string& str)
{
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    wstring wideStr = converter.from_bytes(str);
    return wideStr.length();
}

string createBox(const std::string& title, const std::string& content) {
    stringstream ss;

    // Strip ANSI codes from title and content to get actual display lengths
    string cleanTitle = stripAnsiCodes(title);
    vector<string> contentLines = splitLines(content);
    vector<string> cleanContentLines;
    for (const auto& line : contentLines) {
        cleanContentLines.push_back(stripAnsiCodes(line));
    }

    // Calculate the maximum width required for the box, considering wide characters
    int boxWidth = calculateDisplayWidth(cleanTitle);
    for (const string& line : cleanContentLines) {
        boxWidth = max(boxWidth, calculateDisplayWidth(line));
    }

    string line(boxWidth + 2, '-'); // Horizontal border line

    // Calculate title padding
    int titlePaddingLeft = (boxWidth - calculateDisplayWidth(cleanTitle)) / 2;
    int titlePaddingRight = titlePaddingLeft + (boxWidth - calculateDisplayWidth(cleanTitle)) % 2;

    // Build the box
    ss << "┌" << line << "┐" << endl;
    ss << "│ " << string(titlePaddingLeft, ' ') << title << string(titlePaddingRight, ' ') << " │" << endl;
    ss << "├" << line << "┤" << endl;

    // Process each line of content with padding
    for (size_t i = 0; i < contentLines.size(); ++i) {
        int contentPaddingLeft = (boxWidth - calculateDisplayWidth(cleanContentLines[i])) / 2;
        int contentPaddingRight = contentPaddingLeft + (boxWidth - calculateDisplayWidth(cleanContentLines[i])) % 2;
        ss << "│ " << string(contentPaddingLeft, ' ') << contentLines[i] << string(contentPaddingRight, ' ') << " │" << endl;
    }

    ss << "└" << line << "┘";
    return ss.str();
}
