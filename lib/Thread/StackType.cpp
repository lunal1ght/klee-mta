//===-- StackType.cpp -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/StackType.h"
#include "../Core/Memory.h"
#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"
#include "klee/Module/InstructionInfoTable.h"
#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ilist.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>

namespace klee {

StackType::StackType(AddressSpace *addressSpace) : addressSpace(addressSpace) {}

StackType::StackType(AddressSpace *addressSpace, StackType *stack) : addressSpace(addressSpace) {
  for (std::vector<StackFrame>::iterator ie = stack->realStack.begin(), ee = stack->realStack.end(); ie != ee; ie++) {
    realStack.push_back(*ie);
  }
}

StackType::~StackType() {}

void StackType::pushFrame(KInstIterator caller, KFunction *kf) { realStack.push_back(StackFrame(caller, kf)); }

void StackType::popFrame() {
  StackFrame &sf = realStack.back();
  for (std::vector<const MemoryObject *>::iterator it = sf.allocas.begin(), ie = sf.allocas.end(); it != ie; ++it) {
    addressSpace->unbindObject(*it);
  }
  realStack.pop_back();
}

void StackType::dumpStack(llvm::raw_ostream &out, KInstIterator prevPC) const {
  const KInstruction *target = prevPC;
  for (std::vector<StackFrame>::const_reverse_iterator it = realStack.rbegin(), ie = realStack.rend(); it != ie; ++it) {
    const StackFrame &sf = *it;
    llvm::Function *f = sf.kf->function;
    const InstructionInfo &ii = *target->info;
    std::stringstream AssStream;
    AssStream << std::setw(8) << std::setfill('0') << ii.assemblyLine;
    out << AssStream.str();
    out << " in " << f->getName().str() << " (";
    // Yawn, we could go up and print varargs if we wanted to.
    unsigned index = 0;
    for (llvm::Function::arg_iterator ai = f->arg_begin(), ae = f->arg_end(); ai != ae; ++ai) {
      if (ai != f->arg_begin())
        out << ", ";
      out << ai->getName().str();
      // XXX should go through function
      ref<Expr> value = sf.locals[sf.kf->getArgRegister(index++)].value;
      if (isa<ConstantExpr>(value))
        out << "=" << value;
    }
    out << ")";
    if (ii.file != "")
      out << " at " << ii.file << ":" << ii.line;
    out << "\n";
    target = sf.caller;
  }
}

} /* namespace klee */
