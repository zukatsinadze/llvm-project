//== ModelConstQualifiedReturnChecker.cpp ----------------------- -*- C++ -*--=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines ModelConstQualifiedReturnChecker.
// Return values of certain functions should be treated as const-qualified.
//
// Checker is based on CERT SEI Rule ENV30-C.
// For more information see: https://wiki.sei.cmu.edu/confluence/x/79UxBQ
//===----------------------------------------------------------------------===//

#include "../StoreToImmutable.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class ModelConstQualifiedReturnChecker : public Checker<check::PostCall> {
private:
  using HandlerFn = void (ModelConstQualifiedReturnChecker::*)(
      const CallEvent &Call, CheckerContext &C) const;

  void evalConstQualifiedReturnCall(const CallEvent &Call,
                                    CheckerContext &C) const;

  // SEI CERT ENV30-C
  const CallDescriptionMap<HandlerFn> ConstQualifiedReturnFunctions = {
      {{"getenv", 1},
       &ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall},
      {{"setlocale", 2},
       &ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall},
      {{"strerror", 1},
       &ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall},
      {{"localeconv", 0},
       &ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall},
      {{"asctime", 1},
       &ModelConstQualifiedReturnChecker::evalConstQualifiedReturnCall},
  };

public:
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
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

  StringRef FunctionName = Call.getCalleeIdentifier()->getName();

  const NoteTag *Note = C.getNoteTag(
      [SymReg, FunctionName](PathSensitiveBugReport &BR, llvm::raw_ostream &Out) {
        if (&BR.getBugType() != immutablestore::getImmutableMemoryBindBugType() ||
            !BR.isInteresting(SymReg))
          return;
        Out << "Return value of '" << FunctionName
            << "' should be treated as const-qualified";
      });
  C.addTransition(State, Note);
}

void ModelConstQualifiedReturnChecker::checkPostCall(const CallEvent &Call, CheckerContext &C) const {
  // Check if function return value should be const qualified
  const auto *Handler = ConstQualifiedReturnFunctions.lookup(Call);
  if (!Handler)
    return;

  (this->**Handler)(Call, C);
}

void ento::registerModelConstQualifiedReturnChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<ModelConstQualifiedReturnChecker>();
}

bool ento::shouldRegisterModelConstQualifiedReturnChecker(const CheckerManager &) {
  return true;
}
