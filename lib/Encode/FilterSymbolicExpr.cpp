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
#include "klee/Config/DebugMacro.h"

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

void FilterSymbolicExpr::prepareData(Trace *trace) {
std::string name;
  std::vector<std::pair<std::string, ref<klee::Expr>>> remainingExprs;
  allRelatedSymbolicExprSet.clear();
  allRelatedSymbolicExprVector.clear();
  for (auto it : trace->storeSymbolicExpr) {
    name = getName(it.get()->getKid(1));
    remainingExprs.push_back(make_pair(name, it.get()));
  }

  // Log all the variables name use in branches.
  for (auto brExpr : trace->brSymbolicExpr) {
    std::set<std::string> tempSymbolicExpr;
    resolveSymbolicExpr(brExpr.get(), tempSymbolicExpr);
    trace->brRelatedSymbolicExpr.push_back(tempSymbolicExpr);
    addExprToRelate(tempSymbolicExpr);
  }

  // Log all the variables name use in assertions.
  for (auto assertExpr : trace->assertSymbolicExpr) {
    std::set<std::string> tempSymbolicExpr;
    resolveSymbolicExpr(assertExpr.get(), tempSymbolicExpr);
    trace->assertRelatedSymbolicExpr.push_back(tempSymbolicExpr);
    addExprToRelate(tempSymbolicExpr);
  }

  // Explore all related exprs
  while (!allRelatedSymbolicExprVector.empty()) {
    std::string name = allRelatedSymbolicExprVector.back();
    allRelatedSymbolicExprVector.pop_back();
#if FILTER_USELESS_DEBUG
    llvm::errs() << "allRelatedSymbolicExprSet name : " << name << "\n";
#endif
    if (remainingExprs.empty())
      break;
    auto it = remainingExprs.begin();
    for (; it != remainingExprs.end(); ) {
      if (name != it->first) {
        ++it;
        continue;
      }
#if FILTER_USELESS_DEBUG
      llvm::errs() << it->second << "\n";
#endif
      trace->pathCondition.push_back(it->second);
      std::set<std::string> tempSymbolicExpr;
      resolveSymbolicExpr(it->second, tempSymbolicExpr);
      addExprToRelate(tempSymbolicExpr);

      if (trace->allRelatedSymbolicExprs.find(name) != trace->allRelatedSymbolicExprs.end()) {
        addExprToSet(tempSymbolicExpr, trace->allRelatedSymbolicExprs[name]);
      } else {
        trace->allRelatedSymbolicExprs[name] = tempSymbolicExpr;
      }
      it = remainingExprs.erase(it);
    }
  }

#if FILTER_USELESS_DEBUG
  llvm::errs() << "allRelatedSymbolicExprSet\n";
  for (auto name : allRelatedSymbolicExprSet) {
    llvm::errs() << name.c_str() << "\n";
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
#if FILTER_USELESS_DEBUG
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
#if FILTER_USELESS_DEBUG
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
}

void FilterSymbolicExpr::filterExprs(Trace *trace, std::set<std::string> RelatedSymbolicExpr) {
  trace->pathConditionRelatedToBranch.clear();
  for (auto it : trace->pathCondition) {
    std::string name = getName(it.get()->getKid(1));
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      trace->pathConditionRelatedToBranch.push_back(it);
    }
  }

  trace->readSetRelatedToBranch.clear();
  for (auto nit : trace->readSet) {
    std::string name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      trace->readSetRelatedToBranch[nit.first] = nit.second;
    }
  }

  trace->writeSetRelatedToBranch.clear();
  for (auto nit : trace->writeSet) {
    std::string name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      trace->writeSetRelatedToBranch[nit.first] = nit.second;
    }
  }

  trace->global_variable_initializer_RelatedToBranch.clear();
  for (auto nit : trace->global_variable_initializer) {
    std::string name = nit.first;
    if (RelatedSymbolicExpr.find(name) != RelatedSymbolicExpr.end() || !OPTIMIZATION1) {
      trace->global_variable_initializer_RelatedToBranch[nit.first] = nit.second;
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

void FilterSymbolicExpr::filterUnusedExprs(Trace *trace) {
  #if FILTER_USELESS_DEBUG
  std::cerr << "Before Filtering:" << std::endl;
  std::cerr << "storeSymbolicExpr = " << trace->storeSymbolicExpr.size() << std::endl;
  for (std::vector<ref<Expr>>::iterator it = trace->storeSymbolicExpr.begin(), ie = trace->storeSymbolicExpr.end();
       it != ie; ++it) {
    it->get()->dump();
  }
  std::cerr << "brSymbolicExpr = " << trace->brSymbolicExpr.size() << std::endl;
  for (std::vector<ref<Expr>>::iterator it = trace->brSymbolicExpr.begin(), ie = trace->brSymbolicExpr.end(); it != ie;
       ++it) {
    it->get()->dump();
  }
  std::cerr << "assertSymbolicExpr = " << trace->assertSymbolicExpr.size() << std::endl;
  for (std::vector<ref<Expr>>::iterator it = trace->assertSymbolicExpr.begin(), ie = trace->assertSymbolicExpr.end();
       it != ie; ++it) {
    it->get()->dump();
  }
#endif
  prepareData(trace);
  filterExprs(trace, allRelatedSymbolicExprSet);
#if FILTER_USELESS_DEBUG
  std::cerr << "After Filtering:" << std::endl;
  std::cerr << "pathCondition = " << trace->pathCondition.size() << std::endl;
  for (std::vector<ref<Expr>>::iterator it = trace->pathCondition.begin(), ie = trace->pathCondition.end(); it != ie;
       ++it) {
    it->get()->dump();
  }
#endif
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
    //		filterExprs(trace, RelatedSymbolicExpr);
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
