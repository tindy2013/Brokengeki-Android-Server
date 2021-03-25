#include <windows.h>
#include <xinput.h>

#include <limits.h>
#include <stdint.h>

#include "mu3io/mu3io.h"
#include "mu3io/config.h"

static uint8_t mu3_opbtn;
static uint8_t mu3_left_btn;
static uint8_t mu3_right_btn;
static int16_t mu3_lever_pos;
// static int16_t mu3_lever_xpos;
static int16_t width;

static struct mu3_io_config mu3_io_cfg;

struct IPCMemoryInfo
{
    uint8_t buttons[15];
    uint8_t lever[4];
    uint8_t coinInsertion;
    uint8_t cardRead;
    uint8_t leds[18];
};
typedef struct IPCMemoryInfo IPCMemoryInfo;
static HANDLE FileMappingHandle;
IPCMemoryInfo* FileMapping;

void initSharedMemory()
{
    if (FileMapping)
    {
        return;
    }
    if ((FileMappingHandle = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(IPCMemoryInfo), "Local\\BROKENGEKI_SHARED_BUFFER")) == 0)
    {
        return;
    }

    if ((FileMapping = (IPCMemoryInfo*)MapViewOfFile(FileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(IPCMemoryInfo))) == 0)
    {
        return;
    }

    memset(FileMapping, 0, sizeof(IPCMemoryInfo));
    SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
}

HRESULT mu3_io_init(void)
{
    mu3_lever_pos = 0;
    width = GetSystemMetrics(SM_CXSCREEN) - 1;
    mu3_io_config_load(&mu3_io_cfg, L".\\segatools.ini");
    initSharedMemory();
    return S_OK;
}

HRESULT mu3_io_poll(void)
{
    int lever;
    // int xlever;
    mu3_opbtn = 0;
    mu3_left_btn = 0;
    mu3_right_btn = 0;

    //功能键
    if ((GetAsyncKeyState(mu3_io_cfg.vk_test) & 0x8000) || (FileMapping && FileMapping->buttons[0]))
    {
        mu3_opbtn |= MU3_IO_OPBTN_TEST;
    }

    if ((GetAsyncKeyState(mu3_io_cfg.vk_service) & 0x8000) || (FileMapping && FileMapping->buttons[1]))
    {
        mu3_opbtn |= MU3_IO_OPBTN_SERVICE;
    }
	
	// use brokengeki shared memory to pass coin insertion
	if ((GetAsyncKeyState(mu3_io_cfg.vk_coin) & 0x8000) && FileMapping)
    {
        FileMapping->coinInsertion = 1;
    }

    lever = mu3_lever_pos;

    if (mu3_io_cfg.input_mode == 1)
    {
        XINPUT_STATE xi;
        WORD xb;
        memset(&xi, 0, sizeof(xi));
        XInputGetState(0, &xi);
        xb = xi.Gamepad.wButtons;

        //手柄检测部分
        if (xb & XINPUT_GAMEPAD_DPAD_LEFT)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_1;
        }

        if (xb & XINPUT_GAMEPAD_DPAD_UP)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_2;
        }

        if (xb & XINPUT_GAMEPAD_DPAD_RIGHT)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_3;
        }

        if (xb & XINPUT_GAMEPAD_X)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_1;
        }

        if (xb & XINPUT_GAMEPAD_Y)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_2;
        }

        if (xb & XINPUT_GAMEPAD_B)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_3;
        }

        if (xb & XINPUT_GAMEPAD_BACK)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_MENU;
        }

        if (xb & XINPUT_GAMEPAD_START)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_MENU;
        }

        if (xb & XINPUT_GAMEPAD_LEFT_SHOULDER)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_SIDE;
        }

        if (xb & XINPUT_GAMEPAD_RIGHT_SHOULDER)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_SIDE;
        }
        //手柄检测部分
        lever = xi.Gamepad.sThumbLX * 1.7;
    }
    else if (mu3_io_cfg.input_mode == 2)
    {
        //键盘检测部分
        if (GetAsyncKeyState(mu3_io_cfg.vk_la) & 0x8000)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_1;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_lb) & 0x8000)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_2;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_lc) & 0x8000)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_3;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_ra) & 0x8000)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_1;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_rb) & 0x8000)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_2;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_rc) & 0x8000)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_3;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_lm) & 0x8000)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_MENU;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_rm) & 0x8000)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_MENU;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_ls) & 0x8000)
        {
            mu3_left_btn |= MU3_IO_GAMEBTN_SIDE;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_rs) & 0x8000)
        {
            mu3_right_btn |= MU3_IO_GAMEBTN_SIDE;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_sliderLeft) & 0x8000)
        {
            lever -= mu3_io_cfg.sliderSpeed;
        }

        if (GetAsyncKeyState(mu3_io_cfg.vk_sliderRight) & 0x8000)
        {
            lever += mu3_io_cfg.sliderSpeed;
        }
        
        if (mu3_io_cfg.mouse == 1)
        {
            POINT ptB;
            GetCursorPos(&ptB);
            lever = ptB.x * 65535 / width + INT16_MIN;
        }
    }
    else if (mu3_io_cfg.input_mode == 3)
    {
        // brokengeki
		if (FileMapping && FileMapping->buttons[5])
		{
			mu3_left_btn |= MU3_IO_GAMEBTN_1;
		}

		if (FileMapping && FileMapping->buttons[6])
		{
			mu3_left_btn |= MU3_IO_GAMEBTN_2;
		}

		if (FileMapping && FileMapping->buttons[7])
		{
			mu3_left_btn |= MU3_IO_GAMEBTN_3;
		}

		if (FileMapping && FileMapping->buttons[8])
		{
			mu3_right_btn |= MU3_IO_GAMEBTN_1;
		}

		if (FileMapping && FileMapping->buttons[9])
		{
			mu3_right_btn |= MU3_IO_GAMEBTN_2;
		}

		if (FileMapping && FileMapping->buttons[10])
		{
			mu3_right_btn |= MU3_IO_GAMEBTN_3;
		}

		if (FileMapping && FileMapping->buttons[13])
		{
			mu3_left_btn |= MU3_IO_GAMEBTN_MENU;
		}

		if (FileMapping && FileMapping->buttons[12])
		{
			mu3_right_btn |= MU3_IO_GAMEBTN_MENU;
		}

		if (FileMapping && FileMapping->buttons[4])
		{
			mu3_left_btn |= MU3_IO_GAMEBTN_SIDE;
		}

		if (FileMapping && FileMapping->buttons[11])
		{
			mu3_right_btn |= MU3_IO_GAMEBTN_SIDE;
		}
		
		float flever = *((float*)FileMapping->lever);
		lever = flever * INT16_MAX;
    }

    // if (abs(xi.Gamepad.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    // {
    //     lever += xi.Gamepad.sThumbLX / 24;
    // }

    // if (abs(xi.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    // {
    //     lever += xi.Gamepad.sThumbRX / 24;
    // }
    // if (mu3_io_cfg.input_mode == 2)
    // {
    //     lever = xi.Gamepad.sThumbLX;
    // }

    if (lever < INT16_MIN)
    {
        lever = INT16_MIN;
    }

    if (lever > INT16_MAX)
    {
        lever = INT16_MAX;
    }

    // mu3_lever_pos = lever;

    // xlever = mu3_lever_pos - xi.Gamepad.bLeftTrigger * 64 + xi.Gamepad.bRightTrigger * 64;

    // if (xlever < INT16_MIN)
    // {
    //     xlever = INT16_MIN;
    // }

    // if (xlever > INT16_MAX)
    // {
    //     xlever = INT16_MAX;
    // }

    mu3_lever_pos = lever;
    // mu3_lever_xpos = xlever;

    return S_OK;
}

void mu3_io_get_opbtns(uint8_t *opbtn)
{
    if (opbtn != NULL)
    {
        *opbtn = mu3_opbtn;
    }
}

void mu3_io_get_gamebtns(uint8_t *left, uint8_t *right)
{
    if (left != NULL)
    {
        *left = mu3_left_btn;
    }

    if (right != NULL)
    {
        *right = mu3_right_btn;
    }
}

void mu3_io_get_lever(int16_t *pos)
{
    if (pos != NULL)
    {
        *pos = mu3_lever_pos;
    }
}


void mu3_io_set_leds(const uint8_t *leds)
{
	if (FileMapping) {
		memcpy(FileMapping->leds, leds, 18);
	}
}
