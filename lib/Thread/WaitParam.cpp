//===-- WaitParam.cpp -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/WaitParam.h"

using namespace ::std;

namespace klee {

WaitParam::WaitParam() {}

WaitParam::WaitParam(string mutexName, unsigned threadId) {
  this->mutexName = mutexName;
  this->threadId = threadId;
}

WaitParam::~WaitParam() {}

} // namespace klee
