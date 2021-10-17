//===-- DTAM.h --------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_DTAM_
#define LIB_CORE_DTAM_

#include <map>
#include <set>
#include <string>
#include <sys/time.h>

#include "klee/Encode/DTAMPoint.h"
#include "klee/Encode/FilterSymbolicExpr.h"
#include "klee/Encode/RuntimeDataManager.h"
#include "klee/Encode/Trace.h"

namespace klee {

class DTAM {
private:
  RuntimeDataManager *runtimeData;
  Trace *trace;
  std::map<std::string, DTAMPoint *> allWrite;
  std::map<std::string, DTAMPoint *> allRead;
  struct timeval start, finish;
  double cost;
  FilterSymbolicExpr filter;

public:
  DTAM(RuntimeDataManager *data);
  virtual ~DTAM();

  void DTAMParallel();
  void DTAMhybrid();
  void initTaint(std::vector<DTAMPoint *> &remainPoint);
  void propagateTaint(std::vector<DTAMPoint *> &remainPoint);
  void getTaint(std::set<std::string> &taint);
  void dtam();
};

} // namespace klee
#endif /* LIB_CORE_DTAM_ */
