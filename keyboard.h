#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum {
	KEY_NONE = 0,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ENTER,
	KEY_ESC,
	KEY_SPACE,
	KEY_OTHER
} Key;

// Discard any bytes the controller is still holding from before boot.
void kbd_init(void);

// Returns key just pressed or KEY_NONE.
// // WASD is mapped to the arrow keys. Key releases are ignored.
Key kbd_poll(void);

#endif
