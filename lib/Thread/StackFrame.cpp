//===-- StackFrame.cpp ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/StackFrame.h"

namespace klee {

StackFrame::StackFrame(KInstIterator _caller, KFunction *_kf)
    : caller(_caller), kf(_kf), callPathNode(0), minDistToUncoveredOnReturn(0), varargs(0) {
  locals = new Cell[kf->numRegisters];
}

StackFrame::StackFrame(const StackFrame &s)
    : caller(s.caller), kf(s.kf), callPathNode(s.callPathNode), allocas(s.allocas),
      minDistToUncoveredOnReturn(s.minDistToUncoveredOnReturn), varargs(s.varargs) {
  locals = new Cell[s.kf->numRegisters];
  for (unsigned i = 0; i < s.kf->numRegisters; i++)
    locals[i] = s.locals[i];
}

StackFrame::~StackFrame() {
  delete[] locals;
}

} /* namespace klee */
