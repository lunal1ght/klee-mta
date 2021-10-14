//===-- Trace.h ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LIB_CORE_TRACE_H_
#define LIB_CORE_TRACE_H_

#include "klee/ADT/Ref.h"
#include "klee/Encode/Event.h"
#include "klee/Expr/Expr.h"
#include "klee/Module/KInstruction.h"

#include <llvm/IR/Constant.h>
#include <llvm/Support/raw_ostream.h>

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;
namespace klee {

// added by xdzhang
struct Wait_Lock {
  Event *wait;
  Event *lock_by_wait; // lock op included in wait function
};

struct LockPair {
  unsigned threadId;
  std::string mutex;
  Event *lockEvent;
  Event *unlockEvent;
};

class Trace {

public:
  enum TraceType
  {
    UNIQUE,    // the trace is a new trace
    REDUNDANT, // the trace is a redundant trace
    FAILED     // the trace is failed during execution
  };
  unsigned Id;
  unsigned nextEventId;
  // all event, sorted by threadId and event Id
  std::vector<std::vector<Event *>> eventList;
  std::vector<std::string> abstract;
  std::stringstream ss;
  // original execution trace
  std::vector<Event *> path;
  // whether this trace is a untested trace
  bool isUntested;
  // the type of trace
  TraceType traceType;

  std::vector<ref<klee::Expr>> storeSymbolicExpr;
  std::vector<ref<klee::Expr>> taintExpr;
  std::vector<ref<klee::Expr>> rwSymbolicExpr;
  std::vector<ref<klee::Expr>> brSymbolicExpr;
  std::vector<ref<klee::Expr>> assertSymbolicExpr;
  std::vector<ref<klee::Expr>> pathCondition;
  std::vector<ref<klee::Expr>> pathConditionRelatedToBranch;
  std::vector<std::set<std::string>> brRelatedSymbolicExpr;
  std::vector<std::set<std::string>> assertRelatedSymbolicExpr;
  std::set<std::string> RelatedSymbolicExpr;
  std::map<std::string, std::set<std::string>> allRelatedSymbolicExprs;
  std::map<std::string, long> varThread;
  std::vector<Event *> rwEvent;
  std::vector<Event *> brEvent;
  std::vector<Event *> assertEvent;

  std::set<std::string> Send_Data_Expr;
  std::set<std::string> initTaintSymbolicExpr;
  std::set<std::string> taintSymbolicExpr;
  std::set<std::string> unTaintSymbolicExpr;
  std::set<std::string> potentialTaint;

  std::set<std::string> DTAMSerial;
  std::set<std::string> DTAMParallel;
  std::set<std::string> DTAMhybrid;

  std::vector<std::string> PTS;
  std::vector<std::string> taintPTS;
  std::vector<std::string> noTaintPTS;

  std::set<std::string> taintMap;
  std::set<std::string> DTAMSerialMap;
  std::set<std::string> DTAMParallelMap;
  std::set<std::string> DTAMhybridMap;

  Trace();

  virtual ~Trace();
  void insertEvent(Event *event, unsigned threadId);
  void insertThreadCreateOrJoin(std::pair<Event *, uint64_t> item, bool isThreadCreate);
  void insertWait(std::string condName, Event *wait, Event *associatedLock);
  void insertSignal(std::string condName, Event *event);
  void insertLockOrUnlock(unsigned threadId, std::string mutex, Event *event, bool isLock);
  void insertGlobalVariableInitializer(std::string name, llvm::Constant *initializer);
  void insertPrintfParam(std::string name, llvm::Constant *param);
  void insertGlobalVariableLast(std::string name, llvm::Constant *finalValue);
  void insertBarrierOperation(std::string barrierName, Event *event);
  void insertPath(Event *event);
  void insertArgc(int argc);
  void insertReadSet(std::string name, Event *item);
  void insertWriteSet(std::string name, Event *item);
  // This function is deprecated, should remove it later.
  Event *createEvent(unsigned threadId, KInstruction *inst, uint64_t address, bool isLoad, int time,
                     Event::EventType eventType);
  Event *createEvent(unsigned threadId, KInstruction *inst, Event::EventType eventType);

  void printAllEvent(llvm::raw_ostream &out);
  void printThreadCreateAndJoin(llvm::raw_ostream &out);
  void printWaitAndSignal(llvm::raw_ostream &out);
  void printReadSetAndWriteSet(llvm::raw_ostream &out);
  void printLockAndUnlock(llvm::raw_ostream &out);
  void printBarrierOperation(llvm::raw_ostream &out);
  void printPrintfParam(llvm::raw_ostream &out);
  void printGlobalVariableLast(llvm::raw_ostream &out);
  void printGlobalVariableInitializer(llvm::raw_ostream &out);
  void printExecutionPath(raw_ostream &out);
  void printDetailedInfo(raw_ostream &out);

  void createAbstract();
  bool isEqual(Trace *trace);

  std::string getAssemblyLine(std::string name);
  std::string getLine(std::string name);
  Event *getEvent(std::string name);

public:
  // 线程创建与终止操作数据-->生成偏序约束
  // key--event, value--created thread id
  std::map<Event *, uint64_t> createThreadPoint;
  // key--event, value--joined thread id
  std::map<Event *, uint64_t> joinThreadPoint;

  // 全局变量读写操作数据-->生成读写关系约束
  std::map<std::string, std::vector<Event *>> allReadSet;
  std::map<std::string, std::vector<Event *>> allWriteSet;
  // key--global variable, value--the whole events that read global vars.
  std::map<std::string, std::vector<Event *>> readSet;
  std::map<std::string, std::vector<Event *>> writeSet;
  std::map<std::string, std::vector<Event *>> readSetRelatedToBranch;
  std::map<std::string, std::vector<Event *>> writeSetRelatedToBranch;

  // 锁操作集合，以lock/unlock为对收集-->生成同步语义约束
  // key--mutex（锁名，一个地址就ok，每个锁全局必唯一）, value--the whole lock/unlock pairs with respect to one mutex
  std::map<std::string, std::vector<LockPair *>> all_lock_unlock;
  // key--condition 变量标识, value--the whole wait events that wait this conditional var
  std::map<std::string, std::vector<Wait_Lock *>> all_wait;
  // key--condition 变量标识, value--the whole signal events that signal this conditional var
  std::map<std::string, std::vector<Event *>> all_signal;
  // key--barrier地址#release次数, value--the whole wait events that wait this barrier var
  std::map<std::string, std::vector<Event *>> all_barrier;

  std::map<std::string, llvm::Constant *> global_variable_initializer;
  std::map<std::string, llvm::Constant *> global_variable_initializer_RelatedToBranch;

  //全局变量最终值-只记录使用的全局变量
  std::map<std::string, llvm::Constant *> global_variable_final;
  //输出语句printf产生的变量值
  // key--full name of var
  std::map<std::string, llvm::Constant *> printf_variable_value;
};

} /* namespace klee */

#endif /* LIB_CORE_TRACE_H_ */
