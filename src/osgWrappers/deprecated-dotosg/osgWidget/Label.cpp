// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Label.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Label>

bool osgWidget_Label_readData(osg::Object& /*obj*/, osgDB::Input& fr)
{
	osgWidget::warn() << "Label read" << std::endl;

	return false;
}

bool osgWidget_Label_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Label& model = static_cast<const osgWidget::Label&>(obj);

	fw.indent() << fw.wrapString("Label stuff...") << std::endl;

	return true;
}

REGISTER_DOTOSGWRAPPER(g_osgWidget_LabelProxy)
(
	new osgWidget::Label("unset"),
	"osgWidget::Label",
	"Object Drawable Geometry osgWidget::Widget osgWidget::Label",
	&osgWidget_Label_readData,
	&osgWidget_Label_writeData
);
