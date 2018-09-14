/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 *
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.c - Input/output functions (serial/ethernet)
 */

#include <stdbool.h>
#include <serial.h>
#include <stdint.h>
#include <stdlib.h>
#include "io.h"
#include "protocol.h"
#include "config.h"
#include "prefs.h"

#define NULL 0

uint8_t xoff_enabled;	// XXX -- remove
uint8_t io_load_successful=false;

uint8_t (*io_serial_buffer_size)(void);
void (*io_recv_serial_flow_off)(void);
void (*io_recv_serial_flow_on)(void);

static uint8_t ch=0;
static uint8_t io_res;
static uint8_t flow_stopped;		// (boolean)
#if defined(__C64__)
static uint8_t recv_buffer[1024];	// the size must be a power of two
#else
static uint8_t recv_buffer[384];
#endif
static uint16_t recv_buffer_head;	// new characters go in at this index
static uint16_t recv_buffer_tail;	// old characters come out at this index
static uint16_t recv_buffer_free;	// amount of available space

static struct ser_params params = {
  SER_BAUD_38400,
  SER_BITS_8,
  SER_STOP_1,
  SER_PAR_NONE,
  SER_HS_HW
};

/**
 * io_init() - Set-up the I/O
 */
void io_init(void)
{
  prefs_display("loading serial driver...");
  config_init_hook(); // Do any special configuration initiation.

  io_res=ser_load_driver(config.driver_ser);
  io_load_successful = (io_res==SER_ERR_OK);

  if (io_load_successful)
    {
      io_init_funcptrs();
      io_open();
      prefs_clear();
    }
  else
    {
      prefs_display("error: could not load serial driver.");
      prefs_select("");
    }
}

/**
 * io_open() - Open the device
 */
void io_open(void)
{
  if (config.io_mode == IO_MODE_SERIAL)
    {
      prefs_display("openning serial port...");
      params.baudrate = config.baud;

      io_res=ser_open(&params);
      if (io_res!=SER_ERR_OK)
	{
	  io_load_successful=false;
	  prefs_display("error: could not open serial port.");
	  prefs_select("");
	}
      else
	{
	  // Enable up2400 early.  Ignored by swlink.
	  //ser_ioctl(1, NULL);

	  flow_stopped = false;
	  recv_buffer_head = recv_buffer_tail = 0;
	  recv_buffer_free = sizeof recv_buffer;
	}
    }
  else if (config.io_mode == IO_MODE_ETHERNET)
    {
      // Not implemented, yet.
    }
}

/**
 * io_recv_serial() - Receive a serial character.
 *
 * Return "true" if a character is available.
 */
static uint8_t io_recv_serial(void)
{
  // Drain primary serial FIFO as fast as possible.
  while (ser_get(&ch)==SER_ERR_OK)
    {
      recv_buffer[recv_buffer_head] = ch;
      recv_buffer_head = ++recv_buffer_head % sizeof recv_buffer;
      if (--recv_buffer_free == 0)
	{
	  // The second buffer is full.
	  break;
	}
    }

  if (flow_stopped == false)
    {
      if (recv_buffer_free < config.xoff_threshold)
	{
	  io_recv_serial_flow_off();
	  flow_stopped = true;
	}
    }
  else
    {
      if (recv_buffer_free >= config.xon_threshold)
	{
	  io_recv_serial_flow_on();
	  flow_stopped = false;
	}
    }

  if (recv_buffer_free == sizeof recv_buffer)
    {
      return false;
    }
  ch = recv_buffer[recv_buffer_tail];
  recv_buffer_tail = ++recv_buffer_tail % sizeof recv_buffer;
  ++recv_buffer_free;
  return true;
}

/**
 * io_main() - The I/O main loop
 */
void io_main(void)
{
  if (io_load_successful)
    {
      if (io_recv_serial())
	{
	  ShowPLATO(&ch, 1);
	}
    }
}

/**
 * io_done() - Called to close I/O
 */
void io_done(void)
{
  if (io_load_successful)
    {
      ser_close();
      ser_unload();
    }
}
