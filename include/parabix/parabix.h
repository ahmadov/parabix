#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <cinttypes>
#include <string>

namespace parabix {

  uint64_t parabix_cpp(std::string& input, const char* pattern);

  uint64_t parabix_llvm(llvm::orc::ThreadSafeContext& context, std::string& input, const char* pattern);

} // namespace parabix
