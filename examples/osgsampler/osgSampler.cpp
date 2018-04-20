/*  -*-c++-*-
 *  Copyright (C) 2018 Julien Valentin <mp3butcher@hotmail.com>
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
#include <osg/Sampler>
#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

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


class SamplersKeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    SamplersKeyboardEventHandler(osg::Sampler* tessInnerU, osg::Sampler* tessOuterU):
        _sampler1(tessInnerU),
        _sampler2(tessOuterU){}

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

};

class StateCollec: public osg::NodeVisitor{
public:
    StateCollec():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){}
    virtual void apply(osg::Node& node){
        osg::StateSet *ss=node.getStateSet();
        if(ss) _statesets.insert(ss);
        traverse(node);
    }
    std::set<osg::StateSet*> _statesets;
    std::set<osg::StateSet*> & getStateSets(){return _statesets;}
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

    osg::ref_ptr<osg::Sampler > sampler1, sampler2;
    if (loadedModel)
    {
        ///ensure loaded have Sampler
        geode=loadedModel;
        StateCollec sc;
        geode->accept(sc);
        for(std::set<osg::StateSet*>::iterator it=sc.getStateSets().begin();it != sc.getStateSets().end(); ++it)
        {
            osg::Sampler::generateSamplerObjects(*(*it));
            if(!sampler1.valid())
                sampler1 = sampler2 = (osg::Sampler*)(*it)->getTextureAttribute(0, osg::StateAttribute::SAMPLER);
            else
                sampler2=(osg::Sampler*)(*it)->getTextureAttribute(0, osg::StateAttribute::SAMPLER);
        }
        if(sampler1.valid()&&sampler2.valid()){
            OSG_WARN<<"2samplers manipulator set"<<std::endl;
            viewer.addEventHandler(new SamplersKeyboardEventHandler(sampler1.get(), sampler2.get()));
        }
    }
    else
    {
        ///createQuadWith2TexturesMix
        geode = new osg::Geode;
        ((osg::Geode*)geode.get())->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(0,0,0),osg::Vec3(1,0,0),osg::Vec3(0,0,1) ));

        osg::ref_ptr< osg::Texture2D> tex1, tex2;

        tex1=new osg::Texture2D();
        tex2=new osg::Texture2D();

        sampler1=new osg::Sampler();
        sampler2=new osg::Sampler();

        osg::Vec2ui resolution(4,4);
        {
            ///first texture//NO RED
            unsigned char *data = new  unsigned char[(resolution.x()* resolution.y()) * 4];
            unsigned char * ptr=data;
            unsigned int   sw=0, cptx=0;
            while (ptr != data + sizeof(unsigned char)*(resolution.x()* resolution.y()) * 4)
            {
                if(sw==1)
                {
                    *ptr++=0;                    *ptr++=0xff;                *ptr++=0xff;                  *ptr++=0xff;
                }
                else
                {
                    *ptr++=0;                    *ptr++=0;                    *ptr++=0;                    *ptr++=0xff;
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
                    *ptr++=255;                  *ptr++=0;                    *ptr++=0;                    *ptr++=255;
                }
                else
                {
                    *ptr++=0;                    *ptr++=0;                    *ptr++=0;                    *ptr++=255;
                }
                if(++cptx<resolution.x())sw=sw?0:1;
                else cptx=0;
            }
            osg::ref_ptr<osg::Image >im = new osg::Image();
            im->setImage(resolution.x(), resolution.y(), 1,GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
            im->dirty();
            tex2->setImage(im);
        }
        ///Overridden Filtering setup
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

        ss->setTextureAttribute(0, tex1,osg::StateAttribute::ON);
        ss->setTextureAttribute(1, tex2,osg::StateAttribute::ON);
    //#define TEST_SAMPLER_OBJECT_AUTO_PORT 1
    #ifdef TEST_SAMPLER_OBJECT_AUTO_PORT

        osg::Sampler::generateSamplerObjects(*ss)
        if(
        (sampler1= static_cast<osg::Sampler*>(ss->getTextureAttribute(0,osg::StateAttribute::SAMPLER)))&&
        (sampler2= static_cast<osg::Sampler*>(ss->getTextureAttribute(1,osg::StateAttribute::SAMPLER)))
        )
        OSG_WARN<<"both samplers are generated"<<std::endl;
    #else
        ss->setTextureAttribute(0,sampler1,osg::StateAttribute::ON);
        ss->setTextureAttribute(1,sampler2,osg::StateAttribute::ON);
    #endif
        ss->addUniform(new osg::Uniform("tex1",(int)0));
        ss->addUniform(new osg::Uniform("tex2",(int)1));
        ss->setAttribute(program.get());

        viewer.addEventHandler(new SamplersKeyboardEventHandler(sampler1.get(), sampler2.get()));
    }

    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());

    viewer.setSceneData(geode.get());
    return viewer.run();
}
