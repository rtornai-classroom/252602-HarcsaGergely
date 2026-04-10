#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


GLuint renderingProgram;
GLuint pointProgram;
GLuint vao[1];
GLuint vbo[2];


std::vector<glm::vec2> controlPoints = {
    {100.0f, 300.0f},
    {300.0f, 500.0f},
    {500.0f, 100.0f},
    {700.0f, 300.0f}
};


int draggedPointIndex = -1;
std::vector<float> bezierVertices;


// Shaderek
const char* vShaderSource =
"#version 330 \n"
"layout (location = 0) in vec2 pos; \n"
"uniform mat4 mvp_matrix; \n"
"void main(void) { \n"
"    gl_Position = mvp_matrix * vec4(pos, 0.0, 1.0); \n"
"    gl_PointSize = 8.0; \n"
"}";


const char* fShaderSourceLine =
"#version 330 \n"
"out vec4 color; \n"
"uniform vec4 u_color; \n"
"void main(void) { \n"
"    color = u_color; \n"
"}";


const char* fShaderSourcePoint =
"#version 330 \n"
"out vec4 color; \n"
"uniform vec4 u_color; \n"
"void main(void) { \n"
"    vec2 coord = gl_PointCoord - vec2(0.5); \n"
"    if (length(coord) > 0.5) discard; \n"
"    color = u_color; \n"
"}";


GLuint createProgram(const char* vSource, const char* fSource) {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vShader, 1, &vSource, NULL);
    glShaderSource(fShader, 1, &fSource, NULL);
    glCompileShader(vShader);
    glCompileShader(fShader);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glLinkProgram(prog);
    return prog;
}


glm::vec2 getCubicBezierPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    return (uuu * p0) + (3 * uu * t * p1) + (3 * u * tt * p2) + (ttt * p3);
}


void updateBezierCurve(int segments) {
    bezierVertices.clear();
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        glm::vec2 point = getCubicBezierPoint(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3], t);
        bezierVertices.push_back(point.x);
        bezierVertices.push_back(point.y);
    }
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            float mx = (float)xpos;
            float my = 600.0f - (float)ypos;

            for (int i = 0; i < 4; ++i) {
                if (glm::distance(glm::vec2(mx, my), controlPoints[i]) < 10.0f) {
                    draggedPointIndex = i;
                    break;
                }
            }
        }
        else if (action == GLFW_RELEASE) {
            draggedPointIndex = -1;
        }
    }
}


// vezérlés
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (draggedPointIndex != -1) {
        controlPoints[draggedPointIndex].x = (float)xpos;
        controlPoints[draggedPointIndex].y = 600.0f - (float)ypos;
    }
}


void init(GLFWwindow* window) {
    renderingProgram = createProgram(vShaderSource, fShaderSourceLine);
    pointProgram = createProgram(vShaderSource, fShaderSourcePoint);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(2, vbo);
}


void display(GLFWwindow* window, double currentTime) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 pMat = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    glm::mat4 mMat = glm::mat4(1.0f);
    glm::mat4 mvp = pMat * mMat;

    updateBezierCurve(100);

    // görbe
    glUseProgram(renderingProgram);
    glUniformMatrix4fv(glGetUniformLocation(renderingProgram, "mvp_matrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(glGetUniformLocation(renderingProgram, "u_color"), 1.0f, 1.0f, 1.0f, 1.0f); // Fehér

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, bezierVertices.size() * sizeof(float), bezierVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINE_STRIP, 0, bezierVertices.size() / 2);

    // kontrollpontok
    glUseProgram(pointProgram);
    glUniformMatrix4fv(glGetUniformLocation(pointProgram, "mvp_matrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(glGetUniformLocation(pointProgram, "u_color"), 1.0f, 0.0f, 0.0f, 1.0f);

    float cpData[8] = {
        controlPoints[0].x, controlPoints[0].y,
        controlPoints[1].x, controlPoints[1].y,
        controlPoints[2].x, controlPoints[2].y,
        controlPoints[3].x, controlPoints[3].y
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cpData), cpData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_POINTS, 0, 4);
}


int main(void) {
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Szamitogepes Grafika Beadando 2", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    init(window);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}