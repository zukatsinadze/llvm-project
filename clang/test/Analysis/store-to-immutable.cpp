// RUN: %clang_analyze_cc1 \
// RUN: -analyzer-checker=core,alpha.security.cert.env.ModelConstQualifiedReturn \
// RUN: -analyzer-output=text -verify -Wno-unused %s


int opterr = 1, optind = 1; // some globals
void top(int *p) {
  optind = 1; // triggers the checker for some reason

  // or this
  p[optind++] = 42;
//  ^^^^^^^^------------ triggers the checker as well.
}
