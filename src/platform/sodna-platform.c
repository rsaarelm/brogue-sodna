#ifdef BROGUE_SODNA
#include "sodna.h"
#include "platform.h"
#include <time.h>

extern playerCharacter rogue;

static void gameLoop(){
    sodna_init(8, 8, COLS, ROWS, "Brogue");
    rogueMain();
    sodna_exit();
}

static sodna_Event stored_event;
static boolean ctrl_pressed = 0;
static boolean shift_pressed = 0;
static int old_mouse_x = 0;
static int old_mouse_y = 0;
static int mouse_x = 0;
static int mouse_y = 0;
static int last_mouse_timestamp = 0;
#define MOUSE_SAMPLE_DELAY 36

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
    sodna_save_screenshot(buf);
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

    if (e.type == SODNA_EVENT_KEY_UP) {
        switch (e.key.layout) {
            case SODNA_KEY_LEFT_CONTROL:
            case SODNA_KEY_RIGHT_CONTROL:
                ctrl_pressed = 0;
                break;
            case SODNA_KEY_LEFT_SHIFT:
            case SODNA_KEY_RIGHT_SHIFT:
                shift_pressed = 0;
                break;
        }
    }

    if (e.type == SODNA_EVENT_KEY_DOWN) {
        switch (e.key.layout) {
            case SODNA_KEY_LEFT_CONTROL:
            case SODNA_KEY_RIGHT_CONTROL:
                ctrl_pressed = 1;
                break;
            case SODNA_KEY_LEFT_SHIFT:
            case SODNA_KEY_RIGHT_SHIFT:
                shift_pressed = 1;
                break;
            case SODNA_KEY_PRINT_SCREEN:
                screenshot();
        }
    }

    return e;
}

static boolean sodna_pauseForMilliseconds(short milliseconds) {
    sodna_flush();
    sodna_sleep_ms(milliseconds);
    sodna_Event e = get_event(false);
    return e.type == SODNA_EVENT_KEY_DOWN ||
        e.type == SODNA_EVENT_CHARACTER ||
        e.type == SODNA_EVENT_MOUSE_DOWN;
}

static void sodna_nextKeyOrMouseEvent(
        rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
retry:
    sodna_flush();
    sodna_Event e = get_event(true);
    if (colorsDance) {
        shuffleTerrainColors(3, true);
        commitDraws();
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

    if (e.type == SODNA_EVENT_MOUSE_MOVED &&
            sodna_ms_elapsed() - last_mouse_timestamp > MOUSE_SAMPLE_DELAY) {
        last_mouse_timestamp = sodna_ms_elapsed();
        if (mouse_pos_changed()) {
            returnEvent->eventType = MOUSE_ENTERED_CELL;
            returnEvent->param1 = mouse_x;
            returnEvent->param2 = mouse_y;
            return;
        }
    }

    if (e.type & 0x7f == SODNA_EVENT_MOUSE_DOWN) {
        returnEvent->param1 = mouse_x;
        returnEvent->param2 = mouse_y;
        if (e.button.id == SODNA_LEFT_BUTTON) {
	    returnEvent->eventType =
                (e.type == SODNA_EVENT_MOUSE_UP ? MOUSE_UP : MOUSE_DOWN);
            return;
        }
        if (e.button.id == SODNA_RIGHT_BUTTON) {
	    returnEvent->eventType =
                (e.type == SODNA_EVENT_MOUSE_UP ? RIGHT_MOUSE_UP : RIGHT_MOUSE_DOWN);
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

    goto retry;
}

static void sodna_plotChar(
        uchar ch, short xLoc, short yLoc,
        short foreRed, short foreGreen, short foreBlue,
        short backRed, short backGreen, short backBlue) {
    sodna_Cell c;
#ifdef USE_UNICODE
    // because we can't look at unicode and ascii without messing with Rogue.h, reinterpret until some later version comes along:
    switch (ch) {
        case FLOOR_CHAR: ch = '.'; break;
        case CHASM_CHAR: ch = ':'; break;
        case TRAP_CHAR: ch = '%'; break;
        case FIRE_CHAR: ch = '^'; break;
        case FOLIAGE_CHAR: ch = '&'; break;
        case AMULET_CHAR: ch = ','; break;
        case SCROLL_CHAR: ch = '?'; break;
        case RING_CHAR: ch = '='; break;
        case WEAPON_CHAR: ch = '('; break;
        case GEM_CHAR: ch = '+'; break;
        case TOTEM_CHAR: ch = '0'; break;
        case BAD_MAGIC_CHAR: ch = '+'; break;
        case GOOD_MAGIC_CHAR: ch = '$'; break;

        case DOWN_ARROW_CHAR: ch = 'v'; break;
        case LEFT_ARROW_CHAR: ch = '<'; break;
        case RIGHT_ARROW_CHAR: ch = '>'; break;

        case UP_TRIANGLE_CHAR: ch = '^'; break;
        case DOWN_TRIANGLE_CHAR: ch = 'v'; break;

        case CHARM_CHAR: ch = '7'; break;

        case OMEGA_CHAR: ch = '<'; break;
        case THETA_CHAR: ch = '0'; break;
        case LAMDA_CHAR: ch = '^'; break;
        case KOPPA_CHAR: ch = '0'; break;

        case LOZENGE_CHAR: ch = 'o'; break;
        case CROSS_PRODUCT_CHAR: ch = 'x'; break;

        case STATUE_CHAR: ch = '5'; break;
        case UNICORN_CHAR: ch = 'U'; break;
    }
#endif
    c.symbol = ch % 256;

    c.fore_r = foreRed * 15 / 100;
    c.fore_g = foreGreen * 15 / 100;
    c.fore_b = foreBlue * 15 / 100;
    c.back_r = backRed * 15 / 100;
    c.back_g = backGreen * 15 / 100;
    c.back_b = backBlue * 15 / 100;
    if (xLoc >= 0 && yLoc >= 0 && xLoc < COLS && yLoc < ROWS)
        sodna_cells()[xLoc + COLS * yLoc] = c;
}

static void sodna_remap(const char *input_name, const char *output_name) {
    // TODO
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
