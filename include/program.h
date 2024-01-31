#ifndef PROGRAM_H
#define PROGRAM_H
#include "Lexer.h"

class Program {
public:
    Program(std::string & text); 
    std::string extract(const Token & token) const;

private:
    const std::string & text;
};

#endif // PROGRAM_H
