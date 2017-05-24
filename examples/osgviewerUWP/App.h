#pragma once

#include <string>

#include "pch.h"

#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

namespace example_osgViewerUWP
{
    ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
    {
    public:
        App();

        // IFrameworkView Methods.
        virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
        virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
        virtual void Load(Platform::String^ entryPoint);
        virtual void Run();
        virtual void Uninitialize();

    private:

        // Application lifecycle event handlers.
        void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);

        // Window event handlers.
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
        void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

		void TransformMouseXY(float& x, float& y, float width, float height);

		void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

        void InitializeEGL(Windows::UI::Core::CoreWindow^ window);
        void CleanupEGL();

        bool mWindowClosed;
        bool mWindowVisible;
        
        EGLDisplay mEglDisplay;
        EGLContext mEglContext;
        EGLSurface mEglSurface;

		osg::ref_ptr<osg::MatrixTransform> _root;
		osg::ref_ptr<osgViewer::Viewer> _viewer;
    };

}