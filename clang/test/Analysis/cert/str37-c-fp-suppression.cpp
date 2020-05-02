// RUN: %clang_analyze_cc1 \
// RUN:  -analyzer-checker=alpha.security.cert.str.37c\
// RUN:  -verify %s

#include "../Inputs/system-header-simulator.h"

int isspace(int c);
int toupper(int c);
int isalnum(int c);
int isalpha(int c);
int isascii(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isupper(int c);
int isxdigit(int c);
int toascii(int c);
int tolower(int c);

namespace correct_use {

int test_tolower(const char *c) {
  char lowA = tolower('A'); // no warning
  return tolower(97);       // no warning
}

int test_toascii(const char *c) {
  return toascii((unsigned char)*c); // no warning
}

void test_toupper(const char *s) {
  char c = 98;
  c = toupper(c); // no warning
  char b = -2;
  b = toupper((unsigned char)b); // no warning
}

int test_isalnum() {
  int a = 50;
  int testEOF = isalnum(EOF); // no warning
  return isalnum(a);          // no warning
}

int test_isalpha() {
  char c = 'a';
  int b = isalpha(255); // no warning
  return isalpha(c);    // no warning
}

int test_isascii() {
  char c = 'a';
  int b = isascii(200); // no warning
  int A = isascii(65);  // no warning
  return isascii(c);    // no warning
}

int test_isblank() {
  char c = ' ';
  bool isEOFBlank = isblank(EOF); // no warning
  return isblank(c);              // no warning
}

int test_iscntrl() {
  char c = 2;
  return iscntrl(c); // no warning
}

int test_isdigit() {
  char c = '2';
  return isdigit(c); // no warning
}

int test_isgraph() {
  char c = '9';
  return isgraph(c); // no warning
}

int test_islower() {
  char c = 'a';
  return islower(c); // no warning
}

int test_isprint(char c) {
  return isprint((unsigned char)c); // no warning
}

int test_ispunct() {
  char c = 'a';
  char b = -2;
  bool test_unsigned = ispunct((unsigned char)b); // no warning
  bool not_true = ispunct(c);                     // no warning
  return ispunct(2);                              // no warning
}

int test_isupper(const char *b) {
  char c = 'A';
  bool A_is_upper = isupper(c);      // no warning
  return isupper((unsigned char)*b); // no warning
}

int test_isxdigit() {
  char hexa = '9';
  char not_hexa = 'M';
  return isxdigit(hexa) || isxdigit(not_hexa); // no warning
}
} // namespace correct_use

namespace incorrect_use {

int test_tolower() {
  int a = 5;
  int b = 7;
  char c = a - b;
  return tolower(c);
  // expected-warning@-1 {{arguments to character-handling function `tolower` must be representable as an unsigned char}}
}

int test_toascii(const char *c) {
  return toascii(*c);
  // expected-warning@-1 {{arguments to character-handling function `toascii` must be representable as an unsigned char}}
}

void test_toupper(const char *s) {
  char c = -2;
  c = toupper(c);
  // expected-warning@-1 {{arguments to character-handling function `toupper` must be representable as an unsigned char}}
}

int test_isalnum() {
  char c = -2;
  return isalnum(c);
  // expected-warning@-1 {{arguments to character-handling function `isalnum` must be representable as an unsigned char}}
}

int test_isalpha() {
  return isalpha(256);
  // expected-warning@-1 {{arguments to character-handling function `isalpha` must be representable as an unsigned char}}
}

int test_isascii() {
  int b = isascii(269);
  // expected-warning@-1 {{arguments to character-handling function `isascii` must be representable as an unsigned char}}
  return isascii(100); // no warning
}

int test_isblank() {
  char c = -2;
  return isblank(c);
  // expected-warning@-1 {{arguments to character-handling function `isblank` must be representable as an unsigned char}}
}

int test_iscntrl() {
  char c = -22;
  return iscntrl(c);
  // expected-warning@-1 {{arguments to character-handling function `iscntrl` must be representable as an unsigned char}}
}

int test_isdigit() {
  char c = -2;
  return isdigit(c);
  // expected-warning@-1 {{arguments to character-handling function `isdigit` must be representable as an unsigned char}}
}

int test_isgraph() {
  return isgraph(269);
  // expected-warning@-1 {{arguments to character-handling function `isgraph` must be representable as an unsigned char}}
}

int test_islower() {
  return islower(269);
  // expected-warning@-1 {{arguments to character-handling function `islower` must be representable as an unsigned char}}
}

int test_isprint(char c) {
  return isprint(c);
  // expected-warning@-1 {{arguments to character-handling function `isprint` must be representable as an unsigned char}}
}

int test_ispunct() {
  char c;
  return ispunct(c);
  // expected-warning@-1 {{arguments to character-handling function `ispunct` must be representable as an unsigned char}}
}

int test_isupper(const char *b) {
  return isupper(*b);
  // expected-warning@-1 {{arguments to character-handling function `isupper` must be representable as an unsigned char}}
}

int test_isxdigit() {
  char hexa = -10;
  return isxdigit(hexa);
  // expected-warning@-1 {{arguments to character-handling function `isxdigit` must be representable as an unsigned char}}
}

} // namespace incorrect_use
