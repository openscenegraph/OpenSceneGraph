/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef WIN32
#  pragma warning (disable:4786)
#  pragma warning (disable:4541)
#endif
#include "readerwriter.h"

#include <stdio.h>
#include <iostream>

#include <osg/Notify>

#include "mynode.h"
#include "osgvisitor.h"

extern int yyparse();
extern int yydebug;
extern MyNode *getRoot();
extern FILE *yyin;
int isatty(int) { return 0; }
extern void flush_scanner();


osgDB::ReaderWriter::ReadResult VrmlReaderWriter::readNode(const std::string& fileName,
							 const osgDB::ReaderWriter::Options*)
{
    std::string ext = osgDB::getFileExtension(fileName);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    yydebug=0;
    yyin=fopen(fileName.c_str(),"r");
    osg::notify(osg::INFO) << "Parsing..." << std::endl;
    if (yyparse()!=0)
    {
        flush_scanner();
        return ReadResult::FILE_NOT_HANDLED;
    }
    osg::ref_ptr<MyNode> n=getRoot();
    try
    {
	osg::notify(osg::INFO) << "Generating OSG tree..." << std::endl;
	osg::ref_ptr<OSGVisitor> visitante=new OSGVisitor(n.get());
	return visitante->getRoot();
    }
    catch (...)
    {
	osg::notify(osg::INFO) << "VRML: error reading" << std::endl;
	return ReadResult::ERROR_IN_READING_FILE;
    }
}

osgDB::RegisterReaderWriterProxy<VrmlReaderWriter> g_readerWriter_VRML_Proxy;
