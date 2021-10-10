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

// Timer
#include <chrono>

#include <iostream>
using namespace std;

// Key control boolean
bool iKey, oKey, pKey, cKey;

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
//Eigen::MatrixXf V(2,3);
std::vector<glm::vec2> V(3);

// Contains the per-vertex color
std::vector<glm::vec3> C(3);
std::vector<glm::vec3> temp_C(3);

glm::vec2 cursor;
int insertIndex = 0;
int triangle;
bool drag = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

glm::vec2 getCurrentWorldPos(GLFWwindow* window) {
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    return glm::vec2(xworld, yworld);
}

bool pointInTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y) {
    float denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
    float a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
    float b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
    float c = 1 - a - b;

    return 0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1;
}

int getCurrentTriangle(glm::vec2 cursor) {
    for (int i = 0; i < insertIndex / 3; i ++) {
        //glDrawArrays(GL_LINE_LOOP, i * 3, 3);
        int index = i * 3;
        bool temp = pointInTriangle(V[index][0], V[index][1], V[index+1][0], V[index+1][1], V[index+2][0], V[index+2][1], cursor[0], cursor[1]);
        if (temp) 
            return i;
    }
    return -1;
}

void deleteTriangle(int index) {
    for (int i = 0; i < 3; i ++) {
        V.pop_back();
    }
    int temp = index * 3;
    V.erase(V.begin() + temp, V.begin() + temp + 3);

    insertIndex = (int)V.size();
    V.resize(V.size() + 3);
}

void translateTriangle(int index, GLFWwindow* window) {
    
    glm::vec2 temp = getCurrentWorldPos(window);
    float trans_x = temp[0] - cursor[0];
    float trans_y = temp[1] - cursor[1];

    int t = triangle * 3;

    for (int i = 0; i < 3; i ++) {
        V[t + i][0] = V[t + i][0] + trans_x;
        V[t + i][1] = V[t + i][1] + trans_y;
        C[t + i] = glm::vec3(0, 0, 1);       
    }

    cursor = getCurrentWorldPos(window);

    VBO.update(V);
    VBO_C.update(C);
}

void resetColor(int i_triangle, float r, float g, float b) {
    int t = i_triangle * 3;

    for (int i = 0; i < 3; i ++) {
        C[t + i] = glm::vec3(r, g, b);       
    }

    VBO_C.update(C);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Triangle insertion mode on
    if (iKey && action == GLFW_PRESS) {
        V[insertIndex] = getCurrentWorldPos(window);
        insertIndex += 1;
        if (insertIndex >= V.size() - 3) {
            iKey = false;
            resetColor(insertIndex/3 - 1, 1.0, 0.0, 0.0);
        }
    }

    // Triangle translation mode on
    if (oKey && action == GLFW_PRESS) {
        cursor = getCurrentWorldPos(window);
        triangle = getCurrentTriangle(cursor); // select triangle
        if (triangle != -1) {
            drag = true;
        }
    } else if (oKey && action == GLFW_RELEASE) {
        drag = false;
        oKey = false;
        resetColor(triangle, 1.0, 0.0, 0.0);
    }

    // Triangle deletion mode on
    if (pKey && action == GLFW_PRESS) {
        cursor = getCurrentWorldPos(window);
        triangle = getCurrentTriangle(cursor); // select triangle
        deleteTriangle(triangle); // delete triangle
        pKey = false;
    }

    // Upload the change to the GPU
    VBO.update(V);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
        // 'i': triangle insertion mode
        case GLFW_KEY_I:
            if (action == GLFW_PRESS) {
                iKey = true;
                V.resize(V.size() + 3);
                C.resize(C.size() + 3);
                cout << "triangle insertion mode start \n";
            }
            break;

        // 'o': triangle translation mode
        case GLFW_KEY_O:
            if (action == GLFW_PRESS) {
                oKey = true;
                cout << "triangle translation mode start \n";
            }
            break;

        // 'p': triangle delete mode
        case GLFW_KEY_P:
            if (action == GLFW_PRESS) {
                pKey = true;
                cout << "triangle delete mode start \n";
            }
            break;

        default:
            break;
    }

    // Upload the change to the GPU
    VBO.update(V);
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
    window = glfwCreateWindow(640, 480, "Assignment2", NULL, NULL);
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

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

    V.resize(3);
    VBO.update(V);

    // Second VBO for colors
    VBO_C.init();

    C.resize(3);
    VBO_C.update(C);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "in vec3 color;"
                    "out vec3 f_color;"
                    "void main()"
                    "{"
                    "    gl_Position = vec4(position, 0.0, 1.0);"
                    "    f_color = color;"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "in vec3 f_color;"
                    "out vec4 outColor;"
                    "uniform vec3 triangleColor;"
                    "void main()"
                    "{"
                    "    outColor = vec4(f_color, 1.0);"
                    "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",VBO_C);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // Set the uniform value depending on the time difference
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        glUniform3f(program.uniform("triangleColor"), 0.0f, 0.0f, 0.0f);

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Insertion Mode
        if (iKey) {
            V[insertIndex] = getCurrentWorldPos(window);
            VBO.update(V); 

            C[insertIndex] = glm::vec3(0, 0, 0);
            VBO_C.update(C);

            if (insertIndex % 3 == 0) {
                glPointSize(3.f);
                glDrawArrays(GL_POINTS, insertIndex, 1);
            } else if (insertIndex % 3 == 1) {
                glLineWidth(3.f);
                glDrawArrays(GL_LINES, insertIndex - 1, 2);
            } else if (insertIndex % 3 == 2) {
                glDrawArrays(GL_LINE_LOOP, insertIndex - 2, 3);
            }
        }

        glDrawArrays(GL_TRIANGLES, 0, insertIndex);
     
        for (int i = 0; i < insertIndex / 3; i ++) {
            temp_C = C;
            resetColor(i, 0.0, 0.0, 0.0);
            glDrawArrays(GL_LINE_LOOP, i * 3, 3);
            C = temp_C;
            VBO_C.update(C);
        }
        
        // Translation Mode
        if (oKey) {
            if (drag) {
                translateTriangle(triangle, window); // highlight & translation
            }
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
