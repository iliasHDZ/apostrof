#pragma once

#include "../utils.h"

#define KEY_ESC       0x0001

#define KEY_ONE       0x0002
#define KEY_TWO       0x0003
#define KEY_THREE     0x0004
#define KEY_FOUR      0x0005
#define KEY_FIVE      0x0006
#define KEY_SIX       0x0007
#define KEY_SEVEN     0x0008
#define KEY_EIGHT     0x0009
#define KEY_NINE      0x000A
#define KEY_ZERO      0x000B
#define KEY_MINUS     0x000C
#define KEY_EQUAL     0x000D
#define KEY_BACKSPACE 0x000E

#define KEY_TAB      0x000F
#define KEY_Q        0x0010
#define KEY_W        0x0011
#define KEY_E        0x0012
#define KEY_R        0x0013
#define KEY_T        0x0014
#define KEY_Y        0x0015
#define KEY_U        0x0016
#define KEY_I        0x0017
#define KEY_O        0x0018
#define KEY_P        0x0019
#define KEY_LBRACKET 0x001A
#define KEY_RBRACKET 0x001B
#define KEY_ENTER    0x001C

#define KEY_LCTRL 0x001D

#define KEY_A         0x001E
#define KEY_S         0x001F
#define KEY_D         0x0020
#define KEY_F         0x0021
#define KEY_G         0x0022
#define KEY_H         0x0023
#define KEY_J         0x0024
#define KEY_K         0x0025
#define KEY_L         0x0026
#define KEY_SEMICOLON 0x0027
#define KEY_BSLASH    0x0028
#define KEY_QUOTE     0x0029

#define KEY_LSHIFT     0x002A
#define KEY_INTLBSLASH 0x002B
#define KEY_Z          0x002C
#define KEY_X          0x002D
#define KEY_C          0x002E
#define KEY_V          0x002F
#define KEY_B          0x0030
#define KEY_N          0x0031
#define KEY_M          0x0032
#define KEY_COMMA      0x0033
#define KEY_PERIOD     0x0034
#define KEY_SLASH      0x0035
#define KEY_RSHIFT     0x0036

#define KEY_LALT     0x0038
#define KEY_SPACE    0x0039

#define KEY_CAPSLOCK 0x003A

#define KEY_F1  0x003B
#define KEY_F2  0x003C
#define KEY_F3  0x003D
#define KEY_F4  0x003E
#define KEY_F5  0x003F
#define KEY_F6  0x0040
#define KEY_F7  0x0041
#define KEY_F8  0x0042
#define KEY_F9  0x0043
#define KEY_F10 0x0044

#define KEY_PAD_SEVEN  0x0047
#define KEY_PAD_EIGHT  0x0048
#define KEY_PAD_NINE   0x0049
#define KEY_PAD_MINUS  0x004A
#define KEY_PAD_FOUR   0x004B
#define KEY_PAD_FIVE   0x004C
#define KEY_PAD_SIX    0x004D
#define KEY_PAD_PLUS   0x004E
#define KEY_PAD_ONE    0x004F
#define KEY_PAD_TWO    0x0050
#define KEY_PAD_THREE  0x0051
#define KEY_PAD_ZERO   0x0052
#define KEY_PAD_PERIOD 0x0053

#define KEY_PAD_F11    0x0057
#define KEY_PAD_F12    0x0058

#define KEY_RCTRL    0xE01D

#define KEY_RALT     0xE038

#define KEY_HOME     0xE047
#define KEY_UP       0xE048
#define KEY_PAGEUP   0xE049

#define KEY_LEFT     0xE04B
#define KEY_RIGHT    0xE04D

#define KEY_END      0xE04F
#define KEY_DOWN     0xE050
#define KEY_PAGEDOWN 0xE051

#define KEY_INSERT   0xE052
#define KEY_DELETE   0xE053

#define KEY_LMETA    0xE05B
#define KEY_RMETA    0xE05C
#define KEY_CONTEXT  0xE05D

const char* keys_keyname(u16 scancode);