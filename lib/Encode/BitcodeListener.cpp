//===-- BitcodeListener.cpp --------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Encode/BitcodeListener.h"
#include "klee/Thread/StackType.h"

namespace klee {

BitcodeListener::BitcodeListener(RuntimeDataManager *rdManager) : kind(defaultKind), rdManager(rdManager) {
  stack[1] = new StackType(&addressSpace);
  stack[1]->realStack.reserve(10);
}

BitcodeListener::~BitcodeListener() {}

} // namespace klee
