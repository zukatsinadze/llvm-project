//== StoreToImmutableChecker.cpp ------------------------------- -*- C++ -*--=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines StoreToImmutableChecker which warns binds on immutable
// memory.
//===----------------------------------------------------------------------===//

#include "clang/AST/Type.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

static const BugType *ImmutableMemoryBindPtr;

class StoreToImmutableChecker : public Checker<check::Bind> {
public:
  BugType ImmutableMemoryBind{this,
                              "Immutable memory should not be overwritten",
                              categories::MemoryError};
  void checkBind(SVal loc, SVal val, const Stmt *S, CheckerContext &C) const;
};
} // namespace


namespace clang {
namespace ento {
namespace immutablestore {

const BugType *getImmutableMemoryBindBugType() { return ImmutableMemoryBindPtr; }

} // namespace immutablestore
} // namespace ento
} // namespace clang




// Check if region that should be immutable is being modified
void StoreToImmutableChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                        CheckerContext &C) const {
  const MemRegion *R = Loc.getAsRegion();
  if (!R)
    return;

  if (!isa<GlobalImmutableSpaceRegion>(R->getMemorySpace()))
    return;

  ExplodedNode *ErrorNode = C.generateErrorNode();
  if (!ErrorNode)
    return;

  auto Report = std::make_unique<PathSensitiveBugReport>(
      ImmutableMemoryBind, "Modifying immutable memory", ErrorNode);
  Report->markInteresting(R);

  if (const auto *BP = dyn_cast<BinaryOperator>(S))
    if (const auto *UP = dyn_cast<UnaryOperator>(BP->getLHS())) {
      bugreporter::trackExpressionValue(
          Report->getErrorNode(), UP->getSubExpr()->IgnoreImpCasts(), *Report);
    }

  C.emitReport(std::move(Report));
}

void ento::registerStoreToImmutableChecker(CheckerManager &Mgr) {
  StoreToImmutableChecker *Checker = Mgr.registerChecker<StoreToImmutableChecker>();
  ImmutableMemoryBindPtr = &Checker->ImmutableMemoryBind;
}

bool ento::shouldRegisterStoreToImmutableChecker(const CheckerManager &) {
  return true;
}
