//===-- FilterSymbolicExpr.cpp -----------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_FILTERSYMBOLICEXPR_C_
#define LIB_CORE_FILTERSYMBOLICEXPR_C_

#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/Casting.h>

#include "klee/Encode/Event.h"
#include "klee/Encode/FilterSymbolicExpr.h"
#include "klee/Module/KInstruction.h"

#define OPTIMIZATION1 1

namespace klee {

std::string FilterSymbolicExpr::getName(ref<klee::Expr> value) {
  std::string globalName = getGlobalName(value);
  std::string name = getName(globalName);
  return name;
}

std::string FilterSymbolicExpr::getName(std::string globalName) {
  std::stringstream name;
  name.str("");
  unsigned int i = 0;
  while ((globalName.at(i) != 'S') && (globalName.at(i) != 'L')) {
    name << globalName.at(i);
    i++;
  }
  return name.str();
}

std::string FilterSymbolicExpr::getGlobalName(ref<klee::Expr> value) {
  ReadExpr *revalue;
  if (value->getKind() == Expr::Concat) {
    ConcatExpr *ccvalue = cast<ConcatExpr>(value);
    revalue = cast<ReadExpr>(ccvalue->getKid(0));
  } else if (value->getKind() == Expr::Read) {
    revalue = cast<ReadExpr>(value);
  } else {
    assert(0 && "getGlobalName");
  }
  std::string globalName = revalue->updates.root->name;
  return globalName;
}

void FilterSymbolicExpr::resolveSymbolicExpr(ref<klee::Expr> symbolicExpr, std::set<std::string> &relatedSymbolicExpr) {
  if (symbolicExpr->getKind() == Expr::Read) {
    std::string name = getName(symbolicExpr);
    if (relatedSymbolicExpr.find(name) == relatedSymbolicExpr.end()) {
      relatedSymbolicExpr.insert(name);
    }
    return;
  } else {
    unsigned kidsNum = symbolicExpr->getNumKids();
    if (kidsNum == 2 && symbolicExpr->getKid(0) == symbolicExpr->getKid(1)) {
      resolveSymbolicExpr(symbolicExpr->getKid(0), relatedSymbolicExpr);
    } else {
      for (unsigned int i = 0; i < kidsNum; i++) {
        resolveSymbolicExpr(symbolicExpr->getKid(i), relatedSymbolicExpr);
      }
    }
  }
}

void FilterSymbolicExpr::resolveTaintExpr(ref<klee::Expr> taintExpr, std::vector<ref<klee::Expr>> &relatedTaintExpr,
                                          bool &isTaint) {
  if (taintExpr->getKind() == Expr::Concat || taintExpr->getKind() == Expr::Read) {
    unsigned i;
    for (i = 0; i < relatedTaintExpr.size(); i++) {
      if (relatedTaintExpr[i] == taintExpr) {
        break;
      }
    }
    if (i == relatedTaintExpr.size()) {
      relatedTaintExpr.push_back(taintExpr);
      if (taintExpr->isTaint) {
        isTaint = true;
      }
    }
    return;
  } else if (taintExpr->getKind() == Expr::Constant) {
    if (taintExpr->isTaint) {
      isTaint = true;
    }
  } else {
    unsigned kidsNum = taintExpr->getNumKids();
    if (kidsNum == 2 && taintExpr->getKid(0) == taintExpr->getKid(1)) {
      resolveTaintExpr(taintExpr->getKid(0), relatedTaintExpr, isTaint);
    } else {
      for (unsigned int i = 0; i < kidsNum; i++) {
        resolveTaintExpr(taintExpr->getKid(i), relatedTaintExpr, isTaint);
      }
    }
  }
}

void FilterSymbolicExpr::addExprToSet(const std::set<std::string> &Expr, std::set<std::string> &relatedSymbolicExpr) {
  for (auto name : Expr) {
    if (relatedSymbolicExpr.find(name) == relatedSymbolicExpr.end()) {
      relatedSymbolicExpr.insert(name);
    }
  }
}

void FilterSymbolicExpr::addExprToVector(const std::vector<std::string> &Expr,
                                         std::vector<std::string> &relatedSymbolicExpr) {
  for (auto nvarName : Expr) {
    bool isFind = false;
    for (auto RSExprVar : relatedSymbolicExpr) {
      if (RSExprVar == nvarName) {
        isFind = true;
      }
    }
    if (!isFind) {
      relatedSymbolicExpr.push_back(nvarName);
    }
  }
}

void FilterSymbolicExpr::addExprToVector(const std::set<std::string> &Expr,
                                         std::vector<std::string> &relatedSymbolicExpr) {

  for (auto varName : Expr) {
    bool isFind = false;
    for (auto RSExprVar : relatedSymbolicExpr) {
      if (RSExprVar == varName) {
        isFind = true;
      }
    }
    if (!isFind) {
      relatedSymbolicExpr.push_back(varName);
    }
  }
}

void FilterSymbolicExpr::addExprToRelate(const std::set<std::string> &Exprs) {
  for (auto name : Exprs) {
    if (allRelatedSymbolicExprSet.find(name) == allRelatedSymbolicExprSet.end()) {
      allRelatedSymbolicExprSet.insert(name);
      allRelatedSymbolicExprVector.push_back(name);
    }
  }
}

bool FilterSymbolicExpr::isRelated(std::string varName) {
  if (allRelatedSymbolicExprSet.find(varName) != allRelatedSymbolicExprSet.end()) {
    return true;
  } else {
    return false;
  }
}

void FilterSymbolicExpr::fillterTrace(Trace *trace, std::set<std::string> RelatedSymbolicExpr) {
  std::string name;

  const auto &pathCondition = trace->pathCondition;
  auto &pathConditionRelatedToBranch = trace->pathConditionRelatedToBranch;
  pathConditionRelatedToBranch.clear();
  for (auto it : pathCondition) {
    name = getName(it.get()->getKid(1));
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      pathConditionRelatedToBranch.push_back(it);
    }
  }

  const auto &readSet = trace->readSet;
  auto &readSetRelatedToBranch = trace->readSetRelatedToBranch;
  readSetRelatedToBranch.clear();
  for (auto nit : readSet) {
    name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      readSetRelatedToBranch[nit.first] = nit.second;
    }
  }

  const auto &writeSet = trace->writeSet;
  auto &writeSetRelatedToBranch = trace->writeSetRelatedToBranch;
  writeSetRelatedToBranch.clear();
  for (auto nit : writeSet) {
    name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      writeSetRelatedToBranch[nit.first] = nit.second;
    }
  }

  const auto &global_variable_initializer = trace->global_variable_initializer;
  auto &global_variable_initisalizer_RelatedToBranch = trace->global_variable_initializer_RelatedToBranch;
  global_variable_initisalizer_RelatedToBranch.clear();
  for (auto nit : global_variable_initializer) {
    name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      global_variable_initisalizer_RelatedToBranch[nit.first] = nit.second;
    }
  }

  for (auto currentEvent : trace->path) {
    if (currentEvent->isGlobal != true) {
      continue;
    }
    if (currentEvent->inst->inst->getOpcode() == llvm::Instruction::Load ||
        currentEvent->inst->inst->getOpcode() == llvm::Instruction::Store) {
      if (RelatedSymbolicExpr.find(currentEvent->name) == RelatedSymbolicExpr.end() && OPTIMIZATION1) {
        currentEvent->isEventRelatedToBranch = false;
      } else {
        currentEvent->isEventRelatedToBranch = true;
      }
    }
  }
}

void FilterSymbolicExpr::filterUseless(Trace *trace) {
  std::string name;
  std::vector<std::string> remainingExprName;
  std::vector<ref<klee::Expr>> remainingExpr;
  allRelatedSymbolicExprSet.clear();
  allRelatedSymbolicExprVector.clear();
  remainingExprName.clear();
  remainingExpr.clear();
  for (auto it : trace->storeSymbolicExpr) {
    name = getName(it.get()->getKid(1));
    remainingExprName.push_back(name);
    remainingExpr.push_back(it.get());
  }

  std::vector<std::set<std::string>> &brRelatedSymbolicExpr = trace->brRelatedSymbolicExpr;
  for (auto brExpr : trace->brSymbolicExpr) {
    std::set<std::string> tempSymbolicExpr;
    resolveSymbolicExpr(brExpr.get(), tempSymbolicExpr);
    brRelatedSymbolicExpr.push_back(tempSymbolicExpr);
    addExprToRelate(tempSymbolicExpr);
  }

  std::vector<std::set<std::string>> &assertRelatedSymbolicExpr = trace->assertRelatedSymbolicExpr;
  for (auto assertExpr : trace->assertSymbolicExpr) {
    std::set<std::string> tempSymbolicExpr;
    resolveSymbolicExpr(assertExpr.get(), tempSymbolicExpr);
    assertRelatedSymbolicExpr.push_back(tempSymbolicExpr);
    addExprToRelate(tempSymbolicExpr);
  }

  std::vector<ref<klee::Expr>> &pathCondition = trace->pathCondition;
  auto &varRelatedSymbolicExpr = trace->allRelatedSymbolicExprs;

  for (auto name : allRelatedSymbolicExprVector) {
#if FILTER_DEBUG
    llvm::errs() << "allRelatedSymbolicExprSet name : " << name << "\n";
#endif
    if (remainingExpr.empty())
      break;
    std::vector<ref<Expr>>::iterator itt = remainingExpr.begin();
    std::vector<std::string>::iterator it = remainingExprName.begin();
    for (; it != remainingExprName.end();) {
      if (name == *it) {
        it = remainingExprName.erase(it);
        pathCondition.push_back(*itt);
#if FILTER_DEBUG
        llvm::errs() << *itt << "\n";
#endif
        std::set<std::string> tempSymbolicExpr;
        resolveSymbolicExpr(*itt, tempSymbolicExpr);
        if (varRelatedSymbolicExpr.find(name) != varRelatedSymbolicExpr.end()) {
          addExprToSet(tempSymbolicExpr, varRelatedSymbolicExpr[name]);
        } else {
          varRelatedSymbolicExpr[name] = tempSymbolicExpr;
        }
        addExprToRelate(tempSymbolicExpr);
        itt = remainingExpr.erase(itt);
      } else {
        ++it;
        ++itt;
      }
    }
  }

#if FILTER_DEBUG
  llvm::errs() << "allRelatedSymbolicExprSet"
               << "\n";
  for (std::set<std::string>::iterator nit = allRelatedSymbolicExprSet.begin(); nit != allRelatedSymbolicExprSet.end();
       ++nit) {
    llvm::errs() << (*nit).c_str() << "\n";
  }
#endif

  std::map<std::string, long> &varThread = trace->varThread;

  std::map<std::string, std::vector<Event *>> usefulReadSet;
  std::map<std::string, std::vector<Event *>> &readSet = trace->readSet;
  for (auto oneVarReads : readSet) {
    trace->allReadSet.insert(oneVarReads);
    name = oneVarReads.first;
    if (!isRelated(name) && OPTIMIZATION1) {
      continue;
    }
    usefulReadSet.insert(oneVarReads);
    if (varThread.find(name) == varThread.end()) {
      varThread[name] = (*(oneVarReads.second.begin()))->threadId;
    }
    long id = varThread[name];
    if (id == 0) {
      continue;
    }
    for (auto read : oneVarReads.second) {
      if (id == read->threadId) {
        continue;
      }
      varThread[name] = 0;
      break;
    }
  }
  readSet.clear();
  for (auto UR : usefulReadSet) {
    readSet.insert(UR);
  }

  std::map<std::string, std::vector<Event *>> usefulWriteSet;
  std::map<std::string, std::vector<Event *>> &writeSet = trace->writeSet;
  std::map<std::string, std::vector<Event *>> &allWriteSet = trace->allWriteSet;
  usefulWriteSet.clear();
  for (auto oneVarWrites : writeSet) {
    allWriteSet.insert(oneVarWrites);
    name = oneVarWrites.first;
    if (isRelated(name) || !OPTIMIZATION1) {
      usefulWriteSet.insert(oneVarWrites);
      if (varThread.find(name) == varThread.end()) {
        varThread[name] = (*(oneVarWrites.second.begin()))->threadId;
      }
      long id = varThread[name];
      if (id == 0) {
        continue;
      }
      for (auto write : oneVarWrites.second) {
        if (id != write->threadId) {
          varThread[name] = 0;
          break;
        }
      }
    }
  }
  writeSet.clear();
  for (auto UW : usefulWriteSet) {
    writeSet.insert(UW);
  }

  for (auto vt : varThread) {
    if (usefulWriteSet.find(vt.first) == usefulWriteSet.end()) {
      vt.second = -1;
    }
  }

  std::map<std::string, llvm::Constant *> usefulGlobal_variable_initializer;
  std::map<std::string, llvm::Constant *> &global_variable_initializer = trace->global_variable_initializer;
#if FILTER_DEBUG
  llvm::errs() << "global_variable_initializer = " << trace->global_variable_initializer.size() << "\n";
  for (std::map<std::string, llvm::Constant *>::iterator it = trace->global_variable_initializer.begin(),
                                                         ie = trace->global_variable_initializer.end();
       it != ie; ++it) {
    llvm::errs() << it->first << "\n";
  }
#endif
  usefulGlobal_variable_initializer.clear();
  for (auto G : global_variable_initializer) {
    name = G.first;
    if (isRelated(name) || !OPTIMIZATION1) {
      usefulGlobal_variable_initializer.insert(G);
    }
  }
  global_variable_initializer.clear();
  for (auto UG : usefulGlobal_variable_initializer) {
    global_variable_initializer.insert(UG);
  }
#if FILTER_DEBUG
  llvm::errs() << "global_variable_initializer = " << trace->global_variable_initializer.size() << "\n";
  for (std::map<std::string, llvm::Constant *>::iterator it = trace->global_variable_initializer.begin(),
                                                         ie = trace->global_variable_initializer.end();
       it != ie; ++it) {
    llvm::errs() << it->first << "\n";
  }
#endif
  for (auto currentEvent : trace->path) {
    if (currentEvent->isGlobal == true) {
      if (currentEvent->inst->inst->getOpcode() == llvm::Instruction::Load ||
          currentEvent->inst->inst->getOpcode() == llvm::Instruction::Store) {
        if (isRelated(currentEvent->name) && OPTIMIZATION1) {
          currentEvent->isEventRelatedToBranch = false;
        } else {
          currentEvent->isEventRelatedToBranch = true;
        }
      } else {
        currentEvent->isEventRelatedToBranch = true;
      }
    }
  }

  std::vector<ref<klee::Expr>> rwSymbolicExprRelatedToBranch;
  std::vector<ref<klee::Expr>> &rwSymbolicExpr = trace->rwSymbolicExpr;
  for (auto rwExpr : rwSymbolicExpr) {
    name = getName(rwExpr->getKid(1));
    if (isRelated(name) || !OPTIMIZATION1) {
      rwSymbolicExprRelatedToBranch.push_back(rwExpr);
    }
  }
  rwSymbolicExpr.clear();
  for (auto rwExprBr : rwSymbolicExprRelatedToBranch) {
    rwSymbolicExpr.push_back(rwExprBr);
  }

  fillterTrace(trace, allRelatedSymbolicExprSet);
}

bool FilterSymbolicExpr::filterUselessWithSet(Trace *trace, std::set<std::string> &relatedSymbolicExpr) {
  std::set<std::string> &RelatedSymbolicExpr = trace->RelatedSymbolicExpr;
  RelatedSymbolicExpr.clear();
  addExprToSet(relatedSymbolicExpr, RelatedSymbolicExpr);

  std::map<std::string, std::set<std::string>> &allRelatedSymbolicExpr = trace->allRelatedSymbolicExprs;
  for (auto name : RelatedSymbolicExpr) {
    if (allRelatedSymbolicExpr.find(name) != allRelatedSymbolicExpr.end()) {
      addExprToSet(allRelatedSymbolicExpr[name], RelatedSymbolicExpr);
    }
  }

  bool branch = false;
  std::map<std::string, long> &varThread = trace->varThread;
  for (auto name : RelatedSymbolicExpr) {
    if (varThread[name] == 0) {
      branch = true;
      break;
    }
  }
  if (branch) {
    //		fillterTrace(trace, RelatedSymbolicExpr);
    return true;
  } else {
    return false;
  }
}

void FilterSymbolicExpr::filterUselessByTaint(Trace *trace) {

  std::set<std::string> &taintSymbolicExpr = trace->taintSymbolicExpr;
  std::vector<std::string> taint;
  for (auto taintExpr : taintSymbolicExpr) {
    taint.push_back(taintExpr);
  }

  std::vector<std::string> unTaint;
  for (auto unTaintExpr : trace->unTaintSymbolicExpr) {
    unTaint.push_back(unTaintExpr);
  }

  std::map<std::string, std::set<std::string>> &allRelatedSymbolicExpr = trace->allRelatedSymbolicExprs;
  for (auto name : taint) {
    for (std::vector<std::string>::iterator itt = unTaint.begin(); itt != unTaint.end();) {
      if (allRelatedSymbolicExpr.find(*itt) == allRelatedSymbolicExpr.end()) {
        itt++;
      } else if (allRelatedSymbolicExpr[*itt].find(name) != allRelatedSymbolicExpr[*itt].end()) {
        taint.push_back(*itt);
        unTaint.erase(itt);
      } else {
        itt++;
      }
    }
  }

  std::set<std::string> &potentialTaintSymbolicExpr = trace->potentialTaint;
  for (auto name : taint) {
    if (taintSymbolicExpr.find(name) == taintSymbolicExpr.end()) {
      potentialTaintSymbolicExpr.insert(name);
    }
  }
}

} // namespace klee

#endif /* LIB_CORE_FILTERSYMBOLICEXPR_C_ */
