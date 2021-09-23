//===-- BarrierManager.h -----------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef BARRIERMANAGER_H_
#define BARRIERMANAGER_H_

#include "klee/Thread/Barrier.h"
#include <map>

namespace klee {

class BarrierManager {
private:
  std::map<std::string, Barrier *> barrierPool;

public:
  BarrierManager();
  virtual ~BarrierManager();
  bool init(std::string barrierName, unsigned count, std::string &errorMsg);
  bool wait(std::string barrierName, unsigned threadId, bool &isReleased, std::vector<unsigned> &blockedList,
            std::string &errorMsg);
  bool addBarrier(std::string barrierName, std::string &errorMsg);
  Barrier *getBarrier(std::string barrierName);
  void clear();
  void print(std::ostream &out);
};

} /* namespace klee */

#endif /* BARRIERMANAGER_H_ */
