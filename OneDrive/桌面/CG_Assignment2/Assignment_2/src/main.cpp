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

int insertIndex = 0;

// VertexBufferObject wrapper
VertexBufferObject VBO;

// Contains the vertex positions
//Eigen::MatrixXf V(2,3);
std::vector<glm::vec2> V(3);

glm::vec2 current;

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Triangle insertion mode on
    if (iKey && action == GLFW_PRESS) {
        V[insertIndex] = getCurrentWorldPos(window);
        insertIndex += 1;
        if (insertIndex >= V.size() - 3)
            iKey = false;
    }

    // Triangle translation mode on
    if (oKey && action == GLFW_PRESS) {
        // translation

        oKey = false;
    } else if (oKey && action == GLFW_RELEASE) {
        // stop translation

        oKey = false;
    }

    // Triangle deletion mode on
    if (pKey && action == GLFW_PRESS) {
        // delete triangle under the cursor

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
                cout << "triangle insertion mode start";
            }
            break;

        // 'o': triangle translation mode
        case GLFW_KEY_O:
            if (action == GLFW_PRESS) {
                oKey = true;
                cout << "triangle translation mode start";
            }
            break;

        // 'p': triangle delete mode
        case GLFW_KEY_P:
            if (action == GLFW_PRESS) {
                pKey = true;
                cout << "triangle delete mode start";
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

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "void main()"
                    "{"
                    "    gl_Position = vec4(position, 0.0, 1.0);"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "out vec4 outColor;"
                    "uniform vec3 triangleColor;"
                    "void main()"
                    "{"
                    "    outColor = vec4(triangleColor, 1.0);"
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

        glUniform3f(program.uniform("triangleColor"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, insertIndex);

 
        glUniform3f(program.uniform("triangleColor"), 0.0f, 0.0f, 0.0f);
        for (int i = 0; i < insertIndex / 3; i ++) {
            glDrawArrays(GL_LINE_LOOP, i * 3, 3);
        }
        

        // Translation Mode
        if (oKey) {

        }


        // Delete Mode
        if (pKey) {
            current = getCurrentWorldPos(window);
            

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
