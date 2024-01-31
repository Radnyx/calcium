#include "../include/Program.h"

Program::Program(std::string & text) : text(text) {}

std::string Program::extract(const Token & token) const {
    return text.substr(token.startIndex, token.endIndex - token.startIndex);
}
