//===-- Prefix.h ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_PREFIX_H_
#define LIB_CORE_PREFIX_H_

#include "klee/Encode/Event.h"
#include "klee/Module/KInstruction.h"
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace klee {

class Prefix {
public:
  typedef std::vector<Event *>::iterator EventIterator;

private:
  std::vector<Event *> eventList;
  std::map<Event *, uint64_t> threadIdMap;
  EventIterator position;
  std::string name;

public:
  Prefix(std::vector<Event *> &eventList, std::map<Event *, uint64_t> &threadIdMap, std::string name);
  virtual ~Prefix();
  std::vector<Event *> *getEventList();
  void increasePosition();
  void reuse();
  bool isFinished();
  EventIterator begin();
  EventIterator end();
  EventIterator current();
  uint64_t getNextThreadId();
  unsigned getCurrentEventThreadId();
  void print(std::ostream &out);
  void print(llvm::raw_ostream &out);
  KInstruction *getCurrentInst();
  std::string getName();
};

} /* namespace klee */

#endif /* LIB_CORE_PREFIX_H_ */
