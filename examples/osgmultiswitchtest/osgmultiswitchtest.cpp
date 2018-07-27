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

#include <iostream>

void assert2(bool success) {
    if (!success) throw std::runtime_error("");
}

void testSerializer(const osgSim::MultiSwitch::SwitchSetList &values,
    const osgSim::MultiSwitch::SwitchSetNameList &names) {

    osg::ref_ptr<osgSim::MultiSwitch> ms(new osgSim::MultiSwitch);
    if (values.size() > 0) {
        int nchildren = values[0].size();
        for (int i = 0; i < nchildren; i++) {
            ms->addChild(new osg::Node);
        }
    }

    ms->setSwitchSetList(values);
    for (int i = 0; i < names.size(); i++) {
        ms->setValueName(i, names[i]);
    }

    osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension("osgb");
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options;
    std::stringstream ss;

    // write
    auto wresult = rw->writeNode(*ms, ss, options);

    // read
    auto rresult = rw->readNode(ss, options);
    osg::ref_ptr<osg::Node> node = rresult.takeNode();

    osg::ref_ptr<osgSim::MultiSwitch> ms2(dynamic_cast<osgSim::MultiSwitch*>(node.get()));
    assert2(ms2 != nullptr);

    assert2(ms2->getSwitchSetList() == values);
    for (int i = 0; i < values.size(); i++) {
        assert2(ms2->getValueName(i) == names[i]);
    }
    assert2(ms->getNumChildren() == ms2->getNumChildren());
}

int main( int argc, char **argv )
{
    testSerializer(
        {
            {false, false},
            {true, true}
        },
        { "offs", "ons" }
    );

    testSerializer({}, {});

    testSerializer(
        {
            { true, true, false, true, false, true }
        },
        {"stuff"}
    );

    return 0;
}
