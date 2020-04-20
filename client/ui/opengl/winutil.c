#include "client/ui/opengl/winutil.h"

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
