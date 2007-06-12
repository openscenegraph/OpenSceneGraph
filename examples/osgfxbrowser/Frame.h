/* -*-c++-*-
*
*  OpenSceneGraph example, osgfxbrowser.
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

#ifndef FRAME_H_
#define FRAME_H_

#include <osg/Geode>
#include <osg/Geometry>

namespace osgfxbrowser {

struct Rect {
	float x0, y0, x1, y1;
	Rect() {}
	Rect(float x0_, float y0_, float x1_, float y1_): x0(x0_), y0(y0_), x1(x1_), y1(y1_) {}
	inline float width() const { return x1 - x0; }
	inline float height() const { return y0 - y1; }
};

class Frame: public osg::Geode {
public:
	Frame();
	Frame(const Frame &copy, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY);

	META_Node(osgfxbrowser, Frame);

	inline const std::string &getCaption() const              { return caption_; }
	inline void setCaption(const std::string &caption)        { caption_ = caption; }

	inline const osg::Vec4 &getBackgroundColor() const        { return bgcolor_; }
	inline void setBackgroundColor(const osg::Vec4 &bgcolor)  { bgcolor_ = bgcolor; }

	inline const Rect &getRect() const                { return rect_; }
	inline void setRect(const Rect &rect)             { rect_ = rect; }

	static osg::Geometry *build_quad(const Rect &rect, const osg::Vec4 &color, bool shadow = true, float z = 0);

	virtual void rebuild();

protected:
	virtual ~Frame() {}
	Frame &operator()(const Frame &) { return *this; }	

	virtual void rebuild_client_area(const Rect & /*client_rect*/) {}

private:
	osg::Vec4 bgcolor_;
	Rect rect_;
	std::string caption_;
};

}

#endif
