// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Box.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/FileUtils>
#include <osgWidget/Box>

bool osgWidget_Box_readData(osg::Object& /*obj*/, osgDB::Input& fr) 
{
	/*
	osgWidget::Box& box = static_cast<osgWidgegt::Box&>(obj);

	if(fr[0].matchWord("skeleton") and fr[1].isString()) iter = loadFile(
		"skeleton",
		&osgCal::osgWidget_Box::loadSkeleton,
		model,
		fr
	);
		
	if(fr[0].matchWord("animation") and fr[1].isString()) iter = loadFile(
		"animation",
		&osgCal::osgWidget_Box::loadAnimation,
		model,
		fr
	);

	if(fr[0].matchWord("mesh") and fr[1].isString()) iter = loadFile(
		"mesh",
		&osgCal::osgWidget_Box::loadMesh,
		model,
		fr
	);

	if(fr[0].matchWord("material") and fr[1].isString()) iter = loadFile(
		"material",
		&osgCal::osgWidget_Box::loadMaterial,
		model,
		fr
	);
	*/
	
	osgWidget::warn() << "Box read" << std::endl;

	return false;
}

bool osgWidget_Box_writeData(const osg::Object& /*obj*/, osgDB::Output& fw)
{
	// const osgWidget::Box& model = static_cast<const osgWidget::Box&>(obj);
	
	fw.indent() << fw.wrapString("Box stuff...") << std::endl;

	return true;
}

/*
bool Model_readData(osg::Object& obj, osgDB::Input& fr) {
	bool iter = false;

	osgCal::Model&     model = static_cast<osgCal::Model&>(obj);
	osgCal::osgWidget_Box* core  = static_cast<osgCal::osgWidget_Box*>(
		fr.readObjectOfType(osgCal::osgWidget_Box("dummy"))
	);

	if(core) {
		model.create(core);
	
		iter = true;
	}

	if(fr.matchSequence("StartAnimation")) {
		if(fr[1].isString()) {
			int animation = core->getAnimationId(fr[1].getStr());

			if(animation >= 0) model.startLoop(animation, 1.0f, 0.0f);

			else osg::notify(osg::WARN)
				<< "Couldn't start animation: " << fr[1].getStr()
				<< std::endl
			;

			iter  = true;
			fr   += 2;
		}
	}

	return iter;
}

bool Model_writeData(const osg::Object& obj, osgDB::Output& fw) {
	const osgCal::Model& model = static_cast<const osgCal::Model&>(obj);

	fw.writeObject(*model.getosgWidget_Box());

	return true;
}

osgDB::RegisterDotOsgWrapperProxy g_ModelProxy(
	new osgCal::Model,
	"Model",
	"Object Node Model",
	&Model_readData,
	&Model_writeData
);
*/

osgDB::RegisterDotOsgWrapperProxy g_osgWidget_BoxProxy(
	new osgWidget::Box("unset"),
	"osgWidget::Box",
	"Object Node Group Transform MatrixTransform osgWidget::Box",
	&osgWidget_Box_readData,
	&osgWidget_Box_writeData
);
