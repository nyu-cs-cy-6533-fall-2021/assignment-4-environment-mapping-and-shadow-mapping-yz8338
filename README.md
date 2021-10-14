# CS-GY 6533 A – Interactive Computer Graphics - Fall 2021

### Assignment 2

*Yangfan Zhou*

<yz8338@nyu.edu>

# Implementation & Result

## Triangle Soup Editor (Task 1.1)

* Insertion mode (i): Firstly, detect whether the mode is active by capturing GLFW_KEY_I in key_callback function. If the 'i' key is pressed, we set the variable 'iKey' to true. Then enlarge the vector size of V and C by incrementing 3. 
```bash
case GLFW_KEY_I:
    if (action == GLFW_PRESS) {
        iKey = true;
        V.resize(V.size() + 3);
        C.resize(C.size() + 3);
        cout << "triangle insertion mode start \n";
    }
    break;
```
We begin drawing triangle with preview in the main loop then. Detect if the insertion mode is on by checking 'iKey'; we also use variable 'insertIndex' which starts at 0 and record the inserted number of points to draw triangles. If the 'insertIndex' can be divided by 3, we draw point; if it divides 3 and the remainder is 1, we draw line; otherwise, draw the border of triangle. The color of the inserting point is set to black at the beginning and the point position is obtained by 'getCurrentWorldPos' function which I defined before.
```bash
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
```
```bash
glm::vec2 getCurrentWorldPos(GLFWwindow* window) {
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    glm::vec4 p_screen(xpos,height-1-ypos,0,1);
    glm::vec4 p_canonical((p_screen.x/width)*2-1,(p_screen.y/height)*2-1,0,1);
    glm::vec4 p_world = glm::inverse(view) * p_canonical;

    return glm::vec2(p_world.x, p_world.y);
}
```
The 'insertIndex' and all the points of triangle is determined by catching mouse click event in mouse_button_callback function. If iKey is true and the mosue is clicked, we assign the current position of cursor into the current point. Then increment the recorder 'insertIndex' by 1. Next, we determine whether it is the third point of the current triangle, if it is, then set iKey to false and set the color of all points in this triangle to red by 'resetColor' function that I defined.
```bash
void resetColor(int i_triangle, float r, float g, float b) {
    int t = i_triangle * 3;

    for (int i = 0; i < 3; i ++) {
        C[t + i] = glm::vec3(r, g, b);       
    }

    VBO_C.update(C);
}
```
```bash
    // Triangle insertion mode (in mouse_button_callback)
    if (iKey && action == GLFW_PRESS) {
        V[insertIndex] = getCurrentWorldPos(window);
        insertIndex += 1;
        if (insertIndex >= V.size() - 3) {
            iKey = false;
            resetColor(insertIndex/3 - 1, 1.0, 0.0, 0.0);
        }
    }
```
The triangles is displayed all the time by the following code in main loop. We first draw all triangles in its color, and then for all triangle, we reset vertex color to black and draw border by GL_LINE_LOOP for them. The program record the original color at first and then re-assign the original color back after drawing the border.
```bash
glDrawArrays(GL_TRIANGLES, 0, insertIndex);
     
for (int i = 0; i < insertIndex / 3; i ++) {
    temp_C = C;
    resetColor(i, 0.0, 0.0, 0.0);
    glDrawArrays(GL_LINE_LOOP, i * 3, 3);
    C = temp_C;
    VBO_C.update(C);
}
```
Note that VBO_C is another VertexBufferObject for color buffer and C is another vectors to store the per-vertex color.




# Compilation Instructions

```bash
cd Assignment_2
mkdir build
cd build
cmake ../ # re-run cmake when you add/delete source files
make # use "cmake --build ." for Windows
```
