//== Str37Checker.cpp --------------------------------- -*- C++ -*--=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines Str37Checker which finds calls of character-handling
// functions with arguments which are not representable as an unsigned char.
// https://wiki.sei.cmu.edu/confluence/x/BNcxBQ
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class Str37Checker : public Checker<check::PreCall> {
private:
  BugType BT{this,
             "arguments to character-handling functions must be representable "
             "as an unsigned char",
             categories::SecurityError};

  using ArgCheck = std::function<void(const Str37Checker *, const CallEvent &,
                                      CheckerContext &)>;
  // The pairs are in the following form:
  // {{function-name, argument-count}, {callback, index-of-argument}}
  const CallDescriptionMap<std::pair<ArgCheck, unsigned>> CDM = {
      {{"isalnum", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isalpha", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isascii", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isblank", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"iscntrl", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isdigit", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isgraph", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"islower", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isprint", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"ispunct", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isspace", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isupper", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"isxdigit", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"toascii", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"toupper", 1}, {&Str37Checker::checkPreCall, 0}},
      {{"tolower", 1}, {&Str37Checker::checkPreCall, 0}}};

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};
} // namespace

void Str37Checker::checkPreCall(const CallEvent &Call,
                                CheckerContext &C) const {
  const auto *Lookup = CDM.lookup(Call);
  if (!Lookup)
    return;

  unsigned ArgIndex = (*Lookup).second;
  SValBuilder &SVB = C.getSValBuilder();
  ProgramStateRef State = C.getState();

  const Expr *ArgExpr = Call.getArgExpr(ArgIndex);
  QualType Ty = ArgExpr->IgnoreImpCasts()->getType();
  if (Ty->isPointerType())
    Ty = Ty->getPointeeType();
  const auto *B = dyn_cast<BuiltinType>(Ty);
  if (!B)
    return;
  if (B->getKind() == BuiltinType::UChar)
    return;

  SVal ArgV = Call.getArgSVal(ArgIndex);
  if (const auto AcutalVal = SVB.getKnownValue(State, ArgV))
    // -1 is included because of EOF
    if (-1 <= *AcutalVal && *AcutalVal <= 255)
      return;

  StringRef CallName = Call.getCalleeIdentifier()->getName();
  SmallString<128> Msg;
  llvm::raw_svector_ostream Out(Msg);
  Out << '\'' << "arguments to character-handling function `" << CallName
      << "` must be representable as an unsigned char";
  std::string ReportMsg = Out.str().str();

  ExplodedNode *N = C.generateErrorNode();
  auto Report = std::make_unique<PathSensitiveBugReport>(BT, ReportMsg, N);

  C.emitReport(std::move(Report));
}

void ento::registerStr37(CheckerManager &Mgr) {
  Mgr.registerChecker<Str37Checker>();
}

bool ento::shouldRegisterStr37(const CheckerManager &) { return true; }
