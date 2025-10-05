#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <cstring>

enum Mode { SCALE, ROTATE, TRANSLATE };
Mode currentMode = ROTATE;

// transformation parameters
float scaleVal = 1.0f;
float rotX = 0.0f, rotY = 0.0f;
float transX = 0.0f, transY = 0.0f, transZ = -3.0f;

const char* vertexShaderSource = R"(
#version 130
in vec3 vPos;
in vec3 vColor;
out vec3 ourColor;
uniform mat4 transform;
uniform mat4 projection;
void main() {
    gl_Position = projection * transform * vec4(vPos, 1.0);
    ourColor = vColor;
}
)";

const char* fragmentShaderSource = R"(
#version 130
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

GLuint createShaderProgram(const char* vsrc, const char* fsrc) {
    GLint success; GLchar infoLog[512];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(vs,512,nullptr,infoLog); std::cerr<<"VS error:\n"<<infoLog<<std::endl; }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(fs,512,nullptr,infoLog); std::cerr<<"FS error:\n"<<infoLog<<std::endl; }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) { glGetProgramInfoLog(prog,512,nullptr,infoLog); std::cerr<<"Link error:\n"<<infoLog<<std::endl; }
    glDeleteShader(vs); glDeleteShader(fs); return prog;
}

void multMatrix(float* a, const float* b) {
    float r[16];
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++) {
            r[i*4+j]=0;
            for (int k=0;k<4;k++)
                r[i*4+j]+=a[i*4+k]*b[k*4+j];
        }
    std::memcpy(a,r,16*sizeof(float));
}

void makeTransform(float* m) {
    // identity
    std::memset(m, 0, 16*sizeof(float));
    m[0]=m[5]=m[10]=m[15]=1.0f;

    // Scale
    float S[16]={scaleVal,0,0,0, 0,scaleVal,0,0, 0,0,scaleVal,0, 0,0,0,1};
    multMatrix(m,S);

    // Rotate X
    float cx=cos(rotX), sx=sin(rotX);
    float Rx[16]={1,0,0,0, 0,cx,sx,0, 0,-sx,cx,0, 0,0,0,1};
    multMatrix(m,Rx);

    // Rotate Y
    float cy=cos(rotY), sy=sin(rotY);
    float Ry[16]={cy,0,-sy,0, 0,1,0,0, sy,0,cy,0, 0,0,0,1};
    multMatrix(m,Ry);

    // Translate
    float T[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, transX,transY,transZ,1};
    multMatrix(m,T);
}

// keyboard input
void key_callback(GLFWwindow*, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(glfwGetCurrentContext(), true); break;
        case GLFW_KEY_M:
            currentMode = (Mode)((currentMode + 1) % 3);
            std::cout << "Mode: " << (currentMode==SCALE?"Scale":currentMode==ROTATE?"Rotate":"Translate") << std::endl;
            break;
        case GLFW_KEY_UP:
            if (currentMode==SCALE) scaleVal += 0.1f;
            else if (currentMode==ROTATE) rotX += 0.1f;
            else if (currentMode==TRANSLATE) transY += 0.1f;
            break;
        case GLFW_KEY_DOWN:
            if (currentMode==SCALE) scaleVal -= 0.1f;
            else if (currentMode==ROTATE) rotX -= 0.1f;
            else if (currentMode==TRANSLATE) transY -= 0.1f;
            break;
        case GLFW_KEY_LEFT:
            if (currentMode==ROTATE) rotY -= 0.1f;
            else if (currentMode==TRANSLATE) transX -= 0.1f;
            break;
        case GLFW_KEY_RIGHT:
            if (currentMode==ROTATE) rotY += 0.1f;
            else if (currentMode==TRANSLATE) transX += 0.1f;
            break;
        case GLFW_KEY_EQUAL: case GLFW_KEY_KP_ADD:
            if (currentMode==TRANSLATE) transZ += 0.1f;
            break;
        case GLFW_KEY_MINUS: case GLFW_KEY_KP_SUBTRACT:
            if (currentMode==TRANSLATE) transZ -= 0.1f;
            break;
    }
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
    GLFWwindow* window = glfwCreateWindow(600,600,"Interactive Cube",nullptr,nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glfwSetKeyCallback(window,key_callback);

    GLuint program = createShaderProgram(vertexShaderSource,fragmentShaderSource);
    glUseProgram(program);

    float vertices[]={
        -0.5f,-0.5f,-0.5f, 1,0,0,
         0.5f,-0.5f,-0.5f, 0,1,0,
         0.5f, 0.5f,-0.5f, 0,0,1,
        -0.5f, 0.5f,-0.5f, 1,1,0,
        -0.5f,-0.5f, 0.5f, 1,0,1,
         0.5f,-0.5f, 0.5f, 0,1,1,
         0.5f, 0.5f, 0.5f, 1,1,1,
        -0.5f, 0.5f, 0.5f, 0,0,0
    };
    unsigned int indices[]={0,1,2,2,3,0,1,5,6,6,2,1,5,4,7,7,6,5,4,0,3,3,7,4,3,2,6,6,7,3,4,5,1,1,0,4};

    GLuint VAO,VBO,EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    GLint transformLoc=glGetUniformLocation(program,"transform");
    GLint projLoc=glGetUniformLocation(program,"projection");
    float aspect=1.0f, fov=1.0f/tan(45.0f*3.14159f/360.0f);
    float proj[16]={fov/aspect,0,0,0, 0,fov,0,0, 0,0,-1,-1, 0,0,-0.2,0};
    glUniformMatrix4fv(projLoc,1,GL_FALSE,proj);
    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window)){
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float m[16]; makeTransform(m);
        glUniformMatrix4fv(transformLoc,1,GL_FALSE,m);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

