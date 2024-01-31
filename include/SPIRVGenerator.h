#ifndef SPIRV_GENERATOR_H
#define SPIRV_GENERATOR_H
#include <vector>
#include "Program.h"
#include "AST.h"

typedef uint32_t spirv_id;

class SPIRVGenerator {
public:
    SPIRVGenerator(const Program & program);
    std::vector<uint32_t> generate(const FunctionDefinitionAST * definition); 
private:
    const Program & program;
    std::vector<uint32_t> headerSection;
    std::vector<uint32_t> constantSection;
    std::vector<uint32_t> codeSection;

    // Number of IDs generated
    uint32_t ids;
    // Common IDs
    spirv_id entryPoint;
    spirv_id outputVariable;
    spirv_id voidType;
    spirv_id entryFunctionType;
    spirv_id floatType;
    spirv_id vec4Type;

    void generate(const std::unique_ptr<BodyAST> & body);
    spirv_id generate(const std::unique_ptr<ExpressionAST> & body);

    spirv_id requestId();
};

#endif // SPIRV_GENERATOR_H