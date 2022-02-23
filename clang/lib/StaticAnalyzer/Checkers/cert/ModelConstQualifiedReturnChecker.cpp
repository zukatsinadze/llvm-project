//== ModelConstQualifiedReturnChecker.cpp ----------------------- -*- C++ -*--=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines ModelConstQualifiedReturnChecker
// Return values of certain functions should be treated as const-qualified
//
// CERT SEI Rule ENV30-C
// For more information see: https://wiki.sei.cmu.edu/confluence/x/79UxBQ
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class ModelConstQualifiedReturnChecker : public Checker<eval::Call> {
private:
  void evalConstQualifiedReturnCall(const CallEvent &Call,
                                    CheckerContext &C) const;

  // SEI CERT ENV30-C
  const CallDescriptionSet ConstQualifiedReturnFunctions = {
      {"getenv", 1},
      {"setlocale", 2},
      {"strerror", 1},
      {"localeconv", 0},
      {"asctime", 1},
  };

public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};
} // namespace

void ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall(
    const CallEvent &Call, CheckerContext &C) const {
  const LocationContext *LCtx = C.getLocationContext();
  SValBuilder &SVB = C.getSValBuilder();
  MemRegionManager &RegMgr = SVB.getRegionManager();

  // Function call will return a pointer to the new symbolic region.
  SymbolRef FreshSym = SVB.getSymbolManager().conjureSymbol(
      Call.getOriginExpr(), LCtx, C.blockCount());
  const SymbolicRegion *SymReg = RegMgr.getSymbolicRegion(
      FreshSym,
      RegMgr.getGlobalsRegion(MemRegion::GlobalImmutableSpaceRegionKind));

  ProgramStateRef State = C.getState();
  State =
      State->BindExpr(Call.getOriginExpr(), LCtx, loc::MemRegionVal{SymReg});

  StringRef FunName = Call.getCalleeIdentifier()->getName();
  const NoteTag *Note = C.getNoteTag(
      [SymReg, FunName](PathSensitiveBugReport &BR, llvm::raw_ostream &Out) {
        if (BR.getBugType().getCheckerName() != "core.StoreToImmutable" ||
            !BR.isInteresting(SymReg))
          return;
        Out << "return value of '" << FunName
            << "' should be treated as const-qualified";
      });

  C.addTransition(State, Note);
}

bool ModelConstQualifiedReturnChecker::evalCall(const CallEvent &Call, CheckerContext &C) const {
    if (!ConstQualifiedReturnFunctions.contains(Call))
        return false;

    evalConstQualifiedReturnCall(Call, C);
    return true;
}

void ento::registerModelConstQualifiedReturnChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<ModelConstQualifiedReturnChecker>();
}

bool ento::shouldRegisterModelConstQualifiedReturnChecker(const CheckerManager &) {
  return true;
}
