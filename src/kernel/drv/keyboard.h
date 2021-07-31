#pragma once

#include "../utils.h"

typedef void(*KEY_HANDLER)(u16);
typedef void(*CHAR_HANDLER)(char);

void keyboard_init();

void keyboard_keydown(KEY_HANDLER keydown);

void keyboard_keyup(KEY_HANDLER keyup);

void keyboard_char(CHAR_HANDLER handler);