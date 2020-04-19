#ifndef CLIENT_UI_OPENGL_INPUT_H
#define CLIENT_UI_OPENGL_INPUT_H
struct GLFWwindow;

void handle_held_keys(void);
void set_input_callbacks(struct GLFWwindow *window);
#endif
