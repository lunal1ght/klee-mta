//===-- FilterSymbolicExpr.h -------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_FILTERSYMBOLICEXPR_H_
#define LIB_CORE_FILTERSYMBOLICEXPR_H_

#include <set>
#include <string>
#include <vector>

#include "klee/ADT/Ref.h"
#include "klee/Encode/Trace.h"
#include "klee/Expr/Expr.h"

namespace klee {

class FilterSymbolicExpr {

private:
  std::set<std::string> allRelatedSymbolicExprSet;
  std::vector<std::string> allRelatedSymbolicExprVector;

public:
  static std::string getName(ref<Expr> value);
  static std::string getName(std::string globalVarFullName);
  static std::string getGlobalName(ref<Expr> value);
  static void resolveSymbolicExpr(ref<Expr> value, std::set<std::string> &relatedSymbolicExpr);
  static void resolveTaintExpr(ref<klee::Expr> value, std::vector<ref<klee::Expr>> &relatedSymbolicExpr, bool &isTaint);
  static void addExprToSet(const std::set<std::string> &Expr, std::set<std::string> &relatedSymbolicExpr);
  static void addExprToVector(const std::vector<std::string> &Expr, std::vector<std::string> &exprVector);
  static void addExprToVector(const std::set<std::string> &Expr, std::vector<std::string> &exprVectr);
  void addExprToRelate(const std::set<std::string> &Exprs);
  bool isRelated(std::string varName);
  void fillterTrace(Trace *trace, std::set<std::string> allRelatedSymbolicExpr);
  void filterUseless(Trace *trace);
  void filterUselessByTaint(Trace *trace);
  bool filterUselessWithSet(Trace *trace, std::set<std::string> &relatedSymbolicExpr);
};

} // namespace klee

#endif /* LIB_CORE_FILTERSYMBOLICEXPR_H_ */
