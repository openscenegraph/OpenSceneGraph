/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

// main viewer code at bottom of file.

// experimental templated rendering code, please ignore...
// will move to osg::Geometry once complete.
// Robert Osfield, August 2003.
#if 0


struct DrawArrays
{
    virtual void draw() const = 0;
};

struct DrawVertex
{
    inline void operator () (const osg::Vec2* v) const { glVertex2fv(v->ptr()); }
    inline void operator () (const osg::Vec3* v) const { glVertex3fv(v->ptr()); }
    inline void operator () (const osg::Vec4* v) const { glVertex4fv(v->ptr()); }
};

struct DrawNormal
{
    inline void operator () (const osg::Vec3* v) const { glNormal3fv(v->ptr()); }
};

struct DrawColor
{
    inline void operator () (const osg::Vec4* v) const { glColor4fv(v->ptr()); }
};

struct DrawTexCoord
{
    inline void operator () (const osg::Vec2* v) const { glTexCoord2fv(v->ptr()); }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< class F1, class T1>
struct DrawFunctor_T : public DrawArrays
{
    inline DrawFunctor_T(T1* begin1, T1* end1): _begin1(begin1),_end1(end1) {}
        
    virtual void draw() const
    {
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1)
        {
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
};

template< class F1, class T1, class I1>
struct DrawFunctor_TI : public DrawArrays
{
    inline DrawFunctor_TI(T1* begin1, I1* ibegin1, I1* iend1): _begin1(begin1),_ibegin1(ibegin1),_iend1(end1) {}
        
    virtual void draw() const
    {
        for(I1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1)
        {
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< class F1, class T1, class F2, class T2>
struct DrawFunctor_TT : public DrawArrays
{
    inline DrawFunctor_TT(T1* begin1, T1* end1, T2* begin2): _begin1(begin1),_end1(end1),_begin2(begin2) {}
        
    virtual void draw() const
    {
        T2* ptr2=_begin2;
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1,++ptr2)
        {
            F2(ptr2);
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
    
    T2* _begin2;
};

template< class F1, class T1, class I1, class F2, class T2>
struct DrawFunctor_TIT : public DrawArrays
{
    inline DrawFunctor_TIT(T1* begin1, I1* ibegin1, I1* iend1, T2* begin2): _begin1(begin1),_ibegin1(ibegin1),_iend1(end1) {}
        
    virtual void draw() const
    {
        T2* ptr2=_begin2;
        for(I1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1,++ptr2)
        {
            F2(_ptr2);
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
    T2* _begin2;
    
};

template< class F1, class T1, class F2, class T2, class I2>
struct DrawFunctor_TTI : public DrawArrays
{
    inline DrawFunctor_TTI(T1* begin1, T1* end1, T2* begin2, I2* ibegin2): _begin1(begin1),_end1(end1),_begin2(begin2),_ibegin2(ibegin2) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1,++iptr2)
        {
            F2(_begin2[*iptr2]);
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
    
    T2* _begin2;
    I2* _ibegin2;
};

template< class F1, class T1, class F2, class I1, class T2, class I2>
struct DrawFunctor_TITI : public DrawArrays
{
    inline DrawFunctor_TITI(T1* begin1, I1* ibegin1, I2* iend1, T2* begin2, I2* ibegin2): _begin1(begin1),_ibegin1(ibegin1), _iend1(end1),_begin2(begin2),_ibegin2(ibegin2) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        for(T1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1,++iptr2)
        {
            F2(_begin2[*iptr2]);
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
    T2* _begin2;
    I2* _ibegin2;
};


template< class F1, class T1, class F2, class T2, class I>
struct DrawFunctor_TT_I : public DrawArrays
{
    inline DrawFunctor_TT_I(T1* begin1, T2* begin2, I* ibegin, I* iend): _begin1(begin1),_begin2(begin2),_ibegin(ibegin),_iend(iend) {}
        
    virtual void draw() const
    {
        I index;
        for(I* iptr=_ibegin;
            iptr!=_iend;
            ++iptr)
        {
            index = *iptr;
            F2(_begin2[index]);
            F1(_begin1[index]);
        }
    }

    T1* _begin1;
    T2* _begin2;  
    
    I* _ibegin;
    I* _iend;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template< class F1, class T1, class F2, class T2, class F3, class T3>
struct DrawFunctor_TTT : public DrawArrays
{
    inline DrawFunctor_TTT(T1* begin1, T1* end1, T2* begin2, T3* begin): _begin1(begin1),_end1(end1),_begin2(begin2),_begin3(begin3) {}
        
    virtual void draw() const
    {
        T2* ptr2=_begin2;
        T3* ptr3=_begin3;
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1,++ptr2,++ptr3)
        {
            F3(ptr3);
            F2(ptr2);
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
    
    T2* _begin2;
    T3* _begin3;
};

template< class F1, class T1, class I1, class F2, class T2, class F3, class T3>
struct DrawFunctor_TITT : public DrawArrays
{
    inline DrawFunctor_TITT(T1* begin1, I1* ibegin1, I1* iend1, T2* begin2, T3* begin): _begin1(begin1),_ibegin1(ibegin1),_iend1(end1),_begin3(begin3) {}
        
    virtual void draw() const
    {
        T2* ptr2=_begin2;
        T3* ptr3=_begin3;
        for(I1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1,++ptr2,++ptr3)
        {
            F3(_ptr3);
            F2(_ptr2);
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
    T2* _begin2;
    T3* _begin3;
};

template< class F1, class T1, class F2, class T2, class I2, class F3, class T3>
struct DrawFunctor_TTIT : public DrawArrays
{
    inline DrawFunctor_TTIT(T1* begin1, T1* end1, T2* begin2, I2* ibegin2, T3* begin): _begin1(begin1),_end1(end1),_begin2(begin2),_ibegin2(ibegin2),_begin3(begin3) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        T3* ptr3 = _begin3;
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1,++iptr2,++ptr3)
        {
            F3(ptr3);
            F2(_begin2[*iptr2]);
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
    
    T2* _begin2;
    I2* _ibegin2;

    T3* _begin3;
};


template< class F1, class T1, class F2, class I1, class T2, class I2, class F3, class T3>
struct DrawFunctor_TITIT : public DrawArrays
{
    inline DrawFunctor_TITIT(T1* begin1, I1* ibegin1, I2* iend1, T2* begin2, I2* ibegin2, T3* begin3): _begin1(begin1),_ibegin1(ibegin1), _iend1(end1),_begin2(begin2),_ibegin2(ibegin2),_begin3(begin3) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        T3* ptr3 = _begin3;
        for(I1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1,++iptr2,++ptr3)
        {
            F3(ptr3);
            F2(_begin2[*iptr2]);
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
    T2* _begin2;
    I2* _ibegin2;
    
    T3* _begin3;
};

template< class F1, class T1, class F2, class T2, class I2, class F3, class T3, class I3>
struct DrawFunctor_TTITI : public DrawArrays
{
    inline DrawFunctor_TTITI(T1* begin1, T1* end1, T2* begin2, I2* ibegin2, T3* begin3, I3* ibegin3): _begin1(begin1),_end1(end1),_begin2(begin2),_ibegin2(ibegin2),_begin3(begin3),_ibegin3(begin3) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        I3* iptr3 = _ibegin3;
        for(T1* ptr1=_begin1;
            ptr1!=_end1;
            ++ptr1,++iptr2,++iptr3)
        {
            F3(_begin3[*iptr3]);
            F2(_begin2[*iptr2]);
            F1(ptr1);
        }
    }

    T1* _begin1;
    T1* _end1;
    
    T2* _begin2;
    I2* _ibegin2;
    
    T3* _begin3;
    I3* _ibegin3;
};

template< class F1, class T1, class I1, class F2, class T2, class I2, class F3, class T3, class I3>
struct DrawFunctor_TITITI : public DrawArrays
{
    inline DrawFunctor_TITITI(T1* begin1, I1* ibegin1, I2* iend1, T2* begin2, I2* ibegin2, T3* begin3, I3* ibegin3): _begin1(begin1),_ibegin1(ibegin1), _iend1(end1),_begin2(begin2),_ibegin2(ibegin2),_begin3(begin3),_ibegin3(begin3) {}
        
    virtual void draw() const
    {
        I2* iptr2 = _ibegin2;
        I3* iptr3 = _ibegin3;
        for(I1* iptr1=_ibegin1;
            iptr1!=_iend1;
            ++iptr1,++iptr2,++iptr3)
        {
            F3(_begin3[*iptr3]);
            F2(_begin2[*iptr2]);
            F1(_begin1[*iptr1]);
        }
    }

    T1* _begin1;
    I1* _ibegin1;
    I1* _iend1;
    
    T2* _begin2;
    I2* _ibegin2;
    
    T3* _begin3;
    I3* _ibegin3;
};

template< class F1, class T1, class F2, class T2, class F3, class T3, class I>
struct DrawFunctor_TTT_I : public DrawArrays
{
    inline DrawFunctor_TTT_I(T1* begin1, T2* begin2, T3* begin3, I* ibegin, I* iend): _begin1(begin1),_begin2(begin2),_begin3(begin3),_ibegin(ibegin),_iend(iend) {}
        
    virtual void draw() const
    {
        I index;
        for(I* iptr=_ibegin;
            iptr!=_iend;
            ++iptr)
        {
            index = *iptr;
            F3(_begin3[index]);
            F2(_begin2[index]);
            F1(_begin1[index]);
        }
    }

    T1* _begin1;
    T2* _begin2;  
    T3* _begin3;
    
    I* _ibegin;
    I* _iend;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// single vertex attribute
typedef DrawFunctor_T<DrawVertex,osg::Vec3>                                                                     V3;
typedef DrawFunctor_TI<DrawVertex,osg::Vec3,unsigned short>                                                     V3i;

// two attributes - vertex + one other attribute
typedef DrawFunctor_TT<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3>                                               V3N3;
typedef DrawFunctor_TT<DrawVertex,osg::Vec3,DrawColor,osg::Vec4>                                                V3C4;
typedef DrawFunctor_TT<DrawVertex,osg::Vec3,DrawTexCoord,osg::Vec2>                                             V3T2;

typedef DrawFunctor_TIT<DrawVertex,osg::Vec3,unsigned short,DrawNormal,osg::Vec3>                               V3iN3;
typedef DrawFunctor_TIT<DrawVertex,osg::Vec3,unsigned short,DrawColor,osg::Vec4>                                V3iC4;
typedef DrawFunctor_TIT<DrawVertex,osg::Vec3,unsigned short,DrawTexCoord,osg::Vec2>                             V3iT2;

typedef DrawFunctor_TTI<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short>                               V3N3i;
typedef DrawFunctor_TTI<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,unsigned short>                                V3C4i;
typedef DrawFunctor_TTI<DrawVertex,osg::Vec3,DrawTexCoord,osg::Vec2,unsigned short>                             V3T2i;

typedef DrawFunctor_TT_I<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short>                              V3N3_i;
typedef DrawFunctor_TT_I<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,unsigned short>                               V3C4_i;
typedef DrawFunctor_TT_I<DrawVertex,osg::Vec3,DrawTexCoord,osg::Vec2,unsigned short>                            V3T2_i;

// three attributes - vertex + two other attributes.

typedef DrawFunctor_TTT<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,DrawColor,osg::Vec4>                          V3N3C4;
typedef DrawFunctor_TTT<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,DrawTexCoord,osg::Vec2>                       V3N3T2;
typedef DrawFunctor_TTT<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,DrawTexCoord,osg::Vec2>                        V3C4T2;

typedef DrawFunctor_TITT<DrawVertex,osg::Vec3,unsigned short,DrawNormal,osg::Vec3,DrawColor,osg::Vec4>          V3iN3C4;
typedef DrawFunctor_TITT<DrawVertex,osg::Vec3,unsigned short,DrawNormal,osg::Vec3,DrawTexCoord,osg::Vec2>       V3iN3T2;
typedef DrawFunctor_TITT<DrawVertex,osg::Vec3,unsigned short,DrawColor,osg::Vec4,DrawTexCoord,osg::Vec2>        V3iC4T2;

typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short,DrawColor,osg::Vec4>          V3N3iC4;
typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short,DrawTexCoord,osg::Vec2>       V3N3iT2;
typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,unsigned short,DrawNormal,osg::Vec3>          V3C4iN3;
typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,unsigned short,DrawTexCoord,osg::Vec2>        V3C4iT2;
typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawTexCoord,osg::Vec2,unsigned short,DrawColor,osg::Vec4>        V3T2iC4;
typedef DrawFunctor_TTIT<DrawVertex,osg::Vec3,DrawTexCoord,osg::Vec2,unsigned short,DrawNormal,osg::Vec3>       V3T2iN3;

typedef DrawFunctor_TTITI<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short,DrawColor,osg::Vec4,unsigned short>          V3N3iC4i;
typedef DrawFunctor_TTITI<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,unsigned short,DrawTexCoord,osg::Vec2,unsigned short>       V3N3iT2i;
typedef DrawFunctor_TTITI<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,unsigned short,DrawTexCoord,osg::Vec2,unsigned short>        V3C4iT2i;

typedef DrawFunctor_TTT_I<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,DrawColor,osg::Vec4,unsigned short>                          V3N3C4_i;
typedef DrawFunctor_TTT_I<DrawVertex,osg::Vec3,DrawNormal,osg::Vec3,DrawTexCoord,osg::Vec2,unsigned short>                       V3N3T2_i;
typedef DrawFunctor_TTT_I<DrawVertex,osg::Vec3,DrawColor,osg::Vec4,DrawTexCoord,osg::Vec2,unsigned short>                        V3C4T2_i;



class MyDrawable : public osg::Drawable
{
    public:
    
        MyDrawable();
        
        virtual void drawImplementation(osg::State& state);
        
        virtual bool computeBound() const;


        enum RenderingMode
        {
            v,
            vI,// 2

            vn,
            vIn,
            vnI,
            vInI,
            vn_sI, // 5

            vc,
            vIc,
            vnc,
            vInc,
            vnIc,
            vInIc,
            vcI,
            vIcI,
            vncI,
            vIncI,
            vnIcI,
            vInIcI,
            vnc_sI, // 13

            vt,
            vIt,
            vnt,
            vInt,
            vnIt,
            vInIt,
            vct,
            vIct,
            vnct,
            vInct,
            vnIct,
            vInIct,
            vcIt,
            vIcIt,
            vncIt,
            vIncIt,
            vnIcIt,
            vInIcIt,
            vtI,
            vItI,
            vntI,
            vIntI,
            vnItI,
            vInItI,
            vctI,
            vIctI,
            vnctI,
            vInctI,
            vnIctI,
            vInIctI,
            vcItI,
            vIcItI,
            vncItI,
            vIncItI,
            vnIcItI,
            vInIcItI
            vnct_sI // 37
        }; // 57 combinations.

        
        GLenum _primitiveType;
        
        ref_ptr<Vec3Array> _vertices;
        ref_ptr<UShortArray> _vertexIndices;
        
        ref_ptr<Vec3Array> _normals;
        ref_ptr<UShortArray> _normalIndices;

        ref_ptr<Vec4Array> _colors;
        ref_ptr<UShortArray> _colorIndices;

        ref_ptr<Vec2Array> _texcoords;
        ref_ptr<UShortArray> _texcoordIndices;
}

#endif

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}

