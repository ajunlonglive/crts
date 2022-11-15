#include "posix.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shared/input/keyboard.h"
#include "shared/input/mouse.h"
#include "shared/ui/gl/window.h"
#include "shared/util/log.h"

#define WIN_NAME "crts"
#define WIN_HEIGHT 600
#define WIN_WIDTH 800

static struct gl_win win;
static GLFWwindow *glfw_win;
static bool initialized = false;
static void *key_callback_ctx;

static void
glfw_check_err(void)
{
	const char* description;

	int err_code;
	err_code = glfwGetError(&description);
	if (description) {
		LOG_W(log_gui, "GLFW error: %d, %s", err_code, description);
	}
}

static char *gl_debug_msg_types[] = {
	[GL_DEBUG_TYPE_ERROR] = "error",
	[GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR] = "deprecated",
	[GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR] = "UB",
	[GL_DEBUG_TYPE_PORTABILITY] = "portability",
	[GL_DEBUG_TYPE_PERFORMANCE] = "perf",
	[GL_DEBUG_TYPE_MARKER] = "marker",
	[GL_DEBUG_TYPE_PUSH_GROUP] = "push",
	[GL_DEBUG_TYPE_POP_GROUP] = "pop",
	[GL_DEBUG_TYPE_OTHER] = "other",
};

static char *gl_debug_msg_sources[] = {
	[GL_DEBUG_SOURCE_API] = "opengl",
	[GL_DEBUG_SOURCE_WINDOW_SYSTEM] = "window",
	[GL_DEBUG_SOURCE_SHADER_COMPILER] = "compiler",
	[GL_DEBUG_SOURCE_THIRD_PARTY] = "3rd party",
	[GL_DEBUG_SOURCE_APPLICATION] = "app",
	[GL_DEBUG_SOURCE_OTHER] = "other",
};

static char *gl_debug_msg_severity[] = {
	[GL_DEBUG_SEVERITY_HIGH] = "high",
	[GL_DEBUG_SEVERITY_MEDIUM] = "med",
	[GL_DEBUG_SEVERITY_LOW] = "low",
	[GL_DEBUG_SEVERITY_NOTIFICATION] = "info",
};

static void GLAPIENTRY
gl_debug(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
	case GL_DEBUG_SEVERITY_MEDIUM:
	case GL_DEBUG_SEVERITY_LOW:
		LOG_W(log_gui, "GL[%d]: [%s][%s][%s] %s", id, gl_debug_msg_sources[source],
			gl_debug_msg_types[type], gl_debug_msg_severity[severity],
			message);
		break;
	default:
		L(log_gui, "GL[%d]: [%s][%s][%s] %s", id, gl_debug_msg_sources[source],
			gl_debug_msg_types[type], gl_debug_msg_severity[severity],
			message);
		break;
	}
}

static void
resize_callback(struct GLFWwindow *_win, int width, int height)
{
	win.px_width = width;
	win.px_height = height;

	glfwGetWindowSize(glfw_win, (int *)&win.sc_width, (int *)&win.sc_height);

	win.resized = true;
}

static uint32_t
transform_glfw_key(int k)
{
	switch (k) {
	case GLFW_KEY_UP:
		return skc_up;
	case GLFW_KEY_DOWN:
		return skc_down;
	case GLFW_KEY_LEFT:
		return skc_left;
	case GLFW_KEY_RIGHT:
		return skc_right;
	case GLFW_KEY_ENTER:
		return '\n';
	case GLFW_KEY_TAB:
		return '\t';
	case GLFW_KEY_F1:
		return skc_f1;
	case GLFW_KEY_F2:
		return skc_f2;
	case GLFW_KEY_F3:
		return skc_f3;
	case GLFW_KEY_F4:
		return skc_f4;
	case GLFW_KEY_F5:
		return skc_f5;
	case GLFW_KEY_F6:
		return skc_f6;
	case GLFW_KEY_F7:
		return skc_f7;
	case GLFW_KEY_F8:
		return skc_f8;
	case GLFW_KEY_F9:
		return skc_f9;
	case GLFW_KEY_F10:
		return skc_f10;
	case GLFW_KEY_F11:
		return skc_f11;
	case GLFW_KEY_F12:
		return skc_f12;
	case GLFW_KEY_BACKSPACE:
		return '\b';
	case GLFW_KEY_PAGE_UP:
		return skc_pgup;
	case GLFW_KEY_PAGE_DOWN:
		return skc_pgdn;
	case GLFW_KEY_HOME:
		return skc_home;
	case GLFW_KEY_END:
		return skc_end;
	case GLFW_KEY_ESCAPE:
		return '\033';
	case GLFW_KEY_RIGHT_SHIFT:
	case GLFW_KEY_LEFT_SHIFT:
		return skc_shift;
	case GLFW_KEY_RIGHT_CONTROL:
	case GLFW_KEY_LEFT_CONTROL:
		return skc_control;
	default:
		if ('A' <= k && k <= 'Z') {
			k = k - 'A' + 'a';
		}
		return k;
	}
}

static enum modifier_types
key_to_mod(int32_t m)
{
	switch (m) {
	case skc_shift:
		return mod_shift;
	case skc_control:
		return mod_ctrl;
	default:
		return 0;
	}
}

static void
held_key_pushpop(uint8_t key, bool held)
{
	if (held) {
		if (win.keyboard.held_len < GL_WIN_MAX_HELD_KEYS) {
			win.keyboard.held[win.keyboard.held_len] = key;

			++win.keyboard.held_len;
		}
	} else {
		uint32_t j;
		assert(win.keyboard.held_len);

		for (j = 0; j < win.keyboard.held_len; ++j) {
			if (win.keyboard.held[j] == key) {
				if (j < (uint32_t)(win.keyboard.held_len - 1)) {
					win.keyboard.held[j] = win.keyboard.held[win.keyboard.held_len - 1];
				}

				break;
			}
		}

		assert(j != win.keyboard.held_len);

		--win.keyboard.held_len;
	}
}

static void
key_callback(GLFWwindow *window, int32_t _key, int32_t _scancode, int32_t action, int32_t _mods)
{
	int32_t key;
	if (_key < 0) {
		L(log_misc, "skipping unknown key: %d", _scancode);
		return;
	} else if ((key = transform_glfw_key(_key)) > 256) {
		L(log_misc, "skipping unknown key: %d", _key);
		return;
	}


	enum modifier_types mod = key_to_mod(key);

	if (!mod) {
		goto no_mod;
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		win.keyboard.mod |= mod;
	} else {
		win.keyboard.mod &= ~mod;
	}

	return;
no_mod:
	switch (action) {
	case GLFW_PRESS:
		held_key_pushpop(key, true);
		win.key_input_callback(key_callback_ctx, win.keyboard.mod, key, key_action_oneshot);
		break;
	case GLFW_REPEAT:
		/* nothing */
		break;
	case GLFW_RELEASE:
		held_key_pushpop(key, false);
		return;
	}
}

static void
char_callback(GLFWwindow *window, uint32_t codepoint)
{
	win.text_input_callback(key_callback_ctx, codepoint);
}

static void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	win.mouse.x = xpos;
	win.mouse.y = ypos;

	if (win.mouse.init) {
		win.mouse.dx = win.mouse.x - win.mouse.lx;
		win.mouse.dy = win.mouse.y - win.mouse.ly;
	} else {
		win.mouse.dx = 0;
		win.mouse.dy = 0;
		win.mouse.init = true;
	}

	win.mouse.lx = win.mouse.x;
	win.mouse.ly = win.mouse.y;

	win.mouse.still = false;
}

static void
scroll_callback(GLFWwindow* window, double xoff, double yoff)
{
	win.mouse.scroll = yoff;
	win.mouse.still = false;
}

static enum modifier_types
glfw_mod_to_mod(int32_t m)
{
	switch (m) {
	case GLFW_MOD_SHIFT:
		return mod_shift;
	case GLFW_MOD_CONTROL:
		return mod_ctrl;
	default:
		return 0;
	}
}

static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mod)
{
	assert(button < 8);

	uint8_t key;
	switch (button) {
	case GLFW_MOUSE_BUTTON_1:
		key = skc_mb1;
		break;
	case GLFW_MOUSE_BUTTON_2:
		key = skc_mb2;
		break;
	case GLFW_MOUSE_BUTTON_3:
		key = skc_mb3;
		break;
	default:
		key = 0;
	}

	if (key) {
		switch (action) {
		case GLFW_PRESS:
			held_key_pushpop(key, true);
			win.key_input_callback(key_callback_ctx, glfw_mod_to_mod(mod), key, key_action_oneshot);
			break;
		case GLFW_RELEASE:
			held_key_pushpop(key, false);
			break;
		default:
			break;
		}
	}

	if (action == GLFW_PRESS) {
		win.mouse.buttons |= 1 << (button + 1);
	} else {
		win.mouse.buttons &= ~(1 << (button + 1));
	}

	win.mouse.still = false;
}

void
gl_win_swap_buffers(void)
{
	glfwSwapBuffers(glfw_win);
}

void
gl_win_poll_events(void *ctx)
{
	win.mouse.dx = 0;
	win.mouse.dy = 0;

	key_callback_ctx = ctx;
	glfwPollEvents();
	key_callback_ctx = NULL;
}

void
gl_win_terminate(void)
{
	glfwTerminate();
}

bool
gl_win_is_focused(void)
{
	return glfwGetWindowAttrib(glfw_win, GLFW_FOCUSED);
}

void
gl_win_set_cursor_display(bool mode)
{
	if (mode) {
		glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
		glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

static void
default_key_input_callback(void *ctx, uint8_t mod, uint8_t key, uint8_t action)
{
}

static void
default_text_input_callback(void *ctx, uint32_t codepoint)
{
}

struct gl_win *
gl_win_init(void)
{
	win.key_input_callback = default_key_input_callback;
	win.text_input_callback = default_text_input_callback;

	win.resized = true;
	win.mouse.init = false;

	if (initialized) {
		return &win;
	}

	initialized = true;

	if (!glfwInit()) {
		glfw_check_err();
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_ALPHA_BITS, 0);

	if (!(glfw_win = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_NAME, NULL, NULL))) {
		LOG_W(log_gui, "failed to create GLFW window\n");
		glfw_check_err();

		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(glfw_win);

	int version = gladLoadGL(glfwGetProcAddress);
	L(log_gui, "glad successfully loaded GL %d.%d",
		GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug, 0);

		/* enable all messages */
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

		/* disable "SIMD32 shader inefficient" messages */
		glDebugMessageControl(
			GL_DEBUG_SOURCE_SHADER_COMPILER,
			GL_DEBUG_TYPE_PERFORMANCE,
			GL_DONT_CARE,
			2,
			(GLuint []){ 6, 9 },
			GL_FALSE);
	} else {
		LOG_W(log_gui, "GL_DEBUG_OUTPUT not supported");
	}

	glfwGetFramebufferSize(glfw_win, (int *)&win.px_width, (int *)&win.px_height);
	glViewport(0, 0, win.px_width, win.px_height);
	glfwGetWindowSize(glfw_win, (int *)&win.sc_width, (int *)&win.sc_height);

	glfwSetFramebufferSizeCallback(glfw_win, resize_callback);

	/* input callbacks */
	glfwSetKeyCallback(glfw_win, key_callback);
	glfwSetCharCallback(glfw_win, char_callback);
	glfwSetCursorPosCallback(glfw_win, mouse_callback);
	glfwSetScrollCallback(glfw_win, scroll_callback);
	glfwSetMouseButtonCallback(glfw_win, mouse_button_callback);

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	return &win;
}
