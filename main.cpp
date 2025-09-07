#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <glad/glad.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "util.h"
using namespace std;

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640

// Globals
SDL_Window *gGraphicsAppWindow = nullptr;
SDL_GLContext *gOpenGLContext = nullptr;
bool gQuit = false;

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;
// ShaderGraphics
GLuint gGraphicsPipelineShaderProgram = 0;

#define ERROR_EXIT(...) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define PRINTF(format, ...) \
    do { \
        fprintf(stderr, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
// #define printf(...) fprintf(stdout, ##__VA_ARGS__)

GLuint CompileShader(GLuint type, const string& source){
    GLuint shaderObject;
    if(type==GL_VERTEX_SHADER){
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }else if(type==GL_FRAGMENT_SHADER){
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }
    const char *src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);
    return shaderObject;
}

GLuint CreateShaderProgram(const char *vertexFile, const char *fragmentFile){
    std::string vertexShaderSource = get_file_contents(vertexFile);
    std::string fragmentShaderSource = get_file_contents(fragmentFile);
    GLuint programObject = glCreateProgram();
    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    glValidateProgram(programObject);

    return programObject;
}

void CreateGraphicsPipeline(){
    gGraphicsPipelineShaderProgram = CreateShaderProgram("Shader/vert.glsl","Shader/frag.glsl");
}

void VertexSpecification(){
    // Lives on the CPU
    const vector<GLfloat> vertexPosition{
        // x  y   z
        -0.8f, -0.8f, 0.0f,  //vertex1
        0.8f, -0.8f, 0.0f,   //vertex2
        0.0f, 0.8f, 0.0f     //vertex3
    };

    // Setting things up on GPU
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    // Start generating our VBO
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexPosition.size() * sizeof(GLfloat), vertexPosition.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Unbind
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
}

void InitializeProgram(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("SDL2 could not initialize video subsystem");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsAppWindow = SDL_CreateWindow("OpenGL Window", 0, 0,SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
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
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_CULL_FACE);

     glViewport(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);
     glClearColor(1.f, 1.f, 0.f, 1.f);

     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

     glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw(){
    glBindVertexArray(gVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void MainLoop(){
    while(!gQuit){
        Input();

        PreDraw();

        Draw();

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

    VertexSpecification();

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}
