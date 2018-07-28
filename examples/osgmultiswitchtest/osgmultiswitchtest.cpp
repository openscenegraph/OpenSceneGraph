/* OpenSceneGraph example, osgmultiswitchtest.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgSim/MultiSwitch>
#include <osg/Node>
#include <osgDB/Registry>
#include <stdexcept>

#include <iostream>

void assert2(bool success) {
    if (!success) throw std::runtime_error("");
}

void testSerializer(const osgSim::MultiSwitch::SwitchSetList &values,
    const osgSim::MultiSwitch::SwitchSetNameList &names) {

    osg::ref_ptr<osgSim::MultiSwitch> ms(new osgSim::MultiSwitch);
    if (values.size() > 0) {
        size_t nchildren = values[0].size();
        for (size_t i = 0; i < nchildren; i++) {
            ms->addChild(new osg::Node);
        }
    }

    ms->setSwitchSetList(values);
    for (size_t i = 0; i < names.size(); i++) {
        ms->setValueName(i, names[i]);
    }

    osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension("osgb");
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options;
    std::stringstream ss;

    // write
    osgDB::ReaderWriter::WriteResult wresult = rw->writeNode(*ms, ss, options);

    // read
	osgDB::ReaderWriter::ReadResult rresult = rw->readNode(ss, options);
    osg::ref_ptr<osg::Node> node = rresult.takeNode();

    osg::ref_ptr<osgSim::MultiSwitch> ms2(dynamic_cast<osgSim::MultiSwitch*>(node.get()));
    assert2(ms2 != NULL);

    assert2(ms2->getSwitchSetList() == values);
    for (size_t i = 0; i < values.size(); i++) {
        assert2(ms2->getValueName(i) == names[i]);
    }
    assert2(ms->getNumChildren() == ms2->getNumChildren());
}

int main()
{
	osgSim::MultiSwitch::SwitchSetList values;
	osgSim::MultiSwitch::SwitchSetNameList names;
    //testSerializer(
    //    {
    //        {false, false},
    //        {true, true}
    //    },
    //    { "offs", "ons" }
    //);
	std::vector<bool> offs(false, 2);
	std::vector<bool> ons(true, 2);
	values.push_back(offs);
	values.push_back(ons);
	names.push_back("offs");
	names.push_back("ons");
	testSerializer(values, names);

    //testSerializer({}, {});
	values.clear();
	names.clear();
	testSerializer(values, names);

    //testSerializer(
    //    {
    //        { true, true, false, true, false, true }
    //    },
    //    {"stuff"}
    //);
	values.clear();
	std::vector<bool> stuff;
	stuff.push_back(true);
	stuff.push_back(true);
	stuff.push_back(false);
	stuff.push_back(true);
	stuff.push_back(false);
	stuff.push_back(true);
	values.push_back(stuff);

	names.clear();
	names.push_back("stuff");
	testSerializer(values, names);

    return 0;
}
