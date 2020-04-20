#include "client/ui/opengl/winutil.h"
#include "shared/util/log.h"

static void
glfw_check_err(void)
{
	const char* description;

	int err_code;
	err_code = glfwGetError(&description);
	if (description) {
		fprintf(stderr, "GLFW error: %d, %s\n", err_code, description);
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
	L("GL[%d]: [%s][%s][%s] %s", id, gl_debug_msg_sources[source],
		gl_debug_msg_types[type], gl_debug_msg_severity[severity],
		message);
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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (!(window = glfwCreateWindow(800, 600, "gltut", NULL, NULL))) {
		fprintf(stderr, "failed to create GLFW window\n");
		glfw_check_err();

		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "failed to initialize GLAD\n");
		return NULL;
	}

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
	}

	return window;
}

#define BUFLEN 2048
#define CHUNKSIZE 64
static bool
compile_shader(const char *path, GLenum type, uint32_t *id)
{
	FILE *sf;
	size_t b, i = 0;
	char buf[BUFLEN] = { 0 };
	int32_t ret;

	if (!(sf = fopen(path, "r"))) {
		fprintf(stderr, "failed to open '%s'", path);
		return false;
	}

	while ((b = fread(&buf[i], 1, CHUNKSIZE, sf))) {
		i += b;

		if (b < CHUNKSIZE) {
			break;
		}
	}

	const char *src = buf;
	const char * const* srcp = &src;

	*id = glCreateShader(type);
	glShaderSource(*id, 1, srcp, NULL);
	glCompileShader(*id);

	glGetShaderiv(*id, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		glGetShaderInfoLog(*id, BUFLEN, NULL, (char *)buf);
		fprintf(stderr, "failed to compile '%s'\n%s", path, buf);
		return false;
	}

	return true;
}

bool
link_shaders(struct shader_src *shaders, uint32_t *program)
{
	size_t i = 0;
	int32_t ret;
	char errinfo[512];

	*program = glCreateProgram();

	for (i = 0; *shaders[i].path; ++i) {
		if (!compile_shader(shaders[i].path, shaders[i].type,
			&shaders[i].id)) {
			return false;
		}
		glAttachShader(*program, shaders[i].id);
	}

	glLinkProgram(*program);

	glGetProgramiv(*program, GL_LINK_STATUS, &ret);
	if (!ret) {
		glGetProgramInfoLog(*program, 512, NULL, errinfo);
		fprintf(stderr, "failed to link program\n%s", errinfo);
		return false;
	}

	for (i = 0; *shaders[i].path; ++i) {
		glDeleteShader(shaders[i].id);
	}

	return true;
}
