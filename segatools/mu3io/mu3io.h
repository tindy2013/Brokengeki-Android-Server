#pragma once

#include <windows.h>

#include <stdint.h>

enum {
    MU3_IO_OPBTN_TEST = 0x01,
    MU3_IO_OPBTN_SERVICE = 0x02,
	MU3_IO_OPBTN_COIN = 0x04,
};

enum {
    MU3_IO_GAMEBTN_1 = 0x01,
    MU3_IO_GAMEBTN_2 = 0x02,
    MU3_IO_GAMEBTN_3 = 0x04,
    MU3_IO_GAMEBTN_SIDE = 0x08,
    MU3_IO_GAMEBTN_MENU = 0x10,
};

HRESULT mu3_io_init(void);

HRESULT mu3_io_poll(void);

void mu3_io_get_opbtns(uint8_t *opbtn);

void mu3_io_get_gamebtns(uint8_t *left, uint8_t *right);

void mu3_io_get_lever(int16_t *pos);

void mu3_io_set_leds(const uint8_t *leds);
