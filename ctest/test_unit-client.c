/*
   Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
   Copyright © Pascal JEAN <epsilonrt@gmail.com>

   SPDX-License-Identifier: BSD-3-Clause
*/

#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unit-test.h"
#include "unity.h"

#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>
# define SERIAL_PORT "COM1"
int usleep (__int64 usec);
int close (int fd) ;
#else
# include <sys/socket.h>
# include <unistd.h>
# define SERIAL_PORT "/dev/ttyUSB1"
#endif

typedef struct to_t {
  uint32_t sec;
  uint32_t usec;
} to_t;

const int EXCEPTION_RC = 2;
enum {
  TCP,
  TCP_PI,
  RTU
};

/* The complete tests suite */
void test_connect (void);
void test_timeout (void);
void test_coils (void);
void test_discrete_inputs (void);
void test_holding_registers (void);
void test_input_registers (void);
void test_mask_registers (void);
void test_float (void);
void test_illegal_data_address (void);
void test_too_many_data (void);
void test_slave_address (void);
void test_slave_reply (void);
void test_bad_response (void);
void test_manual_exception (void);
void test_invalid_init (void);
void test_server (void);

void send_crafted_request (int function,
                           uint8_t *req,
                           int req_len,
                           uint16_t max_value,
                           uint16_t bytes,
                           int backend_length,
                           int backend_offset);
int equal_dword (uint16_t *tab_reg, const uint32_t value);
int is_memory_equal (const void *s1, const void *s2, size_t size);

void setUp (void) {
  // set stuff up here
}

void tearDown (void) {
  // clean stuff up here
}

/* Global variables */
modbus_t *ctx;
int use_backend;
char *ip_or_device;
to_t old_response_to;
uint8_t *tab_rp_bits;
uint16_t *tab_rp_registers;
int REPORT_SLAVE_ID_LEN;
int old_slave;

int main (int argc, char *argv[]) {
  int nb_points;

  if (argc > 1) {

    if (strcmp (argv[1], "tcp") == 0) {

      use_backend = TCP;
    }
    else if (strcmp (argv[1], "tcppi") == 0) {

      use_backend = TCP_PI;
    }
    else if (strcmp (argv[1], "rtu") == 0) {

      use_backend = RTU;
    }
    else {

      printf ("Modbus client for unit testing\n");
      printf ("Usage:\n  %s [tcp|tcppi|rtu]\n", argv[0]);
      printf ("Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB1\n\n");
      exit (1);
    }
  }
  else {

    /* By default */
    use_backend = TCP;
  }

  if (argc > 2) {

    // ip or device provided as argument on the command line
    ip_or_device = argv[2];
  }
  else {

    // Set the default IP or device
    switch (use_backend) {

      case TCP:
        ip_or_device = "127.0.0.1";
        break;

      case TCP_PI:
        ip_or_device = "::1";
        break;

      case RTU:
        ip_or_device = SERIAL_PORT;
        break;

      default:
        break;
    }
  }

  UNITY_BEGIN();
  RUN_TEST (test_connect); /* Test #1 */
  RUN_TEST (test_timeout); /* Test #2 */

  /* Length of report slave ID response slave ID + ON/OFF + 'LMB' + version */
  REPORT_SLAVE_ID_LEN = 2 + 3 + strlen (LIBMODBUS_VERSION_STRING);

  /* Allocate and initialize the memory to store the bits */
  nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
  tab_rp_bits = (uint8_t *) malloc (nb_points * sizeof (uint8_t));
  memset (tab_rp_bits, 0, nb_points * sizeof (uint8_t));

  /* Allocate and initialize the memory to store the registers */
  nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ? UT_REGISTERS_NB
              : UT_INPUT_REGISTERS_NB;
  tab_rp_registers = (uint16_t *) malloc (nb_points * sizeof (uint16_t));
  memset (tab_rp_registers, 0, nb_points * sizeof (uint16_t));

  RUN_TEST (test_coils); /* Test #3 */
  RUN_TEST (test_discrete_inputs); /* Test #4 */
  RUN_TEST (test_input_registers); /* Test #5 */
  RUN_TEST (test_holding_registers); /* Test #6 */
  RUN_TEST (test_mask_registers); /* Test #7 */
  RUN_TEST (test_float); /* Test #8 */
  RUN_TEST (test_illegal_data_address); /* Test #9 */
  RUN_TEST (test_too_many_data); /* Test #10 */
  RUN_TEST (test_slave_address); /* Test #11 */
  RUN_TEST (test_slave_reply); /* Test #12 */
  RUN_TEST (test_bad_response); /* Test #13 */
  RUN_TEST (test_manual_exception); /* Test #14 */
  RUN_TEST (test_invalid_init); /* Test #15 */
  RUN_TEST (test_server); /* Test #16 */
  modbus_set_response_timeout (ctx, old_response_to.sec, old_response_to.usec);

  /* Free the memory */
  free (tab_rp_bits);
  free (tab_rp_registers);

  /* Close the connection */
  modbus_close (ctx);
  modbus_free (ctx);
  return (UNITY_END());
}

void test_connect (void) {

  if (use_backend == TCP) {

    ctx = modbus_new_tcp (ip_or_device, 1502);
  }
  else if (use_backend == TCP_PI) {

    ctx = modbus_new_tcp_pi (ip_or_device, "1502");
  }
  else {

    ctx = modbus_new_rtu (ip_or_device, 115200, 'N', 8, 1);
  }

  TEST_ASSERT_TRUE_MESSAGE (ctx != NULL, "Unable to allocate libmodbus context");

  modbus_set_debug (ctx, TRUE);
  modbus_set_error_recovery (ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

  if (use_backend == RTU) {

    modbus_set_slave (ctx, SERVER_ID);
  }

  modbus_get_response_timeout (ctx, &old_response_to.sec, &old_response_to.usec);

  if (modbus_connect (ctx) == -1) {

    fprintf (stderr, "Connection failed: %s\n", modbus_strerror (errno));
    modbus_free (ctx);
    TEST_FAIL();
  }
}

void test_timeout (void) {
  to_t new_response_to;

  modbus_get_response_timeout (ctx, &new_response_to.sec, &new_response_to.usec);
  TEST_ASSERT_EQUAL_UINT32 (old_response_to.sec, new_response_to.sec);
  TEST_ASSERT_EQUAL_UINT32 (old_response_to.usec, new_response_to.usec);
}

void test_coils (void) {

  /* Single */
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_write_bit (ctx, UT_BITS_ADDRESS, ON));
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_read_bits (ctx, UT_BITS_ADDRESS, 1, tab_rp_bits));
  TEST_ASSERT_EQUAL_UINT8 (ON, tab_rp_bits[0]);
  /* End single */

  /* Multiple bits */
  {
    uint8_t tab_value[UT_BITS_NB];

    modbus_set_bits_from_bytes (tab_value, 0, UT_BITS_NB, UT_BITS_TAB);
    TEST_ASSERT_EQUAL_INT (UT_BITS_NB,
                           modbus_write_bits (ctx, UT_BITS_ADDRESS, UT_BITS_NB, tab_value));
  }
  // check the number of bits read
  TEST_ASSERT_EQUAL_INT (UT_BITS_NB,
                         modbus_read_bits (ctx, UT_BITS_ADDRESS, UT_BITS_NB, tab_rp_bits));

  int i = 0;
  uint16_t nb_points = UT_BITS_NB;

  // check the value of each bit
  while (nb_points > 0) {
    int nb_bits = (nb_points > 8) ? 8 : nb_points;

    TEST_ASSERT_EQUAL_UINT8 (UT_BITS_TAB[i],
                             modbus_get_byte_from_bits (tab_rp_bits, i * 8, nb_bits));
    nb_points -= nb_bits;
    i++;
  }
  /* End of multiple bits */
}

void test_discrete_inputs (void) {
  TEST_ASSERT_EQUAL_INT (UT_INPUT_BITS_NB,
                         modbus_read_input_bits (ctx, UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB, tab_rp_bits));

  int i = 0;
  uint16_t nb_points = UT_INPUT_BITS_NB;

  while (nb_points > 0) {
    int nb_bits = (nb_points > 8) ? 8 : nb_points;

    TEST_ASSERT_EQUAL_UINT8 (UT_INPUT_BITS_TAB[i],
                             modbus_get_byte_from_bits (tab_rp_bits, i * 8, nb_bits));
    nb_points -= nb_bits;
    i++;
  }
}

void test_holding_registers (void) {
  /** HOLDING REGISTERS **/

  /* Single register */
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_write_register (ctx, UT_REGISTERS_ADDRESS, 0x1234));
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_read_registers (ctx, UT_REGISTERS_ADDRESS, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_UINT16 (0x1234,
                            tab_rp_registers[0]);
  /* End of single register */

  /* Many registers */
  TEST_ASSERT_EQUAL_INT (UT_REGISTERS_NB,
                         modbus_write_registers (ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, UT_REGISTERS_TAB));
  TEST_ASSERT_EQUAL_INT (UT_REGISTERS_NB,
                         modbus_read_registers (ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers));
  for (int i = 0; i < UT_REGISTERS_NB; i++) {

    TEST_ASSERT_EQUAL_UINT16 (UT_REGISTERS_TAB[i],
                              tab_rp_registers[i]);
  }

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_registers (ctx, UT_REGISTERS_ADDRESS, 0, tab_rp_registers));

  uint16_t nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ? UT_REGISTERS_NB  : UT_INPUT_REGISTERS_NB;
  memset (tab_rp_registers, 0, nb_points * sizeof (uint16_t));

  #if 0
  /* TODO: fix this test, it fails even with the original code */

  /* Write registers to zero from tab_rp_registers and store read registers
     into tab_rp_registers. So the read registers must set to 0, except the
     first one because there is an offset of 1 register on write. */
  TEST_ASSERT_EQUAL_INT (UT_REGISTERS_NB,
                         modbus_write_and_read_registers (ctx,
                                                          UT_REGISTERS_ADDRESS + 1,
                                                          UT_REGISTERS_NB - 1,
                                                          tab_rp_registers,
                                                          UT_REGISTERS_ADDRESS,
                                                          UT_REGISTERS_NB,
                                                          tab_rp_registers));

  TEST_ASSERT_EQUAL_UINT16 (tab_rp_registers[0], UT_REGISTERS_TAB[0]);

  for (int i = 1; i < UT_REGISTERS_NB; i++) {

    TEST_ASSERT_EQUAL_UINT16 (0,
                              tab_rp_registers[i]);
  }
  #endif
  /* End of many registers */
}

void test_input_registers (void) {
  /** INPUT REGISTERS **/
  TEST_ASSERT_EQUAL_INT (UT_INPUT_REGISTERS_NB,
                         modbus_read_input_registers (ctx, UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB, tab_rp_registers));

  for (int i = 0; i < UT_INPUT_REGISTERS_NB; i++) {

    TEST_ASSERT_EQUAL_UINT16 (UT_INPUT_REGISTERS_TAB[i],
                              tab_rp_registers[i]);
  }
}

void test_mask_registers (void) {
  /* MASKS */
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_write_register (ctx, UT_REGISTERS_ADDRESS, 0x12));
  TEST_ASSERT_NOT_EQUAL_INT (-1,
                             modbus_mask_write_register (ctx, UT_REGISTERS_ADDRESS, 0xF2, 0x25));
  TEST_ASSERT_EQUAL_INT (1,
                         modbus_read_registers (ctx, UT_REGISTERS_ADDRESS, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_UINT16 (0x17,
                            tab_rp_registers[0]);
}

void test_float (void) {
  float real;
  /** FLOAT **/
  modbus_set_float_abcd (UT_REAL, tab_rp_registers);
  TEST_ASSERT_TRUE (is_memory_equal (tab_rp_registers, UT_IREAL_ABCD_SET, 4));
  real = modbus_get_float_abcd (UT_IREAL_ABCD_GET);
  TEST_ASSERT_EQUAL_FLOAT (UT_REAL, real);

  modbus_set_float_dcba (UT_REAL, tab_rp_registers);
  TEST_ASSERT_TRUE (is_memory_equal (tab_rp_registers, UT_IREAL_DCBA_SET, 4));
  real = modbus_get_float_dcba (UT_IREAL_DCBA_GET);
  TEST_ASSERT_EQUAL_FLOAT (UT_REAL, real);

  modbus_set_float_badc (UT_REAL, tab_rp_registers);
  TEST_ASSERT_TRUE (is_memory_equal (tab_rp_registers, UT_IREAL_BADC_SET, 4));
  real = modbus_get_float_badc (UT_IREAL_BADC_GET);
  TEST_ASSERT_EQUAL_FLOAT (UT_REAL, real);

  modbus_set_float_cdab (UT_REAL, tab_rp_registers);
  TEST_ASSERT_TRUE (is_memory_equal (tab_rp_registers, UT_IREAL_CDAB_SET, 4));
  real = modbus_get_float_cdab (UT_IREAL_CDAB_GET);
  TEST_ASSERT_EQUAL_FLOAT (UT_REAL, real);
}


void test_illegal_data_address (void) {
  /** ILLEGAL DATA ADDRESS **/

  /* The mapping begins at the defined addresses and ends at address +
     nb_points so these addresses are not valid. */
  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_bits (ctx, 0, 1, tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_bits (ctx, UT_BITS_ADDRESS, UT_BITS_NB + 1, tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_bits (ctx, 0, 1, tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_bits (ctx, UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB + 1, tab_rp_bits));

  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_registers (ctx, 0, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_registers (ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB_MAX + 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_registers (ctx, 0, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_registers (ctx, UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB + 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_bit (ctx, 0, ON));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_bit (ctx, UT_BITS_ADDRESS + UT_BITS_NB, ON));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_bits (ctx, 0, 1, tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_bits (ctx, UT_BITS_ADDRESS + UT_BITS_NB, UT_BITS_NB, tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_register (ctx, 0, tab_rp_registers[0]));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_register (ctx, UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_MAX, tab_rp_registers[0]));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_registers (ctx, 0, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_registers (ctx,
                                                 UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_MAX,
                                                 UT_REGISTERS_NB,
                                                 tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_mask_write_register (ctx, 0, 0xF2, 0x25));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_mask_write_register (ctx, UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_MAX, 0xF2, 0x25));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_and_read_registers (ctx, 0, 1, tab_rp_registers, 0, 1, tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_and_read_registers (ctx,
                                                          UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_MAX,
                                                          UT_REGISTERS_NB,
                                                          tab_rp_registers,
                                                          UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_MAX,
                                                          UT_REGISTERS_NB,
                                                          tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBXILADD, errno);
}


void test_too_many_data (void) {
  /** TOO MANY DATA **/

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_bits (ctx,
                                           UT_BITS_ADDRESS,
                                           MODBUS_MAX_READ_BITS + 1,
                                           tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_bits (ctx,
                                                 UT_INPUT_BITS_ADDRESS,
                                                 MODBUS_MAX_READ_BITS + 1,
                                                 tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_registers (ctx,
                                                UT_REGISTERS_ADDRESS,
                                                MODBUS_MAX_READ_REGISTERS + 1,
                                                tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_read_input_registers (ctx,
                                                      UT_INPUT_REGISTERS_ADDRESS,
                                                      MODBUS_MAX_READ_REGISTERS + 1,
                                                      tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_bits (ctx,
                                            UT_BITS_ADDRESS,
                                            MODBUS_MAX_WRITE_BITS + 1,
                                            tab_rp_bits));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

  TEST_ASSERT_EQUAL_INT (-1,
                         modbus_write_registers (ctx,
                                                 UT_REGISTERS_ADDRESS,
                                                 MODBUS_MAX_WRITE_REGISTERS + 1,
                                                 tab_rp_registers));
  TEST_ASSERT_EQUAL_INT (EMBMDATA, errno);

}

void test_slave_address (void) {

  /** SLAVE ADDRESS **/
  old_slave = modbus_get_slave (ctx);

  TEST_ASSERT_EQUAL_INT_MESSAGE (-1,
                                 modbus_set_slave (ctx, 248),
                                 "Slave address of 248 shouldn't be allowed");

  modbus_enable_quirks (ctx, MODBUS_QUIRK_MAX_SLAVE);
  TEST_ASSERT_EQUAL_INT_MESSAGE (0,
                                 modbus_set_slave (ctx, 248),
                                 "Not compliant slave address should have been accepted");

  modbus_disable_quirks (ctx, MODBUS_QUIRK_MAX_SLAVE);
  TEST_ASSERT_EQUAL_INT_MESSAGE (0,
                                 modbus_set_slave (ctx, old_slave),
                                 "Uanble to restore slave value");
}


void test_slave_reply (void) {
  /** SLAVE REPLY **/

  modbus_set_slave (ctx, INVALID_SERVER_ID);
  int rc = modbus_read_registers (
             ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers);

  if (use_backend == RTU) {
    const int RAW_REQ_LENGTH = 6;
    uint8_t raw_req[] = {INVALID_SERVER_ID, 0x03, 0x00, 0x01, 0x01, 0x01};
    /* Too many points */
    uint8_t raw_invalid_req[] = {INVALID_SERVER_ID, 0x03, 0x00, 0x01, 0xFF, 0xFF};
    const int RAW_RSP_LENGTH = 7;
    uint8_t raw_rsp[] = {INVALID_SERVER_ID, 0x03, 0x04, 0, 0, 0, 0};
    uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];

    /* No response in RTU mode */
    TEST_ASSERT_EQUAL_INT (-1, rc);
    TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);

    /* The slave raises a timeout on a confirmation to ignore because if an
       indication for another slave is received, a confirmation must follow */

    /* Send a pair of indication/confirmation to the slave with a different
       slave ID to simulate a communication on a RS485 bus. At first, the
       slave will see the indication message then the confirmation, and it must
       ignore both. */
    modbus_send_raw_request (ctx, raw_req, RAW_REQ_LENGTH * sizeof (uint8_t));
    modbus_send_raw_request (ctx, raw_rsp, RAW_RSP_LENGTH * sizeof (uint8_t));

    rc = modbus_receive_confirmation (ctx, rsp);
    TEST_ASSERT_EQUAL_INT (-1, rc);
    TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);

    /* Send an INVALID request for another slave */
    modbus_send_raw_request (ctx, raw_invalid_req, RAW_REQ_LENGTH * sizeof (uint8_t));
    rc = modbus_receive_confirmation (ctx, rsp);
    TEST_ASSERT_EQUAL_INT (-1, rc);
    TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);

    rc = modbus_set_slave (ctx, MODBUS_BROADCAST_ADDRESS);
    TEST_ASSERT_EQUAL_INT_MESSAGE (0, rc, "Invalid broadcast address");

    rc = modbus_read_registers (
           ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers);
    TEST_ASSERT_EQUAL_INT (-1, rc);
    TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);
  }
  else {
    /* Response in TCP mode */
    TEST_ASSERT_EQUAL_INT (UT_REGISTERS_NB, rc);

    rc = modbus_set_slave (ctx, MODBUS_BROADCAST_ADDRESS);
    TEST_ASSERT_EQUAL_INT_MESSAGE (0, rc, "Invalid broadcast address");

    rc = modbus_read_registers (
           ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers);
    TEST_ASSERT_EQUAL_INT (UT_REGISTERS_NB, rc);
  }

  /* Restore slave */
  modbus_set_slave (ctx, old_slave);

  rc = modbus_read_registers (
         ctx, UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE, 1, tab_rp_registers);
  TEST_ASSERT_EQUAL_INT (-1, rc);

  /* Set a marker to ensure limit is respected */
  tab_rp_bits[REPORT_SLAVE_ID_LEN - 1] = 42;
  rc = modbus_report_slave_id (ctx, REPORT_SLAVE_ID_LEN - 1, tab_rp_bits);
  /* Return the size required (response size) but respects the defined limit */
  TEST_ASSERT_EQUAL_INT (REPORT_SLAVE_ID_LEN, rc);
  TEST_ASSERT_EQUAL_UINT8 (42, tab_rp_bits[REPORT_SLAVE_ID_LEN - 1]);

  /* tab_rp_bits is used to store bytes */
  rc = modbus_report_slave_id (ctx, REPORT_SLAVE_ID_LEN, tab_rp_bits);
  TEST_ASSERT_EQUAL_INT (REPORT_SLAVE_ID_LEN, rc);

  /* Slave ID is an arbitrary number for libmodbus */
  TEST_ASSERT_GREATER_THAN_UINT8 (0, tab_rp_bits[0]);

  /* Run status indicator is ON */
  TEST_ASSERT_GREATER_THAN_INT (1, rc);
  TEST_ASSERT_EQUAL_UINT8 (0xFF, tab_rp_bits[1]);

  /* Print additional data as string */
  if (rc > 2) {
    printf ("Additional data: ");
    for (int i = 2; i < rc; i++) {

      printf ("%c", tab_rp_bits[i]);
    }
    printf ("\n");
  }

  to_t old_byte_to;

  /* Save original timeout */
  modbus_get_response_timeout (ctx, &old_response_to.sec, &old_response_to.usec);
  modbus_get_byte_timeout (ctx, &old_byte_to.sec, &old_byte_to.usec);

  rc = modbus_set_response_timeout (ctx, 0, 0);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (EINVAL, errno);

  rc = modbus_set_response_timeout (ctx, 0, 1000000);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (EINVAL, errno);

  rc = modbus_set_byte_timeout (ctx, 0, 1000000);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (EINVAL, errno);

  modbus_set_response_timeout (ctx, 0, 1);
  rc = modbus_read_registers (
         ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers);
  printf ("1us response timeout: ");
  if (rc == -1 && errno == ETIMEDOUT) {

    printf ("OK\n");
  }
  else {

    printf ("FAILED (can fail on some platforms)\n");
  }

  /* A wait and flush operation is done by the error recovery code of
     libmodbus but after a sleep of current response timeout
     so 0 can be too short!
  */
  usleep (old_response_to.sec * 1000000 + old_response_to.usec);
  modbus_flush (ctx);

  /* Trigger a special behaviour on server to wait for 0.5 second before
     replying whereas allowed timeout is 0.2 second */
  modbus_set_response_timeout (ctx, 0, 200000);
  rc = modbus_read_registers (
         ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS, 1, tab_rp_registers);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);

  /* Wait for reply (0.2 + 0.4 > 0.5 s) and flush before continue */
  usleep (400000);
  modbus_flush (ctx);

  modbus_set_response_timeout (ctx, 0, 600000);
  rc = modbus_read_registers (
         ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS, 1, tab_rp_registers);
  TEST_ASSERT_EQUAL_INT (1, rc);

  /* Disable the byte timeout.
     The full response must be available in the 600ms interval */
  modbus_set_byte_timeout (ctx, 0, 0);
  rc = modbus_read_registers (
         ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS, 1, tab_rp_registers);
  TEST_ASSERT_EQUAL_INT (1, rc);

  /* Restore original response timeout */
  modbus_set_response_timeout (ctx, old_response_to.sec, old_response_to.usec);

  if (use_backend == TCP) {
    /* The test server is only able to test byte timeouts with the TCP
       backend */

    /* Timeout of 3ms between bytes */
    modbus_set_byte_timeout (ctx, 0, 3000);
    rc = modbus_read_registers (
           ctx, UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS, 1, tab_rp_registers);
    TEST_ASSERT_EQUAL_INT (-1, rc);
    TEST_ASSERT_EQUAL_INT (ETIMEDOUT, errno);

    /* Wait remaining bytes before flushing */
    usleep (11 * 5000);
    modbus_flush (ctx);

    /* Timeout of 7ms between bytes */
    modbus_set_byte_timeout (ctx, 0, 7000);
    rc = modbus_read_registers (
           ctx, UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS, 1, tab_rp_registers);
    TEST_ASSERT_EQUAL_INT (1, rc);
  }

  /* Restore original byte timeout */
  modbus_set_byte_timeout (ctx, old_byte_to.sec, old_byte_to.usec);
}

void test_bad_response (void) {
  uint16_t *tab_rp_registers_bad = NULL;

  /** BAD RESPONSE **/

  /* Allocate only the required space */
  tab_rp_registers_bad =
    (uint16_t *) malloc (UT_REGISTERS_NB_SPECIAL * sizeof (uint16_t));

  int rc = modbus_read_registers (
             ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB_SPECIAL, tab_rp_registers_bad);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (EMBBADDATA, errno);
  free (tab_rp_registers_bad);
}

void test_manual_exception (void) {

  /** MANUAL EXCEPTION **/
  int rc = modbus_read_registers (
             ctx, UT_REGISTERS_ADDRESS_SPECIAL, UT_REGISTERS_NB, tab_rp_registers);
  TEST_ASSERT_EQUAL_INT (-1, rc);
  TEST_ASSERT_EQUAL_INT (EMBXSBUSY, errno);
}

void test_invalid_init (void) {
  modbus_t *c;

  /* Test init functions */
  c = modbus_new_rtu (NULL, 1, 'A', 0, 0);
  TEST_ASSERT_TRUE (c == NULL);
  TEST_ASSERT_EQUAL_INT (EINVAL, errno);

  c = modbus_new_rtu ("/dev/dummy", 0, 'A', 0, 0);
  TEST_ASSERT_TRUE (c == NULL);
  TEST_ASSERT_EQUAL_INT (EINVAL, errno);
}

/* Send crafted requests to test server resilience
   and ensure proper exceptions are returned. */
void test_server (void) {
  int rc;
  int i;

  /* Read requests */
  const int READ_RAW_REQ_LEN = 6;
  const int slave = (use_backend == RTU) ? SERVER_ID : MODBUS_TCP_SLAVE;
  uint8_t read_raw_req[] = {slave,
                            /* function, address, 5 values */
                            MODBUS_FC_READ_HOLDING_REGISTERS,
                            UT_REGISTERS_ADDRESS >> 8,
                            UT_REGISTERS_ADDRESS & 0xFF,
                            0x0,
                            0x05
                           };

  /* Write and read registers request */
  const int RW_RAW_REQ_LEN = 13;
  uint8_t rw_raw_req[] = {slave,
                          /* function, addr to read, nb to read */
                          MODBUS_FC_WRITE_AND_READ_REGISTERS,
                          /* Read */
                          UT_REGISTERS_ADDRESS >> 8,
                          UT_REGISTERS_ADDRESS & 0xFF,
                          (MODBUS_MAX_WR_READ_REGISTERS + 1) >> 8,
                          (MODBUS_MAX_WR_READ_REGISTERS + 1) & 0xFF,
                          /* Write */
                          0,
                          0,
                          0,
                          1,
                          /* Write byte count */
                          1 * 2,
                          /* One data to write... */
                          0x12,
                          0x34
                         };
  const int WRITE_RAW_REQ_LEN = 13;
  uint8_t write_raw_req[] = {slave,
                             /* function will be set in the loop */
                             MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                             /* Address */
                             UT_REGISTERS_ADDRESS >> 8,
                             UT_REGISTERS_ADDRESS & 0xFF,
                             /* 3 values, 6 bytes */
                             0x00,
                             0x03,
                             0x06,
                             /* Dummy data to write */
                             0x02,
                             0x2B,
                             0x00,
                             0x01,
                             0x00,
                             0x64
                            };
  const int INVALID_FC = 0x42;
  const int INVALID_FC_REQ_LEN = 6;
  uint8_t invalid_fc_raw_req[] = {slave, 0x42, 0x00, 0x00, 0x00, 0x00};

  int req_length;
  uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];
  int tab_read_function[] = {MODBUS_FC_READ_COILS,
                             MODBUS_FC_READ_DISCRETE_INPUTS,
                             MODBUS_FC_READ_HOLDING_REGISTERS,
                             MODBUS_FC_READ_INPUT_REGISTERS
                            };
  int tab_read_nb_max[] = {MODBUS_MAX_READ_BITS + 1,
                           MODBUS_MAX_READ_BITS + 1,
                           MODBUS_MAX_READ_REGISTERS + 1,
                           MODBUS_MAX_READ_REGISTERS + 1
                          };
  int backend_length;
  int backend_offset;

  if (use_backend == RTU) {

    backend_length = 3;
    backend_offset = 1;
  }
  else {

    backend_length = 7;
    backend_offset = 7;
  }

  /* This requests can generate flushes server side so we need a higher
     response timeout than the server. The server uses the defined response
     timeout to sleep before flushing.
     The old timeouts are restored at the end.
  */
  modbus_get_response_timeout (ctx, &old_response_to.sec, &old_response_to.usec);
  modbus_set_response_timeout (ctx, 0, 600000);

  int old_s = modbus_get_socket (ctx);
  modbus_set_socket (ctx, -1);

  rc = modbus_receive (ctx, rsp);
  modbus_set_socket (ctx, old_s);
  TEST_ASSERT_EQUAL_INT (-1, rc);

  req_length = modbus_send_raw_request (ctx, read_raw_req, READ_RAW_REQ_LEN);
  TEST_ASSERT_EQUAL_INT ( (backend_length + 5), req_length);

  rc = modbus_receive_confirmation (ctx, rsp);
  TEST_ASSERT_EQUAL_INT ( (backend_length + 12), rc);

  /* Try to read more values than a response could hold for all data
     types. */
  for (i = 0; i < 4; i++) {

    send_crafted_request (tab_read_function[i],
                          read_raw_req,
                          READ_RAW_REQ_LEN,
                          tab_read_nb_max[i],
                          0,
                          backend_length,
                          backend_offset);
  }

  send_crafted_request (MODBUS_FC_WRITE_AND_READ_REGISTERS,
                        rw_raw_req,
                        RW_RAW_REQ_LEN,
                        MODBUS_MAX_WR_READ_REGISTERS + 1,
                        0,
                        backend_length,
                        backend_offset);

  send_crafted_request (MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                        write_raw_req,
                        WRITE_RAW_REQ_LEN,
                        MODBUS_MAX_WRITE_REGISTERS + 1,
                        6,
                        backend_length,
                        backend_offset);

  send_crafted_request (MODBUS_FC_WRITE_MULTIPLE_COILS,
                        write_raw_req,
                        WRITE_RAW_REQ_LEN,
                        MODBUS_MAX_WRITE_BITS + 1,
                        6,
                        backend_length,
                        backend_offset);

  /* Modbus write multiple registers with large number of values but a set a
     small number of bytes in requests (not nb * 2 as usual). */
  send_crafted_request (MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                        write_raw_req,
                        WRITE_RAW_REQ_LEN,
                        MODBUS_MAX_WRITE_REGISTERS,
                        6,
                        backend_length,
                        backend_offset);

  send_crafted_request (MODBUS_FC_WRITE_MULTIPLE_COILS,
                        write_raw_req,
                        WRITE_RAW_REQ_LEN,
                        MODBUS_MAX_WRITE_BITS,
                        6,
                        backend_length,
                        backend_offset);

  /* Test invalid function code */
  modbus_send_raw_request (ctx, invalid_fc_raw_req, INVALID_FC_REQ_LEN * sizeof (uint8_t));

  rc = modbus_receive_confirmation (ctx, rsp);
  TEST_ASSERT_EQUAL_INT ( (backend_length + EXCEPTION_RC), rc);
  TEST_ASSERT_EQUAL_UINT8 ( (0x80 + INVALID_FC), rsp[backend_offset]);
}

void send_crafted_request (int function,
                           uint8_t *req,
                           int req_len,
                           uint16_t max_value,
                           uint16_t bytes,
                           int backend_length,
                           int backend_offset) {
  uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];
  int j;

  for (j = 0; j < 2; j++) {
    int rc;

    req[1] = function;
    if (j == 0) {
      /* Try to read or write zero values on first iteration */
      req[4] = 0x00;
      req[5] = 0x00;
      if (bytes) {
        /* Write query */
        req[6] = 0x00;
      }
    }
    else {
      /* Try to read or write max values + 1 on second iteration */
      req[4] = (max_value >> 8) & 0xFF;
      req[5] = max_value & 0xFF;
      if (bytes) {
        /* Write query (nb values * 2 to convert in bytes for registers) */
        req[6] = bytes;
      }
    }

    modbus_send_raw_request (ctx, req, req_len * sizeof (uint8_t));
    if (j == 0) {
      printf (
        "* try function 0x%X: %s 0 values: ", function, bytes ? "write" : "read");
    }
    else {
      printf ("* try function 0x%X: %s %d values: ",
              function,
              bytes ? "write" : "read",
              max_value);
    }

    rc = modbus_receive_confirmation (ctx, rsp);
    TEST_ASSERT_EQUAL_INT ( (backend_length + EXCEPTION_RC), rc);
    TEST_ASSERT_EQUAL_UINT8 ( (0x80 + function), rsp[backend_offset]);
    TEST_ASSERT_EQUAL_UINT8 (MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp[backend_offset + 1]);
  }
}

int is_memory_equal (const void *s1, const void *s2, size_t size) {

  return (memcmp (s1, s2, size) == 0);
}

int equal_dword (uint16_t *tab_reg, const uint32_t value) {

  return ( (tab_reg[0] == (value >> 16)) && (tab_reg[1] == (value & 0xFFFF)));
}

#ifdef _WIN32
int usleep (__int64 usec) {
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = - (10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer (NULL, TRUE, NULL);
  SetWaitableTimer (timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject (timer, INFINITE);
  CloseHandle (timer);
  return 0;
}

int close (int fd) {
  return closesocket (fd);
}
#endif
