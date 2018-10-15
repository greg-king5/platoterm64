/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 *
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.c - Input/output functions (serial/ethernet) (c64-specific)
 */

#include <cbm.h>
#if 0
#include <peekpoke.h>
#endif
#include <stdint.h>
#include <string.h>
#include <serial.h>
#include "../io.h"
#include "../config.h"

extern uint8_t io_load_successful;
//extern uint8_t (*io_serial_buffer_size)(void);
extern void (*io_recv_serial_flow_off)(void);
extern void (*io_recv_serial_flow_on)(void);

static void io_recv_serial_flow_off_user_port(void);
static void io_recv_serial_flow_on_user_port(void);
//static uint8_t io_serial_buffer_size_user_port(void);
static void io_recv_serial_flow_off_swiftlink(void);
static void io_recv_serial_flow_on_swiftlink(void);
//static uint8_t io_serial_buffer_size_swiftlink(void);

/**
 * io_init_funcptrs() - Set up I/O function pointers
 */
void io_init_funcptrs(void)
{
  if (strcmp(config.driver_ser,CONFIG_SERIAL_DRIVER_UP2400)==0)
    {
//      io_serial_buffer_size=io_serial_buffer_size_user_port;
      io_recv_serial_flow_off=io_recv_serial_flow_off_user_port;
      io_recv_serial_flow_on=io_recv_serial_flow_on_user_port;
    }
  else if (strcmp(config.driver_ser,CONFIG_SERIAL_DRIVER_SWIFTLINK)==0)
    {
//      io_serial_buffer_size=io_serial_buffer_size_swiftlink;
      io_recv_serial_flow_off=io_recv_serial_flow_off_swiftlink;
      io_recv_serial_flow_on=io_recv_serial_flow_on_swiftlink;
    }
}

/**
 * io_send_byte(b) - Send specified byte out
 */
void io_send_byte(uint8_t b)
{
  if (io_load_successful)
    {
      ser_put(b);
    }
}

/********* USER PORT *****************************/

#if 0
/**
 * Return the serial buffer size
 */
uint8_t io_serial_buffer_size_user_port(void)
{
  return PEEK(0x29B)-PEEK(0x29C);
}
#endif

/**
 * io_recv_serial_flow_off() - Tell modem to stop receiving.
 */
void io_recv_serial_flow_off_user_port(void)
{
  // for now, assume user port.
  CIA2.prb &= ~0x02;
}

/**
 * io_recv_serial_flow_on() - Tell modem to start receiving.
 */
void io_recv_serial_flow_on_user_port(void)
{
  // For now, assume user port.
  CIA2.prb |= 0x02;
}

/************** SWIFTLINK ***********************/

#if 0
/**
 * Return the serial buffer size
 */
uint8_t io_serial_buffer_size_swiftlink(void)
{
  return PEEK(0xF9)-PEEK(0xF8);
}
#endif

/**
 * io_recv_serial_flow_off() - Tell modem to stop receiving.
 */
void io_recv_serial_flow_off_swiftlink(void)
{
  io_send_byte(XOFF);
}

/**
 * io_recv_serial_flow_on() - Tell modem to start receiving.
 */
void io_recv_serial_flow_on_swiftlink(void)
{
  io_send_byte(XON);
}
