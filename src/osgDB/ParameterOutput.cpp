#include <osgDB/ParameterOutput>

using namespace osgDB;

ParameterOutput::ParameterOutput(Output& fw):
    _fw(fw),
    _numItemsPerLine(fw.getNumIndicesPerLine()),
    _column(0) {}

ParameterOutput::ParameterOutput(Output& fw,int numItemsPerLine):
    _fw(fw),
    _numItemsPerLine(numItemsPerLine),
    _column(0) {}

void ParameterOutput::begin()
{
    _fw.indent() << "{"<<std::endl;
    _fw.moveIn();
}

void ParameterOutput::newLine()
{
    if (_column!=0) _fw << std::endl;
    _column = 0;
}

void ParameterOutput::end()
{
    if (_column!=0) _fw << std::endl;
    _fw.moveOut();
    _fw.indent() << "}"<<std::endl;
    _column = 0;
}
