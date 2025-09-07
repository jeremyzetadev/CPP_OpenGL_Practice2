#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <glad/glad.h>
#include <iostream>
#include <cstdio>
using namespace std;

#define SCREEN_HEIGHT 640
#define SCREEN_WIDTH 480

// Globals
SDL_Window *gGraphicsAppWindow = nullptr;
SDL_GLContext *gOpenGLContext = nullptr;
bool gQuit = false;

#define ERROR_EXIT(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define PRINTF(format, ...) \
    do { \
        fprintf(stderr, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
// #define printf(...) fprintf(stdout, ##__VA_ARGS__)

void InitializeProgram(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("SDL2 could not initialize video subsystem");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsAppWindow = SDL_CreateWindow("OpenGL Window", 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SDL_WINDOW_OPENGL);
    if(!gGraphicsAppWindow)
        ERROR_EXIT("SDL_Window was not able to be created");

    gOpenGLContext = (SDL_GLContext*)SDL_GL_CreateContext(gGraphicsAppWindow);
    if(!gOpenGLContext)
        ERROR_EXIT("OpenGL context not available");

    // for GL_VENDOR, GL_RENDERER, GL_VERSION
    gladLoadGL();
    puts("OPENGL Loaded");
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
}

void Input(){
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0){
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye!" << std::endl;
            gQuit = true;
        }
    }
}

void PreDraw(){

}

void Draw(){

}

void MainLoop(){
    while(!gQuit){
        Input();

        PreDraw();

        // Update the screen
        SDL_GL_SwapWindow(gGraphicsAppWindow);
    }
}

void CleanUp(){
    SDL_DestroyWindow(gGraphicsAppWindow);
    SDL_Quit();
}

int main(){
    InitializeProgram();

    MainLoop();

    CleanUp();

    return 0;
}
