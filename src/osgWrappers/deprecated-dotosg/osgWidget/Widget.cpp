// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Widget.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Util>
#include <osgWidget/Widget>

bool osgWidget_Widget_readData(osg::Object& obj, osgDB::Input& fr) {
	osgWidget::warn() << "Widget read" << std::endl;

	return false;
}

bool osgWidget_Widget_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{

	// const osgWidget::Widget& model = static_cast<const osgWidget::Widget&>(obj);

	fw.indent() << fw.wrapString("Widget stuff...") << std::endl;

	return true;
}

bool osgWidget_NotifyWidget_readData(osg::Object& /*obj*/, osgDB::Input& fr)
{
	osgWidget::warn() << "NotifyWidget read" << std::endl;

	return false;
}

bool osgWidget_NotifyWidget_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::NotifyWidget& model = static_cast<const osgWidget::NotifyWidget&>(obj);

	fw.indent() << fw.wrapString("NotifyWidget stuff...") << std::endl;

	return true;
}

bool osgWidget_NullWidget_readData(osg::Object& /*obj*/, osgDB::Input& fr)
{
	osgWidget::warn() << "NullWidget read" << std::endl;

	return false;
}

bool osgWidget_NullWidget_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::NullWidget& model = static_cast<const osgWidget::NullWidget&>(obj);

	fw.indent() << fw.wrapString("NullWidget stuff...") << std::endl;

	return true;
}

REGISTER_DOTOSGWRAPPER(g_osgWidget_WidgetProxy)
(
	new osgWidget::Widget("unset"),
	"osgWidget::Widget",
	"Object Drawable Geometry osgWidget::Widget",
	&osgWidget_Widget_readData,
	&osgWidget_Widget_writeData
);

REGISTER_DOTOSGWRAPPER(g_osgWidget_NotifyWidgetProxy)
(
	new osgWidget::NotifyWidget("unset"),
	"osgWidget::NotifyWidget",
	"Object Drawable Geometry osgWidget::Widget osgWidget::NotifyWidget",
	&osgWidget_NotifyWidget_readData,
	&osgWidget_NotifyWidget_writeData
);

REGISTER_DOTOSGWRAPPER(g_osgWidget_NullWidgetProxy)
(
	new osgWidget::Widget("unset"),
	"osgWidget::NullWidget",
	"Object Drawable Geometry osgWidget::Widget osgWidget::NullWidget",
	&osgWidget_NullWidget_readData,
	&osgWidget_NullWidget_writeData
);
