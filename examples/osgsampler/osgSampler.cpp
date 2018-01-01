/*  -*-c++-*-
 *  Copyright (C) 2017 Julien Valentin <mp3butcher@hotmail.com>
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


#include <osg/Program>
#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

/// add two composite texture color
static const char* fragSource =
{
    "uniform sampler2D tex1;\n"
    "uniform sampler2D tex2;\n"
    "void main(){\n"
    "    vec4 color1 = texture2D(tex1,gl_TexCoord[0].st);\n"
    "    vec4 color2 = texture2D(tex2,gl_TexCoord[0].st);\n"
    "    gl_FragColor = vec4(color1.xyz+color2.xyz,1.0);\n"
    "}\n"
};


class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    KeyboardEventHandler(osg::Sampler* tessInnerU, osg::Sampler* tessOuterU,osg::StateSet* ss):
        _sampler1(tessInnerU),
        _sampler2(tessOuterU),_ss(ss){}

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& gaa)
    {
        osg::Texture:: FilterMode newone=osg::Texture::NEAREST;
        if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
        {
            switch (ea.getKey())
            {
            case osgGA::GUIEventAdapter::KEY_A:
                if(_sampler1->getFilter(osg::Texture::MAG_FILTER)==osg::Texture::NEAREST)
                    newone=osg::Texture::LINEAR;
                _sampler1->setFilter(osg::Texture::MAG_FILTER,newone);
                _sampler1->setFilter(osg::Texture::MIN_FILTER,newone);
                return true;
            case osgGA::GUIEventAdapter::KEY_B:
                if(_sampler2->getFilter(osg::Texture::MAG_FILTER)==osg::Texture::NEAREST)
                    newone=osg::Texture::LINEAR;
                _sampler2->setFilter(osg::Texture::MAG_FILTER,newone);
                _sampler2->setFilter(osg::Texture::MIN_FILTER,newone);
                return true;

            }
        }
        return osgGA::GUIEventHandler::handle(ea, gaa);
    }

private:
    osg::Sampler* _sampler1;
    osg::Sampler* _sampler2;
    osg::StateSet* _ss;

};

int main(int argc, char* argv[])
{
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example for GL3 sampler.");
    osg::ApplicationUsage::UsageMap kmap;
    kmap["a"]="swap Linear/Nearest filtering on first texture";
    kmap["b"]="swap Linear/Nearest filtering on second texture";
    arguments.getApplicationUsage()->setKeyboardMouseBindings(kmap);

    osgViewer::Viewer viewer(arguments);
    osg::ref_ptr<osg::Program> program = new osg::Program();
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragSource));

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    osg::ref_ptr<osg::Node> geode ;
    if (!loadedModel)
    {
        geode = new osg::Geode;
        ((osg::Geode*)geode.get())->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(0,0,0),osg::Vec3(1,0,0),osg::Vec3(0,0,1) ));
    }
    else
    {
        geode=loadedModel;
    }

    osg::ref_ptr< osg::Texture2D> tex1, tex2;
    osg::ref_ptr<osg::Sampler > sampler1, sampler2;

    tex1=new osg::Texture2D();
    tex2=new osg::Texture2D();

    sampler1=new osg::Sampler();
    sampler2=new osg::Sampler();

    osg::Vec2ui resolution(4,4);
    {
        ///first texture//NO RED
        unsigned char *data = new  unsigned char[(resolution.x()* resolution.y()) * 4];
        unsigned char * ptr=data;
        unsigned int   sw=0,cptx=0;
        while (ptr != data + sizeof(unsigned char)*(resolution.x()* resolution.y()) * 4)
        {
            if(sw==1)
            {
                *ptr++=0;
                *ptr++=0xff;
                *ptr++=0xff;
                *ptr++=0xff;
            }
            else
            {
                *ptr++=0;
                *ptr++=0;
                *ptr++=0;
                *ptr++=0xff;
            }
            if(++cptx<resolution.x())sw=sw?0:1;
            else cptx=0;
        }
        osg::ref_ptr<osg::Image >im = new osg::Image();
        im->setImage(resolution.x(), resolution.y(), 1,GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
        im->dirty();
        tex1->setImage(im);
    }
    {
        ///second texture// RED ONLY
        unsigned char *data = new unsigned char[(resolution.x()* resolution.y()) * 4];
        unsigned char * ptr=data;
        unsigned int   sw=0,cptx=0;
        while (ptr != data + sizeof(unsigned char)*(resolution.x()* resolution.y()) * 4)
        {
            if(sw==1)
            {
                *ptr++=255;
                *ptr++=0;
                *ptr++=0;
                *ptr++=255;
            }
            else
            {
                *ptr++=0;
                *ptr++=0;
                *ptr++=0;
                *ptr++=255;
            }
            if(++cptx<resolution.x())sw=sw?0:1;
            else cptx=0;
        }
        osg::ref_ptr<osg::Image >im = new osg::Image();
        im->setImage(resolution.x(), resolution.y(), 1,GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
        im->dirty();
        tex2->setImage(im);
    }
    ///Overrided Filtering setup
    tex1->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
    tex1->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);

    tex2->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
    tex2->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);

    ///Filter Override samplers setup
    sampler1->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    sampler1->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);

    sampler2->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    sampler2->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);


    osg::StateSet *ss;
    ss = geode->getOrCreateStateSet();

    tex1->setSampler(sampler1);
    tex2->setSampler(sampler2);
    ss->setTextureAttribute(0,tex1,osg::StateAttribute::ON);
    //ss->setTextureAttribute(0,sampler1,osg::StateAttribute::ON);
    ss->setTextureAttribute(1,tex2,osg::StateAttribute::ON);
    //ss->setTextureAttribute(1,sampler2,osg::StateAttribute::ON);

    ss->addUniform(new osg::Uniform("tex1",(int)0));
    ss->addUniform(new osg::Uniform("tex2",(int)1));
    ss->setAttribute(program.get());


    viewer.addEventHandler(new KeyboardEventHandler(sampler1, sampler2,ss));
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    viewer.setSceneData(geode.get());
    return viewer.run();
}

