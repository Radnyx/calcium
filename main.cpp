#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "lexer.h"

int main(int, char**) {
    std::string filename = "../examples/test.sdh";
    std::ifstream stream(filename);
    if (!stream) {
        std::cerr << "ERR: could not find file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();

    const std::string program = buffer.str();
    Lexer lexer(program);
    
    std::vector<Token> tokens;
    auto err = lexer.tokenize(tokens);
    if (err != NO_ERR) return 1;

    for (auto token : tokens) {
        std::string tok = program.substr(token.startIndex, token.endIndex - token.startIndex);
        std::cout << ':' << tok << ' ' << std::endl;
    }

    return 0;
}