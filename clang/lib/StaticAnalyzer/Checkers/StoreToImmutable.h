//=== StoreToImmutable.h - Expose ImmutableMemoryBind BugType ------*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines getter function for ImmutableMemoryBind BugType
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_STORETOIMMUTABLE_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_STORETOIMMUTABLE_H


#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang {
namespace ento {
namespace immutablestore {

const BugType *getImmutableMemoryBindBugType();

} // namespace immutablestore
} // namespace ento
} // namespace clang

#endif // LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_STORETOIMMUTABLE_H
