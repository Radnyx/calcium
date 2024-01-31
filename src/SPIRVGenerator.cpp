#include "../include/SPIRVGenerator.h"
#include <cassert>
#include <string>
#include <fstream>
#include <algorithm>

constexpr char ENTRY_POINT_NAME[] = "main";
constexpr char EXTENSION_NAME[] = "GLSL.std.450";

const size_t ID_BOUND_INDEX = 3;

enum Capability {
    CAP_SHADER = 1
};

enum ExecutionModel {
    MODEL_FRAGMENT = 4
};

enum ExecutionMode {
    MODE_ORIGIN_UPPER_LEFT = 7
};

enum Decoration {
    DEC_LOCATION = 30
};

enum StorageClass {
    STORE_OUTPUT = 3
};

enum FunctionControl {
    FUNC_NONE = 0  
};


SPIRVGenerator::SPIRVGenerator(const Program & program) : program(program) {
    ids = 0;
}

std::vector<uint32_t> SPIRVGenerator::generate(const FunctionDefinitionAST * definition) {
    auto name = program.extract(definition->prototype->name);

    headerSection = {
        0x07230203, // MAGIC NUMBER
        0x00010000, // VERSION NUMBER
        0x00000000, // VENDOR ID (can be reserved: https://github.com/KhronosGroup/SPIRV-Headers)
        0x00000000, // ID BOUND
        0x00000000
    };

    // OpCapability
    emit<OP_CAPABILITY, CAP_SHADER>();
    // OpExtension
    // OpExtInstImport
    emit<OP_EXT_INST_IMPORT>({ requestId(), EXTENSION_NAME });
    // OpMemoryModel
    emit<OP_MEMORY_MODEL, 0, 1>();
    // OpEntryPoint
    emit<OP_ENTRY_POINT>({
        MODEL_FRAGMENT,
        entryPoint = requestId(),
        ENTRY_POINT_NAME,
        outputVariable = requestId()
    });
    // OpExecutionMode
    emit<OP_EXECUTION_MODE>({ entryPoint, MODE_ORIGIN_UPPER_LEFT });
    // OpString
    // OpSourceExtension
    // OpSource
    // OpSourceContinued
    // OpName
    // OpMemberName
    // Annotation instructions
    emit<OP_DECORATE>({
        outputVariable,
        DEC_LOCATION,
        0 // location = 0
    });
    // Type declarations
    emit<OP_TYPE_VOID>({ voidType = requestId() });
    emit<OP_TYPE_FUNCTION>({ entryFunctionType = requestId(), voidType });
    emit<OP_TYPE_FLOAT>({ floatType = requestId(), 32 /* bits */ });
    emit<OP_TYPE_VECTOR>({
        vec4Type = requestId(),
        floatType,
        4 // components
    });

    spirv_id outputPointerType;
    emit<OP_TYPE_POINTER>({
        outputPointerType = requestId(),
        STORE_OUTPUT,
        vec4Type // pointer to vec4
    });

    // Input/Output variables
    // TODO: generate from function prototype
    emit<OP_VARIABLE>({ outputPointerType, outputVariable, STORE_OUTPUT });

    // Global variables

    // Function declarations
    
    // Function definitions
    emit<OP_FUNCTION>({
        voidType,
        entryPoint,
        FUNC_NONE,
        entryFunctionType
    });

    emit<OP_LABEL>({ requestId() });

    generate(definition->body);

    emit<OP_RETURN>();
    emit<OP_FUNCTION_END>();

    headerSection[ID_BOUND_INDEX] = ids;

    std::vector<uint32_t> result(headerSection.begin(), headerSection.end());
    result.insert(result.end(), constantSection.begin(), constantSection.end());
    result.insert(result.end(), codeSection.begin(), codeSection.end());

    std::ofstream fs(name + ".spv", std::ios::out | std::ios::binary);
    fs.write(reinterpret_cast<const char *>(result.data()), result.size() * 4);
    fs.close();

    // TODO: optimizer pass, see: https://github.com/KhronosGroup/SPIRV-Tools/blob/main/examples/cpp-interface/main.cpp

    return result;
}

void SPIRVGenerator::generate(const std::unique_ptr<BodyAST> & body) {
    for (auto & statement : body->statements) {
        if (statement->isReturn()) {
            auto returnStatement = reinterpret_cast<const ReturnAST *>(statement.get());
            emit<OP_STORE>({
                outputVariable, 
                generate(returnStatement->expression)
            });
        }
    }
}

spirv_id SPIRVGenerator::generate(const std::unique_ptr<ExpressionAST> & expression) {
    switch (expression->getExpressionID()) {
        case EXPRESSION_FUNCTION_CALL:
        {
            auto functionCall = reinterpret_cast<const FunctionCallAST *>(expression.get());
            auto name = program.extract(functionCall->name);
            assert(name == "vec4");
            // TODO: handle other function calls and constructors
            // TODO: handle constant vec4 (all arguments constant)

            spirv_id id = requestId();

            std::vector<std::variant<uint32_t, const char *>> words { vec4Type, id };
            for (const auto & arg : functionCall->arguments) {
                words.push_back(generate(arg));
            }

            emit<OP_COMPOSITE_CONSTRUCT>(std::move(words));

            return id;
        }
        case EXPRESSION_FLOAT_LITERAL:
        {
            // TODO: cache re-used constants
            static_assert(sizeof(float) == sizeof(uint32_t)); // TODO: support other sizes

            auto floatLiteral = reinterpret_cast<const FloatLiteralAST *>(expression.get());
            float value = std::stof(program.extract(floatLiteral->text));
            const uint32_t * data = reinterpret_cast<const uint32_t *>(&value);

            spirv_id id = requestId();
            emit<OP_CONSTANT>({ floatType, id, *data });

            return id;
        }
        break;
        default:
            assert(false);
    }
    return -1;
}

spirv_id SPIRVGenerator::requestId() {
    return ++ids;
}