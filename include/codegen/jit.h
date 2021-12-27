#ifndef INCLUDE_CODEGEN_JIT_H
#define INCLUDE_CODEGEN_JIT_H

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace codegen {

    class JIT {
        private:
        /// The target machine.
        std::unique_ptr<llvm::TargetMachine> target_machine;
        /// The data layout.
        const llvm::DataLayout data_layout;
        /// The execution session
        llvm::orc::ExecutionSession execution_session;
        /// The context
        llvm::orc::ThreadSafeContext& context;

        /// Optimization function using OptimizeFunction = std::function<std::unique_ptr<llvm::Module>(std::unique_ptr<llvm::Module>)>;

        /// The object layer.
        llvm::orc::RTDyldObjectLinkingLayer object_layer;
        /// The compile layer.
        llvm::orc::IRCompileLayer compile_layer;
        /// The optimize layer.
        llvm::orc::IRTransformLayer optimize_layer;
         /// The main JITDylib
        llvm::orc::JITDylib& mainDylib;

        public:
        /// The constructor.
        explicit JIT(llvm::orc::ThreadSafeContext& ctx);

        ~JIT() {
          // copied from https://github.com/llvm/llvm-project/blob/705c722ba5eeaea732d069c39a1b05de9dc2ca6d/llvm/examples/Kaleidoscope/BuildingAJIT/Chapter4/KaleidoscopeJIT.h#L165
          if (auto error = execution_session.endSession()) {
            execution_session.reportError(std::move(error));
          }
        } 

        /// Get the target machine.
        auto& getTargetMachine() { return *target_machine; }
        /// Add a module.
        llvm::Error addModule(std::unique_ptr<llvm::Module> module);

        /// Get pointer to function.
        void* getPointerToFunction(const std::string& name);
    };

}  // namespace codegen

#endif
