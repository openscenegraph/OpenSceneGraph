#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>


/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Linden Lab Inc. (http://lindenlab.com) code.
 *
 * The Initial Developer of the Original Code is:
 *   Callum Prentice (callum@ubrowser.com)
 *
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Callum Prentice (callum@ubrowser.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <iostream>

#include <GL/glut.h>

#include "llmozlib2.h"

////////////////////////////////////////////////////////////////////////////////
// Implementation of the test app - implemented as a class and derrives from 
// the observer so we can catch events emitted by LLMozLib
//
class testGL :
	public LLEmbeddedBrowserWindowObserver
{
	public:
		testGL() :
			mAppWindowWidth( 800 ),						// dimensions of the app window - can be anything 
			mAppWindowHeight( 600 ),
			mBrowserWindowWidth( mAppWindowWidth ),		// dimensions of the embedded browser - can be anything
			mBrowserWindowHeight( mAppWindowHeight ),	// but looks best when it's the same as the app window
			mAppTextureWidth( -1 ),						// dimensions of the texture that the browser is rendered into
			mAppTextureHeight( -1 ),					// calculated at initialization
			mAppTexture( 0 ),
			mNeedsUpdate( true ),						// flag to indicate if browser texture needs an update 
			mBrowserWindowId( 0 ),
			mAppWindowName( "testGL" ),					
			//mHomeUrl( "http://www.google.com" )			
			mHomeUrl( "http://www.google.com" )			
		{
			std::cout << "LLMozLib version: " << LLMozLib::getInstance()->getVersion() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void init( char* arg0 )
		{
			// OpenGL initialization
			glClearColor( 0.0f, 0.0f, 0.0f, 0.5f);
			glEnable( GL_COLOR_MATERIAL );
			glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
			glEnable( GL_TEXTURE_2D );
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
			glEnable( GL_CULL_FACE );

			// calculate texture size required (next power of two above browser window size
			for ( mAppTextureWidth = 1; mAppTextureWidth < mBrowserWindowWidth; mAppTextureWidth <<= 1 )
			{
			};

			for ( mAppTextureHeight = 1; mAppTextureHeight < mBrowserWindowHeight; mAppTextureHeight <<= 1 )
			{
			};

			// create the texture used to display the browser data
			glGenTextures( 1, &mAppTexture );
			glBindTexture( GL_TEXTURE_2D, mAppTexture );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexImage2D( GL_TEXTURE_2D, 0,
				GL_RGB,
					mAppTextureWidth, mAppTextureHeight,
						0, GL_RGB, GL_UNSIGNED_BYTE, 0 );

			// create a single browser window and set things up.
			std::string applicationDir = osgDB::getFilePath(arg0);
                        if (applicationDir.empty()) applicationDir = osgDB::getRealPath(".");                        
                        else applicationDir = osgDB::getRealPath(applicationDir);
                        
			std::string componentDir = "/usr/lib/xulrunner";
			std::string profileDir = applicationDir + "/" + "testGL_profile";
			LLMozLib::getInstance()->init( applicationDir, componentDir, profileDir, getNativeWindowHandle() );
                        
                        
			mBrowserWindowId = LLMozLib::getInstance()->createBrowserWindow( mBrowserWindowWidth, mBrowserWindowHeight );
                        
                        std::cout<<"after createBrowserWindow("<<mBrowserWindowWidth<<", "<<mBrowserWindowHeight<<")  mBrowserWindowId = "<<mBrowserWindowId<<std::endl;

			// tell LLMozLib about the size of the browser window
			LLMozLib::getInstance()->setSize( mBrowserWindowId, mBrowserWindowWidth, mBrowserWindowHeight );

			// observer events that LLMozLib emits
			LLMozLib::getInstance()->addObserver( mBrowserWindowId, this );

			// append details to agent string
			LLMozLib::getInstance()->setBrowserAgentId( mAppWindowName );

			// don't flip bitmap
			LLMozLib::getInstance()->flipWindow( mBrowserWindowId, false );

			// go to the "home page"
			LLMozLib::getInstance()->navigateTo( mBrowserWindowId, mHomeUrl );
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void reset( void )
		{
			// unhook observer
			LLMozLib::getInstance()->remObserver( mBrowserWindowId, this );

			// clean up
			LLMozLib::getInstance()->reset();
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void reshape( int widthIn, int heightIn )
		{
			if ( heightIn == 0 )
				heightIn = 1;

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();

			glViewport( 0, 0, widthIn, heightIn );
			glOrtho( 0.0f, widthIn, heightIn, 0.0f, -1.0f, 1.0f );

			// we use these elsewhere so save
			mAppWindowWidth = widthIn;
			mAppWindowHeight = heightIn;

			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glutPostRedisplay();
		};

		void idle();

		////////////////////////////////////////////////////////////////////////////////
		//
		void display()
		{
                    //std::cout<<"display() mBrowserWindowId = "<<mBrowserWindowId<<std::endl;

			// clear screen
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			glLoadIdentity();

			// use the browser texture
			glBindTexture( GL_TEXTURE_2D, mAppTexture );

			// valid window ?
			if ( mBrowserWindowId )
			{
                                static int count = 0;
                                count++;
                        
				// needs to be updated?
				if ( mNeedsUpdate && (count%1)==0)
				{
					// grab the page
                                        // std::cout<<"mNeedsUpdate = true"<<std::endl;

					const unsigned char* pixels = LLMozLib::getInstance()->getBrowserWindowPixels( mBrowserWindowId );
					if ( pixels )
					{
                                        
                                            //std::cout<<"Texture subload "<<LLMozLib::getInstance()->getBrowserRowSpan( mBrowserWindowId ) / LLMozLib::getInstance()->getBrowserDepth( mBrowserWindowId )<<", "<<mBrowserWindowHeight<<std::endl;
                                        
						// write them into the texture
						glTexSubImage2D( GL_TEXTURE_2D, 0,
							0, 0,
								// because sometimes the rowspan != width * bytes per pixel (mBrowserWindowWidth)
								LLMozLib::getInstance()->getBrowserRowSpan( mBrowserWindowId ) / LLMozLib::getInstance()->getBrowserDepth( mBrowserWindowId ),
									mBrowserWindowHeight,
										LLMozLib::getInstance()->getBrowserDepth( mBrowserWindowId ) == 3 ? GL_BGR_EXT : GL_BGRA_EXT,
											GL_UNSIGNED_BYTE,
												pixels );
					}
                                        else
                                        {
                                            std::cout<<"No data to subload"<<std::endl;
                                        }

					// flag as already updated
					//mNeedsUpdate = false;
				};
			};

			// scale the texture so that it fits the screen
			GLfloat textureScaleX = ( GLfloat )mBrowserWindowWidth / ( GLfloat )mAppTextureWidth;
			GLfloat textureScaleY = ( GLfloat )mBrowserWindowHeight / ( GLfloat )mAppTextureHeight;

			// draw the single quad full screen (orthographic)
			glMatrixMode( GL_TEXTURE );
			glPushMatrix();
			glScalef( textureScaleX, textureScaleY, 1.0f );

			glEnable( GL_TEXTURE_2D );
			glColor3f( 1.0f, 1.0f, 1.0f );
			glBegin( GL_QUADS );
				glTexCoord2f( 1.0f, 0.0f ); 
				glVertex2d( mAppWindowWidth, 0 );

				glTexCoord2f( 0.0f, 0.0f ); 
				glVertex2d( 0, 0 );

				glTexCoord2f( 0.0f, 1.0f ); 
				glVertex2d( 0, mAppWindowHeight );

				glTexCoord2f( 1.0f, 1.0f ); 
				glVertex2d( mAppWindowWidth, mAppWindowHeight );
			glEnd();

			glMatrixMode( GL_TEXTURE );
			glPopMatrix();

			glutSwapBuffers();
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void mouseButton( int button, int state, int xIn, int yIn )
		{
			// texture is scaled to fit the screen so we scale mouse coords in the same way
			xIn = ( xIn * mBrowserWindowWidth ) / mAppWindowWidth;
			yIn = ( yIn * mBrowserWindowHeight ) / mAppWindowHeight;

			if ( button == GLUT_LEFT_BUTTON )
			{
				if ( state == GLUT_DOWN )
				{
					// send event to LLMozLib
					LLMozLib::getInstance()->mouseDown( mBrowserWindowId, xIn, yIn );
				}
				else
				if ( state == GLUT_UP )
				{
					// send event to LLMozLib
					LLMozLib::getInstance()->mouseUp( mBrowserWindowId, xIn, yIn );

					// this seems better than sending focus on mouse down (still need to improve this)
					LLMozLib::getInstance()->focusBrowser( mBrowserWindowId, true );
				};
			};

			// force a GLUT  update 
			glutPostRedisplay();
		}

		////////////////////////////////////////////////////////////////////////////////
		//
		void mouseMove( int xIn , int yIn )
		{
			// texture is scaled to fit the screen so we scale mouse coords in the same way
			xIn = ( xIn * mBrowserWindowWidth ) / mAppWindowWidth;
			yIn = ( yIn * mBrowserWindowHeight ) / mAppWindowHeight;

			// send event to LLMozLib
			LLMozLib::getInstance()->mouseMove( mBrowserWindowId, xIn, yIn );

			// force a GLUT  update 
			glutPostRedisplay();
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void keyboard( unsigned char keyIn, int xIn, int yIn )
		{
			// ESC key exits
			if ( keyIn == 27 )
			{
				reset();

				exit( 0 );
			};

			// send event to LLMozLib
                        if (keyIn<32) LLMozLib::getInstance()->keyPress( mBrowserWindowId, keyIn );
                        else LLMozLib::getInstance()->unicodeInput( mBrowserWindowId, keyIn );
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onPageChanged( const EventType& eventIn )
		{
			// flag that an update is required - page grab happens in idle() so we don't stall
			mNeedsUpdate = true;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onNavigateBegin( const EventType& eventIn )
		{
			std::cout << "Event: begin navigation to " << eventIn.getEventUri() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onNavigateComplete( const EventType& eventIn )
		{
			std::cout << "Event: end navigation to " << eventIn.getEventUri() << " with response status of " << eventIn.getIntValue() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onUpdateProgress( const EventType& eventIn )
		{
			std::cout << "Event: progress value updated to " << eventIn.getIntValue() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onStatusTextChange( const EventType& eventIn )
		{
			std::cout << "Event: status updated to " << eventIn.getStringValue() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onLocationChange( const EventType& eventIn )
		{
			std::cout << "Event: location changed to " << eventIn.getStringValue() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		// virtual
		void onClickLinkHref( const EventType& eventIn )
		{
			std::cout << "Event: clicked on link to " << eventIn.getStringValue() << std::endl;
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		int getAppWindowWidth()
		{
			return mAppWindowWidth;
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		int getAppWindowHeight()
		{
			return mAppWindowHeight;
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		std::string getAppWindowName()
		{
			return mAppWindowName;
		};

		////////////////////////////////////////////////////////////////////////////////
		//
		void* getNativeWindowHandle();

	private:
		int mAppWindowWidth;
		int mAppWindowHeight;
		int mBrowserWindowWidth;
		int mBrowserWindowHeight;
		int mAppTextureWidth;
		int mAppTextureHeight;
		GLuint mAppTexture;
		int mBrowserWindowId;
		std::string mAppWindowName;
		std::string mHomeUrl;
		bool mNeedsUpdate;
};

testGL* theApp;

////////////////////////////////////////////////////////////////////////////////
//
#ifdef _WINDOWS

////////////////////////////////////////////////////////////////////////////////
//
void idle()
{
   //std::cout<<"idle()"<<std::endl;

	// onPageChanged event sets this
	// if ( mNeedsUpdate )
		// grab a page but don't reset 'needs update' flag until we've written it to the texture in display()
		LLMozLib::getInstance()->grabBrowserWindow( mBrowserWindowId );

	// lots of updates for smooth motion
	glutPostRedisplay();
};


void* testGL::getNativeWindowHandle()
{
    // My implementation of the embedded browser needs a native window handle
    // Can't get this via GLUT so had to use this hack
        return FindWindow( NULL, mAppWindowName.c_str() );
}
#else

#include <gtk/gtk.h>

////////////////////////////////////////////////////////////////////////////////
//
void testGL::idle()
{
    // pump the GTK+Gecko event queue for a (limited) while.  this should
    // be done so that the Gecko event queue doesn't starve, and done
    // *here* so that mNeedsUpdate[] can be populated by callbacks
    // from Gecko.
    gtk_main_iteration_do(0);
    for (int iter=0; iter<10; ++iter)
    if (gtk_events_pending())
	    gtk_main_iteration();


   //std::cout<<"idle()"<<std::endl;

	// onPageChanged event sets this
	// if ( mNeedsUpdate )
		// grab a page but don't reset 'needs update' flag until we've written it to the texture in display()
		LLMozLib::getInstance()->grabBrowserWindow( mBrowserWindowId );

	// lots of updates for smooth motion
	glutPostRedisplay();
};


void* testGL::getNativeWindowHandle()
{
	gtk_disable_setlocale();
	gtk_init(NULL, NULL);

	GtkWidget *win = gtk_window_new(GTK_WINDOW_POPUP);
	// Why a layout widget?  A MozContainer would be ideal, but
	// it involves exposing Mozilla headers to mozlib-using apps.
	// A layout widget with a GtkWindow parent has the desired
	// properties of being plain GTK, having a window, and being
	// derived from a GtkContainer.
	GtkWidget *rtnw = gtk_layout_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(win), rtnw);
	gtk_widget_realize(rtnw);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(rtnw), GTK_NO_WINDOW);

	return rtnw;
};
#endif

////////////////////////////////////////////////////////////////////////////////
//
void glutReshape( int widthIn, int heightIn )
{
	if ( theApp )
		theApp->reshape( widthIn, heightIn );
};

////////////////////////////////////////////////////////////////////////////////
//
void glutDisplay()
{
	if ( theApp )
		theApp->display();
};

////////////////////////////////////////////////////////////////////////////////
//
void glutIdle()
{
	if ( theApp )
		theApp->idle();
};

////////////////////////////////////////////////////////////////////////////////
//
void glutKeyboard( unsigned char keyIn, int xIn, int yIn )
{
	if ( keyIn == 27 )
	{
		if ( theApp )
			theApp->reset();

		exit( 0 );
	};

	if ( theApp )
		theApp->keyboard( keyIn, xIn, yIn );
};

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseMove( int xIn , int yIn )
{
	if ( theApp )
		theApp->mouseMove( xIn, yIn );
}

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseButton( int buttonIn, int stateIn, int xIn, int yIn )
{
	if ( theApp )
		theApp->mouseButton( buttonIn, stateIn, xIn, yIn );
}

static void on_destroy(GtkWidget * widget, gpointer data) {
        gtk_main_quit ();

} 


////////////////////////////////////////////////////////////////////////////////
//
int main( int argc, char* argv[] )
{

	// implementation in a class so we can observer events
	// means we need this painful GLUT <--> class shim...
	theApp = new testGL;

	if ( theApp )
	{
		glutInit( &argc, argv );
		glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );

		glutInitWindowPosition( 80, 0 );
		glutInitWindowSize( theApp->getAppWindowWidth(), theApp->getAppWindowHeight() );

		glutCreateWindow( theApp->getAppWindowName().c_str() );

		theApp->init( argv[ 0 ] );

		glutKeyboardFunc( glutKeyboard );

		glutMouseFunc( glutMouseButton );
		glutPassiveMotionFunc( glutMouseMove );
		glutMotionFunc( glutMouseMove );

		glutDisplayFunc( glutDisplay );
		glutReshapeFunc( glutReshape );

		glutIdleFunc( glutIdle );

		glutMainLoop();

		delete theApp;
	};

	return 0;
}
