#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "board/io4.h"

#include "mu3io/mu3io.h"

#include "util/dprintf.h"

static HRESULT mu3_io4_poll(void *ctx, struct io4_state *state);
static HRESULT mu3_io4_gpout(void *ctx, const uint8_t *payload);

static const struct io4_ops mu3_io4_ops = {
    .poll = mu3_io4_poll,
	.gpout = mu3_io4_gpout,
};

HRESULT mu3_io4_hook_init(void)
{
    HRESULT hr;

    hr = io4_hook_init(&mu3_io4_ops, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    return mu3_io_init();
}

static HRESULT mu3_io4_poll(void *ctx, struct io4_state *state)
{
    uint8_t opbtn;
    uint8_t left;
    uint8_t right;
    int16_t lever;
    HRESULT hr;

    memset(state, 0, sizeof(*state));

    hr = mu3_io_poll();

    if (FAILED(hr)) {
        return hr;
    }

    opbtn = 0;
    left = 0;
    right = 0;
    lever = 0;

    mu3_io_get_opbtns(&opbtn);
    mu3_io_get_gamebtns(&left, &right);
    mu3_io_get_lever(&lever);

    if (opbtn & MU3_IO_OPBTN_TEST) {
        state->buttons[0] |= IO4_BUTTON_TEST;
    }

    if (opbtn & MU3_IO_OPBTN_SERVICE) {
        state->buttons[0] |= IO4_BUTTON_SERVICE;
    }

    if (left & MU3_IO_GAMEBTN_1) {
        state->buttons[0] |= 1 << 0;
    }

    if (left & MU3_IO_GAMEBTN_2) {
        state->buttons[0] |= 1 << 5;
    }

    if (left & MU3_IO_GAMEBTN_3) {
        state->buttons[0] |= 1 << 4;
    }

    if (right & MU3_IO_GAMEBTN_1) {
        state->buttons[0] |= 1 << 1;
    }

    if (right & MU3_IO_GAMEBTN_2) {
        state->buttons[1] |= 1 << 0;
    }

    if (right & MU3_IO_GAMEBTN_3) {
        state->buttons[0] |= 1 << 15;
    }

    if (left & MU3_IO_GAMEBTN_MENU) {
        state->buttons[1] |= 1 << 14;
    }

    if (right & MU3_IO_GAMEBTN_MENU) {
        state->buttons[0] |= 1 << 13;
    }

    if (!(left & MU3_IO_GAMEBTN_SIDE)) {
        state->buttons[1] |= 1 << 15;   /* L-Side, active-low */
    }

    if (!(right & MU3_IO_GAMEBTN_SIDE)) {
        state->buttons[0] |= 1 << 14;   /* R-Side, active-low */
    }

    /* Lever increases right-to-left, not left-to-right.

       Use 0x7FFF as the center point instead of 0x8000; the latter would
       overflow when the lever pos is INT16_MIN. */

    state->adcs[0] = 0x7FFF - lever;

    return S_OK;
}

/* Ongeki only has 6 buttons with mutable light color: Left1, Left2, Left3, Right1, Right2, Right3,
   each button only has 1 bit to represent R/G/B.

   Here we use the same RGB output order as Chunitum: B, R, G */
static uint8_t mu3_led_mapping[18] = {1, 0, 3, 5, 4, 2, 8, 6, 7, 11, 9, 10, 14, 12, 13, 17, 15, 16};

static HRESULT mu3_io4_gpout(void *ctx, const uint8_t *payload)
{
	uint32_t leds;
	// flip payload bytes
	((uint8_t*)&leds)[0] = payload[3];
	((uint8_t*)&leds)[1] = payload[2];
	((uint8_t*)&leds)[2] = payload[1];
	((uint8_t*)&leds)[3] = payload[0];
	uint8_t outleds[18];
	for (uint8_t i = 0; i < 18; i++) {
		outleds[i] = ((leds >> (31 - mu3_led_mapping[i])) & 1) ? 0xff : 0x00;
	}
	
	mu3_io_set_leds(outleds);
	return S_OK;
}
