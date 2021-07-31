#include "keys.h"

const char* keys_keyname(u16 scancode) {
    switch (scancode) {
        case KEY_ESC: return "Esc";

        case KEY_ONE: return "One";
        case KEY_TWO: return "Two";
        case KEY_THREE: return "Three";
        case KEY_FOUR: return "Four";
        case KEY_FIVE: return "Five";
        case KEY_SIX: return "Six";
        case KEY_SEVEN: return "Seven";
        case KEY_EIGHT: return "Eight";
        case KEY_NINE: return "Nine";
        case KEY_ZERO: return "Zero";
        case KEY_MINUS: return "Minus";
        case KEY_EQUAL: return "Equal";
        case KEY_BACKSPACE: return "Backspace";

        case KEY_TAB: return "Tab";
        case KEY_Q: return "KeyQ";
        case KEY_W: return "KeyW";
        case KEY_E: return "KeyE";
        case KEY_R: return "KeyR";
        case KEY_T: return "KeyT";
        case KEY_Y: return "KeyY";
        case KEY_U: return "KeyU";
        case KEY_I: return "KeyI";
        case KEY_O: return "KeyO";
        case KEY_P: return "KeyP";
        case KEY_LBRACKET: return "LeftBracket";
        case KEY_RBRACKET: return "RightBracket";
        case KEY_ENTER: return "Enter";

        case KEY_LCTRL: return "LCtrl";

        case KEY_A: return "KeyA";
        case KEY_S: return "KeyS";
        case KEY_D: return "KeyD";
        case KEY_F: return "KeyF";
        case KEY_G: return "KeyG";
        case KEY_H: return "KeyH";
        case KEY_J: return "KeyJ";
        case KEY_K: return "KeyK";
        case KEY_L: return "KeyL";
        case KEY_SEMICOLON: return "Semicolon";
        case KEY_BSLASH: return "Backslash";
        case KEY_QUOTE: return "Quote";

        case KEY_LSHIFT: return "LShift";
        case KEY_INTLBSLASH: return "IntlBackslash";
        case KEY_Z: return "KeyZ";
        case KEY_X: return "KeyX";
        case KEY_C: return "KeyC";
        case KEY_V: return "KeyV";
        case KEY_B: return "KeyB";
        case KEY_N: return "KeyN";
        case KEY_M: return "KeyM";
        case KEY_COMMA: return "Comma";
        case KEY_PERIOD: return "Period";
        case KEY_SLASH: return "Slash";
        case KEY_RSHIFT: return "RShift";

        case KEY_LALT: return "LAlt";
        case KEY_SPACE: return "Space";

        case KEY_F1: return "F1";
        case KEY_F2: return "F2";
        case KEY_F3: return "F3";
        case KEY_F4: return "F4";
        case KEY_F5: return "F5";
        case KEY_F6: return "F6";
        case KEY_F7: return "F7";
        case KEY_F8: return "F8";
        case KEY_F9: return "F9";
        case KEY_F10: return "F10";

        case KEY_PAD_SEVEN: return "KeypadSeven";
        case KEY_PAD_EIGHT: return "KeypadEight";
        case KEY_PAD_NINE: return "KeypadNine";
        case KEY_PAD_MINUS: return "KeypadMinus";
        case KEY_PAD_FOUR: return "KeypadFour";
        case KEY_PAD_FIVE: return "KeypadFive";
        case KEY_PAD_SIX: return "KeypadSix";
        case KEY_PAD_PLUS: return "KeypadPlus";
        case KEY_PAD_ONE: return "KeypadOne";
        case KEY_PAD_TWO: return "KeypadTwo";
        case KEY_PAD_THREE: return "KeypadThree";
        case KEY_PAD_ZERO: return "KeypadZero";
        case KEY_PAD_PERIOD: return "KeypadPeriod";

        case KEY_PAD_F11: return "F11";
        case KEY_PAD_F12: return "F12";

        case KEY_RCTRL: return "RCtrl";

        case KEY_RALT: return "RAlt";

        case KEY_HOME: return "Home";
        case KEY_PAGEUP: return "PageUp";

        case KEY_LEFT: return "Left";
        case KEY_UP: return "Up";
        case KEY_RIGHT: return "Right";

        case KEY_END: return "End";
        case KEY_DOWN: return "Down";
        case KEY_PAGEDOWN: return "PageDown";

        case KEY_INSERT: return "Insert";
        case KEY_DELETE: return "Delete";

        case KEY_LMETA: return "LMeta";
        case KEY_RMETA: return "RMeta";
        case KEY_CONTEXT: return "Context";

        default: return 0;
    }
}