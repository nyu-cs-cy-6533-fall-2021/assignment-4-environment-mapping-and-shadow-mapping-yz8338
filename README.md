# CS-GY 6533 A – Interactive Computer Graphics - Fall 2021

### Assignment 2

*Yangfan Zhou*

<yz8338@nyu.edu>

# Implementation & Result

## Triangle Soup Editor (Task 1.1)

* Insertion mode (i): Firstly, detect whether the mode is active by capturing GLFW_KEY_I in key_callback function. If the 'i' key is pressed, we set the variable 'iKey' to true. Then enlarge the vector size of V and C by incrementing 3. 
```bash
// 'i': triangle insertion mode
        case GLFW_KEY_I:
            if (action == GLFW_PRESS) {
                iKey = true;
                V.resize(V.size() + 3);
                C.resize(C.size() + 3);
                cout << "triangle insertion mode start \n";
            }
            break;
```

# Compilation Instructions

```bash
cd Assignment_2
mkdir build
cd build
cmake ../ # re-run cmake when you add/delete source files
make # use "cmake --build ." for Windows
```
