// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: EmbeddedWindow.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Window>

bool osgWidget_EmbeddedWindow_readData(osg::Object& /*obj*/, osgDB::Input& fr)
{
	osgWidget::warn() << "EmbeddedWindow read" << std::endl;

	return false;
}

bool osgWidget_EmbeddedWindow_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Window::EmbeddedWindow& model = static_cast<const osgWidget::Window::EmbeddedWindow&>(obj);

	fw.indent() << fw.wrapString("EmbeddedWindow stuff...") << std::endl;

	return true;
}

REGISTER_DOTOSGWRAPPER(g_osgWidget_EmbeddedWindowProxy)
(
	new osgWidget::Window::EmbeddedWindow("unset"),
	"osgWidget::Window::EmbeddedWindow",
	"Object Drawable Geometry osgWidget::Widget osgWidget::Window::EmbeddedWindow",
	&osgWidget_EmbeddedWindow_readData,
	&osgWidget_EmbeddedWindow_writeData
);
