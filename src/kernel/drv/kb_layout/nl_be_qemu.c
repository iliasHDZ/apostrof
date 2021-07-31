#include "nl_be_qemu.h"
#include "../keys.h"

void send_char(char c);

u8 lshift = 0;
u8 rshift = 0;

u8 lalt   = 0;
u8 ralt   = 0;

u8 lctrl  = 0;
u8 rctrl  = 0;

u8 caps   = 0;

u8 set_key_state(u16 scancode, u8 state) {
    switch (scancode) {
        case KEY_LSHIFT:   lshift = state; return 1;
        case KEY_RSHIFT:   rshift = state; return 1;
        case KEY_LALT:     lalt   = state; return 1;
        case KEY_RALT:     ralt   = state; return 1;
        case KEY_LCTRL:    lctrl  = state; return 1;
        case KEY_RCTRL:    rctrl  = state; return 1;

        case KEY_CAPSLOCK:
            if (state) caps = !caps;
            return 1;
    }

    return 0;
}

u8 shift_down() {
    return lshift || rshift;
}

u8 alt_down() {
    return lalt || ralt;
}

u8 ctrl_down() {
    return lctrl || rctrl;
}

void nl_be_qemu_keydown(u16 scancode) {
    if (!set_key_state(scancode, 1)) {
        u8 shift = shift_down();
        u8 alt   = alt_down();
        u8 ctrl  = ctrl_down();

        if (!shift && !alt && !ctrl && !caps) {
            switch (scancode) {
                case KEY_ONE: send_char('&'); break;
                //case KEY_TWO: send_char('é'); break;
                case KEY_THREE: send_char('"'); break;
                case KEY_FOUR: send_char('\''); break;
                case KEY_FIVE: send_char('('); break;
                //case KEY_SIX: send_char('§'); break;
                //case KEY_SEVEN: send_char('è'); break;
                case KEY_EIGHT: send_char('!'); break;
                //case KEY_NINE: send_char('ç'); break;
                //case KEY_ZERO: send_char('à'); break;

                case KEY_LBRACKET: send_char(')'); break;
                case KEY_MINUS: send_char('-'); break;

                case KEY_Q: send_char('q'); break;
                case KEY_W: send_char('w'); break;
                case KEY_E: send_char('e'); break;
                case KEY_R: send_char('r'); break;
                case KEY_T: send_char('t'); break;
                case KEY_Y: send_char('y'); break;
                case KEY_U: send_char('u'); break;
                case KEY_I: send_char('i'); break;
                case KEY_O: send_char('o'); break;
                case KEY_P: send_char('p'); break;
                
                case KEY_RBRACKET: send_char('^'); break;
                case KEY_SEMICOLON: send_char('$'); break;

                case KEY_A: send_char('a'); break;
                case KEY_S: send_char('s'); break;
                case KEY_D: send_char('d'); break;
                case KEY_F: send_char('f'); break;
                case KEY_G: send_char('g'); break;
                case KEY_H: send_char('h'); break;
                case KEY_J: send_char('j'); break;
                case KEY_K: send_char('k'); break;
                case KEY_L: send_char('l'); break;
                
                //case KEY_QUOTE: send_char('ù'); break;
                //case KEY_INTLBSLASH: send_char('µ'); break;

                case KEY_Z: send_char('z'); break;
                case KEY_X: send_char('x'); break;
                case KEY_C: send_char('c'); break;
                case KEY_V: send_char('v'); break;
                case KEY_B: send_char('b'); break;
                case KEY_N: send_char('n'); break;
                case KEY_M: send_char('m'); break;
                
                case KEY_COMMA: send_char(','); break;
                case KEY_PERIOD: send_char(';'); break;
                case KEY_SLASH: send_char(':'); break;
                case KEY_EQUAL: send_char('='); break;
                
                case KEY_SPACE: send_char(' '); break;
            }
            return;
        }

        if ((shift && !alt && !ctrl) || (!shift && caps)) {
            switch (scancode) {
                case KEY_ONE: send_char('1'); break;
                case KEY_TWO: send_char('2'); break;
                case KEY_THREE: send_char('3'); break;
                case KEY_FOUR: send_char('4'); break;
                case KEY_FIVE: send_char('5'); break;
                case KEY_SIX: send_char('6'); break;
                case KEY_SEVEN: send_char('7'); break;
                case KEY_EIGHT: send_char('8'); break;
                case KEY_NINE: send_char('9'); break;
                case KEY_ZERO: send_char('0'); break;

                //case KEY_LBRACKET: send_char('°'); break;
                case KEY_MINUS: send_char('_'); break;

                case KEY_Q: send_char('Q'); break;
                case KEY_W: send_char('W'); break;
                case KEY_E: send_char('E'); break;
                case KEY_R: send_char('R'); break;
                case KEY_T: send_char('T'); break;
                case KEY_Y: send_char('Y'); break;
                case KEY_U: send_char('U'); break;
                case KEY_I: send_char('I'); break;
                case KEY_O: send_char('O'); break;
                case KEY_P: send_char('P'); break;
                
                //case KEY_RBRACKET: send_char('¨'); break;
                case KEY_SEMICOLON: send_char('*'); break;

                case KEY_A: send_char('A'); break;
                case KEY_S: send_char('S'); break;
                case KEY_D: send_char('D'); break;
                case KEY_F: send_char('F'); break;
                case KEY_G: send_char('G'); break;
                case KEY_H: send_char('H'); break;
                case KEY_J: send_char('J'); break;
                case KEY_K: send_char('K'); break;
                case KEY_L: send_char('L'); break;
                
                case KEY_QUOTE: send_char('%'); break;
                //case KEY_INTLBSLASH: send_char('£'); break;

                case KEY_Z: send_char('Z'); break;
                case KEY_X: send_char('X'); break;
                case KEY_C: send_char('C'); break;
                case KEY_V: send_char('V'); break;
                case KEY_B: send_char('B'); break;
                case KEY_N: send_char('N'); break;
                case KEY_M: send_char('M'); break;
                
                case KEY_COMMA: send_char('?'); break;
                case KEY_PERIOD: send_char('.'); break;
                case KEY_SLASH: send_char('/'); break;
                case KEY_EQUAL: send_char('+'); break;
                
                case KEY_SPACE: send_char(' '); break;
            }
        }
    }
}

void nl_be_qemu_keyup(u16 scancode) {
    set_key_state(scancode, 0);
}