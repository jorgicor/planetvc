/*
Copyright (c) 2014-2017 Jorge Giner Cordero

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

#ifndef KERNEL_H
#define KERNEL_H

/*
 * Key scan codes.
 */
enum {
	KERNEL_KSC_0,
	KERNEL_KSC_1,
	KERNEL_KSC_2,
	KERNEL_KSC_3,
	KERNEL_KSC_4,
	KERNEL_KSC_5,
	KERNEL_KSC_6,
	KERNEL_KSC_7,
	KERNEL_KSC_8,
	KERNEL_KSC_9,
	KERNEL_KSC_A,
	KERNEL_KSC_B,
	KERNEL_KSC_C,
	KERNEL_KSC_D,
	KERNEL_KSC_E,
	KERNEL_KSC_F,
	KERNEL_KSC_G,
	KERNEL_KSC_H,
	KERNEL_KSC_I,
	KERNEL_KSC_J,
	KERNEL_KSC_K,
	KERNEL_KSC_L,
	KERNEL_KSC_M,
	KERNEL_KSC_N,
	KERNEL_KSC_O,
	KERNEL_KSC_P,
	KERNEL_KSC_Q,
	KERNEL_KSC_R,
	KERNEL_KSC_S,
	KERNEL_KSC_T,
	KERNEL_KSC_U,
	KERNEL_KSC_V,
	KERNEL_KSC_W,
	KERNEL_KSC_X,
	KERNEL_KSC_Y,
	KERNEL_KSC_Z,
	KERNEL_KSC_LEFT,
	KERNEL_KSC_UP,
	KERNEL_KSC_RIGHT,
	KERNEL_KSC_DOWN,
	KERNEL_KSC_LSHIFT,
	KERNEL_KSC_RSHIFT,
	KERNEL_KSC_LALT,
	KERNEL_KSC_LCTRL,
	KERNEL_KSC_RCTRL,
	KERNEL_KSC_SPACE,
	KERNEL_KSC_RETURN,
	KERNEL_KSC_BACKSPACE,
	KERNEL_KSC_ESC,
	KERNEL_KSC_PAD0_A,
	KERNEL_KSC_PAD0_B,
	KERNEL_KSC_PAD0_X,
	KERNEL_KSC_PAD0_Y,
	KERNEL_KSC_PAD0_BACK,
	KERNEL_KSC_PAD0_GUIDE,
	KERNEL_KSC_PAD0_START,
	KERNEL_KSC_PAD0_LSTICK,
	KERNEL_KSC_PAD0_RSTICK,
	KERNEL_KSC_PAD0_LSHOULDER,
	KERNEL_KSC_PAD0_RSHOULDER,
	KERNEL_KSC_PAD0_DUP,
	KERNEL_KSC_PAD0_DDOWN,
	KERNEL_KSC_PAD0_DLEFT,
	KERNEL_KSC_PAD0_DRIGHT,
	KERNEL_KSC_PAD1_A,
	KERNEL_KSC_PAD1_B,
	KERNEL_KSC_PAD1_X,
	KERNEL_KSC_PAD1_Y,
	KERNEL_KSC_PAD1_BACK,
	KERNEL_KSC_PAD1_GUIDE,
	KERNEL_KSC_PAD1_START,
	KERNEL_KSC_PAD1_LSTICK,
	KERNEL_KSC_PAD1_RSTICK,
	KERNEL_KSC_PAD1_LSHOULDER,
	KERNEL_KSC_PAD1_RSHOULDER,
	KERNEL_KSC_PAD1_DUP,
	KERNEL_KSC_PAD1_DDOWN,
	KERNEL_KSC_PAD1_DLEFT,
	KERNEL_KSC_PAD1_DRIGHT,
	KERNEL_NKEYS,
};

/* Some joystick enums. */
enum {
	KERNEL_NPADS = 2,
	KERNEL_NPAD_BUTTONS = KERNEL_KSC_PAD1_A - KERNEL_KSC_PAD0_A,
	KERNEL_AXIS_H1 = 0,	/* horizontal axis */
	KERNEL_AXIS_V1 = 1,	/* vertical axis */
	KERNEL_NAXIS = 2,
	KERNEL_NFINGERS = 8,
};

/*
 * Sound samples per second.
 */
enum {
	KERNEL_SND_11K = 11025,
	KERNEL_SND_22K = KERNEL_SND_11K * 2,
	KERNEL_SND_44K = KERNEL_SND_22K * 2,
};

/*
 * If new error codes are added here, KERNEL_E_MEM must be always the last
 * since it is used by drivers to select their own error codes.
 */
enum {
	KERNEL_E_OK,			/* No error. */
	KERNEL_E_ERROR,			/* Generic error. */
	KERNEL_E_INVALID_ARG,		/* Invalid function argument. */
	KERNEL_E_MEM,			/* Not enough memory */
};

/* A finger touch position. */
struct kernel_finger {
	short valid;
	short released;
	float x;
	float y;
};

/*
 * The canvas where we can paint.
 */
struct kernel_canvas {
	unsigned int *pixels;	/* Pixels in 32 bit xrgb format. */
	int w, h;
	int pitch;		/* Distance between each row in bytes. */
};

struct kernel_config {
	/*
	 * Title for the window, UTF-8, can be NULL.
	 */
	const char *title;

	/*
	 * Dimensions of the desired canvas.
	 */
	int canvas_width;
	int canvas_height;

	/*
	 * Start fullscreen, or windowed if 0.
	 */
	int fullscreen;

	/*
	 * Initial zoom factor on window creation if fullscreen is false,
	 * or, if fullscreen is true, zoom factor of the window when we pass
	 * from fullscreen to windowed for the first time.
	 * The client area of the window will have width and height
	 * equal to framebuf_width and framebuf_height multiplied by this
	 * zoom factor. Less than zero, zero or one set no zoom.
	 * If the zoom factor is too big, the window will be maximized.
	 */
	int zoom_factor;

	/* 
	 * If windowed, starts the window maximized if not 0.
	 */
	int maximized;

	/*
	 * Scaling filtering:
	 * 	0 means no filtering.
	 * 	1 means linear filtering.
	 * 	otherwise no filtering.
	 * Can be ignored by the system.
	 */
	int hint_scale_quality;

	/*
	 * Color to paint the borders your application does not cover.
	 * XRGB 32 bit format (8 bits unused, 8 bits red, 8 green, 8 blue).
	 */
	unsigned int border_color;

	/* 
	 * Times to update per second.
	 */
	int frames_per_second;

	/*
	 * For Windows only, the nCmdShow parameter received on WinMain.
	 */
	int nCmdShow;

	/*
	 * Function that will be called each frame, can be NULL.
	 */
	void (*on_frame)(void *data);

	/*
	 * Function that will be called to fill sound data, can be NULL.
	 */
	void (*on_sound)(void *data, unsigned char *samples, int nsamples);

	/*
	 * Sound buffer parameters
	 */
	int snd_sample_rate;	/* One of KERNEL_SND enum.
       				 * If 0 will default to KERNEL_SND_44K.
				 */
	int snd_nchannels;	/* Must be 1 (mono) or 2 (stereo).
       				 * If 0 will default to stereo.
				 */
	int snd_bit_depth;	/* Must be 8 or 16.
				 * If 0 will default to 16.
				 * If 8, samples will be unsigned (0-255).
				 * If 16, will be signed (-32768 - 32767 ).
				 * Always little endian.
				 */
};

struct kernel_device {

	/*
	 * Runs the kernel loop. Creates the window and calls on_frame()
	 * and on_sound() periodically. 'data' can be NULL, will be passed
	 * to on_frame() and on_sound().
	 *
	 * See struct kernel_config for configuration parameters.
	 *
	 * Returns:
	 * 	KERNEL_E_OK: when everythig went well and you call stop(),
	 * 		the loop will end and you will receive this.
	 * 	KERNEL_E_MEM: if there wasn't enough memory.
	 * 	KERNEL_E_ERROR: for a generic error, the loop is not started.
	 *
	 * Asserts:
	 * 	kcfg.frames_per_second > 0.
	 * 	kcfg.canvas_width > 0.
	 * 	kcfg.canvas_height > 0.
	 *	kcfg.on_frame != NULL.
	 *	sound config is valid: either on_sound is NULL or
	 *		on_sound != NULL and snd parameters are in range.
	 */
	int (*run)(const struct kernel_config *kcfg, void *data);

	/*
	 * Once the kernel is running, schedules a stop on the next frame.
	 */
	void (*stop)(void);

	/*
	 * Gets the canvas where to paint.
	 * You should take it only on each on_frame() call and don't suppose
	 * that it will be the same between one on_frame() call and the next.
	 */
	struct kernel_canvas *(*get_canvas)(void);

	/*
	 * Returns true if a key (by scan code) is down.
	 * Only valid inside the on_frame() call.
	 */
	int (*key_down)(int key_scan_code);

	/* Clears down state for all keys. */
	void (*clear_down_keys)(void);

	/*
	 * Returns 1 if a key (by scan code) was pressed this frame and
	 * wasn't pressed in the previous frame. Note that this can return
	 * 1 and key_down() can return 0 for the same key.
	 * Only valid inside the on_frame() call.
	 */
	int (*key_first_pressed)(int key_scan_code);

	/* Clears first pressed state for all keys. */
	void (*clear_first_pressed_keys)(void);

	/*
	 * Returns the current value of the axis KERNEL_AXIS_H
	 * or KERNEL_AXIS_V for the pad npad.
	 * Negative will be left, up. Positive right, down.
	 */
	int (*get_axis_value)(int npad, int axis);

	void (*get_window_size)(int *w, int *h);
	void (*insert_pad_event)(int down, int ksc);
	const struct kernel_finger * (*get_finger)(int i);

	/*
	 * Path to read only application data.
	 * Only valid after run() is called.
	 * Never returns NULL (after run() is called).
	 */
	const char *(*get_data_path)(void);

	/* Returns the path were we can write application files.
	 * The path is made by an OS path and using org and app.
	 * Can return NULL.
	 * Can be slow a slow call. Store once called.
	 * It is your responsibility to free it.
	 */
	char *(*get_config_path)(const char *org, const char *app);

	/* Trace to console. */
	void (*trace)(const char *msg);

	/* Opens a url on the web browser */
	void (*open_url)(const char *url);
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Access the device.
 */

#ifdef _MSC_VER
__declspec(dllexport)
const struct kernel_device * __cdecl kernel_get_device(void);
#else
const struct kernel_device *kernel_get_device(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
