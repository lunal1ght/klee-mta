//===-- BarrierInfo.h -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_BARRIERINFO_H_
#define LIB_CORE_BARRIERINFO_H_

namespace klee {

class BarrierInfo {
public:
  unsigned count;
  unsigned current;
  unsigned releasedCount;
  BarrierInfo();
  ~BarrierInfo();
  bool addWaitItem();
  void addReleaseItem();
};

} // namespace klee

#endif /* LIB_CORE_BARRIERINFO_H_ */
