#include "keyboard.h"
#include "ports.h"

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

#define STATUS_OUT_FULL 0x01 // A byte is waiting in the data port
#define STATUS_AUX_DATA 0x20 // byte that came from mouse not keyboard

void kbd_init(void) {
	// drain stale bytes (bounded, so a stuck controller cannot hang us)
	for(int i = 0; i < 32 && (inb(KBD_STATUS_PORT) & STATUS_OUT_FULL); i++){
		inb(KBD_DATA_PORT);
	}
}

Key kbd_poll(void) {
	static int extended = 0; // last byte was the 0xE0 prefix
	uint8_t status = inb(KBD_STATUS_PORT);
	if(!(status & STATUS_OUT_FULL)) return KEY_NONE;

	uint8_t sc = inb(KBD_DATA_PORT);
	if(status & STATUS_AUX_DATA) return KEY_NONE;

	if (sc == 0xE0) {
		extended = 1;
		return KEY_NONE;
	}

	int ext = extended;
	extended = 0;

	if (sc & 0x80) return KEY_NONE; // bit 7 set = key release
	if (sc == 0x00 || sc == 0xFA || sc == 0xFE) return KEY_NONE; // error / ACK / resend

	if(ext){
		switch(sc){
			case 0x48: return KEY_UP;
			case 0x50: return KEY_DOWN;
			case 0x4B: return KEY_LEFT;
			case 0x4D: return KEY_RIGHT;
			case 0x1C: return KEY_ENTER;
			default: return KEY_OTHER;
		}
	}
	switch (sc) {
		case 0x11: return KEY_UP; 	// W
		case 0x1F: return KEY_DOWN; 	// S
		case 0x1E: return KEY_LEFT; 	// A
		case 0x20: return KEY_RIGHT;    // D
		case 0x1C: return KEY_ENTER;
		case 0x01: return KEY_ESC;
		case 0x39: return KEY_SPACE;
		default: return KEY_OTHER;
	}

}
