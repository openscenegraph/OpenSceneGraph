// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include <osgGA/SimpleViewer>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <SDL.h>
#include <SDL_events.h>

#include <iostream>

int main( int argc, char **argv )
{
    if (argc<2)
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // init SDL
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    unsigned int windowWidth = 1280;
    unsigned int windowHeight = 1024;

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
        // set up the surface to render to
    SDL_Surface* screen = SDL_SetVideoMode(windowWidth, windowHeight, 32, SDL_OPENGL | SDL_FULLSCREEN | SDL_RESIZABLE);
    if ( screen == NULL )
    {
        std::cerr<<"Unable to set "<<windowWidth<<"x"<<windowHeight<<" video: %s\n"<< SDL_GetError()<<std::endl;
        exit(1);
    }
    
    SDL_EnableUNICODE(1);
    
    osgGA::SimpleViewer viewer;
    viewer.setSceneData(loadedModel.get());
    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.getEventQueue()->windowResize(0, 0, windowWidth, windowHeight );

    bool done = false;
    while( !done )
    {
        SDL_Event event;

        while ( SDL_PollEvent(&event) )
        {
            switch (event.type) {

                case SDL_MOUSEMOTION:
                    viewer.getEventQueue()->mouseMotion(event.motion.x, event.motion.y);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    viewer.getEventQueue()->mouseButtonPress(event.button.x, event.button.y, event.button.button);
                    break;

                case SDL_MOUSEBUTTONUP:
                    viewer.getEventQueue()->mouseButtonRelease(event.button.x, event.button.y, event.button.button);
                    break;

                case SDL_KEYUP:
                    viewer.getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) event.key.keysym.unicode);

                    if (event.key.keysym.sym==SDLK_ESCAPE) done = true;
                    if (event.key.keysym.sym=='f') SDL_WM_ToggleFullScreen(screen);

                    break;

                case SDL_KEYDOWN:
                    viewer.getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) event.key.keysym.unicode);
                    break;

                case SDL_VIDEORESIZE:
                {
                    viewer.getEventQueue()->windowResize(0, 0, event.resize.w, event.resize.h );

                    std::cout<<"event.resize.w="<<event.resize.w<<"  event.resize.h="<<event.resize.h<<std::endl;
                    break;
                }

                case SDL_QUIT:
                    done = true;
            }
        }

        if (done) continue;


        // draw the new frame
        viewer.frame();

    	// Swap Buffers
        SDL_GL_SwapBuffers();
    }
   
    return 0;
}

/*EOF*/
