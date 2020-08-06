#include "posix.h"

#include "shared/opengl/window.h"
#include "shared/util/log.h"

#define WIN_NAME "crts"
#define WIN_HEIGHT 600
#define WIN_WIDTH 800

static void
glfw_check_err(void)
{
	const char* description;

	int err_code;
	err_code = glfwGetError(&description);
	if (description) {
		L("GLFW error: %d, %s\n", err_code, description);
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
		LOG_W("GL[%d]: [%s][%s][%s] %s", id, gl_debug_msg_sources[source],
			gl_debug_msg_types[type], gl_debug_msg_severity[severity],
			message);
		break;
	default:
		LOG_D("GL[%d]: [%s][%s][%s] %s", id, gl_debug_msg_sources[source],
			gl_debug_msg_types[type], gl_debug_msg_severity[severity],
			message);
		break;
	}
}

GLFWwindow *
init_window(void)
{
	GLFWwindow* window;

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

	if (!(window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_NAME, NULL, NULL))) {
		L("failed to create GLFW window\n");
		glfw_check_err();

		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	int version = gladLoadGL(glfwGetProcAddress);
	LOG_D("glad successfully loaded GL %d.%d",
		GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
	} else {
		LOG_W("GL_DEBUG_OUTPUT not supported");
	}

	return window;
}
