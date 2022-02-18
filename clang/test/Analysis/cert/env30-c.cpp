// RUN: %clang_analyze_cc1 \
// RUN:  -analyzer-checker=core.StoreToImmutable,alpha.security.cert.env.ModelConstQualifiedReturn \
// RUN:  -analyzer-output=text -verify -Wno-unused %s

#include "../Inputs/system-header-simulator.h"
char *getenv(const char *name);
char *setlocale(int category, const char *locale);
char *strerror(int errnum);
int setenv(const char *name, const char *value, int overwrite);
void free(void *memblock);
void *malloc(size_t size);
char *strdup(const char *s);

typedef struct {
} tm;
char *asctime(const tm *timeptr);

int strcmp(const char *, const char *);
void non_const_parameter_function(char *e);
void const_parameter_function(const char *);



void getenv_test1() {
  char *p = getenv("VAR");
  // expected-note@-1{{return value of 'getenv' should be treated as const-qualified}}
  *p = 'A';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}

void getenv_test2() {
  char *p;

  p = getenv("VAR");
  non_const_parameter_function(p); // FIXME: Maybe we should warn for these.
}

void getenv_test3() {
  char *p;
  p = getenv("VAR");
  const_parameter_function(p); // no-warning
}

void modify(char *p) {
  *p = 'A';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}

void getenv_test4() {
  char *p = getenv("VAR");
  // expected-note@-1{{return value of 'getenv' should be treated as const-qualified}}
  char *pp = p;
  modify(pp); // Writing to immutable region within the call.
  // expected-note@-1{{Calling 'modify'}}
}

void does_not_modify(char *p) {
  *p;
}

void getenv_test5() {
  char *p = getenv("VAR");
  does_not_modify(p); // no-warning
}

void getenv_test6() {
  static char *array[] = {0};

  if (!array[0]) {
    // expected-note@-1{{Taking true branch}}
    array[0] = getenv("TEMPDIR");
    // expected-note@-1{{return value of 'getenv' should be treated as const-qualified}}
  }

  *array[0] = 'A';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}

void getenv_test7() {
  char *p = getenv("VAR");
  // expected-note@-1{{return value of 'getenv' should be treated as const-qualified}}
  if (!p)
    // expected-note@-1{{Assuming 'p' is non-null}}
    // expected-note@-2{{Taking false branch}}
    return;
  p[0] = '\0';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}

void setlocale_test() {
  char *p = setlocale(0, "VAR");
  // expected-note@-1{{return value of 'setlocale' should be treated as const-qualified}}
  *p = 'A';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}

void strerror_test() {
    char *p = strerror(0);
    // expected-note@-1{{return value of 'strerror' should be treated as const-qualified}}
    *p = 'A';
    // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
    // expected-note@-2{{modifying immutable memory}}
}

void asctime_test() {
  const tm *t;
  char* p = asctime(t);
  // expected-note@-1{{return value of 'asctime' should be treated as const-qualified}}

  *p = 'A';
  // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
  // expected-note@-2{{modifying immutable memory}}
}


// Test cases from CERT Rule Page

namespace noncompliant {
void trstr(char *c_str, char orig, char rep) {
  while (*c_str != '\0') {
    // expected-note@-1{{Assuming the condition is true}}
    // expected-note@-2{{Loop condition is true.  Entering loop body}}
    if (*c_str == orig) {
      // expected-note@-1{{Assuming the condition is true}}
      // expected-note@-2{{Taking true branch}}
      *c_str = rep;
      // expected-warning@-1{{modifying immutable memory [core.StoreToImmutable]}}
      // expected-note@-2{{modifying immutable memory}}
    }
    ++c_str;
  }
}

void func(void) {
  char *env = getenv("TEST_ENV");
  // expected-note@-1{{return value of 'getenv' should be treated as const-qualified}}
  if (env == NULL) {
    // expected-note@-1{{Assuming 'env' is not equal to NULL}}
    // expected-note@-2{{Taking false branch}}
    return;
  }
  trstr(env, '"', '_'); // Writing to immutable region within the call.
  // expected-note@-1{{Calling 'trstr'}}
}

} // namespace noncompliant

namespace compliant1 {
void trstr(char *c_str, char orig, char rep) {
  while (*c_str != '\0') {
    if (*c_str == orig) {
      *c_str = rep; // no-warning
    }
    ++c_str;
  }
}

void func(void) {
  const char *env;
  char *copy_of_env;

  env = getenv("TEST_ENV");
  if (env == NULL) {
    return;
  }

  copy_of_env = (char *)malloc(strlen(env) + 1);
  if (copy_of_env == NULL) {
    return;
  }

  strcpy(copy_of_env, env);
  trstr(copy_of_env, '"', '_');
  /* ... */
  free(copy_of_env);
}
} // namespace compliant1

namespace compliant2 {
void trstr(char *c_str, char orig, char rep) {
  while (*c_str != '\0') {
    if (*c_str == orig) {
      *c_str = rep; // no-warning
    }
    ++c_str;
  }
}

void func(void) {
  const char *env;
  char *copy_of_env;

  env = getenv("TEST_ENV");
  if (env == NULL) {
    return;
  }

  copy_of_env = strdup(env);
  if (copy_of_env == NULL) {
    return;
  }

  trstr(copy_of_env, '"', '_');

  if (setenv("TEST_ENV", copy_of_env, 1) != 0) {
    return;
  }
  /* ... */
  free(copy_of_env);
}
} // namespace compliant2