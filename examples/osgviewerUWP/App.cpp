//
// This file demonstrates how to initialize EGL in a Windows Store app, using ICoreWindow.
//

#include "pch.h"
#include "app.h"

#include <osg/ShapeDrawable>
#include <osg/NodeVisitor>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgGA/OrbitManipulator>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ParticleSystem>

#include "osgPlugins.h"
#include "shaders.h"

//
// Notes:
// To run you need to add the libEGL.dll and libGLES2.dll files to the target and set their Content property to true
// Open package.appxManifest as text and around line 21 change Executable to Executable="osgviewerUWPd.exe" for debug builds or Executable="osgviewerUWP.exe" for release
//

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

using namespace example_osgViewerUWP;


class FindParticleSystem : public osg::NodeVisitor
{
public:
	FindParticleSystem() 
		: osg::NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{}
	virtual ~FindParticleSystem() {}
	virtual	void apply(osg::Node &node) {
		osgParticle::ModularEmitter* asEmmiter = dynamic_cast<osgParticle::ModularEmitter*>(&node);
		if (asEmmiter != NULL) _systems.push_back(asEmmiter->getParticleSystem());
		traverse(node);
	}
	std::vector<osgParticle::ParticleSystem*> _systems;
};


// Helper to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class SimpleApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new App();
    }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto simpleApplicationSource = ref new SimpleApplicationSource();
    CoreApplication::Run(simpleApplicationSource);
    return 0;
}

App::App() :
    mWindowClosed(false),
    mWindowVisible(true),
    mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT),
    mEglSurface(EGL_NO_SURFACE)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated += 
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    // Logic for other event handlers could go here.
    // Information about the Suspending and Resuming event handlers can be found here:
    // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerMoved);
	window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerReleased);


    // The CoreWindow has been created, so EGL can be initialized.
    InitializeEGL(window);

	osg::setNotifyHandler(new osg::WinDebugNotifyHandler());
	osg::setNotifyLevel(osg::INFO);

	osg::DisplaySettings::instance()->setVertexBufferHint(osg::DisplaySettings::VertexBufferHint::VERTEX_BUFFER_OBJECT);

	_viewer = new osgViewer::Viewer();
	_viewer->setUpViewerAsEmbeddedInWindow(0, 0, (int)window->Bounds.Width, (int)window->Bounds.Height);
	_viewer->getEventQueue()->setUseFixedMouseInputRange(true);

	_viewer->getCamera()->setProjectionMatrixAsPerspective(90.0f, window->Bounds.Width/window->Bounds.Height, 1.0f, 1000.0f);
	_viewer->getCamera()->setViewport(0, 0, window->Bounds.Width, window->Bounds.Height);
	_viewer->getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_viewer->getCamera()->setClearColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));

}

// Initializes scene resources
void App::Load(Platform::String^ entryPoint)
{
	_root = new osg::MatrixTransform();

	std::string datapath = osgDB::convertUTF16toUTF8(Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data());
	std::string modelpath = datapath + "\\pallet.osg";

	bool exists = osgDB::fileExists(modelpath);

	osg::Node* model = NULL;
	if (exists) {
		model = osgDB::readNodeFile(modelpath);

		FindParticleSystem findps;
		model->accept(findps);
		for (unsigned int i = 0; i < findps._systems.size(); i++) {
			osgParticle::ParticleSystem* ps = findps._systems[i];
			ps->setUseVertexArray(true);
			ps->setUseShaders(true);
			osgParticle::Particle dp = ps->getDefaultParticleTemplate();
			dp.setShape(osgParticle::Particle::Shape::POINT);
			ps->setDefaultParticleTemplate(dp);

			osg::Program* program = new osg::Program();
			program->addShader(new osg::Shader(osg::Shader::VERTEX, ParticleShaderVert));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, ParticleShaderFrag));
			ps->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);
		}
		OSG_ALWAYS << "Found " << findps._systems.size() << " particle systems." << std::endl;
	}
	else {
		osg::Geode* geode = new osg::Geode();
		osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), 1));
		drawable->setColor(osg::Vec4(0.1, 0.1, 0.9, 1.0));
		drawable->setUseVertexBufferObjects(true);
		geode->addDrawable(drawable);
		model = geode;
	}

	osgUtil::Optimizer optimizer;
	optimizer.optimize(model, osgUtil::Optimizer::TRISTRIP_GEOMETRY);

	osg::Program* program = new osg::Program();
	//program->addShader(new osg::Shader(osg::Shader::VERTEX, ColorShaderVert));
	//program->addShader(new osg::Shader(osg::Shader::FRAGMENT, ColorShaderFrag));
	program->addShader(new osg::Shader(osg::Shader::VERTEX, TextureShaderVert));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, TextureShaderFrag));
	model->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);

	_root->addChild(model);

	_viewer->setSceneData(_root);
	_viewer->setCameraManipulator(new osgGA::OrbitManipulator());
}


// This method is called after the window becomes active.
void App::Run()
{
    while (!mWindowClosed)
    {
        if (mWindowVisible)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			EGLint panelWidth = 0;
			EGLint panelHeight = 0;
			eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, &panelWidth);
			eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, &panelHeight);

			_viewer->getCamera()->setProjectionMatrixAsPerspective(90.0f, panelWidth / panelHeight, 1.0f, 1000.0f);
			_viewer->getCamera()->setViewport(0, 0, panelWidth, panelHeight);

			_viewer->frame();

            // The call to eglSwapBuffers might not be successful (e.g. due to Device Lost)
            // If the call fails, then we must reinitialize EGL and the GL resources.
            if (eglSwapBuffers(mEglDisplay, mEglSurface) != GL_TRUE)
            {
                CleanupEGL();

                InitializeEGL(CoreWindow::GetForCurrentThread());
				// need to re init osgViewer an scenegraph
            }
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }

    CleanupEGL();
}

// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handler.
void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

// Window event handlers.
void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    mWindowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    mWindowClosed = true;
}

void App::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	float mx = args->CurrentPoint->Position.X;
	float my = args->CurrentPoint->Position.Y;
	TransformMouseXY(mx, my, sender->Bounds.Width, sender->Bounds.Height);
	osg::ref_ptr<osgGA::GUIEventAdapter> osgevent = _viewer->getEventQueue()->mouseMotion(mx, my);
}

void App::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	int button = 1;// args->CurrentPoint->Properties->IsLeftButtonPressed ? 1 : 2; // this is a hack as example
	float mx = args->CurrentPoint->Position.X;
	float my = args->CurrentPoint->Position.Y;
	TransformMouseXY(mx, my, sender->Bounds.Width, sender->Bounds.Height);
	osg::ref_ptr<osgGA::GUIEventAdapter> osgevent = _viewer->getEventQueue()->mouseButtonPress(mx, my, button);
}

void App::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
	int button = 1;
	float mx = args->CurrentPoint->Position.X;
	float my = args->CurrentPoint->Position.Y;
	TransformMouseXY(mx, my, sender->Bounds.Width, sender->Bounds.Height);
	osg::ref_ptr<osgGA::GUIEventAdapter> osgevent = _viewer->getEventQueue()->mouseButtonRelease(mx, my, button);
}

void App::TransformMouseXY(float& x, float& y, float width, float height)
{
	if (_viewer->getEventQueue()->getUseFixedMouseInputRange())
	{
		osgGA::GUIEventAdapter* eventState = _viewer->getEventQueue()->getCurrentEventState();

		x = eventState->getXmin() + (eventState->getXmax() - eventState->getXmin())*x / width;
		y = eventState->getYmin() + (eventState->getYmax() - eventState->getYmin())*y / height;
	}
}

void App::InitializeEGL(CoreWindow^ window)
{
    const EGLint configAttributes[] = 
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    const EGLint contextAttributes[] = 
    { 
        EGL_CONTEXT_CLIENT_VERSION, 2, 
        EGL_NONE
    };

    const EGLint surfaceAttributes[] =
    {
        // EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER is part of the same optimization as EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER (see above).
        // If you have compilation issues with it then please update your Visual Studio templates.
        EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_NONE
    };

    const EGLint defaultDisplayAttributes[] =
    {
        // These are the default display attributes, used to request ANGLE's D3D11 renderer.
        // eglInitialize will only succeed with these attributes if the hardware supports D3D11 Feature Level 10_0+.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

        // EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER is an optimization that can have large performance benefits on mobile devices.
        // Its syntax is subject to change, though. Please update your Visual Studio templates if you experience compilation issues with it.
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        
        // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that enables ANGLE to automatically call 
        // the IDXGIDevice3::Trim method on behalf of the application when it gets suspended. 
        // Calling IDXGIDevice3::Trim when an application is suspended is a Windows Store application certification requirement.
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };
    
    const EGLint fl9_3DisplayAttributes[] =
    {
        // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature Level 9_3.
        // These attributes are used if the call to eglInitialize fails with the default display attributes.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };

    const EGLint warpDisplayAttributes[] =
    {
        // These attributes can be used to request D3D11 WARP.
        // They are used if eglInitialize fails with both the default display attributes and the 9_3 display attributes.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };
    
    EGLConfig config = NULL;

    // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in display attributes, used to configure D3D11.
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get function eglGetPlatformDisplayEXT");
    }

    //
    // To initialize the display, we make three sets of calls to eglGetPlatformDisplayEXT and eglInitialize, with varying 
    // parameters passed to eglGetPlatformDisplayEXT:
    // 1) The first calls uses "defaultDisplayAttributes" as a parameter. This corresponds to D3D11 Feature Level 10_0+.
    // 2) If eglInitialize fails for step 1 (e.g. because 10_0+ isn't supported by the default GPU), then we try again 
    //    using "fl9_3DisplayAttributes". This corresponds to D3D11 Feature Level 9_3.
    // 3) If eglInitialize fails for step 2 (e.g. because 9_3+ isn't supported by the default GPU), then we try again 
    //    using "warpDisplayAttributes".  This corresponds to D3D11 Feature Level 11_0 on WARP, a D3D11 software rasterizer.
    //
    
    // This tries to initialize EGL to D3D11 Feature Level 10_0+. See above comment for details.
    mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, defaultDisplayAttributes);
    if (mEglDisplay == EGL_NO_DISPLAY)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
    }

    if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE)
    {
        // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is unavailable (e.g. on some mobile devices).
        mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
        if (mEglDisplay == EGL_NO_DISPLAY)
        {
            throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
        }

        if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE)
        {
            // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is unavailable on the default GPU.
            mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warpDisplayAttributes);
            if (mEglDisplay == EGL_NO_DISPLAY)
            {
                throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
            }

            if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE)
            {
                // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
                throw Exception::CreateException(E_FAIL, L"Failed to initialize EGL");
            }
        }
    }

    EGLint numConfigs = 0;
    if ((eglChooseConfig(mEglDisplay, configAttributes, &config, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0))
    {
        throw Exception::CreateException(E_FAIL, L"Failed to choose first EGLConfig");
    }

    // Create a PropertySet and initialize with the EGLNativeWindowType.
    PropertySet^ surfaceCreationProperties = ref new PropertySet();
    surfaceCreationProperties->Insert(ref new String(EGLNativeWindowTypeProperty), window);

    // You can configure the surface to render at a lower resolution and be scaled up to 
    // the full window size. The scaling is often free on mobile hardware.
    //
    // One way to configure the SwapChainPanel is to specify precisely which resolution it should render at.
    // Size customRenderSurfaceSize = Size(800, 600);
    // surfaceCreationProperties->Insert(ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize(customRenderSurfaceSize));
    //
    // Another way is to tell the SwapChainPanel to render at a certain scale factor compared to its size.
    // e.g. if the SwapChainPanel is 1920x1280 then setting a factor of 0.5f will make the app render at 960x640
    // float customResolutionScale = 0.5f;
    // surfaceCreationProperties->Insert(ref new String(EGLRenderResolutionScaleProperty), PropertyValue::CreateSingle(customResolutionScale));

    mEglSurface = eglCreateWindowSurface(mEglDisplay, config, reinterpret_cast<IInspectable*>(surfaceCreationProperties), surfaceAttributes);
    if (mEglSurface == EGL_NO_SURFACE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL fullscreen surface");
    }

    mEglContext = eglCreateContext(mEglDisplay, config, EGL_NO_CONTEXT, contextAttributes);
    if (mEglContext == EGL_NO_CONTEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL context");
    }

    if (eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make fullscreen EGLSurface current");
    }
}

void App::CleanupEGL()
{
    if (mEglDisplay != EGL_NO_DISPLAY && mEglSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(mEglDisplay, mEglSurface);
        mEglSurface = EGL_NO_SURFACE;
    }

    if (mEglDisplay != EGL_NO_DISPLAY && mEglContext != EGL_NO_CONTEXT)
    {
        eglDestroyContext(mEglDisplay, mEglContext);
        mEglContext = EGL_NO_CONTEXT;
    }

    if (mEglDisplay != EGL_NO_DISPLAY)
    {
        eglTerminate(mEglDisplay);
        mEglDisplay = EGL_NO_DISPLAY;
    }
}
