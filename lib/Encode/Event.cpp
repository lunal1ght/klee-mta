//===-- Event.cpp -----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Encode/Event.h"
#include "klee/Module/InstructionInfoTable.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/raw_ostream.h>

#include <sstream>

using namespace std;
using namespace llvm;

namespace klee {

Event::Event(unsigned threadId, unsigned eventId, string eventName, KInstruction *inst, string varName,
             string globalName, EventType eventType)
    : threadId(threadId), eventId(eventId), eventName(eventName), inst(inst), name(varName), globalName(globalName),
      eventType(eventType), latestWriteEventInSameThread(NULL), isGlobal(false), isEventRelatedToBranch(false),
      isConditionInst(false), brCondition(false), isFunctionWithSourceCode(true), calledFunction(NULL) {
  threadEventId = 0;
}

Event::~Event() {}

// modified by xdzhang
string Event::toString() {
  stringstream ss;
  ss << eventName << ": #Tid = " << threadId << "\n";
  ss << "#Eid = " << eventId << "\n";
  string instStr;
  raw_string_ostream str(instStr);
  inst->inst->print(str);
  ss << " #Inst:" << instStr << "\n";
  ss << " #name = " << name;
  if (isGlobal)
    ss << " #globalName = " << globalName;
  ss << "\n";
  if (isConditionInst) {
    ss << " #CondChoose = " << brCondition << "\n";
  }
  if (isFunctionWithSourceCode) {
    ss << " #FunctionWithSourceCode = YES"
       << "\n";
  }
  ss << " #Function = " << inst->inst->getParent()->getParent()->getName().str() << "\n";
  ss << " #Path = " << inst->info->file << "@" << inst->info->line << "\n";
  ss << " #EventType = ";
  if (eventType == Event::IGNORE)
    ss << "IGNORE\n";
  else if (eventType == Event::NORMAL)
    ss << "NORMAL\n";
  else
    ss << "VIRTURL\n";

  return ss.str();
}

} /* namespace klee */
