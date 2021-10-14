# CS-GY 6533 A – Interactive Computer Graphics - Fall 2021

### Assignment 2

*Yangfan Zhou*

<yz8338@nyu.edu>

# Implementation & Result

## Triangle Soup Editor (Task 1.1)

* Insertion mode (i): 

Firstly, detect whether the mode is active by capturing GLFW_KEY_I in key_callback function. If the 'i' key is pressed, we set the variable 'iKey' to true. Then enlarge the vector size of V and C by incrementing 3. 
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

We begin drawing triangle with preview in the main render loop then. Detect if the insertion mode is on by checking 'iKey'; we also use variable 'insertIndex' which starts at 0 and record the inserted number of points to draw triangles. If the 'insertIndex' can be divided by 3, we draw point; if it divides 3 and the remainder is 1, we draw line; otherwise, draw the border of triangle. The color of the inserting point is set to black at the beginning and the position of point is obtained by 'getCurrentWorldPos' function which I defined before.
```bash
// in render loop
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

The 'insertIndex' and all the points of triangle is determined by catching mouse click event in mouse_button_callback function. If iKey is true and the mouse is clicked, we assign the current position of cursor into the current point. Then increment the recorder 'insertIndex' by 1. Next, we determine whether it is the third point of the current triangle, if it is, then set iKey to false and set the color of all points in this triangle to red by 'resetColor' function that I defined.
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
// in render loop
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

* Translation mode (o): 

Similar to insertion mode, this beginning of this mode is also detected in key_callback as follows:
```bash
case GLFW_KEY_O:
    if (action == GLFW_PRESS) {
        oKey = true;
        cout << "triangle translation mode start \n";
    }
    break;
```

If the mode is active, we detect the press, move and release activities in mouse_button_callback function. If 'oKey' is true and the mouse is pressing, we record the current cursor position by getCurrentWorldPos function. Then use the cursor's position to detect whether there is a triangle under the cursor by 'getCurrentTriangle(cursor)'. If there is no triangle, the return value is -1. If not, we set the variable 'drag' to true and use a temporary vector 'ColorBefore' to store the current colors. Because we need to highlight the triangle as blue when dragging the selected one. 
If the mode is active and the mouse is release, we set 'drag' and 'oKey' to false. Then retrieve the original colors of the triangle.
```bash
// in mouse_button_callback
// Triangle translation mode on
    if (oKey && action == GLFW_PRESS) {
        cursor = getCurrentWorldPos(window);
        triangle = getCurrentTriangle(cursor); // select triangle
        if (triangle != -1) {
            drag = true;
            ColorBefore = C;
        }
    } else if (oKey && action == GLFW_RELEASE) {
        drag = false;
        oKey = false;
        // retrieve original color
        int index = triangle * 3;
        C[index] = ColorBefore[index];
        C[index + 1] = ColorBefore[index + 1];
        C[index + 2] = ColorBefore[index + 2];
    }
```

For the 'getCurrentTriangle' function, it traverse all triangle in vector V and use the current cursor position with 'pointInTriangle' function to detect whether the cursor is upon a triangle. If so, return the index of triangle. If not, return -1.
```bash
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
```

For the 'pointInTriangle' function, we use barycentric theory to determine whether a point is in given triangle. Then return the result as boolean value.
```bash
bool pointInTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y) {
    float denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
    float a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
    float b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
    float c = 1 - a - b;

    return 0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1;
}
```

Finally, we detect whether 'drag' is true in the main loop. If the cursor is dragging, we use 'translateTriangle' function to update the real-time position of selected triangle. The first parameter 'triangle' is just the index returned from the getCurrentTriangle function. 
```bash
// in render loop
// Translation Mode
    if (oKey) {
       if (drag) {
           translateTriangle(triangle, window); // highlight & translation
       }
    }
```

In the 'translateTriangle' function, we get current cursor's position first. In order to calculate the movement of cursor, we use the current position to subtract the old position (which is recorded in global variable 'cursor'). Then, for each vertex in the triangle, we increment its coordinate by the movement of cursor. Simultaneously, change the vertex color to blue in order to highlight the triangle. In the end of this function, we record the current cursor position into 'cursor'.
```bash
void translateTriangle(int index, GLFWwindow* window) {
    
    glm::vec2 temp = getCurrentWorldPos(window);
    float trans_x = temp[0] - cursor[0];
    float trans_y = temp[1] - cursor[1];

    int t = index * 3;

    for (int i = 0; i < 3; i ++) {
        V[t + i][0] = V[t + i][0] + trans_x;
        V[t + i][1] = V[t + i][1] + trans_y;
        C[t + i] = glm::vec3(0, 0, 1); // highlight as blue      
    }

    cursor = getCurrentWorldPos(window);

    VBO.update(V);
    VBO_C.update(C);
}
```




# Compilation Instructions

```bash
cd Assignment_2
mkdir build
cd build
cmake ../ # re-run cmake when you add/delete source files
make # use "cmake --build ." for Windows
```
