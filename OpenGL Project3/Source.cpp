#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint cubeProgram, sunProgram;
GLuint vao[2];
GLuint vbo[4];
GLuint sunTexture;


float camRadius = 8.0f;
float camAngle = 0.0f;
float camZ = 0.0f;

bool isLightOn = true;
glm::vec3 lightColor(1.0f, 0.6f, 0.0f);
float lightAngle = 0.0f;

// --- SHADEREK ---

// Kocka Vertex Shader elõkészítés
const char* vCubeShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNorm;
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
)";

// Kocka Fragment Shader
const char* fCubeShader = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform int lightOn;
uniform vec3 objectColor;

void main() {
    if (lightOn == 1) {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        vec3 ambient = 0.2 * lightColor;
        
        vec3 result = (ambient + diffuse) * objectColor;
        FragColor = vec4(result, 1.0);
    } else {
        FragColor = vec4(objectColor * 0.1, 1.0); 
    }
}
)";

// 2. Nap Vertex Shader
const char* vSunShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    TexCoord = aTex;
    gl_Position = proj * view * vec4(aPos, 1.0);
}
)";

// Nap Fragment Shader
const char* fSunShader = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texMap;
uniform vec3 sunColor;

void main() {
    vec4 texColor = texture(texMap, TexCoord);
    FragColor = texColor * vec4(sunColor, 1.0);
}
)";


// Gombnyomások (Kamera és Világítás kapcsoló)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_RIGHT) camAngle -= 0.05f; // Forgatás Z körül
        if (key == GLFW_KEY_LEFT)  camAngle += 0.05f;
        if (key == GLFW_KEY_UP)    camZ += 0.5f;      // Kamera fel
        if (key == GLFW_KEY_DOWN)  camZ -= 0.5f;      // Kamera le
        if (key == GLFW_KEY_L && action == GLFW_PRESS) {
            isLightOn = !isLightOn; // L betûre világítás kapcsolása
        }
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    }
}

GLuint compileShader(const char* vSource, const char* fSource) {
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

GLuint loadTexture() {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    unsigned char dummyData[] = { 255, 255, 200, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, dummyData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texID;
}

void init(GLFWwindow* window) {
    cubeProgram = compileShader(vCubeShader, fCubeShader);
    sunProgram = compileShader(vSunShader, fSunShader);

    sunTexture = loadTexture();

    glGenVertexArrays(2, vao);
    glGenBuffers(4, vbo);

    // KOCKA
    float cubeVerts[] = {
        // Hátsó lap
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        // Elsõ lap
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        // Bal lap
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        // Jobb lap
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        // Alsó lap
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        // Felsõ lap
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // GÖMB GENERÁLÁSA (Nap)
    std::vector<float> sphereVerts;
    int prec = 48;
    for (int i = 0; i <= prec; i++) {
        for (int j = 0; j <= prec; j++) {
            float y = (float)cos(glm::radians(180.0f - i * 180.0f / prec));
            float x = -(float)cos(glm::radians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
            float z = (float)sin(glm::radians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
            float u = (float)j / prec;
            float v = (float)i / prec;

            sphereVerts.push_back(x * 0.25f);
            sphereVerts.push_back(y * 0.25f);
            sphereVerts.push_back(z * 0.25f);
            sphereVerts.push_back(u);
            sphereVerts.push_back(v);
        }
    }

    std::vector<int> sphereIndices;
    for (int i = 0; i < prec; i++) {
        for (int j = 0; j < prec; j++) {
            sphereIndices.push_back(i * (prec + 1) + j);
            sphereIndices.push_back(i * (prec + 1) + j + 1);
            sphereIndices.push_back((i + 1) * (prec + 1) + j);
            sphereIndices.push_back(i * (prec + 1) + j + 1);
            sphereIndices.push_back((i + 1) * (prec + 1) + j + 1);
            sphereIndices.push_back((i + 1) * (prec + 1) + j);
        }
    }
    std::vector<float> finalSphere;
    for (int idx : sphereIndices) {
        finalSphere.push_back(sphereVerts[idx * 5 + 0]);
        finalSphere.push_back(sphereVerts[idx * 5 + 1]);
        finalSphere.push_back(sphereVerts[idx * 5 + 2]);
        finalSphere.push_back(sphereVerts[idx * 5 + 3]);
        finalSphere.push_back(sphereVerts[idx * 5 + 4]);
    }

    glBindVertexArray(vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, finalSphere.size() * sizeof(float), finalSphere.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);
}

void display(GLFWwindow* window, double currentTime) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // KAMERA BEÁLLÍTÁSA
    float camX = camRadius * cos(camAngle);
    float camY = camRadius * sin(camAngle);

    glm::vec3 cameraPos(camX, camY, camZ);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 0.0f, 1.0f);

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // Vetítés: 55 fok
    glm::mat4 proj = glm::perspective(glm::radians(55.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // FÉNYFORRÁS MOZGATÁSA
    lightAngle += 0.01f;
    //float currentLightAngle = (float)glfwGetTime() * 1.5f;
    float lightRadius = 2.0f * camRadius;
    
    glm::vec3 currentLightPos(lightRadius * cos(lightAngle), lightRadius * sin(lightAngle), 0.0f);

    // KOCKÁK KIRAJZOLÁSA
    glUseProgram(cubeProgram);
    glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(cubeProgram, "lightPos"), 1, glm::value_ptr(currentLightPos));
    glUniform3fv(glGetUniformLocation(cubeProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform1i(glGetUniformLocation(cubeProgram, "lightOn"), isLightOn ? 1 : 0);
    glUniform3f(glGetUniformLocation(cubeProgram, "objectColor"), 1.0f, 1.0f, 1.0f);

    glBindVertexArray(vao[0]);


    // Középsõ kocka
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Felsõ kocka
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Alsó kocka
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
    glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);


    // NAP KIRAJZOLÁSA
    glUseProgram(sunProgram);
    glUniformMatrix4fv(glGetUniformLocation(sunProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sunProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(sunProgram, "sunColor"), 1, glm::value_ptr(lightColor));

    // Textúra bekötése
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glUniform1i(glGetUniformLocation(sunProgram, "texMap"), 0);

    // A gömböt a fényforrás aktuális helyére toljuk
    model = glm::translate(glm::mat4(1.0f), currentLightPos);
    glUniformMatrix4fv(glGetUniformLocation(sunProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, 48 * 48 * 6);
}

int main(void) {
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Harcsa Gergely Sándor", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}