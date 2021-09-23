//===-- WaitParam.h ---------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef WAITPARAM_H_
#define WAITPARAM_H_

#include <string>

namespace klee {

class WaitParam {
public:
  std::string mutexName;
  unsigned threadId;

  WaitParam();
  WaitParam(std::string mutexName, unsigned threadId);
  virtual ~WaitParam();
};

} // namespace klee

#endif /* WAITPARAM_H_ */
