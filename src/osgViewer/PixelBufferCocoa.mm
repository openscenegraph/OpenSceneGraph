/*
 *  PixelBufferCocoa.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include <iostream>
#include <osgViewer/api/Cocoa/PixelBufferCocoa>


namespace osgViewer {


void PixelBufferCocoa::init()
{
	std::cout << "PixelBufferCocoa :: init not implemented yet " << std::endl;
	
	_valid = _initialized = _realized = true;    
	

}

bool PixelBufferCocoa::realizeImplementation() 
{
	std::cout << "PixelBufferCocoa :: realizeImplementation not implemented yet " << std::endl;
	return true;
}

void PixelBufferCocoa::closeImplementation()
{
	std::cout << "PixelBufferCocoa :: closeImplementation not implemented yet " << std::endl;
}


/** Make this graphics context current.*/
bool PixelBufferCocoa::makeCurrentImplementation()
{
	std::cout << "PixelBufferCocoa :: makeCurrentImplementation not implemented yet " << std::endl;
	return true;
}


/** Make this graphics context current with specified read context implementation. */
bool PixelBufferCocoa::makeContextCurrentImplementation(osg::GraphicsContext* readContext)
{
	std::cout << "PixelBufferCocoa :: makeContextCurrentImplementation not implemented yet " << std::endl;
	return true;
}

/** Release the graphics context.*/
bool PixelBufferCocoa::releaseContextImplementation()
{
	std::cout << "PixelBufferCocoa :: releaseContextImplementation not implemented yet " << std::endl;
	return true;
}

/** Bind the graphics context to associated texture implementation.*/
void PixelBufferCocoa::bindPBufferToTextureImplementation( GLenum buffer )
{
	std::cout << "PixelBufferCocoa :: bindPBufferToTextureImplementation not implemented yet " << std::endl;
}

/** Swap the front and back buffers.*/
void PixelBufferCocoa::swapBuffersImplementation()
{
	std::cout << "PixelBufferCocoa :: swapBuffersImplementation not implemented yet " << std::endl;
}

 PixelBufferCocoa::~PixelBufferCocoa()
 {	
 }




}

