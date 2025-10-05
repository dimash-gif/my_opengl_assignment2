# OpenGL Review
##Interaction, Windows, Menus and Animation

The first part of the project demonstrates multiple animated 2D figures created using Modern OpenGL with GLFW and GLAD. It includes three independent windows showing a rotating black & white square, an ellipse, and a synchronized circle-triangle pair.
The project focuses on interactive transformations, color control, and multi-window rendering — great for learning fundamental computer graphics concepts.

##3D Transformations and Geometry in OGL
The second part extends the previous OpenGL 2D work into 3D graphics, focusing on transformation matrices and interactive control. It includes the three fundamental geometric transformation in 3D space such as scalling, rotation and translation 
Just like the previous part, this project emphasizes user interactivity. Implemented menu allows user to modify and control the cube's transformation in real time. 
## How to build
Fistly clone and open the project
```bash
git clone https://github.com/dimash-gif/my_opengl_assignment.git
cd my_opengl_assignment
``` 
Then build either cube or shapes 
```bash
make shapes
make cube
```

## Run the program 
```bash
./part1
./cube
```
./part1 will open three OpenGL windows simultaneously — each demonstrating different shapes and animations. Meanwhile, ./cube display the OpenGL window with a 3D colored cube.

# Controls
##Part 1
Key                     	Action
A	                        Start animation
S	                        Stop animation
W	                        White square
R	                        Red square
G	                        Green square
Left-click (Subwindow)	    Change subwindow background
Right-click (Main window)	Show menu options in console
ESC	                        Exit all windows
##Part 2
Key	    Action
M   	Cycle between Scale → Rotate → Translate
↑ / ↓	Increase / decrease current value
← / →	Adjust X-axis (rotation or translation)
+ / -	Move forward / backward (Z translation)
ESC	    Exit
