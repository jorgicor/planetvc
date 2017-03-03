/*
Copyright (c) 2014, 2015, 2016 Jorge Giner Cordero

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kernel.h"
#include "kernel_snd.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <SDL.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	/* Needed only for openUrl() */
	#include <windows.h>
#endif

#if PP_ANDROID
	/* Needed only for openUrl() */
	#include <jni.h>
#endif

#define GAMECONTROLLERDB_TXT "gamecontrollerdb.txt"

#if !defined(PP_DATADIR)
	#define PP_DATADIR "."
#endif

#if !defined(PP_PROG_DATADIR)
	#define PP_PROG_DATADIR "."
#endif

enum {

#if PP_PHONE_MODE
	FINGERS_ON = 1,
	CONTROLLERS_ON = 0,
#else
	FINGERS_ON = 0,
	CONTROLLERS_ON = 1,
#endif

};

#define TRACE_CONTROLLER	0
#define TRACE_CONTROLLER_AXIS	0

#define KERNEL_JOYSTICK_SUPPORT	0
#define TRACE_JOYSTICK		0
#define TRACE_JOYSTICK_AXIS	0

#define MAX_FRAME_EVENTS	16

static int key_down(int key_scan_code);
static int key_first_pressed(int key_scan_code);

static int s_running;
static SDL_Surface *s_backbuf;
static SDL_Texture *s_backtex;
static SDL_Renderer *s_renderer;
static SDL_Window *s_win;
static struct kernel_canvas s_kcanvas;
static void *s_data;
static char *s_data_path;
static char *s_prog_data_path;
static int s_fullscreen;

static void (*on_frame)(void *data);
static void (*on_sound)(void *data, unsigned char *ptr, int nsamples);

static unsigned char s_key_first_pressed[KERNEL_NKEYS];
static unsigned char s_key_down[KERNEL_NKEYS];

/* Only pcontroller or pjoystick can be non-NULL.  */
struct pad {
	SDL_GameController *pcontroller;
	SDL_Joystick *pjoystick;
	SDL_JoystickID joyid;
	int axis[KERNEL_NAXIS];
};

static struct pad s_pads[KERNEL_NPADS];

static struct kernel_finger s_fingers[KERNEL_NFINGERS];

static SDL_LogOutputFunction s_sdl_log_func;

static SDL_LogOutputFunction get_sdl_log_func(void)
{
	if (s_sdl_log_func == NULL) {
		SDL_LogGetOutputFunction(&s_sdl_log_func, NULL);
	}
	return s_sdl_log_func;
}

static void set_sdl_log_func(SDL_LogOutputFunction pf)
{
	/* Store the original SDL function */
	get_sdl_log_func();
	/* Set ours */
	SDL_LogSetOutputFunction(pf, NULL);
}
	
static void restore_sdl_log_func(void)
{
	if (s_sdl_log_func != NULL) {
		SDL_LogSetOutputFunction(s_sdl_log_func, NULL);
		s_sdl_log_func = NULL;
	}
}

/* This function is called by the SDL to log.
 * kassert can call console_tracev: there we use the original SDL function
 * to avoid an infinite recursion.
 */
static void log_from_sdl(void *userdata, int category, SDL_LogPriority prio,
	const char *message)
{
	ktrace(message);
}

static void trace(const char *msg)
{
	if (get_sdl_log_func() == NULL)
		return;
	
	get_sdl_log_func()(NULL, SDL_LOG_CATEGORY_APPLICATION,
			   SDL_LOG_PRIORITY_INFO, msg);
}

static int translate_sdl_scancode(SDL_Scancode scancode)
{
	int kc;

	switch (scancode) {
	case SDL_SCANCODE_0: kc = KERNEL_KSC_0; break;
	case SDL_SCANCODE_1: kc = KERNEL_KSC_1; break;
	case SDL_SCANCODE_2: kc = KERNEL_KSC_2; break;
	case SDL_SCANCODE_3: kc = KERNEL_KSC_3; break;
	case SDL_SCANCODE_4: kc = KERNEL_KSC_4; break;
	case SDL_SCANCODE_5: kc = KERNEL_KSC_5; break;
	case SDL_SCANCODE_6: kc = KERNEL_KSC_6; break;
	case SDL_SCANCODE_7: kc = KERNEL_KSC_7; break;
	case SDL_SCANCODE_8: kc = KERNEL_KSC_8; break;
	case SDL_SCANCODE_9: kc = KERNEL_KSC_9; break;
	case SDL_SCANCODE_A: kc = KERNEL_KSC_A; break;
	case SDL_SCANCODE_B: kc = KERNEL_KSC_B; break;
	case SDL_SCANCODE_C: kc = KERNEL_KSC_C; break;
	case SDL_SCANCODE_D: kc = KERNEL_KSC_D; break;
	case SDL_SCANCODE_E: kc = KERNEL_KSC_E; break;
	case SDL_SCANCODE_F: kc = KERNEL_KSC_F; break;
	case SDL_SCANCODE_G: kc = KERNEL_KSC_G; break;
	case SDL_SCANCODE_H: kc = KERNEL_KSC_H; break;
	case SDL_SCANCODE_I: kc = KERNEL_KSC_I; break;
	case SDL_SCANCODE_J: kc = KERNEL_KSC_J; break;
	case SDL_SCANCODE_K: kc = KERNEL_KSC_K; break;
	case SDL_SCANCODE_L: kc = KERNEL_KSC_L; break;
	case SDL_SCANCODE_M: kc = KERNEL_KSC_M; break;
	case SDL_SCANCODE_N: kc = KERNEL_KSC_N; break;
	case SDL_SCANCODE_O: kc = KERNEL_KSC_O; break;
	case SDL_SCANCODE_P: kc = KERNEL_KSC_P; break;
	case SDL_SCANCODE_Q: kc = KERNEL_KSC_Q; break;
	case SDL_SCANCODE_R: kc = KERNEL_KSC_R; break;
	case SDL_SCANCODE_S: kc = KERNEL_KSC_S; break;
	case SDL_SCANCODE_T: kc = KERNEL_KSC_T; break;
	case SDL_SCANCODE_U: kc = KERNEL_KSC_U; break;
	case SDL_SCANCODE_V: kc = KERNEL_KSC_V; break;
	case SDL_SCANCODE_W: kc = KERNEL_KSC_W; break;
	case SDL_SCANCODE_X: kc = KERNEL_KSC_X; break;
	case SDL_SCANCODE_Y: kc = KERNEL_KSC_Y; break;
	case SDL_SCANCODE_Z: kc = KERNEL_KSC_Z; break;
	case SDL_SCANCODE_UP: kc = KERNEL_KSC_UP; break;
	case SDL_SCANCODE_DOWN: kc = KERNEL_KSC_DOWN; break;
	case SDL_SCANCODE_LEFT: kc = KERNEL_KSC_LEFT; break;
	case SDL_SCANCODE_RIGHT: kc = KERNEL_KSC_RIGHT; break;
	case SDL_SCANCODE_LSHIFT: kc = KERNEL_KSC_LSHIFT; break;
	case SDL_SCANCODE_RSHIFT: kc = KERNEL_KSC_RSHIFT; break;
	case SDL_SCANCODE_LALT: kc = KERNEL_KSC_LALT; break;
	case SDL_SCANCODE_LCTRL: kc = KERNEL_KSC_LCTRL; break;
	case SDL_SCANCODE_RCTRL: kc = KERNEL_KSC_RCTRL; break;
	case SDL_SCANCODE_SPACE: kc = KERNEL_KSC_SPACE; break;
	case SDL_SCANCODE_RETURN: kc = KERNEL_KSC_RETURN; break;
	case SDL_SCANCODE_BACKSPACE: kc = KERNEL_KSC_BACKSPACE; break;
	/* Map android back button to ESC */
	case SDL_SCANCODE_AC_BACK: kc = KERNEL_KSC_ESC; break;
	/**/
	case SDL_SCANCODE_ESCAPE: kc = KERNEL_KSC_ESC; break;
	default: kc = -1;
	}

	return kc;
}

static int is_valid_pad(int i)
{
	return s_pads[i].pcontroller != NULL ||
		s_pads[i].pjoystick != NULL;
}

static int get_controller_pad_index(SDL_JoystickID joyid)
{
	int i;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (s_pads[i].pcontroller != NULL &&
			s_pads[i].joyid == joyid)
		{
			return i;
		}
	}

	return -1;
}

static int translate_sdl_controller_button(SDL_JoystickID joyid,
					   SDL_GameControllerButton b)
{
	int kc;
	int ipad;

	ipad = get_controller_pad_index(joyid);
	if (ipad < 0)
		return -1;

	switch (b) {
	case SDL_CONTROLLER_BUTTON_A: kc = KERNEL_KSC_PAD0_A; break;
	case SDL_CONTROLLER_BUTTON_B: kc = KERNEL_KSC_PAD0_B; break;
	case SDL_CONTROLLER_BUTTON_X: kc = KERNEL_KSC_PAD0_X; break;
	case SDL_CONTROLLER_BUTTON_Y: kc = KERNEL_KSC_PAD0_Y; break;
	case SDL_CONTROLLER_BUTTON_BACK: kc = KERNEL_KSC_PAD0_BACK; break;
	case SDL_CONTROLLER_BUTTON_GUIDE: kc = KERNEL_KSC_PAD0_GUIDE; break;
	case SDL_CONTROLLER_BUTTON_START: kc = KERNEL_KSC_PAD0_START; break;
	case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		kc = KERNEL_KSC_PAD0_LSTICK;
	       	break;
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
	       	kc = KERNEL_KSC_PAD0_RSTICK;
	       	break;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
	       	kc = KERNEL_KSC_PAD0_LSHOULDER;
	       	break;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
	       	kc = KERNEL_KSC_PAD0_RSHOULDER;
	       	break;
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
	       	kc = KERNEL_KSC_PAD0_DUP;
	       	break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
	       	kc = KERNEL_KSC_PAD0_DDOWN;
	       	break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
	       	kc = KERNEL_KSC_PAD0_DLEFT;
	       	break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
	       	kc = KERNEL_KSC_PAD0_DRIGHT;
	       	break;
	default: kc = -1;
	}

	if (kc >= 0) {
		kc += ipad * KERNEL_NPAD_BUTTONS;
	}

	return kc;
}

#if KERNEL_JOYSTICK_SUPPORT
static int get_joystick_pad_index(SDL_JoystickID joyid)
{
	int i;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (s_pads[i].pjoystick != NULL &&
			s_pads[i].joyid == joyid)
		{
			return i;
		}
	}

	return -1;
}

static int translate_sdl_joystick_button(SDL_JoystickID joyid,
					 unsigned int b)
{
	int kc;
	int ipad;

	ipad = get_joystick_pad_index(joyid);
	if (ipad < 0)
		return -1;

	switch (b) {
	case 2: kc = KERNEL_KSC_PAD0_A; break;
	case 1: kc = KERNEL_KSC_PAD0_B; break;
	case 3: kc = KERNEL_KSC_PAD0_X; break;
	case 0: kc = KERNEL_KSC_PAD0_Y; break;
	case 8: kc = KERNEL_KSC_PAD0_BACK; break;
	case 16: kc = KERNEL_KSC_PAD0_GUIDE; break;
	case 9: kc = KERNEL_KSC_PAD0_START; break;
	case 10: kc = KERNEL_KSC_PAD0_LSTICK; break;
	case 11: kc = KERNEL_KSC_PAD0_RSTICK; break;
	case 6: kc = KERNEL_KSC_PAD0_LSHOULDER; break;
	case 7: kc = KERNEL_KSC_PAD0_RSHOULDER; break;
	default: kc = -1;
	}

	if (kc >= 0) {
		kc += ipad * KERNEL_NPAD_BUTTONS;
	}

	return kc;
}
#endif

static void clean_first_pressed_keys(void)
{
	memset(s_key_first_pressed, 0, sizeof(s_key_first_pressed));
}

static void clean_key_states(void)
{
	memset(s_key_down, 0, sizeof(s_key_down));

#if 0
	if (FINGERS_ON) {
		for (i = 0; i < KERNEL_NFINGERS; i++) {
			if (s_fingers[i].valid) {
				s_fingers[i].released = 1;
			}
		}
	}
#endif
}

static void handle_keydown(const SDL_KeyboardEvent *ev)
{
	int ksc;

	ksc = translate_sdl_scancode(ev->keysym.scancode);
	if (ksc >= 0) {
		s_key_down[ksc] = 1;
		if (ev->repeat == 0)
			s_key_first_pressed[ksc] = 1;
	}
}

static void handle_keyup(const SDL_KeyboardEvent *ev)
{
	int ksc;

	ksc = translate_sdl_scancode(ev->keysym.scancode);
	if (ksc >= 0)
		s_key_down[ksc] = 0;
}

static void add_controller(int index)
{
	int i;
	SDL_GameController *pc;

	if (TRACE_CONTROLLER) {
		ktrace("adding controller index:%d", index);
	}
	pc = SDL_GameControllerOpen(index);
	if (pc == NULL)
		return;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (!is_valid_pad(i)) {
			s_pads[i].pcontroller = pc;
			s_pads[i].joyid = 
				SDL_JoystickInstanceID(
					SDL_GameControllerGetJoystick(pc));
			if (TRACE_CONTROLLER) {
				ktrace("added as id:%d", s_pads[i].joyid);
			}
			break;
		}
	}

	if (i == KERNEL_NPADS) {
		SDL_GameControllerClose(pc);
	}
}

static void on_controller_added(const SDL_ControllerDeviceEvent *ev)
{
	add_controller(ev->which);
}

static void reset_pad_axises(int ipad)
{
	int iaxis;

	for (iaxis = 0; iaxis < KERNEL_NAXIS; iaxis++) {
		s_pads[ipad].axis[iaxis] = 0;
	}
}

static void on_controller_removed(const SDL_ControllerDeviceEvent *ev)
{
	int ipad;

	ipad = get_controller_pad_index(ev->which);

	if (TRACE_CONTROLLER) {
		ktrace("removing controller id:%d pad:%d",
			(int) ev->which, ipad);
	}

	if (ipad >= 0) {
		if (TRACE_CONTROLLER) {
			ktrace("found and removed");
		}
		SDL_GameControllerClose(s_pads[ipad].pcontroller);
		s_pads[ipad].pcontroller = NULL;
		reset_pad_axises(ipad);
	}
}

static void open_controllers(void)
{
#if 0
	int i, n;

	n = SDL_NumJoysticks();
	for (i = 0; i < n; i++) {
		if (SDL_IsGameController(i)) {
			add_controller(i);
		}
	}
#endif
}

static void close_controllers(void)
{
	int i;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (s_pads[i].pcontroller != NULL) {
			SDL_GameControllerClose(s_pads[i].pcontroller);
			s_pads[i].pcontroller = NULL;
		}
	}
}

static void on_controller_buttondown(const SDL_ControllerButtonEvent *ev)
{
	int ksc;

	ksc = translate_sdl_controller_button(ev->which, ev->button);

	if (TRACE_CONTROLLER) {
		ktrace("controller bdown id:%d button:%d ksc:%d",
		     (int) ev->which, (int) ev->button, ksc);
	}	

	if (ksc >= 0) {
		s_key_down[ksc] = 1;
		s_key_first_pressed[ksc] = 1;
	}
}

static void on_controller_buttonup(const SDL_ControllerButtonEvent *ev)
{
	int ksc;

	ksc = translate_sdl_controller_button(ev->which, ev->button);

	if (TRACE_CONTROLLER) {
		ktrace("controller bup id:%d button:%d ksc:%d",
		     (int) ev->which, (int) ev->button, ksc);
	}

	if (ksc >= 0)
		s_key_down[ksc] = 0;
}

static void on_controller_axis(const SDL_ControllerAxisEvent *ev)
{
	int ipad;

	ipad = get_controller_pad_index(ev->which);

	if (TRACE_CONTROLLER_AXIS) {
		ktrace("controller axis id:%d axis:%d pad:%d",
		     (int) ev->which, (int) ev->axis, ipad);
	}

	if (ipad < 0) {
		/* we are not handling this controller */
		return;
	}

	if (ev->axis < 0 || ev->axis >= KERNEL_NAXIS)
		return;

	s_pads[ipad].axis[ev->axis] = ev->value;
}

#if KERNEL_JOYSTICK_SUPPORT
static void add_joystick(int index)
{
	int i;
	SDL_Joystick *joy;

	if (SDL_IsGameController(index))
		return;

	if (TRACE_JOYSTICK) {
		ktrace("adding joystick index:%d", index);
	}
	joy = SDL_JoystickOpen(index);
	if (joy == NULL)
		return;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (!is_valid_pad(i))
		{
			s_pads[i].pjoystick = joy;
			s_pads[i].joyid = SDL_JoystickInstanceID(joy);
			if (TRACE_JOYSTICK) {
				ktrace("added as id:%d", s_pads[i].joyid);
			}
			break;
		}
	}

	if (i == KERNEL_NPADS)
		SDL_JoystickClose(joy);
}

static void on_joystick_added(const SDL_JoyDeviceEvent *ev)
{
	add_joystick(ev->which);
}

static void on_joystick_removed(const SDL_JoyDeviceEvent *ev)
{
	int ipad;

	ipad = get_joystick_pad_index(ev->which);

	if (TRACE_JOYSTICK) {
		ktrace("removing joystick id:%d pad:%d",
			(int) ev->which, ipad);
	}

	if (ipad >= 0) {
		SDL_JoystickClose(s_pads[ipad].pjoystick);
		s_pads[ipad].pjoystick = NULL;
		reset_pad_axises(ipad);
	}
}
#endif

static void open_joysticks(void)
{
#if 0
	int i, n;

	n = SDL_NumJoysticks();
	for (i = 0; i < n; i++) {
		if (!SDL_IsGameController(i)) {
			add_joystick(i);
		}
	}
#endif
}

static void close_joysticks(void)
{
#if KERNEL_JOYSTICK_SUPPORT
	int i;

	for (i = 0; i < KERNEL_NPADS; i++) {
		if (s_pads[i].pjoystick != NULL) {
			SDL_JoystickClose(s_pads[i].pjoystick);
			s_pads[i].pjoystick = NULL;
		}
	}
#endif
}

#if KERNEL_JOYSTICK_SUPPORT
static void on_joystick_buttondown(const SDL_JoyButtonEvent *ev)
{
	int ksc;

	ksc = translate_sdl_joystick_button(ev->which, ev->button);

	if (TRACE_JOYSTICK) {
		ktrace("joystick bdown id:%d button:%d ksc:%d",
		     (int) ev->which, (int) ev->button, ksc);
	}

	if (ksc >= 0) {
		s_key_down[ksc] = 1;
		s_key_first_pressed[ksc] = 1;
	}
}

static void on_joystick_buttonup(const SDL_JoyButtonEvent *ev)
{
	int ksc;

	ksc = translate_sdl_joystick_button(ev->which, ev->button);

	if (TRACE_JOYSTICK) {
		ktrace("joystick bup id:%d button:%d ksc:%d",
		     (int) ev->which, (int) ev->button, ksc);
	}

	if (ksc >= 0)
		s_key_down[ksc] = 0;
}

static void on_joystick_hat(const SDL_JoyHatEvent *ev)
{
	int i, ipad, ksc;
	int d1, d2;
	int was_down[4];

	ipad = get_joystick_pad_index(ev->which);

	if (TRACE_JOYSTICK) {
		ktrace("joystick hat id:%d hat:%d button:%d pad:%d",
		     (int) ev->which, (int) ev->hat, (int) ev->value, ipad);
	}

	if (ipad < 0)
		return;

	if (ev->hat != 0)
		return;

	d1 = -1;
	d2 = -1;
	switch (ev->value) {
	case SDL_HAT_LEFTUP:
		d1 = KERNEL_KSC_PAD0_DLEFT;
	       	d2 = KERNEL_KSC_PAD0_DUP;
	       	break;
	case SDL_HAT_UP:
	       	d2 = KERNEL_KSC_PAD0_DUP;
	       	break;
	case SDL_HAT_RIGHTUP:
		d1 = KERNEL_KSC_PAD0_DRIGHT;
	       	d2 = KERNEL_KSC_PAD0_DUP;
	       	break;
	case SDL_HAT_LEFT:
	       	d1 = KERNEL_KSC_PAD0_DLEFT;
	       	break;
	case SDL_HAT_RIGHT:
		d1 = KERNEL_KSC_PAD0_DRIGHT;
	       	break;
	case SDL_HAT_LEFTDOWN:
		d1 = KERNEL_KSC_PAD0_DLEFT;
	       	d2 = KERNEL_KSC_PAD0_DDOWN;
	       	break;
	case SDL_HAT_DOWN:
	       	d2 = KERNEL_KSC_PAD0_DDOWN;
	       	break;
	case SDL_HAT_RIGHTDOWN:
		d1 = KERNEL_KSC_PAD0_DRIGHT;
	       	d2 = KERNEL_KSC_PAD0_DDOWN;
	       	break;
	}

	/* clear all pad buttons and set again after */
	for (i = KERNEL_KSC_PAD0_DUP; i < KERNEL_KSC_PAD0_DRIGHT + 1; i++) {
		ksc = i + ipad * KERNEL_NPAD_BUTTONS;
		was_down[i - KERNEL_KSC_PAD0_DUP] = s_key_down[ksc];
		s_key_down[ksc] = 0;
	}

	if (d1 >= 0) {
		ksc = d1 + ipad * KERNEL_NPAD_BUTTONS;
		s_key_down[ksc] = 1;
		if (!was_down[d1 - KERNEL_KSC_PAD0_DUP])
			s_key_first_pressed[ksc] = 1;
	}

	if (d2 >= 0) {
		ksc = d2 + ipad * KERNEL_NPAD_BUTTONS;
		s_key_down[ksc] = 1;
		if (!was_down[d2 - KERNEL_KSC_PAD0_DUP])
			s_key_first_pressed[ksc] = 1;
	}
}

static void on_joystick_axis(const SDL_JoyAxisEvent *ev)
{
	int ipad;

	ipad = get_joystick_pad_index(ev->which);

	if (TRACE_JOYSTICK_AXIS) {
		ktrace("joystick axis id:%d axis:%d pad:%d",
		     (int) ev->which, (int) ev->axis, ipad);
	}

	if (ipad < 0) {
		/* we are not handling this joystick */
		return;
	}

	if (ev->axis < 0 || ev->axis >= KERNEL_NAXIS)
		return;

	s_pads[ipad].axis[ev->axis] = ev->value;
}
#endif

static void on_finger_down(const SDL_TouchFingerEvent *ev)
{
	if (ev->fingerId < 0 || ev->fingerId >= KERNEL_NFINGERS)
		return;

	s_fingers[ev->fingerId].valid = 1;
	s_fingers[ev->fingerId].released = 0;
	s_fingers[ev->fingerId].x = ev->x;
	s_fingers[ev->fingerId].y = ev->y;
}

static void on_finger_motion(const SDL_TouchFingerEvent *ev)
{
	if (ev->fingerId < 0 || ev->fingerId >= KERNEL_NFINGERS)
		return;

	s_fingers[ev->fingerId].valid = 1;
	s_fingers[ev->fingerId].released = 0;
	s_fingers[ev->fingerId].x = ev->x;
	s_fingers[ev->fingerId].y = ev->y;
}

static void on_finger_up(const SDL_TouchFingerEvent *ev)
{
	if (ev->fingerId < 0 || ev->fingerId >= KERNEL_NFINGERS)
		return;

	s_fingers[ev->fingerId].released = 1;
}

static void clean_released_fingers(void)
{
	int i;

	if (!FINGERS_ON)
		return;

	for (i = 0; i < KERNEL_NFINGERS; i++) {
		if (s_fingers[i].released) {
			s_fingers[i].valid = 0;
		}
	}
}

static void clean_fingers(void)
{
	memset(s_fingers, 0, sizeof(s_fingers));
}

static void handle_event(const SDL_Event *ev)
{
	switch (ev->type) {
	case SDL_QUIT: s_running = 0; break;
	case SDL_KEYDOWN: handle_keydown(&ev->key); break;
	case SDL_KEYUP: handle_keyup(&ev->key); break;
	case SDL_CONTROLLERDEVICEADDED:
		if (CONTROLLERS_ON) {
			on_controller_added(&ev->cdevice);
		}
	       	break;
	case SDL_CONTROLLERDEVICEREMOVED:
		if (CONTROLLERS_ON) {
			on_controller_removed(&ev->cdevice);
		}
	       	break;
	case SDL_CONTROLLERBUTTONDOWN:
		if (CONTROLLERS_ON) {
			on_controller_buttondown(&ev->cbutton);
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		if (CONTROLLERS_ON) {
			on_controller_buttonup(&ev->cbutton);
		}
		break;
	case SDL_CONTROLLERAXISMOTION:
		if (CONTROLLERS_ON) {
			on_controller_axis(&ev->caxis);
		}
		break;
#if KERNEL_JOYSTICK_SUPPORT
	case SDL_JOYDEVICEADDED:
		on_joystick_added(&ev->jdevice);
		break;
	case SDL_JOYDEVICEREMOVED:
		on_joystick_removed(&ev->jdevice);
		break;
	case SDL_JOYBUTTONDOWN:
		on_joystick_buttondown(&ev->jbutton);
		break;
	case SDL_JOYBUTTONUP:
		on_joystick_buttonup(&ev->jbutton);
		break;
	case SDL_JOYAXISMOTION:
		on_joystick_axis(&ev->jaxis);
		break;
	case SDL_JOYHATMOTION:
		on_joystick_hat(&ev->jhat);
		break;
#endif
	case SDL_FINGERDOWN:
		if (FINGERS_ON) {
			on_finger_down(&ev->tfinger);
		}
		break;
	case SDL_FINGERMOTION:
		if (FINGERS_ON) {
			on_finger_motion(&ev->tfinger);
		}
		break;
	case SDL_FINGERUP:
		if (FINGERS_ON) {
			on_finger_up(&ev->tfinger);
		}
		break;
	}
}

static void check_fullscreen(void)
{
	if (key_down(KERNEL_KSC_LALT) &&
		key_first_pressed(KERNEL_KSC_RETURN))
       	{
		s_fullscreen ^= 1;
		if (s_fullscreen) {
			SDL_SetWindowFullscreen(s_win,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
			SDL_ShowCursor(SDL_DISABLE);
		} else {
			SDL_SetWindowFullscreen(s_win, 0);
			SDL_ShowCursor(SDL_ENABLE);
		}
		clean_first_pressed_keys();
		clean_key_states();
	}
}

static void update(void)
{
	SDL_Rect sr;

	// ktrace("run_update");
	check_fullscreen();
	kernel_snd_update();
	if (on_frame != NULL) {
		on_frame(s_data);
	}

	sr.x = sr.y = 0;
	sr.w = s_backbuf->w;
	sr.h = s_backbuf->h;
	SDL_UpdateTexture(s_backtex, &sr, s_backbuf->pixels, s_backbuf->pitch);
	SDL_RenderClear(s_renderer);
	SDL_RenderCopy(s_renderer, s_backtex, &sr, NULL);
	SDL_RenderPresent(s_renderer);
	// ktrace("run_update end");
}

static int is_soundcfg_valid(const struct kernel_config *kcfg)
{
	if (kcfg->on_sound == NULL)
		return 1;

	switch (kcfg->snd_sample_rate) {
	case 0:
	case KERNEL_SND_11K:
	case KERNEL_SND_22K:
	case KERNEL_SND_44K:
		break;
	default:
		return 0;
	}

	switch (kcfg->snd_nchannels) {
	case 0: case 1: case 2: break;
	default: return 0;
	}

	switch (kcfg->snd_bit_depth) {
	case 0: case 8: case 16: break;
	default: return 0;
	}

	return 1;
}

static void kernel_on_sound(unsigned char *ptr, int nsamples)
{
	if (on_sound != NULL)
		on_sound(s_data, ptr, nsamples);
}

static int run_loop(const struct kernel_config *kcfg)
{
	int nevents;
	SDL_Event ev;
	Uint64 t;
	double frame_ms, passed;

	// ktrace("run_loop ini");

	s_kcanvas.pixels = s_backbuf->pixels;
	s_kcanvas.pitch = s_backbuf->pitch;
	s_kcanvas.w = s_backbuf->w;
	s_kcanvas.h = s_backbuf->h;
	on_frame = kcfg->on_frame;
	on_sound = kcfg->on_sound;

	if (kcfg->fullscreen) {
		SDL_ShowCursor(SDL_DISABLE);
	}

	kernel_snd_init(kcfg->frames_per_second,
			  kcfg->snd_sample_rate,
			  kcfg->snd_nchannels,
			  kcfg->snd_bit_depth);
	kernel_snd_set_callback(kernel_on_sound);
	kernel_snd_play();

	open_controllers();
	open_joysticks();
	clean_key_states();
	clean_first_pressed_keys();
	clean_fingers();

	s_running = 1;
	frame_ms = 1000.0 / kcfg->frames_per_second;
	passed = frame_ms;
	nevents = 0;
	while (s_running) {
		// ktrace("run_loop");
		t = SDL_GetPerformanceCounter();
		if (passed >= frame_ms) {
#if 0
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
				     "ms %f\n", passed);
#endif
			while (passed >= frame_ms) {
				passed -= frame_ms;
			}
			update();
			clean_first_pressed_keys();
			clean_released_fingers();
		}
		if (s_running) {
			// ktrace("run_loop events");
			nevents = 0;
			while (nevents < MAX_FRAME_EVENTS &&
			       SDL_PollEvent(&ev) != 0)
		       	{
				handle_event(&ev);
				nevents++;
			}
			// ktrace("%d events handled", nevents);
			SDL_Delay(1);
			passed += (SDL_GetPerformanceCounter() - t) * 1000.0 /
				   SDL_GetPerformanceFrequency();
		}
	}

	// ktrace("run_loop closing");

	close_controllers();
	close_joysticks();
	kernel_snd_release();
	memset(&s_kcanvas, 0, sizeof(s_kcanvas));
	return KERNEL_E_OK;
}

static int run_backbuf(const struct kernel_config *kcfg)
{
	int ret;

	s_backbuf = SDL_CreateRGBSurface(0, kcfg->canvas_width,
					 kcfg->canvas_height, 32, 0, 0, 0, 0);
	if (s_backbuf == NULL) {
		ret = KERNEL_E_ERROR;
	} else {
		ret = run_loop(kcfg);
		SDL_FreeSurface(s_backbuf);
		s_backbuf = NULL;
	}

	return ret;
}

static int run_backtex(const struct kernel_config *kcfg)
{
	int ret;

	/* +1 to avoid texture clamping effects... */
	s_backtex = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ARGB8888,
				      SDL_TEXTUREACCESS_STREAMING,
	       			      kcfg->canvas_width + 1,
				      kcfg->canvas_height + 1);
	if (s_backtex == NULL) {
		ret = KERNEL_E_ERROR;
	} else {
		ret = run_backbuf(kcfg);
		SDL_DestroyTexture(s_backtex);
		s_backtex = NULL;
	}

	return ret;
}

static int run_renderer(const struct kernel_config *kcfg)
{
	int ret;
	unsigned char r, g, b;

	s_renderer = SDL_CreateRenderer(s_win, -1, 0);
	if (s_renderer == NULL) {
		ret = KERNEL_E_ERROR;
	} else {
		if (kcfg->hint_scale_quality == 1) {
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		}
		r = (kcfg->border_color & 0xff0000) >> 16;
		g = (kcfg->border_color & 0xff00) >> 8;
		b = kcfg->border_color;
		SDL_SetRenderDrawColor(s_renderer, r, g, b, SDL_ALPHA_OPAQUE);
		SDL_RenderSetLogicalSize(s_renderer, kcfg->canvas_width,
					 kcfg->canvas_height);
		ret = run_backtex(kcfg);
		SDL_DestroyRenderer(s_renderer);
		s_renderer = NULL;
	}

	return ret;
}

static int run_win(const struct kernel_config *kcfg)
{
	Uint32 wflags;
	int ret;

	wflags = SDL_WINDOW_RESIZABLE;
	if (kcfg->fullscreen)
		wflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if (kcfg->maximized)
		wflags |= SDL_WINDOW_MAXIMIZED;

	s_fullscreen = kcfg->fullscreen != 0;
	s_win = SDL_CreateWindow(kcfg->title,
				 SDL_WINDOWPOS_UNDEFINED,
				 SDL_WINDOWPOS_UNDEFINED,
				 kcfg->canvas_width, kcfg->canvas_height,
				 wflags);

	if (s_win == NULL) {
		ret = KERNEL_E_ERROR;
	} else {
		ret = run_renderer(kcfg);
		SDL_DestroyWindow(s_win);
		s_win = NULL;
	}

	return ret;
}

static void init_data_paths(void)
{
	if (PP_USE_SDL_DATADIR) {
		s_data_path = SDL_GetBasePath();
		if (s_data_path == NULL) {
#			if PP_ANDROID
				s_data_path = SDL_strdup("");
#			else
				s_data_path = SDL_strdup("./");
#			endif
		}
		s_prog_data_path = SDL_strdup(s_data_path);
	} else {
		s_data_path = SDL_strdup(PP_DATADIR "/");
		s_prog_data_path = SDL_strdup(PP_PROG_DATADIR "/");
	}
}

static int run_load_gamepad_db(const struct kernel_config *kcfg)
{
	int n;
	char *fname;

	fname = NULL;
	if (s_prog_data_path != NULL) {
		fname = malloc(strlen(s_prog_data_path) +
			strlen(GAMECONTROLLERDB_TXT) + 1);
	}

	if (fname != NULL) {
		strcpy(fname, s_prog_data_path);
		strcat(fname, GAMECONTROLLERDB_TXT);
		n = SDL_GameControllerAddMappingsFromFile(fname);
		ktrace("mappings added %d", n);
		free(fname);
	}

	return run_win(kcfg);
}

static int run_init_paths(const struct kernel_config *kcfg)
{
	int ret;

	init_data_paths();
	if (s_data_path != NULL) {
		ret = run_load_gamepad_db(kcfg);
	} else {
		ret = KERNEL_E_ERROR;
	}
	if (s_data_path != NULL) {
		SDL_free(s_data_path);
		s_data_path = NULL;
	}
	if (s_prog_data_path != NULL) {
		SDL_free(s_prog_data_path);
		s_prog_data_path = NULL;
	}
	return ret;
}

static int run(const struct kernel_config *kcfg, void *data)
{
	int ret;
	int flags;

	/* SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); */
	set_sdl_log_func(log_from_sdl);

	kasserta(kcfg != NULL);
	kasserta(kcfg->canvas_width > 0);
       	kasserta(kcfg->canvas_height > 0);
	kasserta(kcfg->frames_per_second > 0);
	kasserta(is_soundcfg_valid(kcfg));
	kasserta(SDL_GetPerformanceFrequency() != 0);

	flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER;
	if (kcfg->on_sound != NULL)
		flags |= SDL_INIT_AUDIO;

	if (SDL_Init(flags) < 0) {
		ret = KERNEL_E_ERROR;
	} else {
		s_data = data;
		ret = run_init_paths(kcfg);
		s_data = NULL;
		SDL_Quit();
	}

	restore_sdl_log_func();
	return ret;
}

static void stop(void)
{
	if (s_running)
		s_running = 0;
}

static struct kernel_canvas *get_canvas(void)
{
	return &s_kcanvas;
}

static int key_down(int key_scan_code)
{
	if (key_scan_code < 0 || key_scan_code >= KERNEL_NKEYS)
		return 0;
	else
		return s_key_down[key_scan_code];
}

static int key_first_pressed(int key_scan_code)
{
	if (key_scan_code < 0 || key_scan_code >= KERNEL_NKEYS)
		return 0;
	else
		return s_key_first_pressed[key_scan_code];
}

static int get_axis_value(int ipad, int iaxis)
{
	if (ipad < 0 || ipad >= KERNEL_NPADS)
		return 0;

	if (iaxis < 0 || iaxis >= KERNEL_NAXIS)
		return 0;

	return s_pads[ipad].axis[iaxis];
}

static const char *get_data_path(void)
{
	return s_data_path;
}

static char *get_config_path(const char *org, const char *app)
{
	if (org == NULL)
		org = "org";

	if (app == NULL)
		app = "app";

	return SDL_GetPrefPath(org, app);
}

static void get_window_size(int *w, int *h)
{
	if (s_win != NULL) {
		SDL_GetWindowSize(s_win, w, h);
	} else {
		if (w != NULL) {
			*w = 0;
		}
		if (h != NULL) {
			*h = 0;
		}
	}
}

static void insert_pad_event(int down, int ksc)
{
	if (down) {
		if (ksc >= 0) {
			s_key_down[ksc] = 1;
			s_key_first_pressed[ksc] = 1;
		}
	} else {
		s_key_down[ksc] = 0;
	}
}

static const struct kernel_finger *get_finger(int i)
{
	if (i < 0 || i >= KERNEL_NFINGERS)
		return NULL;

	if (s_fingers[i].valid == 0)
		return NULL;

	return &s_fingers[i];
}

#if PP_ANDROID

static void open_url(const char *url)
{
	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;
	jstring str;

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass, "openUrl",
			"(Ljava/lang/String;)V");
	str = (*env)->NewStringUTF(env, url);
	if (str != NULL) {
		(*env)->CallStaticVoidMethod(env, theClass, theMethod, str);
		(*env)->DeleteLocalRef(env, str);
	}

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);
}

#elif defined(_WIN32)

static void open_url(const char *url)
{
	SDL_MinimizeWindow(s_win);
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

#else

static void open_url(const char *url)
{
	char cmd[256];

	SDL_MinimizeWindow(s_win);
	snprintf(cmd, sizeof(cmd), "xdg-open %s", url);
	system(cmd);
}

#endif

static const struct kernel_device s_device = {
	.run = run,
	.stop = stop,
	.get_canvas = get_canvas,
	.key_down = key_down,
	.key_first_pressed = key_first_pressed,
	.get_axis_value = get_axis_value,
	.get_data_path = get_data_path,
	.clear_first_pressed_keys = clean_first_pressed_keys,
	.clear_down_keys = clean_key_states,
	.get_config_path = get_config_path,
	.trace = trace,
	.get_window_size = get_window_size,
	.insert_pad_event = insert_pad_event,
	.get_finger = get_finger,
	.open_url = open_url,
};

const struct kernel_device *kernel_get_device(void)
{
	return &s_device;
}
