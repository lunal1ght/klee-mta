//===-- Transfer.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TRANSFER_H_
#define TRANSFER_H_

#include "klee/Config/Version.h"
#include "klee/Expr/Expr.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/DataTypes.h"

#include <sstream>
#include <string>

namespace klee {

class Transfer {
private:
  static std::stringstream ss;

public:
  static std::string uint64toString(uint64_t input);
  static llvm::Constant *expr2Constant(klee::Expr *expr, llvm::Type *type);
  Transfer();
  virtual ~Transfer();
};

} /* namespace klee */

#endif /* TRANSFER_H_ */
