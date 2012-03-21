// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Input.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Input>

bool osgWidget_Input_readData(osg::Object& /*obj*/, osgDB::Input& fr)
{
	osgWidget::warn() << "Input read" << std::endl;

	return false;
}

bool osgWidget_Input_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Input& model = static_cast<const osgWidget::Input&>(obj);

	fw.indent() << fw.wrapString("Input stuff...") << std::endl;

	return true;
}

REGISTER_DOTOSGWRAPPER(g_osgWidget_InputProxy)
(
	new osgWidget::Input("unset"),
	"osgWidget::Input",
	"Object Drawable Geometry osgWidget::Widget osgWidget::Input",
	&osgWidget_Input_readData,
	&osgWidget_Input_writeData
);
