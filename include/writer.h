#ifndef WRITER_H
#define WRITER_H
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include "Errors.h"

class Writer {
public:
    Writer(std::shared_ptr<llvm::Module> & llvmModule);
    Error output(const std::string & filename);
private:
    const std::shared_ptr<llvm::Module> llvmModule;
    llvm::TargetMachine * targetMachine;
};

#endif // WRITER_H
