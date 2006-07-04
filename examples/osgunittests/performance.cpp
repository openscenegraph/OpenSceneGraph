#include "performance.h"

#include <osg/Timer>
#include <iostream>

#include <osg/NodeVisitor>
#include <osg/ref_ptr>
#include <osg/MatrixTransform>
#include <osg/Group>

struct Benchmark
{

    Benchmark()
    {
        calibrate();
    
        _beginTick = _timer.tick();
        _endTick = _timer.tick();
    }
    
    void calibrate(unsigned int numLoops = 100000)
    {
        osg::Timer_t beginTick = _timer.tick();
        for(unsigned int i=0;i<numLoops;++i)
        {
            begin();
            end();
        }
        osg::Timer_t endTick = _timer.tick();
        _averageDelay = _timer.delta_s(beginTick,endTick)/(double)numLoops;
    }
    
    inline void begin()
    {
        _beginTick = _timer.tick();
    }

    inline void end()
    {
        _endTick = _timer.tick();
    }

    inline double time()
    {
        double t = _timer.delta_s(_beginTick,_endTick) - _averageDelay;
        return t<0.0 ? 0.0 : t;
    }

    inline void output(const char* str, double numIterations=1.0)
    {
        std::cout<<str<<"\t";
        double s = time()/numIterations;
        if (s>=1.0) std::cout<<s<<" s"<<std::endl;
        else if (s>=0.001) std::cout<<s*1000.0<<" ms (10 ^ -3)"<<std::endl;
        else if (s>=0.000001) std::cout<<s*1000000.0<<" ns (10 ^ -6)"<<std::endl;
        else std::cout<<s*1000000000.0<<" ps (10 ^ -9)"<<std::endl;
    }

    osg::Timer   _timer;
    osg::Timer_t _beginTick;
    osg::Timer_t _endTick;
    double _averageDelay;
};

#define RUN(A,B,D) { A.begin(); for(unsigned int i=0;i<D;++i) B; A.end(); A.output(#B,D); }


static int v = 0;
#define OPERATION { v=v+1; }

inline void inline_increment() { OPERATION }
void function_increment() { OPERATION }

typedef void ( * IncrementProc) ();
IncrementProc s_functionIncrement = &function_increment;
inline void functionPointer_increment() { s_functionIncrement(); }


struct InlineMethod;
struct Method;
struct VirtualMethod;
struct VirtualMethod2;

struct Visitor
{
    virtual void apply(InlineMethod& m);
    virtual void apply(Method& m);
    virtual void apply(VirtualMethod& m);
    virtual void apply(VirtualMethod2& m);
    virtual ~Visitor() {}
};


struct InlineMethod
{
    void method() { OPERATION }
    virtual void accept(Visitor& visitor) { visitor.apply(*this); }
    virtual ~InlineMethod() {}
};

struct Method
{
    virtual void accept(Visitor& visitor) { visitor.apply(*this); }
    void method();
    virtual ~Method() {}
};

void Method::method() { OPERATION }
 
struct VirtualMethod
{
    virtual void accept(Visitor& visitor) { visitor.apply(*this); }
    virtual void method();
    virtual ~VirtualMethod() {}
};

void VirtualMethod::method() { OPERATION }

struct VirtualMethod2 : public VirtualMethod
{
    VirtualMethod2() { }

    virtual void accept(Visitor& visitor) { visitor.apply(*this); }
    virtual void method();
    virtual ~VirtualMethod2() { }
    
    char a[100];
};

void VirtualMethod2::method() { OPERATION }

void Visitor::apply(Method& m) { m.method(); }
void Visitor::apply(VirtualMethod& m) { m.method(); }
void Visitor::apply(InlineMethod& m) { m.method(); }
void Visitor::apply(VirtualMethod2& m) { m.method(); }

struct CustomVisitor
{
    virtual void apply(InlineMethod& m) { m.method(); }
    virtual void apply(Method& m) { m.method(); }
    virtual void apply(VirtualMethod& m) { m.method(); }
    virtual void apply(VirtualMethod2& m) { m.method(); }
    virtual ~CustomVisitor() {}
};

class CustomNodeVisitor : public osg::NodeVisitor
{
public:
    void apply(osg::Node&) { }
    void apply(osg::Group&) { }
    void apply(osg::Transform&) { }
};


void runPerformanceTests()
{
    Benchmark benchmark;
    
    unsigned int iterations = 10000000;

    RUN(benchmark, {} , iterations)

    v = 0;
    RUN(benchmark, OPERATION , iterations)
    RUN(benchmark, functionPointer_increment() , iterations)
    RUN(benchmark, inline_increment() , iterations)
    RUN(benchmark, function_increment() , iterations)

    VirtualMethod2 m4;
    RUN(benchmark, m4.method() , iterations)

    InlineMethod m1;
    RUN(benchmark, m1.method() , iterations)

    Method m2;
    RUN(benchmark, m2.method() , iterations)

    VirtualMethod m3;
    RUN(benchmark, m3.method() , iterations)
    RUN(benchmark, m3.method() , iterations)

    Visitor visitor;
    RUN(benchmark, m4.accept(visitor), iterations)
    RUN(benchmark, m1.accept(visitor), iterations)
    RUN(benchmark, m2.accept(visitor), iterations)
    RUN(benchmark, m3.accept(visitor), iterations)
    RUN(benchmark, m4.accept(visitor), iterations)

    VirtualMethod* vm4 = &m4;

    RUN(benchmark, (dynamic_cast<VirtualMethod2*>(vm4))->method(), iterations)
    RUN(benchmark, (static_cast<VirtualMethod2*>(vm4))->method(), iterations)
    RUN(benchmark, { VirtualMethod mm; mm.method(); }, iterations)
    RUN(benchmark, { VirtualMethod2 mm; mm.method(); }, iterations)


    osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    osg::Node* m = mt.get();
    CustomNodeVisitor cnv;
    RUN(benchmark, { osg::MatrixTransform* mtl = dynamic_cast<osg::MatrixTransform*>(m); if (mtl) cnv.apply(*mtl); }, 1000)
    RUN(benchmark, { m->accept(cnv); }, 10000)
    
}
