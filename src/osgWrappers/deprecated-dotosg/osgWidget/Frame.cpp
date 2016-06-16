// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Frame.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Frame>

bool osgWidget_Frame_readData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
	osgWidget::warn() << "Frame read" << std::endl;

	return false;
}

bool osgWidget_Frame_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Frame& model = static_cast<const osgWidget::Frame&>(obj);

	fw.indent() << fw.wrapString("Frame stuff...") << std::endl;

	return true;
}

REGISTER_DOTOSGWRAPPER(g_osgWidget_FrameProxy)
(
	new osgWidget::Frame("unset"),
	"osgWidget::Frame",
	"Object Node Group Transform MatrixTransform osgWidget::Table osgWidget::Frame",
	&osgWidget_Frame_readData,
	&osgWidget_Frame_writeData
);
