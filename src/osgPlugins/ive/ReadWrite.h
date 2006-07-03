#ifndef IVE_READWRITE
#define IVE_READWRITE 1

#include "DataInputStream.h"
#include "DataOutputStream.h"

namespace ive {

// Objects and Nodes
#define IVEOBJECT                       0x00000001
#define IVENODE                         0x00000002
#define IVEGROUP                        0x00000003
#define IVEMATRIXTRANSFORM              0x00000004
#define IVESTATESET                     0x00000005
#define IVEGEODE                        0x00000006
#define IVEIMAGE                        0x00000007
#define IVELIGHTSOURCE                  0x00000008
#define IVELIGHT                        0x00000009
#define IVEBILLBOARD                    0x00000010
#define IVELOD                          0x00000011
#define IVESEQUENCE                     0x00000012
#define IVETRANSFORM                    0x00000013
#define IVEPOSITIONATTITUDETRANSFORM    0x00000014
#define IVEANIMATIONPATH                0x00000015
#define IVESWITCH                       0x00000016
#define IVEOCCLUDERNODE                 0x00000017
#define IVEIMPOSTOR                     0x00000018
#define IVECONVEXPLANAROCCLUDER         0x00000019
#define IVECONVEXPLANARPOLYGON          0x00000020
#define IVEPAGEDLOD                     0x00000021
#define IVEDOFTRANSFORM                 0x00000022
#define IVECOORDINATESYSTEMNODE         0x00000023
#define IVEELLIPSOIDMODEL               0x00000024
#define IVETEXGENNODE                   0x00000025
#define IVECLIPNODE                     0x00000026
#define IVEPROXYNODE                    0x00000027
#define IVECAMERANODE                   0x00000028
#define IVECAMERAVIEW                   0x00000029
#define IVEAUTOTRANSFORM                0x00000030

// Node callbacks
#define IVENODECALLBACK                 0x00000050
#define IVEANIMATIONPATHCALLBACK        0x00000051
#define IVECLUSTERCULLINGCALLBACK       0x00000052

// State attributes.
#define IVESTATEATTRIBUTE               0x00000100
#define IVEALPHAFUNC                    0x00000101
#define IVEBLENDFUNC                    0x00000102
#define IVEBLENDFUNCSEPARATE            0x00000103
#define IVEMATERIAL                     0x00000110
#define IVETEXTURE                      0x00000120
#define IVETEXTURE1D                    0x00000121
#define IVETEXTURE2D                    0x00000122
#define IVETEXTURE3D                    0x00000123
#define IVETEXTURECUBEMAP               0x00000124
#define IVETEXENV                       0x00000125
#define IVETEXENVCOMBINE                0x00000126
#define IVETEXGEN                       0x00000127
#define IVECULLFACE                     0x00000128
#define IVEPOLYGONOFFSET                0x00000129
#define IVESHADEMODEL                   0x0000012A
#define IVEPOINT                        0x0000012B
#define IVETEXMAT                       0x0000012C
#define IVELINEWIDTH                    0x0000012D
#define IVEFRAGMENTPROGRAM              0x0000012E
#define IVEVERTEXPROGRAM                0x0000012F
#define IVEDEPTH                        0x00000130
#define IVELIGHTMODEL                   0x00001121
#define IVECLIPPLANE                    0x00001122
#define IVEFRONTFACE                    0x00001123
#define IVEPROGRAM                      0x00001124
#define IVESHADER                       0x00001125
#define IVEUNIFORM                      0x00001126
#define IVEVIEWPORT                     0x00001127
#define IVESCISSOR                      0x00001128
#define IVEPOLYGONMODE                  0x00001129
#define IVETEXTURERECTANGLE             0x00001130

// Drawables
#define IVEDRAWABLE                     0x00001000
#define IVEGEOMETRY                     0x00001001
#define IVESHAPEDRAWABLE                0x00001002

// Shapes
#define IVESHAPE                        0x00002000
#define IVESPHERE                       0x00002001
#define IVEBOX                          0x00002002
#define IVECONE                         0x00002004
#define IVECYLINDER                     0x00002005
#define IVECAPSULE                      0x00002006
#define IVEHEIGHTFIELD                  0x00002007

// Primitive set
#define IVEPRIMITIVESET                 0x00010000
#define IVEDRAWARRAYS                   0x00010001
#define IVEDRAWARRAYLENGTHS             0x00010002
#define IVEDRAWELEMENTSUSHORT           0x00010003
#define IVEDRAWELEMENTSUINT             0x00010004
#define IVEDRAWELEMENTSUBYTE            0x00010005

// osgSim classes
#define IVEBLINKSEQUENCE                0x00100001
#define IVEAZIMELEVATIONSECTOR          0x00100002
#define IVEELEVATIONSECTOR              0x00100003
#define IVEAZIMSECTOR                   0x00100004
#define IVECONESECTOR                   0x00100005
#define IVELIGHTPOINT                   0x00100006
#define IVELIGHTPOINTNODE               0x00100007
#define IVEMULTISWITCH                  0x00100008


#define IVEVISIBILITYGROUP              0x00100009
#define IVEDIRECTIONALSECTOR            0x0010000A

// osgFX classes
#define IVEMULTITEXTURECONTROL          0x01000001

//osgText classes
#define IVETEXT                         0x10000001

class ReadWrite{

public:
    virtual ~ReadWrite() {}
    virtual void write(DataOutputStream* out) const = 0;
    virtual void read(DataInputStream* out) const = 0;
};

}
#endif // IVE_READWRITE
