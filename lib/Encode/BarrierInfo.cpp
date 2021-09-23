//===-- BarrierInfo.cpp -----------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Encode/BarrierInfo.h"

namespace klee {

BarrierInfo::BarrierInfo() {
  this->count = 0x7fffffff;
  this->current = 0;
  this->releasedCount = 0;
}

bool BarrierInfo::addWaitItem() {
  this->current++;
  if (this->current == this->count) {
    return true;
  } else {
    return false;
  }
}

void BarrierInfo::addReleaseItem() {
  this->current = 0;
  this->releasedCount++;
}

BarrierInfo::~BarrierInfo() {}

} // namespace klee
