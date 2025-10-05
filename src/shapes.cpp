#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>

struct Color { float r,g,b; };
bool paused = false;
int selectedShape = 0;

Color colors[3] = {
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f}
};

const char* vertexShaderSrc = R"(
#version 130
attribute vec2 vPos;
attribute vec3 vColor;
varying vec3 ourColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(vPos, 0.0, 1.0);
    ourColor = vColor;
}
)";

const char* fragmentShaderSrc = R"(
#version 130
varying vec3 ourColor;
void main() {
    gl_FragColor = vec4(ourColor, 1.0);
}
)";

GLuint createProgram() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);
    GLint ok;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) { char buf[512]; glGetShaderInfoLog(vs,512,nullptr,buf); std::cerr<<"VS compile: "<<buf<<"\n"; }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) { char buf[512]; glGetShaderInfoLog(fs,512,nullptr,buf); std::cerr<<"FS compile: "<<buf<<"\n"; }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "vPos");
    glBindAttribLocation(prog, 1, "vColor");
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char buf[512]; glGetProgramInfoLog(prog,512,nullptr,buf); std::cerr<<"Link: "<<buf<<"\n"; }

    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

void buildCircle(std::vector<float>& verts, int segments, float r, const Color &c) {
    verts.clear();
    verts.push_back(0.0f); verts.push_back(0.0f);
    verts.push_back(c.r); verts.push_back(c.g); verts.push_back(c.b);
    for (int i=0;i<=segments;i++){
        float theta = 2.0f * M_PI * float(i) / float(segments);
        verts.push_back(r * cos(theta));
        verts.push_back(r * sin(theta));
        verts.push_back(c.r); verts.push_back(c.g); verts.push_back(c.b);
    }
}

void makeTransform(float *m, float tx, float ty, float scale, float angle) {
    float c = cos(angle), s = sin(angle);
    m[0] = scale * c; m[1] = scale * s; m[2] = 0; m[3] = 0;
    m[4] = -scale * s; m[5] = scale * c; m[6] = 0; m[7] = 0;
    m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
    m[12] = tx; m[13] = ty; m[14] = 0; m[15] = 1;
}

int main() {
    srand((unsigned)time(nullptr));
    if (!glfwInit()) { std::cerr<<"GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
    GLFWwindow* w = glfwCreateWindow(800,600,"Shapes - Controls Demo", nullptr, nullptr);
    if (!w) { std::cerr<<"Create window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr<<"GLAD init failed\n"; return -1; }

    GLuint program = createProgram();
    glUseProgram(program);
    GLint transformLoc = glGetUniformLocation(program, "transform");

    // Triangle (3 vertices, each 2 pos + 3 color)
    float triVerts[15] = {
         0.0f,  0.25f, colors[0].r, colors[0].g, colors[0].b,
        -0.25f, -0.25f, colors[0].r, colors[0].g, colors[0].b,
         0.25f, -0.25f, colors[0].r, colors[0].g, colors[0].b
    };
    GLuint VBOtri; glGenBuffers(1, &VBOtri);

    // Square (4 vertices - triangle fan)
    float sqVerts[20] = {
        -0.25f, -0.25f, colors[2].r, colors[2].g, colors[2].b,
         0.25f, -0.25f, colors[2].r, colors[2].g, colors[2].b,
         0.25f,  0.25f, colors[2].r, colors[2].g, colors[2].b,
        -0.25f,  0.25f, colors[2].r, colors[2].g, colors[2].b
    };
    GLuint VBOsq; glGenBuffers(1, &VBOsq);

    // Circle dynamic
    std::vector<float> circle;
    buildCircle(circle, 64, 0.18f, colors[1]);
    GLuint VBOcircle; glGenBuffers(1, &VBOcircle);

    // input state (previous) so we detect single press events
    bool prev1=false, prev2=false, prev3=false, prevC=false, prevSpace=false;

    double timeAccum = 0.0;

    // enable blending for visuals
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // main loop
    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();

        // handle input polling (reliable)
        bool k1 = glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS;
        if (k1 && !prev1) { selectedShape = 0; std::cout<<"Selected: TRIANGLE\n"; }
        prev1 = k1;

        bool k2 = glfwGetKey(w, GLFW_KEY_2) == GLFW_PRESS;
        if (k2 && !prev2) { selectedShape = 1; std::cout<<"Selected: CIRCLE\n"; }
        prev2 = k2;

        bool k3 = glfwGetKey(w, GLFW_KEY_3) == GLFW_PRESS;
        if (k3 && !prev3) { selectedShape = 2; std::cout<<"Selected: SQUARE\n"; }
        prev3 = k3;

        bool kc = glfwGetKey(w, GLFW_KEY_C) == GLFW_PRESS;
        if (kc && !prevC) {
            // randomize color for selected shape
            colors[selectedShape].r = (float)rand() / RAND_MAX;
            colors[selectedShape].g = (float)rand() / RAND_MAX;
            colors[selectedShape].b = (float)rand() / RAND_MAX;
            std::cout<<"Color changed for shape "<<selectedShape<<" -> "
                     <<colors[selectedShape].r<<","<<colors[selectedShape].g<<","<<colors[selectedShape].b<<"\n";
        }
        prevC = kc;

        bool ks = glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (ks && !prevSpace) { paused = !paused; std::cout << (paused? "Paused\n":"Resumed\n"); }
        prevSpace = ks;

        // update animation time if not paused
        if (!paused) timeAccum += 0.016; // ~60Hz step (deterministic)

        // prepare transforms and dynamic buffers
        glClearColor(0.95f,0.95f,0.95f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TRIANGLE - left area
        // update tri colors in CPU array
        for (int v=0; v<3; ++v) {
            triVerts[v*5 + 2] = colors[0].r;
            triVerts[v*5 + 3] = colors[0].g;
            triVerts[v*5 + 4] = colors[0].b;
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBOtri);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triVerts), triVerts, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        float triM[16]; makeTransform(triM, -0.6f, 0.0f, 1.0f, (float)timeAccum);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, triM);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // CIRCLE - center area, dynamic VBO
        buildCircle(circle, 64, 0.18f, colors[1]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOcircle);
        glBufferData(GL_ARRAY_BUFFER, circle.size()*sizeof(float), circle.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        float bounceY = 0.35f * sin((float)timeAccum*1.2f);
        float circM[16]; makeTransform(circM, 0.0f, bounceY, 1.0f, 0.0f);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, circM);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(circle.size()/5));

        // SQUARE - right area
        for (int v=0; v<4; ++v) {
            sqVerts[v*5 + 2] = colors[2].r;
            sqVerts[v*5 + 3] = colors[2].g;
            sqVerts[v*5 + 4] = colors[2].b;
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBOsq);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sqVerts), sqVerts, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        float scale = 0.9f + 0.15f * sin((float)timeAccum * 1.6f);
        float sqM[16]; makeTransform(sqM, 0.6f, 0.0f, scale, 0.0f);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, sqM);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // helpful console hint
        //std::cout << "Frame\n";

        glfwSwapBuffers(w);
    }

    // cleanup
    glDeleteBuffers(1, &VBOtri);
    glDeleteBuffers(1, &VBOcircle);
    glDeleteBuffers(1, &VBOsq);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
}

