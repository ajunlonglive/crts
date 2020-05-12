#ifndef CLIENT_UI_OPENGL_INPUT_H
#define CLIENT_UI_OPENGL_INPUT_H
struct GLFWwindow;
struct hiface;

void handle_gl_mouse(struct hiface *hf);
void handle_held_keys(struct hiface *hf);
void set_input_callbacks(struct GLFWwindow *window);
#endif
