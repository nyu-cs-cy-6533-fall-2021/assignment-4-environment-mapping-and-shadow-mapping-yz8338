// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

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

// // Render mode
// vector<char> renderMode;

// For stencil buffer picking
GLbyte color[4];
GLfloat depth;
GLuint index;

// Projection mode
bool perspective = false;

// Light position
float light_r = 1.0f;

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
    0, 1, 2,
    2, 1, 3,
    3, 7, 1,
    1, 7, 5,
    5, 6, 7,
    6, 4, 5,
    5, 4, 0,
    0, 5, 1,
    0, 6, 4,
    0, 2, 6,
    6, 7, 2,
    2, 3, 7
};

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
        glm::vec3 normal = cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);
        normal = normalize(normal);
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
                temp += V[j];
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
                    temp += V[j];
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

        // // Render mode
        // case GLFW_KEY_I:
        //     if (index > 0)
        //         renderMode[index-1] = 'w'; // wireframe mode
        //     break;
        // case GLFW_KEY_O:
        //     if (index > 0)
        //         renderMode[index-1] = 'f'; // flat shading mode
        //     break;
        // case GLFW_KEY_P:
        //     if (index > 0)
        //         renderMode[index-1] = 'p'; // phong shading mode
        //     break;
        
        // Translation: w-a-s-d
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

        // Rotation: h & j (around y-axis left and right)
        case GLFW_KEY_H:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, 1.0, 0.0));
            break;
        case GLFW_KEY_J:
            if (action == GLFW_PRESS)
                model[index] = glm::rotate(model[index], glm::radians(20.0f), glm::vec3(0.0, -1.0, 0.0));
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
                    VAOs.erase(VAOs.begin() + index - 1, VAOs.begin() + index);
                    VBOs.erase(VBOs.begin() + index - 1, VBOs.begin() + index);
                    NBOs.erase(NBOs.begin() + index - 1, NBOs.begin() + index);
                    vNBOs.erase(vNBOs.begin() + index - 1, vNBOs.begin() + index);
                    index = 0;
                }
            }
            break;

        // Projection mode
        case GLFW_KEY_X:
            perspective = false;
            break;
        case GLFW_KEY_Z:
            perspective = true;
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
    window = glfwCreateWindow(640, 640, "Assignment3", NULL, NULL);
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

    // Initialize the VAO & VBO for point light
    VertexArrayObject lightVAO;
    lightVAO.init();
    lightVAO.bind();

    VertexBufferObject lightVBO;
    std::vector<glm::vec3> V(1);

    lightVBO.init();

    VertexBufferObject lightNBO;
    std::vector<glm::vec3> N(1);
    lightNBO.init();
    N[0] = glm::vec3(0, 0, 1);
    lightNBO.update(N);

    VAOs.push_back(lightVAO);
    VBOs.push_back(lightVBO);
    NBOs.push_back(lightNBO);
    vNBOs.push_back(lightNBO);

    model.push_back(glm::mat4(1.f));
    model.push_back(glm::mat4(1.f));

    // Initialize the OpenGL Program
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec3 position;"
                    "in vec3 normal;"
                    "out vec3 FragPos;"
                    "out vec3 Normal;"
                    "uniform mat4 model;"
                    "uniform mat4 view;"
                    "uniform mat4 projection;"
                    "void main()"
                    "{"
                    "    FragPos = vec3(model * vec4(position, 1.0));"
                    "    Normal = mat3(transpose(inverse(model))) * normal;"
                    "    gl_Position = projection * view * vec4(FragPos, 1.0);"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "in vec3 Normal;"
                    "in vec3 FragPos;"
                    "uniform vec3 lightPos;"
                    "uniform vec3 viewPos;"
                    "uniform vec3 lightColor;"
                    "uniform vec3 objectColor;"
                    "void main()"
                    "{"
                    "    float ambientStrength = 0.1;"
                    "    vec3 ambient = ambientStrength * lightColor;"
                    "    vec3 norm = normalize(Normal);"
                    "    vec3 lightDir = normalize(lightPos - FragPos);"
                    "    float diff = max(dot(norm, lightDir), 0.0);"
                    "    vec3 diffuse = diff * lightColor;"
                    "    float specularStrength = 0.5;"
                    "    vec3 viewDir = normalize(viewPos - FragPos);"
                    "    vec3 reflectDir = reflect(-lightDir, norm);"
                    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);"
                    "    vec3 specular = specularStrength * spec * lightColor;"
                    "    vec3 result = (ambient + diffuse + specular) * objectColor;"
                    "    outColor = vec4(result, 1.0);"
                    "}";
                    
    const GLchar* light_vertex_shader =
            "#version 150 core\n"
                    "in vec3 position;"
                    "uniform mat4 model;"
                    "uniform mat4 view;"
                    "uniform mat4 projection;"
                    "void main()"
                    "{"
                    "    gl_Position = projection * view * model * vec4(position, 1.0);"
                    "}";
    const GLchar* light_fragment_shader =
            "#version 150 core\n"
                    "out vec4 lightColor;"
                    "void main()"
                    "{"
                    "    lightColor = vec4(1.0);"
                    "}";


    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    program.bindVertexAttribArray("position",lightVBO);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set default projection
    projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Set default camera
    cameraPos = glm::vec3(0.0, 0.0, 0.6);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearStencil(0);

        // Set the uniform value
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Get size of the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspect_ratio = float(height)/float(width);

        view = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

        if(perspective) {
            projection = glm::perspective(glm::radians(45.0f), 1/aspect_ratio, 0.5f, 150.f);
        } else {
            projection = glm::ortho(-1.0f, 1.0f, -1.0f * aspect_ratio, 1.0f * aspect_ratio, -10.0f, 10.0f);
        }

        // Enable stencil operations
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // Bind other VAO
        glm::vec3 lightPos = glm::vec3(light_r * cos(glm::radians(time * 10)), 1.0f, light_r * sin(glm::radians(time * 10)));
        for (int i = 1; i < VAOs.size(); i ++) {
            glStencilFunc(GL_ALWAYS, i + 1, -1);

            VAOs[i].bind();

            // Bind program
            program.init(vertex_shader,fragment_shader,"outColor");
            program.bind();

            glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, glm::value_ptr(model[i+1]));
            glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(program.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3f(program.uniform("lightColor"), 1.0f, 1.0f, 1.0f);

            if (i + 1 == index) {
                glUniform3f(program.uniform("objectColor"), 1.0f, 0.0f, 0.0f);
            } else {
                glUniform3f(program.uniform("objectColor"), 0.0f, 1.0f, 0.0f);
            }
            glUniform3f(program.uniform("lightPos"), lightPos[0], lightPos[1], lightPos[2]);
            glUniform3f(program.uniform("viewPos"), 0.0f, 0.0f, 1.0f);
            program.bindVertexAttribArray("position", VBOs[i]);
            program.bindVertexAttribArray("normal", NBOs[i]);

            // Render
            int cols = VBOs[i].cols;
            program.bindVertexAttribArray("normal", vNBOs[i]);
            for (int i = 0; i < cols; i += 3) {
                glDrawArrays(GL_TRIANGLES, i, 3);
            }
        }

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
