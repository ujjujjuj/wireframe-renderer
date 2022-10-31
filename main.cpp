#include <SDL2/SDL.h>
#include <string.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "types.hpp"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
const int SCREEN_FPS = 90;
const int MS_DELAY = 1000 / SCREEN_FPS;
const float ROT_SPEED = 1;
const float SPEED = 0.4;
const float FOV = 75;
const float FOCAL_LENGTH = 1 / tanf32(FOV / 2);

Shape loadFromObj(std::string path) {
    Shape newShape;
    std::ifstream objfile(path);
    if (objfile.fail()) {
        std::cerr << "Failed to open file " << path << "\n";
        exit(1);
    }
    std::string line;
    std::vector<Edge> edges;
    while (std::getline(objfile, line)) {
        std::stringstream lineStream(line);
        std::string linetype;
        std::getline(lineStream, linetype, ' ');
        if (linetype == "o") {
            lineStream >> newShape.name;
        } else if (linetype == "v") {
            float x, y, z;
            lineStream >> x >> y >> z;
            y *= -1;
            newShape.vertices.push_back({x, y, z});
        } else if (linetype == "f") {
            std::string coords;
            int firstFace = -1;
            int lastFace;
            while (std::getline(lineStream, coords, ' ')) {
                int faceIndex = atoi(strtok(const_cast<char*>(coords.c_str()), "/")) - 1;
                if (firstFace == -1) {
                    firstFace = faceIndex;
                } else {
                    int a = std::min(lastFace, faceIndex);
                    int b = std::max(lastFace, faceIndex);
                    if (a != b) newShape.edges.insert({a, b});
                }
                lastFace = faceIndex;
            }
            int a = std::min(lastFace, firstFace);
            int b = std::max(lastFace, firstFace);
            if (a != b) newShape.edges.insert({a, b});
        }
    }

    newShape.color = colors[rand() % colors.size()];

    return newShape;
}

std::vector<Shape> getShapes(int argc, char** argv) {
    std::vector<Shape> shapes;
    if (argc == 1) {
        std::cerr << "Usage: " << argv[0] << " <filename.obj> OR " << argv[0] << " demo\n";
        exit(1);
    }
    if (strcmp(argv[1], "demo") == 0) {
        std::string path = "models";
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (std::filesystem::path(entry.path()).extension() == ".obj") {
                shapes.push_back(loadFromObj(entry.path()));
            }
        }
    } else {
        shapes.push_back(loadFromObj(argv[1]));
    }

    return shapes;
}

int main(int argc, char** argv) {
    srand(time(0));
    SDL_Window* window;
    SDL_Renderer* renderer;

    std::vector<Shape> shapes = getShapes(argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

    SDL_SetWindowTitle(window, "Wireframe Renderer");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Event event;
    bool running = true;
    float cameraDist = 5, rotation = 0;
    bool isRotating = true;
    while (running) {
        int frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) isRotating = false;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) isRotating = true;
        }
        char* kbState = (char*)SDL_GetKeyboardState(NULL);
        if (kbState[SDL_SCANCODE_W]) cameraDist -= SPEED;
        if (kbState[SDL_SCANCODE_S]) cameraDist += SPEED;

        if (kbState[SDL_SCANCODE_Q]) return false;

        if (isRotating) rotation++;

        // ********* DRAW ****** //
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        Shape shape = shapes[(SDL_GetTicks() / 2000) % shapes.size()];
        std::vector<Vertex> transformedVertices = shape.vertices;
        for (int i = 0; i < transformedVertices.size(); i++) {
            // rotate
            float cosAngle = cosf32(rotation * M_PI / 180);
            float sinAngle = sinf32(rotation * M_PI / 180);
            float xOld = std::get<0>(transformedVertices[i]);
            std::get<0>(transformedVertices[i]) = std::get<0>(transformedVertices[i]) * cosAngle + std::get<2>(transformedVertices[i]) * sinAngle;
            std::get<2>(transformedVertices[i]) = -xOld * sinAngle + std::get<2>(transformedVertices[i]) * cosAngle;

            // world to screen
            std::get<2>(transformedVertices[i]) -= cameraDist;
            std::get<0>(transformedVertices[i]) = (SCREEN_HEIGHT / (float)SCREEN_WIDTH * FOCAL_LENGTH * std::get<0>(transformedVertices[i]) / (FOCAL_LENGTH + std::get<2>(transformedVertices[i])) + 1) * SCREEN_WIDTH / 2;
            std::get<1>(transformedVertices[i]) = (FOCAL_LENGTH * std::get<1>(transformedVertices[i]) / (FOCAL_LENGTH + std::get<2>(transformedVertices[i])) + 1) * SCREEN_HEIGHT / 2;
        }

        SDL_SetRenderDrawColor(renderer, std::get<0>(shape.color), std::get<1>(shape.color), std::get<2>(shape.color), 255);
        for (const auto& edge : shape.edges) {
            SDL_RenderDrawLine(renderer, std::get<0>(transformedVertices[std::get<0>(edge)]), std::get<1>(transformedVertices[std::get<0>(edge)]), std::get<0>(transformedVertices[std::get<1>(edge)]), std::get<1>(transformedVertices[std::get<1>(edge)]));
        }

        SDL_RenderPresent(renderer);

        int timeToWait = MS_DELAY - (SDL_GetTicks() - frameStart);
        if (timeToWait > 0) SDL_Delay(timeToWait);
    }

    SDL_Quit();

    return 0;
}
