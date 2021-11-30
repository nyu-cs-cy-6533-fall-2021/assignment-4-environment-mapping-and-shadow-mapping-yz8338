# CS-GY 6533 A – Interactive Computer Graphics - Fall 2021

### Assignment 4

*Yangfan Zhou*

<yz8338@nyu.edu>

# Implementation & Result

## Shadow Mapping

* Light Source

The circular movement of the light source is specified by changing the x, z coordinates each frame according to time at the beginnging of each render loop by the following lines:
```bash
auto t_now = std::chrono::high_resolution_clock::now();
float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
lightPos = glm::vec3(light_r * cos(glm::radians(time * 10)), 1.0f, light_r * sin(glm::radians(time * 10)));
```

* Plane and Objects Render

The plane and objects are rendered in renderScence function. By passing vertex shader, fragment shaders, lightSpaceMatrix and depthMap into this function, it basically use stencil buffer when rendering each object. Hence each object is assigned with incrementing stencil index, which is used for object picking. Everytime an object is clicked, assign the stencil index into global variable 'index'. The default value of 'index' is '0', which corresponds to the plane. If no object is selected, all operations will be done on the plane.
```bash
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
```

* Shadow Mapping

* Change Shadow Color (key '0')

## Environment Mapping

* Skybox Loading

* Skybox Rendering

* Reflection (key 'o')

* Object Control

## Camera Control

* Key (Up & Down & Left & Right)


## Optional Tasks

* Refraction (key 'i')


# Compilation Instructions

```bash
cd Assignment_4
mkdir build
cd build
cmake ../ # re-run cmake when you add/delete source files
make # use "cmake --build ." for Windows
```
