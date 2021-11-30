// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

// Timer
#include <chrono>

std::vector<VertexArrayObject> VAOs;
std::vector<VertexBufferObject> VBOs;
std::vector<VertexBufferObject> NBOs; // plane normals
std::vector<VertexBufferObject> vNBOs; // vertex normals

// Contains the m-v-p matrix
vector<glm::mat4> model;
glm::vec3 cameraPos;
glm::mat4 view;
glm::mat4 projection;

// Light Position
glm::vec3 lightPos;

// For stencil buffer picking
GLbyte color[4];
GLfloat depth;
GLuint index = 0;

// Circular light radius
float light_r = 1.0f;

// shadowColorBlack -> 1 (black)
// shadowColorBlack -> 0 (red)
int shadowColorBlack = 1;

VertexArrayObject skyboxVAO;
VertexBufferObject skyboxVBO;
unsigned int cubemapTexture;

// Render mode
vector<char> renderMode;

// Contains the vertex for a unit cube
static const GLfloat vertex_list[][3] = {
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
};

static const GLint index_list[][3] = {
    0, 1, 3,
    3, 2, 0,
    4, 5, 7,
    7, 6, 4,
    6, 2, 0,
    0, 4, 6,
    7, 3, 1,
    1, 5, 7,
    0, 1, 5,
    5, 4, 0,
    2, 3, 7,
    7, 6, 2
};

static const GLfloat cube_normal[][3] = {
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
};

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

unsigned int loadCubemap(vector<std::string> faces);
void renderScene(Program &program, GLFWwindow* window, glm::mat4 &lightSpaceMatrix, const string &vs, const string &fs, const string &fs_m, const string &fs_r, unsigned int &depthMap);
void renderSkyBox(Program &program, GLFWwindow* window, const string &vs, const string &fs);

void importCube() {
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    VertexBufferObject VBO;
    VertexBufferObject NBO;
    std::vector<glm::vec3> V;
    std::vector<glm::vec3> N;

    VBO.init();
    NBO.init();
    for(int i = 0; i < 12; i ++) {
        glm::vec3 triangle[3];
        for(int j = 0; j < 3; j ++) {
            triangle[j] = glm::vec3(vertex_list[index_list[i][j]][0], vertex_list[index_list[i][j]][1], vertex_list[index_list[i][j]][2]);
            V.push_back(triangle[j]);
        }  
        glm::vec3 normal = glm::vec3(cube_normal[i][0], cube_normal[i][1], cube_normal[i][2]);
        cout << normal[0] << normal[1] << normal[2] << "\n";
        for(int temp = 0; temp < 3; temp ++) {
            N.push_back(normal);
        }
    }
    VBO.update(V);
    NBO.update(N);

    VertexBufferObject vNBO;
    vNBO.init();
    std::vector<glm::vec3> vN;
    for(int i = 0; i < N.size(); i ++) {
        glm::vec3 temp = glm::vec3(0, 0, 0);
        int count = 0;
        for(int j = 0; j < N.size(); j ++) {
            if(V[j] == V[i]) {
                count += 1;
                temp += N[j];
            }
        }
        glm::vec3 avg = glm::vec3(temp[0] / count, temp[1] / count, temp[2] / count);
        avg = glm::normalize(avg);
        vN.push_back(avg);
    }
    vNBO.update(vN);

    VAOs.push_back(VAO);
    VBOs.push_back(VBO);
    NBOs.push_back(NBO);
    vNBOs.push_back(vNBO);

    glm::mat4 trans = glm::mat4(1.f);
    model.push_back(trans);
    renderMode.push_back('p');
}

void importOff(const char* filename) {
    ifstream offFile;

    offFile.open(filename, ios::in);

    if (offFile.is_open()) {
        string line;
        getline(offFile, line);
        stringstream lineStream(line);

        VertexArrayObject VAO;
        VAO.init();
        VAO.bind();

        VertexBufferObject VBO;
        VertexBufferObject NBO;
        std::vector<glm::vec3> V;
        std::vector<glm::vec3> N;

        VBO.init();
        NBO.init();

        if (line == "OFF") {
            getline(offFile, line);
            stringstream ss;
            ss << line;
            
            vector<int> attributes; // vertex_count, face_count, edge_count
            int temp;
            while (ss >> temp) {
                attributes.push_back(temp);
            }

            // store position vertex
            vector<float> vertex;
            for (int i = 0; i < attributes[0]; i ++) {
                getline(offFile, line);
                stringstream ss;
                ss << line;
                float temp;
                while (ss >> temp) {
                    vertex.push_back(temp);
                }
            }
            // center it on the origin
            float minX = vertex[0];
            float minY = vertex[1];
            float minZ = vertex[2];
            float maxX = minX;
            float maxY = minY;
            float maxZ = minZ;
            for (int i = 0; i < vertex.size(); i += 3) {
                float temp = vertex[i];
                if (temp < minX) {
                    minX = temp;
                }
                if (temp > maxX) {
                    maxX = temp;
                }
            }
            for (int i = 1; i < vertex.size(); i += 3) {
                float temp = vertex[i];
                if (temp < minY) {
                    minY = temp;
                }
                if (temp > maxY) {
                    maxY = temp;
                }
            }
            for (int i = 2; i < vertex.size(); i += 3) {
                float temp = vertex[i];
                if (temp < minZ) {
                    minZ = temp;
                }
                if (temp > maxZ) {
                    maxZ = temp;
                }
            }

            float transX = -(maxX + minX)/2.0f;
            float transY = -(maxY + minY)/2.0f;
            float transZ = -(maxZ + minZ)/2.0f;

            for (int i = 0; i < vertex.size(); i += 3) {
                vertex[i] += transX;
                vertex[i + 1] += transY;
                vertex[i + 2] += transZ;
            }

            // scale the object
            float max = 0;
            for (int i = 0; i < vertex.size(); i ++) {
                float temp = vertex[i];
                if (temp < 0) {
                    temp = -temp;
                }
                if (temp >= max) {
                    max = temp;
                }
            }

            float scaleFactor = 0.5f/max;
            for (int i = 0; i < vertex.size(); i ++) {
                vertex[i] *= scaleFactor;
            }

            // get indexes and store positions
            while (getline(offFile, line)) {
                stringstream ss;
                ss << line;
                int temp;
                ss >> temp; // ignore '3'
                while (ss >> temp) {
                    int index = temp * 3;
                    V.push_back(glm::vec3(vertex[index], vertex[index+1], vertex[index+2]));
                }
            }

            for(int i = 0; i < V.size(); i += 3) {
                glm::vec3 triangle[3];
                triangle[0] = V[i];
                triangle[1] = V[i + 1];
                triangle[2] = V[i + 2];

                glm::vec3 normal = cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);
                normal = normalize(normal);
                for(int temp = 0; temp < 3; temp ++) {
                    N.push_back(normal);
                }
            }
        }
        VBO.update(V);
        NBO.update(N);

        VertexBufferObject vNBO;
        vNBO.init();
        std::vector<glm::vec3> vN;
        for(int i = 0; i < N.size(); i ++) {
            glm::vec3 temp = glm::vec3(0, 0, 0);
            int count = 0;
            for(int j = 0; j < N.size(); j ++) {
                if(V[j] == V[i]) {
                    count += 1;
                    temp += N[j];
                }
            }
            glm::vec3 avg = glm::vec3(temp[0] / count, temp[1] / count, temp[2] / count);
            avg = glm::normalize(avg);
            vN.push_back(avg);
        }
        vNBO.update(vN);

        VAOs.push_back(VAO);
        VBOs.push_back(VBO);
        NBOs.push_back(NBO);
        vNBOs.push_back(vNBO);

        glm::mat4 trans = glm::mat4(1.f);
        model.push_back(trans);
        renderMode.push_back('p');

        offFile.close();
    } else {
        cout << "Can not open!";
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    // Get the position of the mouse in the window
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    // Read pixel
    glReadPixels(x, height - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    glReadPixels(x, height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    glReadPixels(x, height - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

    printf("Clicked on pixel %f, %f, color %02hhx%02hhx%02hhx%02hhx, depth %f, stencil index %u\n",
    x, y, color[0], color[1], color[2], color[3], depth, index);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_1:
            if (action == GLFW_PRESS)
                importCube();// add a unit cube in the origin
            break;
        case GLFW_KEY_2:
            if (action == GLFW_PRESS)
                importOff("../data/bumpy_cube.off"); // import a new copy of 'bumpy_cube.off'
            break;
        case GLFW_KEY_3:
            if (action == GLFW_PRESS)
                importOff("../data/bunny.off"); // import a new copy of 'bunny.off'
            break;

        // Translation: w-a-s-d-q-e
        case GLFW_KEY_W:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(0, 0.1, 0));
            break;
        case GLFW_KEY_A:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(-0.1, 0, 0));
            break;
        case GLFW_KEY_S:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(0, -0.1, 0));
            break;
        case GLFW_KEY_D:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(0.1, 0, 0));
            break;
        case GLFW_KEY_Q:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(0, 0, -0.1));
            break;
        case GLFW_KEY_E:
            if (action == GLFW_PRESS)
                model[index] = glm::translate(model[index], glm::vec3(0, 0, 0.1));
            break;

        // Rotation: h & j (around y-axis) y & u (around x-axis) n & m (around z-axis)
        case GLFW_KEY_H:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, 1.0, 0.0));
            break;
        case GLFW_KEY_J:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, -1.0, 0.0));
            break;
        case GLFW_KEY_Y:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(1.0, 0.0, 0.0));
            break;
        case GLFW_KEY_U:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(-1.0, 0.0, 0.0));
            break;
        case GLFW_KEY_N:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, 0.0, 1.0));
            break;
        case GLFW_KEY_M:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, 0.0, -1.0));
            break;

        
        // Rescale: k & l (- & +)
        case GLFW_KEY_K:
            if (action == GLFW_PRESS)
                model[index] = glm::scale(model[index], glm::vec3(0.5));
            break;
        case GLFW_KEY_L:
            if (action == GLFW_PRESS)
                model[index] = glm::scale(model[index], glm::vec3(1.5));
            break;

        // Delete
        case GLFW_KEY_BACKSPACE:
            if (action == GLFW_PRESS){
                cout << index;
                if (index > 0) {
                    VAOs.erase(VAOs.begin() + index);
                    VBOs.erase(VBOs.begin() + index);
                    NBOs.erase(NBOs.begin() + index);
                    vNBOs.erase(vNBOs.begin() + index);
                    model.erase(model.begin() + index);
                    renderMode.erase(renderMode.begin() + index);
                    index = -1;
                }
            }
            break;

        // Camera translation
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS)
                cameraPos += glm::vec3(0.0, 0.1, 0.0);
            break;
        case GLFW_KEY_DOWN:
            if (action == GLFW_PRESS)
                cameraPos += glm::vec3(0.0, -0.1, 0.0);
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS)
                cameraPos += glm::vec3(-0.1, 0.0, 0.0);
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS)
                cameraPos += glm::vec3(0.1, 0.0, 0.0);
            break;

        // Change shadow color between black and red
        case GLFW_KEY_0:
            if (action == GLFW_PRESS)
                shadowColorBlack = 1 - shadowColorBlack;
            break;

        // Render Mode
        // Phong Shading: p
        case GLFW_KEY_P:
            if (action == GLFW_PRESS) {
                if (index > 0)
                    renderMode[index] = 'p';
            }
            break;

        // Mirror Appearance: o
        case GLFW_KEY_O:
            if (action == GLFW_PRESS) {
                if (index > 0)
                    renderMode[index] = 'm';
            }
            break;

        // Refraction: i
        case GLFW_KEY_I:
            if (action == GLFW_PRESS) {
                if (index > 0)
                    renderMode[index] = 'r';
            }
            break;

        default:
            break;
    }
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 640, "Assignment4", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Initialize the VAO & VBO for skybox
    skyboxVAO.init();
    skyboxVAO.bind();

    std::vector<glm::vec3> skybox_V;
    skyboxVBO.init();
    for (int i = 0; i < sizeof(skyboxVertices); i += 3) {
        skybox_V.push_back(glm::vec3(10*skyboxVertices[i], 10*skyboxVertices[i+1], 10*skyboxVertices[i+2]));
    }
    skyboxVBO.update(skybox_V);

    // Initialize the VAO & VBO for plane
    VertexArrayObject planeVAO;
    planeVAO.init();
    planeVAO.bind();

    VertexBufferObject planeVBO;
    std::vector<glm::vec3> V(6);
    planeVBO.init();
    V[0] = glm::vec3(1.0f, -1.0f, 1.0f);
    V[1] = glm::vec3(1.0f, -1.0f, -1.0f);
    V[2] = glm::vec3(-1.0f, -1.0f, -1.0f);
    V[3] = glm::vec3(1.0f, -1.0f, 1.0f);
    V[4] = glm::vec3(-1.0f, -1.0f, -1.0f);
    V[5] = glm::vec3(-1.0f, -1.0f, 1.0f);
    planeVBO.update(V);

    VertexBufferObject planeNBO;
    std::vector<glm::vec3> N(6);
    planeNBO.init();
    N[0] = glm::vec3(0, 1, 0);
    N[1] = glm::vec3(0, 1, 0);
    N[2] = glm::vec3(0, 1, 0);
    N[3] = glm::vec3(0, 1, 0);
    N[4] = glm::vec3(0, 1, 0);
    N[5] = glm::vec3(0, 1, 0);
    planeNBO.update(N);

    VAOs.push_back(planeVAO);
    VBOs.push_back(planeVBO);
    NBOs.push_back(planeNBO);
    vNBOs.push_back(planeNBO);

    model.push_back(glm::mat4(1.f));
    renderMode.push_back('p');

    // Depth map
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize the OpenGL Program
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec3 position;"
                    "in vec3 normal;"
                    "in vec2 aTexCoords;"
                    "out vec2 TexCoords;"
                    "out vec4 FragPosLightSpace;"
                    "out vec3 FragPos;"
                    "out vec3 Normal;"
                    "uniform mat4 model;"
                    "uniform mat4 view;"
                    "uniform mat4 projection;"
                    "uniform mat4 lightSpaceMatrix;"
                    "void main()"
                    "{"
                    "    FragPos = vec3(model * vec4(position, 1.0));"
                    "    Normal = mat3(transpose(inverse(model))) * normal;"
                    "    TexCoords = aTexCoords;"
                    "    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);"
                    "    gl_Position = projection * view * vec4(FragPos, 1.0);"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "in vec3 Normal;"
                    "in vec3 FragPos;"
                    "in vec2 TexCoords;"
                    "in vec4 FragPosLightSpace;"
                    "uniform sampler2D shadowMap;"
                    "uniform vec3 lightPos;"
                    "uniform vec3 viewPos;"
                    "uniform vec3 lightColor;"
                    "uniform vec3 objectColor;"
                    "uniform vec3 shadowColor;"
                    "uniform samplerCube skybox;"
                    "float ShadowCalculation(vec4 fragPosLightSpace, float bias)"
                    "{"
                    "    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;"
                    "    projCoords = projCoords * 0.5 + 0.5;"
                    "    float closestDepth = texture(shadowMap, projCoords.xy).r;"
                    "    float currentDepth = projCoords.z;"
                    "    float shadow = 0.0;"
                    "    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);"
                    "    for(int x = -1; x <= 1; ++x) {"
                    "        for(int y = -1; y <= 1; ++y) {"
                    "            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; "
                    "            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;"
                    "        }"
                    "    }"
                    "    shadow /= 9.0;"
                    "    if(projCoords.z > 1.0)"
                    "        shadow = 0.0;"
                    "    return shadow;"
                    "}"
                    "void main()"
                    "{"
                    "    float ambientStrength = 0.4;"
                    "    vec3 ambient = ambientStrength * lightColor;"
                    "    vec3 norm = normalize(Normal);"
                    "    vec3 lightDir = normalize(lightPos - FragPos);"
                    "    float diff = max(dot(norm, lightDir), 0.0);"
                    "    vec3 diffuse = diff * lightColor;"
                    "    float specularStrength = 0.5;"
                    "    vec3 viewDir = normalize(viewPos - FragPos);"
                    "    vec3 reflectDir = reflect(-lightDir, norm);"
                    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);"
                    "    vec3 specular = specularStrength * spec * lightColor;"
                    "    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005); "
                    "    float shadow = ShadowCalculation(FragPosLightSpace, bias);"
                    "    vec3 result = ambient * objectColor + (1.0 - shadow) * (diffuse + specular) * objectColor + shadow * (diffuse + specular) * shadowColor;"
                    "    outColor = vec4(result, 1.0);"
                    "}";

    const GLchar* fragment_shader_mirror = 
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "in vec3 Normal;"
                    "in vec3 FragPos;"
                    "in vec2 TexCoords;"
                    "in vec4 FragPosLightSpace;"
                    "uniform sampler2D shadowMap;"
                    "uniform vec3 lightPos;"
                    "uniform vec3 viewPos;"
                    "uniform vec3 lightColor;"
                    "uniform vec3 objectColor;"
                    "uniform vec3 shadowColor;"
                    "uniform samplerCube skybox;"
                    "float ShadowCalculation(vec4 fragPosLightSpace, float bias)"
                    "{"
                    "    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;"
                    "    projCoords = projCoords * 0.5 + 0.5;"
                    "    float closestDepth = texture(shadowMap, projCoords.xy).r;"
                    "    float currentDepth = projCoords.z;"
                    "    float shadow = 0.0;"
                    "    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);"
                    "    for(int x = -1; x <= 1; ++x) {"
                    "        for(int y = -1; y <= 1; ++y) {"
                    "            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; "
                    "            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;"
                    "        }"
                    "    }"
                    "    shadow /= 9.0;"
                    "    if(projCoords.z > 1.0)"
                    "        shadow = 0.0;"
                    "    return shadow;"
                    "}"
                    "void main()"
                    "{"
                    "    float ambientStrength = 0.4;"
                    "    vec3 ambient = ambientStrength * lightColor;"
                    "    vec3 norm = normalize(Normal);"
                    "    vec3 lightDir = normalize(lightPos - FragPos);"
                    "    float diff = max(dot(norm, lightDir), 0.0);"
                    "    vec3 diffuse = diff * lightColor;"
                    "    float specularStrength = 0.5;"
                    "    vec3 viewDir = normalize(viewPos - FragPos);"
                    "    vec3 reflectDir = reflect(-lightDir, norm);"
                    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);"
                    "    vec3 specular = specularStrength * spec * lightColor;"
                    "    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005); "
                    "    float shadow = ShadowCalculation(FragPosLightSpace, bias);"
                    "    vec3 I = normalize(FragPos - viewPos);"
                    "    vec3 R = reflect(I, normalize(Normal));"
                    "    vec3 color = texture(skybox, R).rgb;"
                    "    vec3 result = ambient * color + (1.0 - shadow) * (diffuse + specular) * color + shadow * (diffuse + specular) * shadowColor;"
                    "    outColor = vec4(result, 1.0);"
                    "}";

    const GLchar* shadow_vs =
            "#version 150 core\n"
                    "in vec3 position;"
                    "uniform mat4 model;"
                    "uniform mat4 lightSpaceMatrix;"
                    "void main()"
                    "{"
                    "    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0);"
                    "}";         
    
    const GLchar* shadow_fs =
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "void main()"
                    "{"
                    "}";       

    const GLchar* skybox_vs =
            "#version 150 core\n"
                    "in vec3 position;"
                    "uniform mat4 projection;"
                    "uniform mat4 view;"
                    "out vec3 TexCoords;"
                    "void main()"
                    "{"
                    "    TexCoords = position;"
                    "    gl_Position = projection * view * vec4(position, 1.0);"
                    "}";    

    const GLchar* skybox_fs =
            "#version 150 core\n"
                    "out vec4 FragColor;"
                    "in vec3 TexCoords;"
                    "uniform samplerCube skybox;"
                    "void main()"
                    "{"
                    "    FragColor = texture(skybox, TexCoords);"
                    "}"; 

    const GLchar* fragment_shader_r =
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "in vec3 Normal;"
                    "in vec3 FragPos;"
                    "in vec2 TexCoords;"
                    "in vec4 FragPosLightSpace;"
                    "uniform sampler2D shadowMap;"
                    "uniform vec3 lightPos;"
                    "uniform vec3 viewPos;"
                    "uniform vec3 lightColor;"
                    "uniform vec3 objectColor;"
                    "uniform vec3 shadowColor;"
                    "uniform samplerCube skybox;"
                    "void main()"
                    "{"
                    "    float ratio = 1.00 / 1.52;"
                    "    vec3 I = normalize(FragPos - viewPos);"
                    "    vec3 R = refract(I, normalize(Normal), ratio);"
                    "    outColor = vec4(texture(skybox, R).rgb, 1.0);"
                    "}";

    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();
    program.bindVertexAttribArray("position", planeVBO);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set default camera
    cameraPos = glm::vec3(0.0, 0.5f, 4.0f);

    vector<string> faces 
    {
        "../data/night_posx.png",
        "../data/night_negx.png",
        "../data/night_posy.png",
        "../data/night_negy.png",
        "../data/night_posz.png",
        "../data/night_negz.png"
    };
    cubemapTexture = loadCubemap(faces);

    while (!glfwWindowShouldClose(window))
    {
        // Set the uniform value
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        lightPos = glm::vec3(light_r * cos(glm::radians(time * 10)), 1.0f, light_r * sin(glm::radians(time * 10)));

        int original_width, original_height;
        glfwGetWindowSize(window, &original_width, &original_height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearStencil(0);

        // Render depth of scene to texture
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        program.init(shadow_vs,shadow_fs,"outColor");
        program.bind();
        glUniformMatrix4fv(program.uniform("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < VAOs.size(); i ++) {
            glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, glm::value_ptr(model[i]));
            VAOs[i].bind();
            program.bind();
            int cols = VBOs[i].cols;
            program.bindVertexAttribArray("normal", vNBOs[i]);
            for (int i = 0; i < cols; i += 3) {
                glDrawArrays(GL_TRIANGLES, i, 3);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Reset viewport
        glViewport(0, 0, original_width, original_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderScene(program, window, lightSpaceMatrix, vertex_shader, fragment_shader, fragment_shader_mirror, fragment_shader_r, depthMap);
        renderSkyBox(program, window, skybox_vs, skybox_fs);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    for (int i = 0; i < VAOs.size(); i ++) {
        VAOs[i].free();
    }
    for (int i = 0; i < VBOs.size(); i ++) {
        VBOs[i].free();
    }

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}

void renderScene(Program &program, GLFWwindow* window, glm::mat4 &lightSpaceMatrix, const string &vs, const string &fs, const string &fs_m, const string &fs_r, unsigned int &depthMap) {
    // Get size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect_ratio = float(height)/float(width);
    view = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective(glm::radians(45.0f), 1/aspect_ratio, 0.5f, 150.f);

    // Enable stencil operations
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glStencilFunc(GL_ALWAYS, 0, -1);

    // Draw plane
    VAOs[0].bind();
    
    program.init(vs,fs,"outColor");
    program.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, glm::value_ptr(model[0]));
    glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(program.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(program.uniform("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    if (shadowColorBlack == 1) {
        glUniform3f(program.uniform("shadowColor"), 0.0f, 0.0f, 0.0f);
    } else {
        glUniform3f(program.uniform("shadowColor"), 1.0f, 0.0f, 0.0f);
    }

    glUniform3f(program.uniform("lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(program.uniform("objectColor"), 0.3f, 0.3f, 0.3f);
    glUniform3f(program.uniform("lightPos"), lightPos[0], lightPos[1], lightPos[2]);
    glUniform3f(program.uniform("viewPos"), cameraPos[0], cameraPos[1], cameraPos[2]);
    program.bindVertexAttribArray("position", VBOs[0]);
    program.bindVertexAttribArray("normal", vNBOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw other objects
    for (int i = 1; i < VAOs.size(); i ++) {
        glStencilFunc(GL_ALWAYS, i, -1);

        VAOs[i].bind();

        if (renderMode[i] == 'm') {
            program.init(vs,fs_m,"outColor");
        } else if (renderMode[i] == 'p') {
            program.init(vs,fs,"outColor");
        } else if (renderMode[i] == 'r') {
            program.init(vs,fs_r,"outColor");
        }

        program.bind();

        glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, glm::value_ptr(model[i]));
        glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(program.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(program.uniform("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        if (shadowColorBlack == 1) {
            glUniform3f(program.uniform("shadowColor"), 0.0f, 0.0f, 0.0f);
        } else {
            glUniform3f(program.uniform("shadowColor"), 1.0f, 0.0f, 0.0f);
        }

        glUniform3f(program.uniform("lightColor"), 1.0f, 1.0f, 1.0f);
        if (i == index) {
            glUniform3f(program.uniform("objectColor"), 1.0f, 0.8f, 0.5f); // yellow if picked
        } else {
            glUniform3f(program.uniform("objectColor"), 0.3f, 0.5f, 1.0f); // blue if un-picked
        }
        glUniform3f(program.uniform("lightPos"), lightPos[0], lightPos[1], lightPos[2]);
        glUniform3f(program.uniform("viewPos"), cameraPos[0], cameraPos[1], cameraPos[2]);
        program.bindVertexAttribArray("position", VBOs[i]);
        program.bindVertexAttribArray("normal", NBOs[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        // Render
        int cols = VBOs[i].cols;
        program.bindVertexAttribArray("normal", vNBOs[i]);
        for (int i = 0; i < cols; i += 3) {
            glDrawArrays(GL_TRIANGLES, i, 3);
        }
    }
}

void renderSkyBox(Program &program, GLFWwindow* window, const string &vs, const string &fs) {

    glStencilFunc(GL_ALWAYS, 0, -1);

    // Get size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect_ratio = float(height)/float(width);
    view = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    view = glm::mat4(glm::mat3(view));
    projection = glm::perspective(glm::radians(45.0f), 1/aspect_ratio, 0.5f, 150.f);

    // Draw skybox
    skyboxVAO.bind();

    glDepthMask(GL_FALSE);
    program.init(vs,fs,"FragColor");
    program.bind();

    glDepthFunc(GL_LEQUAL);  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(program.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    program.bindVertexAttribArray("position", skyboxVBO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS); 
    glDepthMask(GL_TRUE);
}

unsigned int loadCubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
