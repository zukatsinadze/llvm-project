// RUN: %clang_analyze_cc1 \
// RUN:  -analyzer-checker=alpha.security.cert.str.37c\
// RUN:  -verify %s

// Examples from the CERT rule's page.
// https://wiki.sei.cmu.edu/confluence/x/BNcxBQ

#include "../Inputs/system-header-simulator.h"

int isspace(int c);

size_t bad_usage(const char *s) {
  const char *t = s;
  size_t length = strlen(s) + 1;
  while (isspace(*t) && (t - s < length)) {
    // expected-warning@-1 {{arguments to character-handling function `isspace` must be representable as an unsigned char}}
    ++t;
  }
  return t - s;
}

size_t good_usage(const char *s) {
  const char *t = s;
  size_t length = strlen(s) + 1;
  while (isspace((unsigned char)*t) && (t - s < length)) { // no warning
    ++t;
  }
  return t - s;
}
