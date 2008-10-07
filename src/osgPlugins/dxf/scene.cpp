/* Importation de fichiers DXF dans OpenSceneGraph (see www.openscenegraph.org)
Copyright (C) 2004 by Paul de Repentigny <pdr@grapharchitecture.com>
*/

#include "scene.h"
#include "dxfTable.h"
#include "aci.h"

using namespace osg;
using namespace std;


osg::Vec4        
sceneLayer::getColor(unsigned short color)
{
    // you're supposed to have a correct color in hand
    unsigned short r = color * 3;
    unsigned short g = color * 3 + 1;
    unsigned short b = color * 3 + 2;
    Vec4 c(aci::table[r], aci::table[g], aci::table[b], 1.0f);
    return c;
}


scene::scene(dxfLayerTable* lt) : _layerTable(lt)
{ 
    _m.makeIdentity(); 
    _r.makeIdentity(); 
}

void 
scene::setLayerTable(dxfLayerTable* lt)
{ 
    _layerTable = lt; 
}

Vec3d scene::addVertex(Vec3d v) 
{
    v += _t;
    v = preMultd(_r, v);
    osg::Matrixd m = osg::Matrixd::translate(v.x(), v.y(), v.z());
    m = m * _m;
    Vec3d a = preMultd(m, Vec3d(0,0,0));
    _b.expandBy(a);
    return a;
}

Vec3d scene::addNormal(Vec3d v) 
{
    // to do: vertices are not always listed in order. find why.
    return v;
}

void 
scene::addLine(const std::string & l, unsigned short color, Vec3d & s, Vec3d & e) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    sceneLayer* ly = findOrCreateSceneLayer(l);
    Vec3d a(addVertex(s)), b(addVertex(e));
    ly->_lines[correctedColorIndex(l, color)].push_back(a);
    ly->_lines[correctedColorIndex(l, color)].push_back(b);
}
void scene::addLineStrip(const std::string & l, unsigned short color, std::vector<Vec3d> & vertices) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    sceneLayer* ly = findOrCreateSceneLayer(l);
    std::vector<Vec3d> converted;
    for (std::vector<Vec3d>::iterator itr = vertices.begin();
        itr != vertices.end(); ++itr) {
            converted.push_back(addVertex(*itr));
    }
    ly->_linestrips[correctedColorIndex(l, color)].push_back(converted);
}
void scene::addLineLoop(const std::string & l, unsigned short color, std::vector<Vec3d> & vertices) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    sceneLayer* ly = findOrCreateSceneLayer(l);
    std::vector<Vec3d> converted;
    for (std::vector<Vec3d>::iterator itr = vertices.begin();
        itr != vertices.end(); ++itr) {
            converted.push_back(addVertex(*itr));
    }
    converted.push_back(addVertex(vertices.front()));
    ly->_linestrips[correctedColorIndex(l, color)].push_back(converted);
}


void scene::addTriangles(const std::string & l, unsigned short color, std::vector<Vec3d> & vertices, bool inverted) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    sceneLayer* ly = findOrCreateSceneLayer(l);
    for (VList::iterator itr = vertices.begin();
        itr != vertices.end(); ) {
            VList::iterator a;
            VList::iterator b;
            VList::iterator c;
            if (inverted) {
                c = itr++;
                b = itr++;
                a = itr++;
            } else {
                a = itr++;
                b = itr++;
                c = itr++;
            }            
            if (a != vertices.end() &&
                b != vertices.end() &&
                c != vertices.end()) {
                Vec3d n = ((*b - *a) ^ (*c - *a));
                n.normalize();
                ly->_trinorms[correctedColorIndex(l, color)].push_back( n );
                ly->_triangles[correctedColorIndex(l, color)].push_back(addVertex(*a));
                ly->_triangles[correctedColorIndex(l, color)].push_back(addVertex(*b));
                ly->_triangles[correctedColorIndex(l, color)].push_back(addVertex(*c));
            }
    }
}
void scene::addQuads(const std::string & l, unsigned short color, std::vector<Vec3d> & vertices, bool inverted) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    
    sceneLayer* ly = findOrCreateSceneLayer(l);
    for (VList::iterator itr = vertices.begin();
        itr != vertices.end(); ) {
            VList::iterator a = vertices.end();
            VList::iterator b = vertices.end();
            VList::iterator c = vertices.end();
            VList::iterator d = vertices.end();
            if (inverted) {
                d = itr++;
                if (itr != vertices.end())
                    c = itr++;
                if (itr != vertices.end())
                    b = itr++;
                if (itr != vertices.end())
                    a = itr++;
            } else {
                a = itr++;
                if (itr != vertices.end())
                    b = itr++;
                if (itr != vertices.end())
                    c = itr++;
                if (itr != vertices.end())
                    d = itr++;
            }
            if (a != vertices.end() &&
                b != vertices.end() &&
                c != vertices.end()&&
                d != vertices.end()) {
                Vec3d n = ((*b - *a) ^ (*c - *a));
                n.normalize();
                short cindex = correctedColorIndex(l, color);
                ly->_quadnorms[cindex].push_back( n );
                MapVList mvl = ly->_quads;
                VList vl = mvl[cindex];
                vl.push_back(addVertex(*a));
                vl.push_back(addVertex(*b));
                vl.push_back(addVertex(*c));
                vl.push_back(addVertex(*d));
                mvl[cindex] = vl;
                ly->_quads = mvl;
            }
    }
}    


void scene::addText(const std::string & l, unsigned short color, Vec3d & point, osgText::Text *text) 
{
    dxfLayer* layer = _layerTable->findOrCreateLayer(l);
    if (layer->getFrozen()) return;
    sceneLayer* ly = findOrCreateSceneLayer(l);

    // Apply the scene settings to the text size and rotation

    Vec3d pscene(addVertex(point));
    Vec3d xvec = addVertex( point + (text->getRotation() * X_AXIS) ) - pscene;
    Vec3d yvec = addVertex( point + (text->getRotation() * Y_AXIS) ) - pscene;
    text->setCharacterSize( text->getCharacterHeight()*yvec.length(), text->getCharacterAspectRatio()*yvec.length()/xvec.length() );

    Matrix qm = _r * _m;
    Vec3d t, s;
    Quat ro, so;
    qm.decompose( t, ro, s, so );
    text->setRotation( text->getRotation() * ro );
    
    sceneLayer::textInfo ti( correctedColorIndex(l,color), pscene, text );
    ly->_textList.push_back(ti);
}


unsigned short 
scene::correctedColorIndex(const std::string & l, unsigned short color)
{
    if (color >= aci::MIN && color <= aci::MAX)
    {
        return color;
    }
    else if (!color || color == aci::BYLAYER)
    {
        dxfLayer* layer = _layerTable->findOrCreateLayer(l);
        unsigned short lcolor = layer->getColor();
        if (lcolor >= aci::MIN && lcolor <= aci::MAX)
        {
            return lcolor;
        }
    }
    return aci::WHITE;
}
