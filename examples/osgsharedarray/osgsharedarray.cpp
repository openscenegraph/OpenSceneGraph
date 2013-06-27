/* OpenSceneGraph example, osgsharedarray.
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

#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgViewer/Viewer>

/** This class is an example of how to create your own subclass of osg::Array. This
  * is useful if your application has data in its own form of storage and you don't
  * want to make another copy into one of the predefined osg::Array classes.
  *
  * @note This is not really intended to be a useful subclass of osg::Array. It
  * doesn't do anything smart about memory management. It is simply intended as
  * an example you can follow to create your own subclasses of osg::Array for
  * your application's storage requirements.
  */
class MyArray : public osg::Array {
public:
    /** Default ctor. Creates an empty array. */
    MyArray() :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(0),
        _ptr(NULL) {
    }

    /** "Normal" ctor.
      *
      * @param no The number of elements in the array.
      * @param ptr Pointer to the data. This class just keeps that
      * pointer. It doesn't manage the memory.
      */
    MyArray(unsigned int no, osg::Vec3* ptr) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(no),
        _ptr(ptr) {
    }

    /** Copy ctor. */
    MyArray(const MyArray& other, const osg::CopyOp& copyop) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(other._numElements),
        _ptr(other._ptr) {
    }

    /** What type of object would clone return? */
    virtual Object* cloneType() const {
        return new MyArray();
    }

    /** Create a copy of the object. */
    virtual osg::Object* clone(const osg::CopyOp& copyop) const {
        return new MyArray(*this,copyop);
    }

    /** Accept method for ArrayVisitors.
      *
      * @note This will end up in ArrayVisitor::apply(osg::Array&).
      */
    virtual void accept(osg::ArrayVisitor& av) {
        av.apply(*this);
    }

    /** Const accept method for ArrayVisitors.
      *
      * @note This will end up in ConstArrayVisitor::apply(const osg::Array&).
      */
    virtual void accept(osg::ConstArrayVisitor& cav) const {
        cav.apply(*this);
    }

    /** Accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ValueVisitor& vv) {
        vv.apply(_ptr[index]);
    }

    /** Const accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ConstValueVisitor& cvv) const {
        cvv.apply(_ptr[index]);
    }

    /** Compare method.
      * Return -1 if lhs element is less than rhs element, 0 if equal,
      * 1 if lhs element is greater than rhs element.
      */
    virtual int compare(unsigned int lhs,unsigned int rhs) const {
        const osg::Vec3& elem_lhs = _ptr[lhs];
        const osg::Vec3& elem_rhs = _ptr[rhs];
        if (elem_lhs<elem_rhs) return -1;
        if (elem_rhs<elem_lhs) return  1;
        return 0;
    }

    virtual unsigned int getElementSize() const { return sizeof(osg::Vec3); }

    /** Returns a pointer to the first element of the array. */
    virtual const GLvoid* getDataPointer() const {
        return _ptr;
    }

    /** Returns the number of elements in the array. */
    virtual unsigned int getNumElements() const {
        return _numElements;
    }

    /** Returns the number of bytes of storage required to hold
      * all of the elements of the array.
      */
    virtual unsigned int getTotalDataSize() const {
        return _numElements * sizeof(osg::Vec3);
    }

    virtual void reserveArray(unsigned int num) { OSG_NOTICE<<"reserveArray() not supported"<<std::endl; }
    virtual void resizeArray(unsigned int num) { OSG_NOTICE<<"resizeArray() not supported"<<std::endl; }

private:
    unsigned int _numElements;
    osg::Vec3*   _ptr;
};

/** The data values for the example. Simply defines a cube with
  * per-face colors and normals.
  */

const osg::Vec3 myVertices[] = { osg::Vec3(-1.,-1., 1.),
                                 osg::Vec3( 1.,-1., 1.),
                                 osg::Vec3( 1., 1., 1.),
                                 osg::Vec3(-1., 1., 1.),

                                 osg::Vec3( 1.,-1., 1.),
                                 osg::Vec3( 1.,-1.,-1.),
                                 osg::Vec3( 1., 1.,-1.),
                                 osg::Vec3( 1., 1., 1.),

                                 osg::Vec3( 1.,-1.,-1.),
                                 osg::Vec3(-1.,-1.,-1.),
                                 osg::Vec3(-1., 1.,-1.),
                                 osg::Vec3( 1., 1.,-1.),

                                 osg::Vec3(-1.,-1.,-1.),
                                 osg::Vec3(-1.,-1., 1.),
                                 osg::Vec3(-1., 1., 1.),
                                 osg::Vec3(-1., 1.,-1.),

                                 osg::Vec3(-1., 1., 1.),
                                 osg::Vec3( 1., 1., 1.),
                                 osg::Vec3( 1., 1.,-1.),
                                 osg::Vec3(-1., 1.,-1.),

                                 osg::Vec3(-1.,-1.,-1.),
                                 osg::Vec3( 1.,-1.,-1.),
                                 osg::Vec3( 1.,-1., 1.),
                                 osg::Vec3(-1.,-1., 1.),
                               };


const osg::Vec3 myNormals[] = { osg::Vec3( 0., 0., 1.),
                                osg::Vec3( 0., 0., 1.),
                                osg::Vec3( 0., 0., 1.),
                                osg::Vec3( 0., 0., 1.),

                                osg::Vec3( 1., 0., 0.),
                                osg::Vec3( 1., 0., 0.),
                                osg::Vec3( 1., 0., 0.),
                                osg::Vec3( 1., 0., 0.),

                                osg::Vec3( 0., 0.,-1.),
                                osg::Vec3( 0., 0.,-1.),
                                osg::Vec3( 0., 0.,-1.),
                                osg::Vec3( 0., 0.,-1.),

                                osg::Vec3(-1., 0., 0.),
                                osg::Vec3(-1., 0., 0.),
                                osg::Vec3(-1., 0., 0.),
                                osg::Vec3(-1., 0., 0.),

                                osg::Vec3( 0., 1., 0.),
                                osg::Vec3( 0., 1., 0.),
                                osg::Vec3( 0., 1., 0.),
                                osg::Vec3( 0., 1., 0.),

                                osg::Vec3( 0.,-1., 0.),
                                osg::Vec3( 0.,-1., 0.),
                                osg::Vec3( 0.,-1., 0.),
                                osg::Vec3( 0.,-1., 0.)
                              };

const osg::Vec4 myColors[] = { osg::Vec4( 1., 0., 0., 1.),
                               osg::Vec4( 1., 0., 0., 1.),
                               osg::Vec4( 1., 0., 0., 1.),
                               osg::Vec4( 1., 0., 0., 1.),

                               osg::Vec4( 0., 1., 0., 1.),
                               osg::Vec4( 0., 1., 0., 1.),
                               osg::Vec4( 0., 1., 0., 1.),
                               osg::Vec4( 0., 1., 0., 1.),

                               osg::Vec4( 1., 1., 0., 1.),
                               osg::Vec4( 1., 1., 0., 1.),
                               osg::Vec4( 1., 1., 0., 1.),
                               osg::Vec4( 1., 1., 0., 1.),

                               osg::Vec4( 0., 0., 1., 1.),
                               osg::Vec4( 0., 0., 1., 1.),
                               osg::Vec4( 0., 0., 1., 1.),
                               osg::Vec4( 0., 0., 1., 1.),

                               osg::Vec4( 1., 0., 1., 1.),
                               osg::Vec4( 1., 0., 1., 1.),
                               osg::Vec4( 1., 0., 1., 1.),
                               osg::Vec4( 1., 0., 1., 1.),

                               osg::Vec4( 0., 1., 1., 1.),
                               osg::Vec4( 0., 1., 1., 1.),
                               osg::Vec4( 0., 1., 1., 1.),
                               osg::Vec4( 0., 1., 1., 1.)
                             };

/** Create a Geode that describes a cube using our own
  * subclass of osg::Array for the vertices. It uses
  * the "regular" array classes for all of the other
  * arrays.
  *
  * Creating your own Array class isn't really very
  * useful for a tiny amount of data like this. You
  * could just go ahead and copy the data into one of
  * the "regular" Array classes like this does for
  * normals and colors. The point of creating your
  * own subclass of Array is for use with datasets
  * that are much larger than anything you could
  * create a simple example from. In that case, you
  * might not want to create a copy of the data in
  * one of the Array classes that comes with OSG, but
  * instead reuse the copy your application already
  * has and wrap it up in a subclass of osg::Array
  * that presents the right interface for use with
  * OpenSceneGraph.
  *
  * Note that I'm only using the shared array for the
  * vertices. You could do something similar for any
  * of the Geometry node's data arrays.
  */
osg::Geode* createGeometry()
{
    osg::Geode* geode = new osg::Geode();

    // create Geometry
    osg::ref_ptr<osg::Geometry> geom(new osg::Geometry());

    // add vertices using MyArray class
    unsigned int numVertices = sizeof(myVertices)/sizeof(myVertices[0]);
    geom->setVertexArray(new MyArray(numVertices,const_cast<osg::Vec3*>(&myVertices[0])));

    // add normals
    unsigned int numNormals = sizeof(myNormals)/sizeof(myNormals[0]);
    geom->setNormalArray(new osg::Vec3Array(numNormals,const_cast<osg::Vec3*>(&myNormals[0])), osg::Array::BIND_PER_VERTEX);

    // add colors
    unsigned int numColors = sizeof(myColors)/sizeof(myColors[0]);
    osg::Vec4Array* normal_array = new osg::Vec4Array(numColors,const_cast<osg::Vec4*>(&myColors[0]));
    geom->setColorArray(normal_array, osg::Array::BIND_PER_VERTEX);

    // add PrimitiveSet
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, numVertices));

    // Changing these flags will tickle different cases in
    // Geometry::drawImplementation. They should all work fine
    // with the shared array.
    geom->setUseVertexBufferObjects(false);
    geom->setUseDisplayList(false);

    geode->addDrawable( geom.get() );

    return geode;
}

int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData( createGeometry() );

    // create the windows and run the threads.
    return viewer.run();
}
