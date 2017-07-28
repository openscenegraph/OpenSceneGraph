/* OpenSceneGraph example, emscripten.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <emscripten.h>
#include <SDL2/SDL.h>

#include "osgemscripten.h"

// We use app global variable in loop() function.
Application *app = 0;

// Stand alone function that is called by Emscripten to run the app.
void loop()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (app)
        {
            app->handleEvent(e);
        }
    }
    if (app)
    {
        app->frame();
    }
}

int main(int argc, char *argv[])
{
    // Make sure SDL is working.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("OSGWeb. Could not init SDL: '%s'\n", SDL_GetError());
        return 1;
    }
    // Clean SDL up at exit.
    atexit(SDL_Quit);
    // Configure rendering context.
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Create rendering window.
    int width = 800;
    int height = 600;
    SDL_Window* window =
        SDL_CreateWindow(
            "OSGWeb",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_OPENGL);
    if (!window)
    {
        printf("OSGWeb. Could not create window: '%s'\n", SDL_GetError());
        return 1;
    }
    SDL_GL_CreateContext(window);
    // Create application.
    app = new Application;
    app->setupWindow(width, height);
    app->loadScene("box.osgt");
    // Render asynchronously.
    emscripten_set_main_loop(loop, -1, 0);
    return 0;
}
