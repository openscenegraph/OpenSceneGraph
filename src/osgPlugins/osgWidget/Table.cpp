// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Table.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Table>

bool osgWidget_Table_readData(osg::Object& /*obj*/, osgDB::Input& fr) {
	osgWidget::warn() << "Table read" << std::endl;
	
	return false;
}

bool osgWidget_Table_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Table& model = static_cast<const osgWidget::Table&>(obj);
	
	fw.indent() << fw.wrapString("Table stuff...") << std::endl;

	return true;
}

osgDB::RegisterDotOsgWrapperProxy g_osgWidget_TableProxy(
	new osgWidget::Table("unset"),
	"osgWidget::Table",
	"Object Node Group Transform MatrixTransform osgWidget::Table",
	&osgWidget_Table_readData,
	&osgWidget_Table_writeData
);
