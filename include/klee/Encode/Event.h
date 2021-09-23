//===-- Event.h -------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef EVENT_H_
#define EVENT_H_

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"
#include "klee/Module/KInstruction.h"

#include <llvm/IR/Function.h>

#include <string>
#include <vector>

namespace klee {

class Event {
public:
  enum EventType {
    NORMAL, // which participates in the encode and has associated inst
    IGNORE, // which does not participate in the encode
    VIRTUAL // which has no associated inst
  };

public:
  unsigned threadId;
  unsigned eventId;
  unsigned threadEventId;
  std::string eventName;
  KInstruction *inst;
  // name of load or store variable
  std::string name; 
  // globalVarName + the read / write sequence      
  std::string globalName; 
  EventType eventType;
  Event *latestWriteEventInSameThread;
  // is global variable  load, store, call strcpy in these three instruction this attribute will be assigned
  bool isGlobal; 
  bool isEventRelatedToBranch;
  // is this event associated with a Br which has two targets
  bool isConditionInst; 
  // Br's condition          
  bool brCondition; 
  // only use by call, whether the called function is defined by user              
  bool isFunctionWithSourceCode; 
  // set for called function. all callinst use it.@14.12.02 
  llvm::Function *calledFunction; 
  std::vector<unsigned> vectorClock;
  std::vector<ref<klee::Expr>> instParameter;
  std::vector<ref<klee::Expr>> relatedSymbolicExpr;

  Event();
  Event(unsigned threadId, unsigned eventId, std::string eventName, KInstruction *inst, std::string globalVarName,
        std::string globalVarFullName, EventType eventType);
  virtual ~Event();
  std::string toString();
};

} /* namespace klee */

#endif /* EVENT_H_ */
