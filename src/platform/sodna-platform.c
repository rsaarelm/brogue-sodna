#ifdef BROGUE_SODNA
#include "sodna.h"
#include "platform.h"
#include "sodna_util.h"
#include <time.h>
#include <string.h>
#include <ctype.h>

extern playerCharacter rogue;

static void gameLoop(){
    sodna_init(8, 14, COLS, ROWS, "Brogue");
    rogueMain();
    sodna_exit();
}

// Switch the case of input characters when caps lock is pressed.
// Currently (2014-11-24) deactivated since a bug in SDL2, which sodna uses as
// the backend, makes caps lock state detection unreliable.
#define CAPS_LOCK_PROTECTION 0

static sodna_Event stored_event;
static boolean ctrl_pressed = 0;
static boolean shift_pressed = 0;
static boolean caps_lock = 0;
static boolean is_fullscreen_mode = 0;
static int old_mouse_x = 0;
static int old_mouse_y = 0;
static int mouse_x = 0;
static int mouse_y = 0;
#define PAUSE_BETWEEN_EVENT_POLLING 36

// Mappings from unprintable keys to unprintable or printable (negative value)
// indices. Zero means no mapping.
static int unprintable_mappings[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// Mappings from printable keys to printable or unprintable (negative value)
// indices. Zero means no mapping.
static int printable_mappings[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static struct { const char* name; int sodna_id; } sodna_key_names[] = {
    { "ESCAPE", SODNA_KEY_ESCAPE },
    { "BACKSPACE", SODNA_KEY_BACKSPACE },
    { "TAB", SODNA_KEY_TAB },
    { "ENTER", SODNA_KEY_ENTER },
    { "PAGEUP", SODNA_KEY_PAGE_UP },
    { "PAGEDOWN", SODNA_KEY_PAGE_DOWN },
    { "END", SODNA_KEY_END },
    { "HOME", SODNA_KEY_HOME },
    { "UP", SODNA_KEY_UP },
    { "LEFT", SODNA_KEY_LEFT },
    { "RIGHT", SODNA_KEY_RIGHT },
    { "DOWN", SODNA_KEY_DOWN },
    { "PRINTSCREEN", SODNA_KEY_PRINT_SCREEN },
    { "PAUSE", SODNA_KEY_PAUSE },
    { "INSERT", SODNA_KEY_INSERT },
    { "DELETE", SODNA_KEY_DELETE },
    { "KP0", SODNA_KEY_KP_0 },
    { "KP1", SODNA_KEY_KP_1 },
    { "KP2", SODNA_KEY_KP_2 },
    { "KP3", SODNA_KEY_KP_3 },
    { "KP4", SODNA_KEY_KP_4 },
    { "KP5", SODNA_KEY_KP_5 },
    { "KP6", SODNA_KEY_KP_6 },
    { "KP7", SODNA_KEY_KP_7 },
    { "KP8", SODNA_KEY_KP_8 },
    { "KP9", SODNA_KEY_KP_9 },
    { "KPENTER", SODNA_KEY_KP_ENTER },
    { "KPDEC", SODNA_KEY_KP_DECIMAL },
    { "KPDIV", SODNA_KEY_KP_DIVIDE },
    { "KPMUL", SODNA_KEY_KP_MULTIPLY },
    { "KPSUB", SODNA_KEY_KP_MINUS },
    { "KPADD", SODNA_KEY_KP_PLUS },
    { "F1", SODNA_KEY_F1 },
    { "F2", SODNA_KEY_F2 },
    { "F3", SODNA_KEY_F3 },
    { "F4", SODNA_KEY_F4 },
    { "F5", SODNA_KEY_F5 },
    { "F6", SODNA_KEY_F6 },
    { "F7", SODNA_KEY_F7 },
    { "F8", SODNA_KEY_F8 },
    { "F9", SODNA_KEY_F9 },
    { "F10", SODNA_KEY_F10 },
    { "F11", SODNA_KEY_F11 },
    { "F12", SODNA_KEY_F11 },
    { NULL, 0 }
};

static boolean mouse_pos_changed() {
    if (old_mouse_x != mouse_x || old_mouse_y != mouse_y) {
        old_mouse_x = mouse_x;
        old_mouse_y = mouse_y;
        return true;
    }
    return false;
}

static void screenshot() {
    // TODO: timestamp filename.
    static char buf[256];
    snprintf(buf, sizeof(buf), "brogue-%u.png", (unsigned)time(NULL));
    sodna_save_screenshot_png(buf);
}

// Use event to update input system state before returning it.
static sodna_Event get_event(boolean consume) {
    sodna_Event e;
    // Consume means the event is expected to be used as input,
    // !consume means that it's being polled for breaking a sleep
    // and should be saved up for processing later.
    if (consume) {
        if (stored_event.type != SODNA_EVENT_NONE) {
            e = stored_event;
            memset(&stored_event, 0, sizeof(sodna_Event));
        } else {
            e = sodna_wait_event(100);
        }
    } else {
        e = sodna_poll_event();
        stored_event = e;
    }

    if (e.type == SODNA_EVENT_MOUSE_MOVED) {
        mouse_x = e.mouse.x;
        mouse_y = e.mouse.y;
    }

    if (e.type == SODNA_EVENT_KEY_DOWN) {
        // PrintScreen to save a screenshot.
        if (e.key.layout == SODNA_KEY_PRINT_SCREEN) {
            screenshot();
        }

        // Alt-enter to toggle fullscreen mode.
        if (e.key.layout == SODNA_KEY_ENTER && e.key.alt) {
            is_fullscreen_mode = !is_fullscreen_mode;
            sodna_set_fullscreen(is_fullscreen_mode);
        }
    }

    if (e.type == SODNA_EVENT_KEY_UP || e.type == SODNA_EVENT_KEY_DOWN) {
        ctrl_pressed = e.key.ctrl;
        shift_pressed = e.key.shift;
        caps_lock = e.key.caps_lock;
    }

    return e;
}

static boolean breaks_pause(sodna_Event e) {
    return e.type == SODNA_EVENT_KEY_DOWN ||
        e.type == SODNA_EVENT_CHARACTER ||
        e.type == SODNA_EVENT_MOUSE_DOWN ||
        e.type == SODNA_EVENT_MOUSE_UP;
}

static boolean sodna_pauseForMilliseconds(short milliseconds) {
    sodna_flush();
    sodna_sleep_ms(milliseconds);

    sodna_Event e;
    do {
        e = get_event(false);
    } while (!breaks_pause(e) && e.type != SODNA_EVENT_NONE);

    return e.type != SODNA_EVENT_NONE;
}

static void sodna_nextKeyOrMouseEvent(
        rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    int time, waitTime;
    boolean mouseMoved = false;
    for (;;) {
        time = sodna_ms_elapsed();
        if (colorsDance) {
            shuffleTerrainColors(3, true);
            commitDraws();
        }
        sodna_flush();

        sodna_Event e = get_event(true);

        // We get lots of mouse events, loop to flush out several consecutive
        // ones. Do break at some point though.
        if (e.type == SODNA_EVENT_MOUSE_MOVED) {
            for (;;) {
                if (mouse_pos_changed())
                    mouseMoved = true;
                // Peek at the future event, try to stop this loop at the last
                // mouse moved event of the sequence.
                if (get_event(false).type != SODNA_EVENT_MOUSE_MOVED)
                    break;
                if (sodna_ms_elapsed() - time > PAUSE_BETWEEN_EVENT_POLLING)
                    break;
                e = get_event(true);
            }
        }

        returnEvent->controlKey = ctrl_pressed;
        returnEvent->shiftKey = shift_pressed;

        if (e.type == SODNA_EVENT_CLOSE_WINDOW) {
            rogue.gameHasEnded = true;
            rogue.nextGame = NG_QUIT; // causes the menu to drop out immediately
            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = ESCAPE_KEY;
            return;
        }


        if (e.type == SODNA_EVENT_MOUSE_DOWN) {
            returnEvent->param1 = mouse_x;
            returnEvent->param2 = mouse_y;
            if (e.button.id == SODNA_LEFT_BUTTON) {
                returnEvent->eventType = MOUSE_DOWN;
                return;
            }
            if (e.button.id == SODNA_RIGHT_BUTTON) {
                returnEvent->eventType = RIGHT_MOUSE_DOWN;
                return;
            }
        }

        if (e.type == SODNA_EVENT_MOUSE_UP) {
            returnEvent->param1 = mouse_x;
            returnEvent->param2 = mouse_y;
            if (e.button.id == SODNA_LEFT_BUTTON) {
                returnEvent->eventType = MOUSE_UP;
                return;
            }
            if (e.button.id == SODNA_RIGHT_BUTTON) {
                returnEvent->eventType = RIGHT_MOUSE_UP;
                return;
            }
        }

        // Reverse the effect of caps lock if it's on.
        if (e.type == SODNA_EVENT_CHARACTER) {
            // XXX: Due to a bug in SDL2, used as backend by Sodna, caps lock
            // state isn't maintained robustly. So we can't rely on the
            // modifier. Look at shift state and the character case instead.
            //if (caps_lock) {
            if ((isupper(e.ch.code) && !shift_pressed) || (islower(e.ch.code) && shift_pressed)) {
                if (isupper(e.ch.code)) {
                    e.ch.code = tolower(e.ch.code);
                } else if (islower(e.ch.code)) {
                    e.ch.code = toupper(e.ch.code);
                }
            }
        }

        // Keymap translation.
        if (e.type == SODNA_EVENT_CHARACTER && e.ch.code < 128) {
            int km = printable_mappings[e.ch.code];
            if (km > 0) {
                e.ch.code = km;
            } else if (km < 0) {
                e.type = SODNA_EVENT_KEY_DOWN;
                e.key.layout = -km;
            }
        } else if (e.type == SODNA_EVENT_KEY_DOWN && e.key.layout < 128) {
            int km = unprintable_mappings[e.key.layout];
            if (km > 0) {
                e.key.layout = km;
            } else if (km < 0) {
                e.type = SODNA_EVENT_CHARACTER;
                e.ch.code = -km;
            }
        }

        // Back to event handling.
        if (e.type == SODNA_EVENT_CHARACTER) {
            returnEvent->param1 = e.ch.code;
            returnEvent->eventType = KEYSTROKE;
            return;
        }

        if (e.type == SODNA_EVENT_KEY_DOWN) {
            returnEvent->eventType = KEYSTROKE;

            switch (e.key.layout) {
#define K(sodna, brogue) case sodna: returnEvent->param1 = brogue; return;
                K(SODNA_KEY_UP, UP_ARROW)
                K(SODNA_KEY_DOWN, DOWN_ARROW)
                K(SODNA_KEY_LEFT, LEFT_ARROW)
                K(SODNA_KEY_RIGHT, RIGHT_ARROW)
                K(SODNA_KEY_HOME, UPLEFT_KEY)
                K(SODNA_KEY_END, DOWNLEFT_KEY)
                K(SODNA_KEY_PAGE_UP, UPRIGHT_KEY)
                K(SODNA_KEY_PAGE_DOWN, DOWNRIGHT_KEY)
                K(SODNA_KEY_KP_1, NUMPAD_1)
                K(SODNA_KEY_KP_2, NUMPAD_2)
                K(SODNA_KEY_KP_3, NUMPAD_3)
                K(SODNA_KEY_KP_4, NUMPAD_4)
                K(SODNA_KEY_KP_5, NUMPAD_5)
                K(SODNA_KEY_KP_6, NUMPAD_6)
                K(SODNA_KEY_KP_7, NUMPAD_7)
                K(SODNA_KEY_KP_8, NUMPAD_8)
                K(SODNA_KEY_KP_9, NUMPAD_9)
                K(SODNA_KEY_KP_0, NUMPAD_0)
                K(SODNA_KEY_TAB, TAB_KEY)
                K(SODNA_KEY_ENTER, RETURN_KEY)
                K(SODNA_KEY_KP_ENTER, ENTER_KEY)
                K(SODNA_KEY_BACKSPACE, DELETE_KEY)
                K(SODNA_KEY_ESCAPE, ESCAPE_KEY)
#undef K
                    break;
            }
        }

        if (mouseMoved) {
            returnEvent->eventType = MOUSE_ENTERED_CELL;
            returnEvent->param1 = mouse_x;
            returnEvent->param2 = mouse_y;
            return;
        }

        waitTime = time + PAUSE_BETWEEN_EVENT_POLLING - sodna_ms_elapsed();
        if (waitTime > 0 && waitTime <= PAUSE_BETWEEN_EVENT_POLLING) {
            sodna_sleep_ms(waitTime);
        }
    }
}

static void sodna_plotChar(
        uchar ch, short xLoc, short yLoc,
        short foreRed, short foreGreen, short foreBlue,
        short backRed, short backGreen, short backBlue) {
    sodna_Cell c;
    if (ch > 128) {
        switch (ch) {
#ifdef USE_UNICODE
            case FLOOR_CHAR: ch = 128 + 0; break;
            case CHASM_CHAR: ch = 128 + 1; break;
            case TRAP_CHAR: ch = 128 + 2; break;
            case FIRE_CHAR: ch = 128 + 3; break;
            case FOLIAGE_CHAR: ch = 128 + 4; break;
            case AMULET_CHAR: ch = 128 + 5; break;
            case SCROLL_CHAR: ch = 128 + 6; break;
            case RING_CHAR: ch = 128 + 7; break;
            case WEAPON_CHAR: ch = 128 + 8; break;
            case GEM_CHAR: ch = 128 + 9; break;
            case TOTEM_CHAR: ch = 128 + 10; break;
            case BAD_MAGIC_CHAR: ch = 128 + 12; break;
            case GOOD_MAGIC_CHAR: ch = 128 + 13; break;

            case DOWN_ARROW_CHAR: ch = 144 + 1; break;
            case LEFT_ARROW_CHAR: ch = 144 + 2; break;
            case RIGHT_ARROW_CHAR: ch = 144 + 3; break;
            case UP_TRIANGLE_CHAR: ch = 144 + 4; break;
            case DOWN_TRIANGLE_CHAR: ch = 144 + 5; break;
            case OMEGA_CHAR: ch = 144 + 6; break;
            case THETA_CHAR: ch = 144 + 7; break;
            case LAMDA_CHAR: ch = 144 + 8; break;
            case KOPPA_CHAR: ch = 144 + 9; break; // is this right?
            case CHARM_CHAR: ch = 144 + 9; break;
            case LOZENGE_CHAR: ch = 144 + 10; break;
            case CROSS_PRODUCT_CHAR: ch = 144 + 11; break;

            case UNICORN_CHAR: ch = 160 + 3; break;

            case STATUE_CHAR: ch = 224 + 1; break;
#endif
            default: ch = '?'; break;
        }
    }
    c.symbol = ch;

    c.fore.r = foreRed * 255 / 100;
    c.fore.g = foreGreen * 255 / 100;
    c.fore.b = foreBlue * 255 / 100;
    c.back.r = backRed * 255 / 100;
    c.back.g = backGreen * 255 / 100;
    c.back.b = backBlue * 255 / 100;
    if (xLoc >= 0 && yLoc >= 0 && xLoc < COLS && yLoc < ROWS)
        sodna_cells()[xLoc + COLS * yLoc] = c;
}

// out_is_printable: True if the input is a printable character and goes to the printable mappings table.
// out_index: The index in either the printable or nonprintable mappings table.
// return: True if a mapping was found, false if not.
static boolean resolve_input_name(boolean* out_is_printable, int* out_index, const char *input_name) {
    if (strlen(input_name) == 1 && input_name[0] >= '!' && input_name[0] <= '~') {
        // Printable ASCII.
        *out_is_printable = true;
        *out_index = input_name[0];
        return true;
    }

    // Special case for SPACE, you can't distinguish the char so it has to be named.
    if (strcmp(input_name, "SPACE") == 0) {
        *out_is_printable = true;
        *out_index = ' ';
        return true;
    }

    int i = 0;
    while (sodna_key_names[i].name) {
        if (strcmp(input_name, sodna_key_names[i].name) == 0) {
            *out_is_printable = false;
            *out_index = sodna_key_names[i].sodna_id;
            return true;
        }
        i++;
    }
    return false;
}

static void sodna_remap(const char *input_name, const char *output_name) {
    int in_idx;
    boolean in_printable;

    int out_idx;
    boolean out_printable;

    if (!resolve_input_name(&in_printable, &in_idx, input_name)) return;
    if (!resolve_input_name(&out_printable, &out_idx, output_name)) return;

    // Use negative numbers to signal table switch.
    out_idx *= (in_printable == out_printable ? 1 : -1);

    if (in_printable) printable_mappings[in_idx] = out_idx;
    else unprintable_mappings[in_idx] = out_idx;
}

static boolean modifier_held(int modifier) {
    if (modifier == 0)
        return shift_pressed;
    if (modifier == 1)
        return ctrl_pressed;
    return 0;
}

struct brogueConsole sodnaConsole = {
    gameLoop,
    sodna_pauseForMilliseconds,
    sodna_nextKeyOrMouseEvent,
    sodna_plotChar,
    sodna_remap,
    modifier_held
};

#endif
