# Wireframe Renderer

A tool to display the wireframe of a Wavefront .obj file using [weak perspective projection](https://www.cse.iitd.ac.in/~suban/vision/affine/node5.html).

![preview gif](preview.gif)

## Usage

Compile with

```sh
g++ -std=c++17 -o wireframe main.cpp $(sdl2-config --libs)
```

Note: you need to have sdl2 libraries and `sdl2-config` installed on your system

Run with

```
./wireframe demo
```

OR

```
./wireframe models/car.obj
```

You can pause the rotation with spacebar, and zoom in/out with keys w/s respectively.
