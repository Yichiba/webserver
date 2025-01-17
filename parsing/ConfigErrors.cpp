#include "../main.h"

void handleSyntaxErrorMessage(const std::string &error, const std::string &line, int lineNumber) {
    std::string message = "Syntax error: " + error + " at line " + _itos_(lineNumber) + ": " + line;
    throw std::runtime_error(message);
}

int isEmptyLine(const std::string &line) {
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] != ' ' && line[i] != '\t')
            return 0;
    }
    return 1;
}

void checkBrackets(const std::string & filename) {
    std::fstream file;
    file.open(filename);
    //check if file is empty
    if (file.peek() == std::ifstream::traits_type::eof())
        throw std::runtime_error("Error: " + filename + " is empty");
    if (!file.is_open())
        throw std::runtime_error("Error: " + filename + " not found");
    std::string line;
    int lineNumber = 2;
    int foundParentise = 0;
    while (std::getline(file, line)) {
        if (line == "server {") {
            while (std::getline(file, line)) {
                if ((line.find("{") != SIZE_T_MAX && line.find("location") == SIZE_T_MAX) || (line.find("location") != SIZE_T_MAX && line.find("{") == SIZE_T_MAX)) {
                    file.close();
                    handleSyntaxErrorMessage("Unexpected {", line, lineNumber);
                }
                if (line.find("{") != SIZE_T_MAX && line.find("location") != SIZE_T_MAX) 
                    foundParentise++;
                if (line == "}" && foundParentise == 0)
                    break;
                else if (line == "}" && foundParentise != 0)
                    handleSyntaxErrorMessage("Unexpected }", line, lineNumber);
                if (line.find("}") != SIZE_T_MAX) {
                    while (line[0] == ' ')
                        line.erase(0, 1);
                    if (line.length() != 1 || foundParentise == 0) {
                        file.close();
                        handleSyntaxErrorMessage("", line, lineNumber);
                    }
                    foundParentise--;
                }
                if (isEmptyLine(line)) {
                    file.close();
                    handleSyntaxErrorMessage("Empty line", line, lineNumber);
                }
                if (line[line.length() - 1] != ';' && line.find("{") == SIZE_T_MAX && line.find("}") == SIZE_T_MAX) {
                    file.close();
                    handleSyntaxErrorMessage("Missing ;", line, lineNumber);
                }
                lineNumber++;
            }
        }else if (!isEmptyLine(line)) {
            file.close();
            handleSyntaxErrorMessage("Unexpected line", line, lineNumber - 1);
        }
        foundParentise = 0;
        lineNumber++;
    }
    file.close();
}

void SyntaxError(const std::string & filename) {
    checkBrackets(filename);
}
