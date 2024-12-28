/*
   Copyright © Pascal JEAN <epsilonrt@gmail.com>

   SPDX-License-Identifier: BSD-3-Clause
*/
#include <modbus.h>
#include <stdio.h>
#include "unity.h"
#include "unit-test.h"

void test_compiled_version (void);
void test_linked_version (void);
void test_modbus_version_check (void);

void setUp (void) {
  // set stuff up here
}

void tearDown (void) {
  // clean stuff up here
}

void test_compiled_version (void) {

  printf ("Compiled with libmodbus version %s (%06X)\n",
          LIBMODBUS_VERSION_STRING,
          LIBMODBUS_VERSION_HEX);
  TEST_ASSERT_EQUAL_INT (UT_VERSION_MAJOR, LIBMODBUS_VERSION_MAJOR);
  TEST_ASSERT_EQUAL_INT (UT_VERSION_MINOR, LIBMODBUS_VERSION_MINOR);
  TEST_ASSERT_EQUAL_INT (UT_VERSION_MICRO, LIBMODBUS_VERSION_MICRO);
  TEST_ASSERT_EQUAL_STRING (UT_VERSION_STRING, LIBMODBUS_VERSION_STRING);
}

void test_linked_version (void) {

  printf ("Linked with libmodbus version %d.%d.%d\n",
          libmodbus_version_major,
          libmodbus_version_minor,
          libmodbus_version_micro);
  TEST_ASSERT_EQUAL_INT (LIBMODBUS_VERSION_MAJOR, libmodbus_version_major);
  TEST_ASSERT_EQUAL_INT (LIBMODBUS_VERSION_MINOR, libmodbus_version_minor);
  TEST_ASSERT_EQUAL_INT (LIBMODBUS_VERSION_MICRO, libmodbus_version_micro);
}

void test_modbus_version_check (void) {
    
  TEST_ASSERT_TRUE_MESSAGE (LIBMODBUS_VERSION_CHECK (2, 1, 0),
                            "The functions to read/write float values are available (2.1.0).");
  TEST_ASSERT_TRUE_MESSAGE (LIBMODBUS_VERSION_CHECK (2, 1, 1),
                            "Oh gosh, brand new API (2.1.1)!");
}

int main (void) {

  UNITY_BEGIN();
  RUN_TEST (test_compiled_version);
  RUN_TEST (test_linked_version);
  RUN_TEST (test_modbus_version_check);

  return UNITY_END();
}
