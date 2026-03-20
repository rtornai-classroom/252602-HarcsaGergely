#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

#define numVAOs 1
#define numVBOs 1

GLuint renderingProgramCircle;
GLuint renderingProgramLine;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Kör és szakasz tulajdonságai
float cx = 300.0f, cy = 300.0f;
float dx = 5.0f, dy = 0.0f;
float lineY = 300.0f;

// Shader fordító segédfüggvény
GLuint createShaderProgram(const char* vShaderSource, const char* fShaderSource) {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glShaderSource(fShader, 1, &fShaderSource, NULL);

    glCompileShader(vShader);
    glCompileShader(fShader);

    GLuint vfProgram = glCreateProgram();
    glAttachShader(vfProgram, vShader);
    glAttachShader(vfProgram, fShader);
    glLinkProgram(vfProgram);

    return vfProgram;
}

void init(GLFWwindow* window) {
    // Vertex Shader
    const char* vShaderSource =
        "#version 330 \n"
        "layout (location = 0) in vec2 aPos; \n"
        "uniform vec2 u_center; \n"
        "uniform vec2 u_size; \n"
        "void main(void) { \n"
        "    vec2 pixelPos = u_center + aPos * (u_size / 2.0); \n"
        "    vec2 ndcPos = (pixelPos / 300.0) - 1.0; \n"
        "    gl_Position = vec4(ndcPos, 0.0, 1.0); \n"
        "}";

    // Fragment Shader a körhöz
    const char* fShaderSourceCircle =
        "#version 330 \n"
        "out vec4 color; \n"
        "uniform vec2 u_center; \n"
        "uniform int u_colorSwap; \n"
        "void main(void) { \n"
        "    float dist = distance(gl_FragCoord.xy, u_center); \n"
        "    if (dist > 50.0) discard; \n"
        "    float t = dist / 50.0; \n"
        "    vec3 colorCenter = vec3(1.0, 0.0, 0.0); \n"
        "    vec3 colorEdge   = vec3(0.0, 1.0, 0.0); \n"
        "    if (u_colorSwap == 1) { \n"
        "        colorCenter = vec3(0.0, 1.0, 0.0); \n"
        "        colorEdge   = vec3(1.0, 0.0, 0.0); \n"
        "    } \n"
        "    color = vec4(mix(colorCenter, colorEdge, t), 1.0); \n"
        "}";

    // Fragment Shader a szakaszhoz
    const char* fShaderSourceLine =
        "#version 330 \n"
        "out vec4 color; \n"
        "void main(void) { \n"
        "    color = vec4(0.0, 0.0, 1.0, 1.0); \n"
        "}";

    renderingProgramCircle = createShaderProgram(vShaderSource, fShaderSourceCircle);
    renderingProgramLine = createShaderProgram(vShaderSource, fShaderSourceLine);

    // VAO és VBO generálás
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);

    glGenBuffers(numVBOs, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    float quadVertices[] = {
        -1.0f, -1.0f,   1.0f, -1.0f,   -1.0f, 1.0f,
         1.0f, -1.0f,   1.0f,  1.0f,   -1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void display(GLFWwindow* window, double currentTime) {
    // Fizika és logika
    if (cx + dx + 50.0f >= 600.0f) {
        float overshoot = (cx + dx + 50.0f) - 600.0f;
        cx = 600.0f - 50.0f - overshoot;
        dx = -dx;
    }
    else if (cx + dx - 50.0f <= 0.0f) {
        float overshoot = 0.0f - (cx + dx - 50.0f);
        cx = 50.0f + overshoot;
        dx = -dx;
    }
    else {
        cx += dx;
    }

    if (cy + dy + 50.0f >= 600.0f) {
        float overshoot = (cy + dy + 50.0f) - 600.0f;
        cy = 600.0f - 50.0f - overshoot;
        dy = -dy;
    }
    else if (cy + dy - 50.0f <= 0.0f) {
        float overshoot = 0.0f - (cy + dy - 50.0f);
        cy = 50.0f + overshoot;
        dy = -dy;
    }
    else {
        cy += dy;
    }

    // Metszésvizsgálat a színcseréhez
    float closestX = std::max(200.0f, std::min(cx, 400.0f));
    float closestY = lineY;
    float distToLineSq = (cx - closestX) * (cx - closestX) + (cy - closestY) * (cy - closestY);
    bool isIntersecting = distToLineSq <= (50.0f * 50.0f);

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao[0]);

    // Szakasz rajzolása
    glUseProgram(renderingProgramLine);
    glUniform2f(glGetUniformLocation(renderingProgramLine, "u_center"), 300.0f, lineY);
    glUniform2f(glGetUniformLocation(renderingProgramLine, "u_size"), 200.0f, 3.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Kör rajzolása
    glUseProgram(renderingProgramCircle);
    glUniform2f(glGetUniformLocation(renderingProgramCircle, "u_center"), cx, cy);
    glUniform2f(glGetUniformLocation(renderingProgramCircle, "u_size"), 100.0f, 100.0f);
    glUniform1i(glGetUniformLocation(renderingProgramCircle, "u_colorSwap"), isIntersecting ? 0 : 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Bemenet kezelése
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            lineY += 5.0f;
        }
        else if (key == GLFW_KEY_DOWN) {
            lineY -= 5.0f;
        }
        else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            float angle = 25.0f * (3.14159265f / 180.0f);
            dx = 10.0f * cos(angle);
            dy = 10.0f * sin(angle);
        }
    }
}

int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "Szamitogepes Grafika Beadando1", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
    glfwSwapInterval(1); // V-Sync

    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Tisztítás
    glDeleteVertexArrays(numVAOs, vao);
    glDeleteBuffers(numVBOs, vbo);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}