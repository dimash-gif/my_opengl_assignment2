#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

const float PI = 3.14159265358979323846f;
const int MAIN_W = 700, MAIN_H = 700;
const int SUB_W  = 360, SUB_H  = 360;
const int W2_W   = 500, W2_H   = 500;

bool animate = true;
float zebraAngle = 0.0f, triAngle = 0.0f, timeAccumulator = 0.0f;
int mainSquareColorMode = -1;

float subBgR = 0.2f, subBgG = 0.2f, subBgB = 0.5f;
float w2_R = 1.0f, w2_G = 1.0f, w2_B = 1.0f;

// ----------------- Shaders -----------------
const char* vertexShaderSrc = R"(
#version 130
in vec2 aPos;
in vec3 aColor;
out vec3 vColor;
uniform vec2 offset;
uniform float scale;
uniform float angle;
uniform int useOverride;
uniform vec3 overrideColor;

void main() {
    float c = cos(angle);
    float s = sin(angle);
    mat2 R = mat2(c, -s, s, c);
    vec2 p = R * (aPos * scale) + offset;
    gl_Position = vec4(p, 0.0, 1.0);
    if (useOverride == 1) vColor = overrideColor;
    else vColor = aColor;
}
)";

const char* fragmentShaderSrc = R"(
#version 130
in vec3 vColor;
out vec4 FragColor;
void main() { FragColor = vec4(vColor, 1.0); }
)";

// ----------------- Helper Structures -----------------
struct Mesh { GLuint VBO=0; GLsizei vertexCount=0; };

static GLuint compileProgram(const char* vsSrc, const char* fsSrc) {
    auto compile = [](GLenum type, const char* src)->GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, NULL);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char buf[512]; glGetShaderInfoLog(s, 512, NULL, buf);
            std::cerr << "Shader compile error: " << buf << std::endl;
        }
        return s;
    };
    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[512]; glGetProgramInfoLog(prog, 512, NULL, buf);
        std::cerr << "Program link error: " << buf << std::endl;
    }
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

static Mesh makeMesh(const std::vector<float>& data) {
    Mesh m;
    glGenBuffers(1, &m.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STATIC_DRAW);
    m.vertexCount = static_cast<GLsizei>(data.size()/5);
    return m;
}

// ----------------- Shape Builders -----------------
static void buildZebra(std::vector<float>& out, int layers=8) {
    out.clear();
    float max = 0.9f;
    float step = max / layers;
    for (int i=0; i<layers; i++){
        float s = max - i*step;
        float c = (i % 2 == 0) ? 1.0f : 0.0f;
        float quad[] = {
            -s, -s, c, c, c,   s, -s, c, c, c,   s,  s, c, c, c,
            -s, -s, c, c, c,   s,  s, c, c, c,  -s,  s, c, c, c
        };
        out.insert(out.end(), quad, quad+30);
    }
}

static void buildEllipse(std::vector<float>& out, int seg=64, float rx=0.5f, float ry=0.3f) {
    out.clear();
    out.insert(out.end(), {0.0f, 0.0f, 1.0f, 0.5f, 0.0f});
    for (int i=0; i<=seg; i++){
        float t = 2.0f * PI * i / seg;
        out.insert(out.end(), {rx * cosf(t), ry * sinf(t), 1.0f, 0.5f, 0.0f});
    }
}

static void buildCircle(std::vector<float>& out, int seg=64, float r=1.0f) {
    out.clear();
    out.insert(out.end(), {0.0f, 0.0f, 1.0f, 1.0f, 1.0f});
    for (int i=0; i<=seg; i++){
        float t = 2.0f * PI * i / seg;
        out.insert(out.end(), {r * cosf(t), r * sinf(t), 1.0f, 1.0f, 1.0f});
    }
}

static void buildTriangle(std::vector<float>& out) {
    out.clear();
    out = {
         0.0f,  0.2f,  1.0f, 1.0f, 1.0f,
        -0.2f, -0.2f,  1.0f, 1.0f, 1.0f,
         0.2f, -0.2f,  1.0f, 1.0f, 1.0f
    };
}

// ----------------- Globals -----------------
GLuint program = 0;
GLint locOffset, locScale, locAngle, locUseOverride, locOverrideColor;
Mesh zebraMesh, ellipseMesh, circleMesh, triangleMesh;
GLFWwindow *mainWin=nullptr, *subWin=nullptr, *win2=nullptr;

// ----------------- Input Callbacks -----------------
void main_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
        std::cout << "\nRight-click menu:\n(A) Start animation\n(S) Stop animation\n(W) White square\n(R) Red square\n(G) Green square\n";
    }
}

void main_key_callback(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_A: animate = true; break;
        case GLFW_KEY_S: animate = false; break;
        case GLFW_KEY_W: mainSquareColorMode = 0; break;
        case GLFW_KEY_R: mainSquareColorMode = 1; break;
        case GLFW_KEY_G: mainSquareColorMode = 2; break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(w, true); break;
    }
}

void sub_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (subBgR > 0.7f) { subBgR = 0.2f; subBgG = 0.8f; subBgB = 0.2f; } 
        else if (subBgG > 0.7f) { subBgR = 0.2f; subBgG = 0.2f; subBgB = 0.5f; } 
        else { subBgR = 0.8f; subBgG = 0.2f; subBgB = 0.2f; }
    }
}

void win2_key_callback(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    switch(key) {
        case GLFW_KEY_R: w2_R=1; w2_G=0; w2_B=0; break;
        case GLFW_KEY_G: w2_R=0; w2_G=1; w2_B=0; break;
        case GLFW_KEY_B: w2_R=0; w2_G=0; w2_B=1; break;
        case GLFW_KEY_Y: w2_R=1; w2_G=1; w2_B=0; break;
        case GLFW_KEY_O: w2_R=1; w2_G=0.5f; w2_B=0; break;
        case GLFW_KEY_P: w2_R=0.6f; w2_G=0.2f; w2_B=0.8f; break;
        case GLFW_KEY_W: w2_R=1; w2_G=1; w2_B=1; break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(mainWin, true); break;
    }
}

// ----------------- Utility + Drawing -----------------
void setUniforms(float ox, float oy, float scale, float angle, bool useOverride, float r, float g, float b) {
    glUniform2f(locOffset, ox, oy);
    glUniform1f(locScale, scale);
    glUniform1f(locAngle, angle);
    glUniform1i(locUseOverride, useOverride ? 1 : 0);
    glUniform3f(locOverrideColor, r, g, b);
}

void drawShape(const Mesh& m, GLenum mode = GL_TRIANGLES) {
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(mode, 0, m.vertexCount);
}

// ----------------- Rendering -----------------
void renderMain() {
    glfwMakeContextCurrent(mainWin);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);

    bool useOverride = (mainSquareColorMode >= 0);
    float r=0,g=0,b=0;
    if (mainSquareColorMode==0){r=1;g=1;b=1;}
    else if(mainSquareColorMode==1){r=1;g=0;b=0;}
    else if(mainSquareColorMode==2){r=0;g=1;b=0;}

    setUniforms(0,0,0.6f,zebraAngle,useOverride,r,g,b);
    drawShape(zebraMesh);

    glfwSwapBuffers(mainWin);
}

void renderSub() {
    glfwMakeContextCurrent(subWin);
    glClearColor(subBgR, subBgG, subBgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    setUniforms(0,0,0.8f,0,false,0,0,0);
    drawShape(ellipseMesh,GL_TRIANGLE_FAN);
    glfwSwapBuffers(subWin);
}

void renderWin2() {
    glfwMakeContextCurrent(win2);
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);

    float circleScale = 0.3f + 0.15f * sinf(timeAccumulator * 1.5f);
    setUniforms(-0.4f, 0.0f, circleScale, 0.0f, true, w2_R, w2_G, w2_B);
    drawShape(circleMesh, GL_TRIANGLE_FAN);

    setUniforms(0.4f, 0.0f, 1.0f, triAngle, true, w2_R, w2_G, w2_B);
    drawShape(triangleMesh);

    glfwSwapBuffers(win2);
}

// ----------------- Main -----------------
int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    mainWin = glfwCreateWindow(MAIN_W, MAIN_H, "Main Window", NULL, NULL);
    if (!mainWin) return -1;
    glfwMakeContextCurrent(mainWin);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    program = compileProgram(vertexShaderSrc, fragmentShaderSrc);
    locOffset = glGetUniformLocation(program, "offset");
    locScale = glGetUniformLocation(program, "scale");
    locAngle = glGetUniformLocation(program, "angle");
    locUseOverride = glGetUniformLocation(program, "useOverride");
    locOverrideColor = glGetUniformLocation(program, "overrideColor");

    std::vector<float> tmp;
    buildZebra(tmp); zebraMesh = makeMesh(tmp);
    buildEllipse(tmp); ellipseMesh = makeMesh(tmp);
    buildCircle(tmp); circleMesh = makeMesh(tmp);
    buildTriangle(tmp); triangleMesh = makeMesh(tmp);

    subWin = glfwCreateWindow(SUB_W, SUB_H, "Sub-Window", NULL, mainWin);
    win2   = glfwCreateWindow(W2_W, W2_H, "Window 2", NULL, mainWin);

    glfwSetMouseButtonCallback(mainWin, main_mouse_callback);
    glfwSetKeyCallback(mainWin, main_key_callback);
    glfwSetMouseButtonCallback(subWin, sub_mouse_callback);
    glfwSetKeyCallback(win2, win2_key_callback);

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(mainWin)) {
        double currentTime = glfwGetTime();
        double dt = currentTime - lastTime;
        lastTime = currentTime;
        if (animate) {
            timeAccumulator += dt;
            zebraAngle += 0.8f * dt;
            triAngle -= 1.2f * dt;
        }
        glfwPollEvents();
        renderMain();
        if (subWin && !glfwWindowShouldClose(subWin)) renderSub();
        if (win2 && !glfwWindowShouldClose(win2)) renderWin2();
    }

    glfwTerminate();
    return 0;
}

