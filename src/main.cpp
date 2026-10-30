#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <bits/stdc++.h>
#include <shader.hpp>

struct CameraState {
    double pos_x = -2.0;
    double pos_y = -2.0;
    double size_x = 4.0;
    double size_y = 4.0;

    // logical window size (matches glfwGetCursorPos units)
    int window_width = 1000;
    int window_height = 1000;

    bool dragging = false;
    double last_cursor_x = 0.0;
    double last_cursor_y = 0.0;
};

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    CameraState* cam = (CameraState*)glfwGetWindowUserPointer(window);

    double cursor_x, cursor_y;
    glfwGetCursorPos(window, &cursor_x, &cursor_y);

    double u = cursor_x / (double)cam->window_width;
    double v = 1.0 - (cursor_y / (double)cam->window_height);

    double world_x = cam->pos_x + u * cam->size_x;
    double world_y = cam->pos_y + v * cam->size_y;

    double zoom_factor = (yoffset > 0) ? 0.9 : 1.0 / 0.9;

    cam->size_x *= zoom_factor;
    cam->size_y *= zoom_factor;

    cam->pos_x = world_x - u * cam->size_x;
    cam->pos_y = world_y - v * cam->size_y;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    CameraState* cam = (CameraState*)glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            cam->dragging = true;
            glfwGetCursorPos(window, &cam->last_cursor_x, &cam->last_cursor_y);
        } else if (action == GLFW_RELEASE) {
            cam->dragging = false;
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    CameraState* cam = (CameraState*)glfwGetWindowUserPointer(window);

    if (!cam->dragging) return;

    double dx = xpos - cam->last_cursor_x;
    double dy = ypos - cam->last_cursor_y;

    double world_dx = (dx / (double)cam->window_width) * cam->size_x;
    double world_dy = (dy / (double)cam->window_height) * cam->size_y;

    cam->pos_x -= world_dx;
    cam->pos_y += world_dy;

    cam->last_cursor_x = xpos;
    cam->last_cursor_y = ypos;
}

// Use WINDOW size (not framebuffer size) for camera math, since
// glfwGetCursorPos is reported in window coordinates.
static void window_size_callback(GLFWwindow* window, int width, int height) {
    CameraState* cam = (CameraState*)glfwGetWindowUserPointer(window);
    cam->window_width = width;
    cam->window_height = height;
}

// Framebuffer size is only used for glViewport (can differ from window
// size on HiDPI displays).
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    int window_width = 1000;
    int window_height = 1000;

    if (!glfwInit()) {
        std::cerr << "glfwInit failed" << std::endl;
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif
    GLFWwindow* window = glfwCreateWindow(
        window_width,
        window_height,
        "name",
        nullptr,
        nullptr
    );
    if (!window) {
        std::cerr << "glfwCreateWindow failed" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGLLoader failed" << std::endl;
        exit(1);
    }
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const char* message, const void* userParam) {
        GLint current_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
        std::cerr << "[GL DEBUG] (program=" << current_program << ") " << message << std::endl;
    }, nullptr);

    // Use actual framebuffer size for the initial viewport call too.
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- camera / input state ---
    CameraState camera;
    // use the logical window size (not framebuffer size!) here
    glfwGetWindowSize(window, &camera.window_width, &camera.window_height);
    camera.pos_x = -2.0;
    camera.pos_y = -2.0;
    camera.size_x = 4.0;
    camera.size_y = 4.0;

    glfwSetWindowUserPointer(window, &camera);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLuint quad_vao{0};
    GLuint quad_vbo{0};
    float quad_verts[] = {
        //   x,     y,    u,    v
          0.0f, 0.0f, 0.0f, 0.0f,
          1.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    Shader* mandelbrot_shader;
    mandelbrot_shader = new Shader(
        "shaders/vertex.gl",
        "shaders/fragment.gl"
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(quad_vao);
    double start_time = glfwGetTime();
    int i = 0;
    while(!glfwWindowShouldClose(window)) {
        i++;
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mandelbrot_shader->use();
        mandelbrot_shader->setDVec2(
            "camera_world_position",
            camera.pos_x,
            camera.pos_y
        );
        mandelbrot_shader->setDVec2(
            "camera_world_size",
            camera.size_x,
            camera.size_y
        );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        std::cout << (glfwGetTime() - start_time) << std::endl;
        start_time = glfwGetTime();
    }
    std::cout << (glfwGetTime() - start_time)<< std::endl;
    std::cout << (double)i / (glfwGetTime() - start_time) << std::endl;
}