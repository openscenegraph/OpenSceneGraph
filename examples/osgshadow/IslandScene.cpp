#include <stdlib.h>
#include <osg/ArgumentParser>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/AlphaFunc>
#include <osg/CullFace>

static unsigned int heightTexture[] = {
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010101, 0x02000000, 0x00020809,
 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x05070200, 0x07090c06, 0x0e010004, 0x01061824,
 0x00010102, 0x00000000, 0x02020000, 0x04050705, 0x02060604, 0x00000000, 0x00000000, 0x00000000,
 0x00010000, 0x00000000, 0x00000000, 0x01000000, 0x2a37250a, 0x262a2d2b, 0x3518121c, 0x1f19273c,
 0x03151a1c, 0x00000000, 0x0e070000, 0x0e0d1b19, 0x161c1d15, 0x00010109, 0x00020905, 0x00010100,
 0x03030000, 0x00000000, 0x00010401, 0x06000000, 0x484c3714, 0x3d403b45, 0x534e443d, 0x42433c4b,
 0x102a2f34, 0x00000001, 0x05010000, 0x110c110c, 0x2a313226, 0x0c090715, 0x0009201d, 0x00010101,
 0x0b040000, 0x00000005, 0x00010301, 0x04000000, 0x45403315, 0x423a3242, 0x5e5a584f, 0x474b4b5a,
 0x20282a36, 0x00000006, 0x00000000, 0x180a0301, 0x3946402d, 0x281b1523, 0x041b3d3d, 0x00000001,
 0x0c020000, 0x0001060e, 0x00000000, 0x05020000, 0x4a3c280f, 0x3a39414e, 0x625a5143, 0x353c4152,
 0x242b3336, 0x00000009, 0x00000000, 0x2c160100, 0x3f46382d, 0x3e322b33, 0x082a5753, 0x00000001,
 0x08020000, 0x040a1114, 0x00000002, 0x0f0e0801, 0x4f382819, 0x41546562, 0x5c625743, 0x21212943,
 0x1b292f28, 0x00000005, 0x00000000, 0x2d180100, 0x30322a2a, 0x3e2d292f, 0x1e3f6058, 0x0000020d,
 0x14060000, 0x0d151a1f, 0x00000309, 0x03070401, 0x402a1509, 0x5c707463, 0x5b6a645d, 0x15142041,
 0x0a131511, 0x00000001, 0x00000000, 0x1d0e0100, 0x1c1d2322, 0x22182226, 0x444e4f3e, 0x00011a34,
 0x2a1a0500, 0x091a222c, 0x00010204, 0x00010000, 0x3e2c1201, 0x7c82765f, 0x6979797d, 0x060f264d,
 0x00020202, 0x00000000, 0x00000000, 0x0b020000, 0x0d141812, 0x0a091514, 0x3e362c1d, 0x00022c4f,
 0x412e0800, 0x09202e3a, 0x00000000, 0x00000000, 0x3c2b1001, 0x8d836f59, 0x7185898d, 0x01112c53,
 0x00000000, 0x00000000, 0x00000000, 0x0e020000, 0x141d1713, 0x01020b10, 0x2a221a09, 0x00032041,
 0x523a0f01, 0x071c2f47, 0x00000000, 0x00000000, 0x3d1d0400, 0x8a796358, 0x708d9392, 0x07263d57,
 0x00000000, 0x00000000, 0x00000000, 0x15030000, 0x1f1e171b, 0x00010b1c, 0x1e130f02, 0x00021a36,
 0x533e1301, 0x0c1d283f, 0x00000001, 0x00000000, 0x30110000, 0x856d5543, 0x6f889293, 0x143a4e5f,
 0x00000001, 0x00000000, 0x00000000, 0x07000000, 0x1919120d, 0x00000919, 0x14040200, 0x0001102b,
 0x46331002, 0x121a2035, 0x00000002, 0x00000000, 0x1c0a0000, 0x80654d2c, 0x677f8e92, 0x1a425058,
 0x00000003, 0x00000000, 0x00000000, 0x00000000, 0x08100801, 0x00000409, 0x05000000, 0x0001040e,
 0x321e0c02, 0x0e171c30, 0x00000001, 0x00000000, 0x1a050000, 0x705a4830, 0x667f8c86, 0x203d434c,
 0x0000020e, 0x00000000, 0x00000000, 0x00000000, 0x0d070000, 0x00021015, 0x00000000, 0x00000001,
 0x291e0c02, 0x08121423, 0x00000001, 0x00000000, 0x250a0000, 0x60524b3d, 0x68808676, 0x1e2c3246,
 0x00000412, 0x00000000, 0x00000000, 0x00000000, 0x22160500, 0x010c232b, 0x00000000, 0x00000000,
 0x1e180a01, 0x080c0e18, 0x00000001, 0x00000000, 0x260b0503, 0x5a514940, 0x6d828373, 0x1322314c,
 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x2f200e03, 0x04192d35, 0x00000000, 0x00000000,
 0x14100701, 0x0302020b, 0x00000001, 0x05000000, 0x2d141716, 0x453e3f45, 0x71817f63, 0x051a2f52,
 0x00000001, 0x00000000, 0x00000000, 0x07020000, 0x3321140f, 0x0d172738, 0x02020103, 0x00000001,
 0x01010000, 0x00000001, 0x00000000, 0x0e010000, 0x24121d21, 0x33323b3c, 0x71796b4f, 0x010e2a57,
 0x00000000, 0x00000000, 0x00000000, 0x20140200, 0x30281f23, 0x1b272d36, 0x09110d0c, 0x00000001,
 0x00000000, 0x00000000, 0x00000000, 0x180a0000, 0x1e182025, 0x33373e33, 0x65625747, 0x02142a4e,
 0x09010000, 0x00000003, 0x00000000, 0x372b0c01, 0x30262736, 0x2c414440, 0x081a211f, 0x00000003,
 0x00000000, 0x00000000, 0x00000000, 0x1f120000, 0x2a1f1c24, 0x3940473f, 0x56574e44, 0x15242b41,
 0x0f060106, 0x00000007, 0x00000000, 0x2e290e00, 0x2f1f242e, 0x374b4f48, 0x0c1b262a, 0x00000004,
 0x00000000, 0x00000000, 0x00000000, 0x19120200, 0x3721181c, 0x45525b51, 0x56554e4c, 0x333c3d48,
 0x09141523, 0x01010001, 0x00000000, 0x121b0d01, 0x2f1c1510, 0x2a3a4445, 0x19232321, 0x0000010c,
 0x00000000, 0x00000000, 0x06020000, 0x17130301, 0x3c231e1e, 0x4b596558, 0x59545452, 0x36424b5a,
 0x04172430, 0x0c0b0200, 0x00000001, 0x03120e03, 0x1e0a0201, 0x282f332d, 0x212e2d23, 0x0000010e,
 0x00000000, 0x00000000, 0x06030000, 0x1d170501, 0x35252422, 0x495a6450, 0x5751514d, 0x2c3b4a5a,
 0x091b2429, 0x0f170c04, 0x00000001, 0x010a0501, 0x09010000, 0x2e2e2c1d, 0x2336352d, 0x00010414,
 0x00000000, 0x00000000, 0x01010000, 0x271a0400, 0x34242324, 0x4f5e6953, 0x4e454951, 0x242c394b,
 0x1e2d2521, 0x040f0f10, 0x00000000, 0x010a0300, 0x01000000, 0x241f1f11, 0x2e3d3226, 0x000b1a23,
 0x00000000, 0x00000000, 0x04020000, 0x35240a04, 0x41353232, 0x58656a5b, 0x3a424d56, 0x171a242f,
 0x272e2218, 0x00030c1c, 0x00000000, 0x01040100, 0x00000000, 0x16100d06, 0x36332117, 0x01183334,
 0x00000000, 0x00000000, 0x07020000, 0x36250a04, 0x43403837, 0x515b5953, 0x29364245, 0x02030a1b,
 0x181d1708, 0x00030b17, 0x00000000, 0x00000000, 0x00000000, 0x231a0a02, 0x3730221c, 0x01162c33,
 0x00000000, 0x00000000, 0x05010000, 0x38260d05, 0x3b3e3c3d, 0x3c464847, 0x20293332, 0x0000010f,
 0x0e160e01, 0x00060c11, 0x00000000, 0x00000000, 0x00000000, 0x2e240f04, 0x31302826, 0x010c1429,
 0x00000000, 0x00000000, 0x00000000, 0x25150602, 0x242d3731, 0x242b3131, 0x0b1a2621, 0x00000002,
 0x080e0800, 0x060e1511, 0x00000001, 0x00000000, 0x00000000, 0x31281707, 0x24242429, 0x00020b1c,
 0x00000000, 0x00000000, 0x00000000, 0x0b030000, 0x08111f1a, 0x1a101314, 0x0109171b, 0x00000000,
 0x17100801, 0x1f262f27, 0x00000311, 0x00000000, 0x01000000, 0x3133230c, 0x1f252829, 0x00010815,
 0x00000000, 0x00000000, 0x00000000, 0x01000000, 0x00020a09, 0x09020402, 0x00010207, 0x00000000,
 0x241c1203, 0x28344037, 0x00030d1c, 0x00000000, 0x0e030000, 0x383b291a, 0x1b222931, 0x00010410,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x01060601, 0x00000000, 0x00000000,
 0x29291e07, 0x25343f34, 0x0208101b, 0x00000000, 0x1e0b0100, 0x353e3229, 0x1618202e, 0x00000206,
 0x00000000, 0x00000000, 0x00000000, 0x00010101, 0x00000103, 0x030f0f03, 0x00000000, 0x00000000,
 0x2b291a08, 0x2131342a, 0x070d1118, 0x00000000, 0x21150c06, 0x2a35312d, 0x0e111929, 0x00000003,
 0x00000000, 0x00000000, 0x00000000, 0x0c0a0a02, 0x00010b16, 0x08151708, 0x00000002, 0x01000000,
 0x26231812, 0x152d2c20, 0x0c060306, 0x04010004, 0x2431321f, 0x192a2d27, 0x01020715, 0x00000001,
 0x00010100, 0x00010100, 0x00000000, 0x170e0a01, 0x00051820, 0x070f0c02, 0x00000307, 0x02000000,
 0x1f1a0d0b, 0x10221915, 0x0f040001, 0x1e0f1418, 0x31475645, 0x0618242d, 0x00000002, 0x00000000,
 0x03050200, 0x01040202, 0x00000000, 0x0f0c0901, 0x00091b19, 0x05020000, 0x0001050a, 0x01010101,
 0x18140503, 0x0b140a0c, 0x17030001, 0x42383d33, 0x44576a62, 0x010d233a, 0x00000000, 0x00000000,
 0x1a130200, 0x03070910, 0x03010001, 0x02040b05, 0x00030d0a, 0x00000000, 0x00000001, 0x05010101,
 0x0e070002, 0x02050104, 0x15010000, 0x51494e3d, 0x556c776a, 0x0b1a3447, 0x00000002, 0x00000000,
 0x3b220801, 0x09101b37, 0x17090105, 0x0006171d, 0x00000101, 0x00000000, 0x00000000, 0x02000000,
 0x03020002, 0x00010001, 0x11010000, 0x4c4a4e37, 0x6677755d, 0x242d4559, 0x0000000e, 0x00000000,
 0x41270700, 0x0a0e1636, 0x1e0b0004, 0x010d1d2a, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x01020000, 0x15050000, 0x48474934, 0x6066594a, 0x2f384955, 0x00000117, 0x00000000,
 0x2f2d0b01, 0x030b2b40, 0x1d120502, 0x020d1d2a, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x09090000, 0x2d1d0903, 0x413f423e, 0x4e55453b, 0x25333f42, 0x0000000f, 0x00000000,
 0x29160c01, 0x1b253b3e, 0x24251a15, 0x0007202b, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x0e0b0000, 0x3c2f150a, 0x373f3f42, 0x3e3b2927, 0x15253132, 0x00000008, 0x00000000,
 0x24100401, 0x3b323939, 0x262f2e36, 0x00072027, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x0f100401, 0x43381e0e, 0x223c4649, 0x261e0c0d, 0x0e212722, 0x00000003, 0x00010000,
 0x2c110200, 0x4e434442, 0x303e3f4a, 0x00081d25, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x1c1a0800, 0x4d413321, 0x1233535b, 0x190e0103, 0x182b2821, 0x00000003, 0x00020000,
 0x2d240800, 0x5d5a5742, 0x46555257, 0x04172a35, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x241f0e02, 0x4d47412e, 0x0f305058, 0x13060003, 0x24312920, 0x0401000b, 0x00010002,
 0x26221201, 0x67635f45, 0x58656064, 0x041c2c3e, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x2d2e200d, 0x3d3a3f37, 0x0c263c3f, 0x08010003, 0x21312516, 0x0601020f, 0x00000003,
 0x1b0d0601, 0x73695f43, 0x5e6e6f77, 0x01102a45, 0x00000000, 0x00000000, 0x00000000, 0x00010000,
 0x04000000, 0x2c312a19, 0x342d3032, 0x202f3432, 0x05000112, 0x1b2d2111, 0x0902030e, 0x00000004,
 0x0b020100, 0x71675432, 0x5367727b, 0x0006253f, 0x00000000, 0x00000000, 0x01000000, 0x080a0302,
 0x0e030201, 0x171f2220, 0x2b211417, 0x2d372823, 0x00000115, 0x09120a03, 0x04010208, 0x00000001,
 0x07050100, 0x655c4627, 0x4b56606d, 0x00021b38, 0x00010000, 0x00000000, 0x0e070000, 0x0e0e080d,
 0x0e111108, 0x06111714, 0x1a100303, 0x23302216, 0x0000000a, 0x02010000, 0x080c1110, 0x00000207,
 0x06030000, 0x645e4324, 0x4f585b66, 0x00051d37, 0x01020102, 0x00000000, 0x1b140501, 0x080c0d19,
 0x0a141607, 0x0b080d0d, 0x0a070509, 0x14212010, 0x00000003, 0x01000000, 0x080c1610, 0x00000311,
 0x12060100, 0x6d5f4730, 0x5a606673, 0x05112a44, 0x00010309, 0x07020000, 0x161e1e18, 0x0204040d,
 0x080a0a03, 0x21100f0e, 0x040e1923, 0x05101107, 0x00000001, 0x00000000, 0x04020503, 0x00020810,
 0x18110200, 0x675d4832, 0x4d52606b, 0x121c2c41, 0x0000010b, 0x1f110100, 0x1d2b3a35, 0x0000010e,
 0x00010000, 0x2f1c1306, 0x000e202f, 0x00020100, 0x00000000, 0x00000000, 0x0e020000, 0x00061618,
 0x16170300, 0x5c614c2c, 0x3d3f4e57, 0x0b102239, 0x00000006, 0x27190400, 0x30384136, 0x01060a1e,
 0x00000000, 0x2d281905, 0x03142029, 0x00000001, 0x00000000, 0x01000000, 0x1c0a0101, 0x00091f25,
 0x22210300, 0x535b4d35, 0x2b2a3948, 0x00062130, 0x00000000, 0x211d0f03, 0x30312e21, 0x0a120e1d,
 0x00000001, 0x26291503, 0x17262826, 0x0000020d, 0x00000000, 0x0d030000, 0x2316090f, 0x00071c28,
 0x311d0200, 0x48504c44, 0x2e2c373f, 0x04102934, 0x03010001, 0x1c271f0c, 0x2a291c11, 0x131a1119,
 0x00000003, 0x1c1f0e01, 0x2e2d2720, 0x00021129, 0x00000000, 0x2e120101, 0x2b282133, 0x010d232c,
 0x34180100, 0x434d4d48, 0x2929343c, 0x0e1c2629, 0x0f070002, 0x141f1b13, 0x2c250c06, 0x111d1a1e,
 0x00000004, 0x17191001, 0x40392f1f, 0x030e2740, 0x04010000, 0x48290702, 0x36474a4f, 0x01112936,
 0x31110000, 0x30424845, 0x20242d2c, 0x19242621, 0x090c0508, 0x13100806, 0x1e150307, 0x0715171b,
 0x00000001, 0x17140a01, 0x4c433723, 0x15203750, 0x0f0a0206, 0x563a170c, 0x5a67675d, 0x01112e49,
 0x26080000, 0x23313836, 0x23292d28, 0x1b2c312a, 0x00040a0f, 0x0b060000, 0x0a030003, 0x0d191715,
 0x00000001, 0x05020000, 0x41342813, 0x393e494f, 0x211f1724, 0x5c4a2f22, 0x71777368, 0x01143758,
 0x1e070000, 0x252d2f2b, 0x34272429, 0x0f283a3f, 0x01000204, 0x00010002, 0x01010001, 0x191f1409,
 0x00000005, 0x00000000, 0x2f180c03, 0x52525349, 0x30303344, 0x5f58483b, 0x7c7f7d75, 0x02174c71,
 0x13050000, 0x161d1815, 0x36271919, 0x0f26383b, 0x01000001, 0x00000001, 0x00000000, 0x1a1d0f01,
 0x00000006, 0x00000000, 0x1a070100, 0x46424234, 0x39383d48, 0x635b594f, 0x8082837c, 0x00063e79,
 0x0b070001, 0x0e120908, 0x2a26170f, 0x15242c28, 0x00000003, 0x00030400, 0x06020000, 0x201e1107,
 0x00000009, 0x00000000, 0x04010000, 0x342f2916, 0x49483f3a, 0x615a5d57, 0x7f828379, 0x0102296b,
 0x01020001, 0x110d0301, 0x1a18110c, 0x121f1e1b, 0x00000002, 0x04080400, 0x20130302, 0x1c291f1c,
 0x00000004, 0x00000000, 0x00000000, 0x31231104, 0x564f372e, 0x5d565d5d, 0x7a828578, 0x02001157,
 0x00000001, 0x0c0d0200, 0x0c0c0503, 0x10120d0b, 0x00000002, 0x09020000, 0x331e0d0b, 0x13343837,
 0x00000001, 0x00000000, 0x00000000, 0x2c140201, 0x564e382f, 0x6054585b, 0x5f7b7c75, 0x03010e37,
 0x00000001, 0x04030000, 0x0e080001, 0x1117120d, 0x00000001, 0x01000000, 0x2d1a0e06, 0x0b2b4040,
 0x00000000, 0x00000000, 0x00000000, 0x210b0000, 0x4e43362e, 0x58484752, 0x345a5d60, 0x03010713,
 0x00000001, 0x00000000, 0x03010000, 0x03070504, 0x00000001, 0x00000000, 0x0b030101, 0x010b1819,
 0x00000000, 0x00000000, 0x00000000, 0x11040000, 0x201f1b1e, 0x20131325, 0x13202426, 0x01000007,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x02000001, 0x02030407, 0x01000003, 0x01030202, 0x00000001,
};

static unsigned int objectTexture[] = {
 0xffff0000, 0xffffffff, 0xffff0000, 0xffffffff, 0x03600020, 0x7f7fffff, 0x08600440, 0x0088ceff,
 0x0be00040, 0x5b5ffeff, 0x14000020, 0xfdf9ffff, 0xffff0000, 0xffffffff, 0xffff0000, 0xffffffff,
 0x78008aa9, 0xb5ed517c, 0x800190a5, 0x5f7555aa, 0x800188c3, 0x555555aa, 0x7801a085, 0x55f7d72a,
 0x80838802, 0x00000055, 0x800180c3, 0x555555aa, 0x80018082, 0x95555500, 0x80018a8b, 0x5e71c537,
 0xffff0000, 0xffffffff, 0x03400040, 0xf6fffff7, 0x072000c0, 0xf64f7f7f, 0x0f010960, 0x0ffda9a9,
 0x13210840, 0x736fffbe, 0x1b210060, 0xfdd7d7f5, 0x13c10000, 0x95feffff, 0xffff0000, 0xffffffff,
 0x8001a866, 0x75555515, 0x80009229, 0x95cd535e, 0x80019824, 0x57557959, 0x78009803, 0xf52d37f5,
 0x78008802, 0xbebc9edf, 0x8001a045, 0xd5575e55, 0x8021826a, 0x5e79c515, 0x7800b0a8, 0x5f575556,
 0xffff0000, 0xffffffff, 0x08a00420, 0xf7fcf8f0, 0x0f400040, 0xaaeafbf5, 0x03a00040, 0xf5ffff5f,
 0x0be000c0, 0x5657cebf, 0x13000000, 0xfffdefed, 0x04800060, 0xfdfff7e6, 0x03400020, 0xf6d5ffff,
 0x78009802, 0x75d575c9, 0x8000a827, 0xa5b555a5, 0x78218248, 0x154d5154, 0x8001b005, 0x555f7b55,
 0x8001b048, 0x5f5555f5, 0x80218269, 0x5c71c515, 0x8001a8a7, 0xf5555554, 0x70008800, 0x7a6f6e68,
 0x03800020, 0xffffefd7, 0x1bc00020, 0xffffbbf5, 0x16a00860, 0xe5fffbaa, 0x03c00000, 0x5f5feffe,
 0x06600060, 0xf5f5f93b, 0x0f2008a0, 0xbf7f637f, 0x176009a0, 0x8eb936f9, 0x03000000, 0xfdfffeff,
 0x7800a004, 0x4de97555, 0x78009804, 0x76dd5ad2, 0x80008862, 0x5555f535, 0x80018a29, 0x354d7b54,
 0x82498802, 0x09249840, 0x800180e3, 0x55555554, 0x7801a005, 0x7b687ff7, 0x6800a004, 0x6bc36fff,
 0x06a00040, 0xffef5f9f, 0x17200080, 0xa8af7fbf, 0x0f600080, 0xfee695d5, 0x0bc00020, 0x5f5befff,
 0x0e0000c0, 0xd5f3ef3b, 0x06e00040, 0xebab5f5b, 0x05e00000, 0x7ffbfaff, 0x07600020, 0xe5f5ffff,
 0x78009002, 0x557d55cd, 0x78009804, 0x7c5d7fd0, 0x7000a804, 0x75dff727, 0x8021826a, 0x5c79c515,
 0x80218227, 0x15c5515c, 0x8001a048, 0x5ee5f5f5, 0x78009002, 0x27d77dd7, 0x70008000, 0xe84a62ea,
 0x02600000, 0xffff7fff, 0x0e800060, 0xfdfeeeaa, 0x14600060, 0xd7fffbfd, 0x0dc00060, 0xfe7c7f6f,
 0x07a01be0, 0xff627355, 0x07600080, 0xf5f5fecf, 0x05c00040, 0xdfff7f7f, 0x07a00020, 0xfffbf5d5,
 0x78009802, 0xc575d535, 0x7800c008, 0x5555775e, 0x80228249, 0x5c71c515, 0x80c38801, 0x00000001,
 0x7801b809, 0x5e5e5a55, 0x88228a28, 0x15c5795c, 0x8001c829, 0x4d5555f5, 0x70009803, 0x877f7f7f,
 0xffff0000, 0xffffffff, 0x06800020, 0xfffffdfd, 0x06800040, 0xcbffd7d7, 0x0b800020, 0xf7f7e7f7,
 0x0f0000c0, 0xffef5795, 0x06800020, 0xeffff5f6, 0x06e00060, 0xebebdb9f, 0xffff0000, 0xffffffff,
 0x7000a806, 0x8dedffc9, 0x78208269, 0x5c71c515, 0x8001c0ca, 0x757d5556, 0x7801a003, 0x35dd55a9,
 0x7801b006, 0x5c77555c, 0x7800b005, 0xa575555a, 0x82478822, 0x40900401, 0x78009865, 0x5c5755ed,
 0xffff0000, 0xffffffff, 0xffff0000, 0xffffffff, 0x06600000, 0x7f6f7ffb, 0x13e00820, 0xfffdffff,
 0x06400060, 0xfff7ebeb, 0x03200000, 0xffffffdf, 0xffff0000, 0xffffffff, 0xffff0000, 0xffffffff,
 0x78008249, 0xf873c515, 0x800180c3, 0xaa555554, 0x78c38802, 0xff000000, 0x78c38001, 0xff000000,
 0x69848001, 0xaa000000, 0x78c38001, 0xff000000, 0x78c48001, 0xff000080, 0x78017a27, 0x37cd715c,
 0x8a02aae7, 0x5e5e505e, 0x79e0a283, 0x2d2d8b0d, 0x79e09263, 0xd0fadcdf, 0x79e0c304, 0x0d0d85bd,
 0x79c09283, 0xdcf8fcdc, 0x81e09a83, 0xf9ea6ae2, 0x79e08a62, 0xdd3fdd9d, 0x81e09263, 0xdf90fad2,
 0xef3cffff, 0x030303fd, 0xf77dffff, 0x00000055, 0xf77dffff, 0x00000055, 0xf77dffff, 0x00000055,
 0xf77dffff, 0x00000055, 0xf77dffff, 0x00000055, 0xf77dffff, 0x00000055, 0xf75cffff, 0xc080c077,
 0x79e09284, 0xf8ba58f0, 0x81e0c325, 0x75bd3535, 0x8200b307, 0x555e5c5a, 0x8200bb04, 0x85050585,
 0x7a01ab06, 0x56545657, 0x82008a40, 0x650149d1, 0x79e08a40, 0xdf3fdf3f, 0x81e08a42, 0x9fbbf1b3,
 0xf77dffff, 0x01010301, 0x5cdcffbe, 0xf7f75700, 0x54daffbd, 0xffff5500, 0x54bbffbd, 0xffff5500,
 0x3c5bef9d, 0xb9fd7d00, 0x54daffbd, 0xffff5500, 0x3c1cef9d, 0x6e7bf700, 0xef9dffff, 0x4143c340,
 0x82008a84, 0x765adcf8, 0x79e08a40, 0x3d1fff3f, 0x79e09263, 0xd3d85eff, 0x79e0a283, 0x9dfdbd0d,
 0x79e08a42, 0xfffadff8, 0x79e09263, 0xf5efbd2f, 0x71e08200, 0x2a208a20, 0x81e08a42, 0xabbfffb3,
 0xf77dffff, 0x01010301, 0x6cfebdf8, 0x070f070f, 0x6d1abdf7, 0x58608000, 0x64d9b5f8, 0x02092d0a,
 0x4cbcae18, 0x21010101, 0x447bbdf8, 0xd8608000, 0x343db5f8, 0x40424b6f, 0xef9effff, 0xc343c143,
 0x82018a62, 0x8ca8f4e8, 0x79e08a40, 0xdf3f9fbf, 0x82009241, 0x57505552, 0x79e08a62, 0xddffbd3f,
 0x79e08a20, 0xffbaf8bb, 0x79e09262, 0xdd3fedef, 0x79e08221, 0xfeb0ffbb, 0x79e08a42, 0xffbfb133,
 0xf77dffff, 0x01010301, 0x6cfeb5f8, 0x070f070f, 0x54bbb5f9, 0x0b0727bc, 0xb5d7be18, 0x5b2a5b2a,
 0x4cbcadf8, 0x01010101, 0x5499bdf8, 0x020e349c, 0x3c1db5f8, 0x40404040, 0xef9effff, 0x4343c143,
 0x8a018a62, 0xfe20f808, 0x79e08a40, 0x1fbfdf3f, 0x8200a2a4, 0x58525f5b, 0x79e08a40, 0xfd3fdf3f,
 0x79c08221, 0xa2a0b9a0, 0x79e09263, 0xad2bedab, 0x79e08221, 0xffbbfbb0, 0x81c08a42, 0xbba39f3b,
 0xf77dffff, 0x01010301, 0x6cfebdf8, 0x070f070f, 0xb5f7be18, 0x55bbddef, 0xadd7be18, 0x5ba27b2a,
 0x4cbcae18, 0x21010101, 0xb5f7be18, 0x55bbddbb, 0x3c3db5f8, 0x40404040, 0xef9effff, 0xc343c143,
 0x82019aa3, 0x48cc7efe, 0x79e08a60, 0x9d1f9d3f, 0x8200a2c4, 0x5f5d5f50, 0x79e08a40, 0xfd3fdfbf,
 0x79e08a42, 0xf3fbdffa, 0x81e08a83, 0xed2defb9, 0x71e08200, 0x8a20a220, 0x81e08222, 0x3f3bbf3b,
 0xf77dffff, 0x01010301, 0x44bdf7df, 0x000057f7, 0x44bcf7df, 0x000055ff, 0x3cbcf7df, 0x0000557f,
 0x3c9cf7df, 0x000055fd, 0x44bcf7df, 0x000055ff, 0x449cf7df, 0x0000557f, 0xf79effff, 0x40404143,
 0x8201a2c4, 0x40c86eea, 0x79e08a60, 0xdd1fddbf, 0x7a01a2e3, 0x7b585f57, 0x79e08a41, 0xbf191fbf,
 0x79e08a41, 0xfff8defa, 0x79c0a2a3, 0xad8bcfbf, 0x79e08200, 0xa802aa22, 0x79c09a83, 0xbf3f9f3f,
 0xf77cffff, 0x03010301, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000,
 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xf79dffff, 0x40404040,
 0x81e0a2a3, 0x77becee8, 0x71c08a20, 0xdd2aaeaa, 0x71e1a2a3, 0x55fb7ef2, 0x71c08a40, 0x9d3bbb3b,
 0x71c09a64, 0x52f2f8f0, 0x8200a2e4, 0xb509e5ad, 0x79c08220, 0xdfb0aaaa, 0x8200bb47, 0xe50da535,
 0xef5cffff, 0xf5020303, 0xf77dffff, 0x55000000, 0xf77dffff, 0x55000000, 0xf77dffff, 0x55000000,
 0xf77dffff, 0x55000000, 0xf77dffff, 0x55000000, 0xf77dffff, 0x55000000, 0xef5cffff, 0x7fc080c0,
};
////////////////////////////////////////////////////////////////////////////////
// Rearange pixels if Big Endian hardware or CPU word is greater than 4 bytes.
// I am on 32 bit Intel so this code is dead on my machine. 
// If you happen to find a bug let us know or better fix it and let us know.
////////////////////////////////////////////////////////////////////////////////
static unsigned char * orderBytes( unsigned int *raster, unsigned int size )
{
    unsigned int endian_check = 0x1;

    unsigned char & little_endian = *(unsigned char*)&endian_check;
    unsigned char * dst = (unsigned char*)raster;
    unsigned char * src = (unsigned char*)raster;

    unsigned int dst_step = 4;
    unsigned int src_step = sizeof( unsigned int );

    if( little_endian && src_step == dst_step )
        return src;

    unsigned counter = size / src_step;

    // pack bytes if 64 (or more) bit machine
    if( src_step > dst_step ) {
        if( !little_endian )
            src += src_step - dst_step;

        for( unsigned int i = 0; i < counter; i++ )
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
            src += src_step;
            dst += dst_step;
        }
    }

    // reorder bytes if big endian machine
    if( !little_endian ) {
        dst = (unsigned char*)raster;
        while( counter-- )
        {
            std::swap( dst[0], dst[3] );
            std::swap( dst[1], dst[2] );
            dst += dst_step;
        }
    }

    return (unsigned char*)raster;
}
////////////////////////////////////////////////////////////////////////////////
static void addTree
    ( const osg::Vec3 & p,
      osg::Vec3Array* vertices, osg::Vec3Array* normals, osg::Vec2Array* texCoords, 
      float r = sqrt( 3.f ), float h = 10.f, float s = 5.f )
{
    float te = 1.0 / 64;

    for( unsigned i = 0; i < 6; i ++ ) {

        float x = sin( i * osg::PI / 3.f );
        float y = cos( i * osg::PI / 3.f );

        vertices->push_back( p + osg::Vec3(  x * r + y, y * r - x,  0 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r - y, y * r + x,  0 ) * s );
        vertices->push_back( p + osg::Vec3(  0, 0, h * 2 / 3 ) * s );
        vertices->push_back( p + osg::Vec3(  0, 0, h * 2 / 3 ) * s );

        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );

        texCoords->push_back( osg::Vec2(0.0 + te, 1.0 - te ) );
        texCoords->push_back( osg::Vec2(0.5 - te, 1.0 - te ) );
        texCoords->push_back( osg::Vec2(0.5 - te, 0.5 + te ) );
        texCoords->push_back( osg::Vec2(0.0 + te, 0.5 + te ) );
    }

    for( unsigned i = 0; i < 6; i ++ ) {

        float x = sin( i * osg::PI / 3.f );
        float y = cos( i * osg::PI / 3.f );

        vertices->push_back( p + osg::Vec3(  x * r + 4 * y, y * r - 4 * x,  h * 2 / 3 - r - 4 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r - 4 * y, y * r + 4 * x,  h * 2 / 3 - r - 4 ) * s );
        vertices->push_back( p + osg::Vec3( -x * r - 4 * y, -y * r + 4 * x, h * 2 / 3 + r + 4 ) * s );
        vertices->push_back( p + osg::Vec3( -x * r + 4 * y, -y * r - 4 * x, h * 2 / 3 + r + 4 ) * s );

        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );

        texCoords->push_back( osg::Vec2( 0.0 + te, 0.5 - te ) );
        texCoords->push_back( osg::Vec2( 0.5 - te, 0.5 - te ) );
        texCoords->push_back( osg::Vec2( 0.5 - te, 0.0 + te ) );
        texCoords->push_back( osg::Vec2( 0.0 + te, 0.0 + te ) );

    }

    vertices->push_back( p + osg::Vec3( -4 - r, -4 -r, h * 2 / 3 ) * s );                
    vertices->push_back( p + osg::Vec3( -4 - r, 4 + r, h * 2 / 3 ) * s );
    vertices->push_back( p + osg::Vec3(  4 + r, 4 + r, h * 2 / 3 ) * s );
    vertices->push_back( p + osg::Vec3(  4 + r, -4 -r, h * 2 / 3 ) * s );                

    normals->push_back( osg::Vec3(0, 0, 1) );
    normals->push_back( osg::Vec3(0, 0, 1) );
    normals->push_back( osg::Vec3(0, 0, 1) );
    normals->push_back( osg::Vec3(0, 0, 1) );

    texCoords->push_back( osg::Vec2(0.0 + te, 0.5 - te ) );
    texCoords->push_back( osg::Vec2(0.5 - te, 0.5 - te ) );
    texCoords->push_back( osg::Vec2(0.5 - te, 0.0 + te ) );
    texCoords->push_back( osg::Vec2(0.0 + te, 0.0 + te ) );
}
////////////////////////////////////////////////////////////////////////////////
static void addHouse
    ( const osg::Vec3 & p,
      osg::Vec3Array* vertices, osg::Vec3Array* normals, osg::Vec2Array* texCoords, 
      float r = 3, float h = 10.f, float s = 3.f )
{
    float te = 0.5 / 64;
    float rot = osg::PI * rand() / RAND_MAX;
    for( unsigned i = 0; i < 4; i ++ ) {

        float x = sin( rot + i * osg::PI / 2.f );
        float y = cos( rot + i * osg::PI / 2.f );

        vertices->push_back( p + osg::Vec3(  x * r + y * r, y * r - x * r,  0 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r - y * r, y * r + x * r,  0 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r - y * r, y * r + x * r,  h * 2 / 3 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r + y * r, y * r - x * r,  h * 2 / 3 ) * s );

        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );
        normals->push_back( osg::Vec3(x, y, 0 ) );

        texCoords->push_back( osg::Vec2(0.5 + te, 1.0 - te) );
        texCoords->push_back( osg::Vec2(1.0 - te, 1.0 - te) );
        texCoords->push_back( osg::Vec2(1.0 - te, 0.5 + te) );
        texCoords->push_back( osg::Vec2(0.5 + te, 0.5 + te) );
    }

    for( unsigned i = 0; i < 4; i ++ ) {

        r ++;
        float x = sin( rot + i * osg::PI / 2.f );
        float y = cos( rot + i * osg::PI / 2.f );

        vertices->push_back( p + osg::Vec3(  x * r + y * r, y * r - x * r,  h * 2 / 3 ) * s );
        vertices->push_back( p + osg::Vec3(  x * r - y * r, y * r + x * r,  h * 2 / 3 ) * s );
        vertices->push_back( p + osg::Vec3( 0, 0, h ) * s );
        vertices->push_back( p + osg::Vec3( 0, 0, h ) * s );

        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );
        normals->push_back( osg::Vec3(x, y, 0.5 ) );

        texCoords->push_back( osg::Vec2(0.5 + te, 0.5 - te) );
        texCoords->push_back( osg::Vec2(1.0 - te, 0.5 - te ) );
        texCoords->push_back( osg::Vec2(1.0 - te, 0.0 + te ) );
        texCoords->push_back( osg::Vec2(0.5 + te, 0.0 + te ) );
        r--;
    }
}
////////////////////////////////////////////////////////////////////////////////
static osg::Geode* createObjects( osg::HeightField * grid, unsigned int density )
{    
    osg::Geode* geode = new osg::Geode;

    // set up the texture of the base.
    osg::StateSet* stateset = new osg::StateSet();

    osg::Image * image = new osg::Image();
    image->setImage( 64,64,1,
                     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                     GL_UNSIGNED_BYTE,
                     orderBytes( objectTexture, sizeof( objectTexture ) ),
                     osg::Image::NO_DELETE );

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setImage(image);

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    stateset->setMode( GL_BLEND,osg::StateAttribute::ON );
              
    stateset->setAttributeAndModes( new osg::CullFace() );
    stateset->setAttributeAndModes( new osg::Depth( osg::Depth::LESS, 0.f, 1.f, true ) );
    stateset->setAttributeAndModes( new osg::AlphaFunc( osg::AlphaFunc::GEQUAL, 0.5f ) );

    geode->setStateSet( stateset );

    osg::Geometry* geometry = new osg::Geometry;
    geode->addDrawable(geometry);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Vec2Array* texCoords = new osg::Vec2Array;
    geometry->setTexCoordArray(0, texCoords);    

    osg::Vec4Array* colours = new osg::Vec4Array;
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    for( unsigned int x = 0; x < grid->getNumColumns() - 1; x++ ) {
        for( unsigned int y = 0; y < grid->getNumRows() - 1; y++ ) {

            float z00 = grid->getHeight( x,y );
            float z01 = grid->getHeight( x,y+1 );
            float z10 = grid->getHeight( x+1,y );
            float z11 = grid->getHeight( x+1,y+1 );

            float z = 0.25 * ( z00 + z10 + z11 + z01 );

            if( z < 400 ) continue;
            if( z > 500 ) continue;

            osg::Vec3 o( x * grid->getXInterval(), y * grid->getYInterval(), 0 );
            o += grid->getOrigin();

            for( unsigned int d = 0; d < density; d++  ) {
                osg::Vec3 p( float( rand() ) / RAND_MAX, float( rand() ) / RAND_MAX, 0 );

                z = ( 1.f - p[0] ) * ( 1.f - p[1] ) * z00 + 
                    ( 1.f - p[0] ) * ( p[1] ) * z01 +
                    ( p[0] ) * ( p[1] ) * z11 +
                    ( p[0] ) * ( 1.f - p[1] ) * z10;                

                p.set( p[0] * grid->getXInterval(), p[1] * grid->getYInterval(), z - 2.0 );

                p += o;                 
     
                if( rand() % 3 > 0 )
                    addTree( p, vertices, normals, texCoords );
                else 
                    addHouse( p, vertices, normals, texCoords );

            }
        }
    }

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vertices->size()));

    geode->addDrawable(geometry);
    return geode;
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* createIsland(const osg::Vec3& center = osg::Vec3( 0,0,0 ), float radius = 8192 * 0.5 )
{
    float height = 1000;

    osg::Geode* geode = new osg::Geode;

    // set up the texture of the base.
    osg::StateSet* stateset = new osg::StateSet();
    geode->setStateSet( stateset );

    osg::Image * heightMap = new osg::Image();
    heightMap->setImage( 64, 64, 1, 
                         GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                         orderBytes( heightTexture, sizeof( heightTexture ) ), 
                         osg::Image::NO_DELETE );

    osg::Image* colorMap = NULL; // osgDB::readImageFile("Images/colorMap.png");
    if ( !colorMap ) {        
        
        struct colorElevation 
        {
            colorElevation(unsigned int elev, const osg::Vec4ub& c):
                elevation(elev), color(c) {}
                
            unsigned int elevation; 
            osg::Vec4ub color;
        };
        
        colorElevation colorElevationMap[] =
            {
                colorElevation(0, osg::Vec4ub( 0, 128, 255, 255 )), 
                colorElevation(8, osg::Vec4ub( 192, 192, 128, 255 )),
                colorElevation(32, osg::Vec4ub( 0, 255, 0, 255 )),
                colorElevation(128, osg::Vec4ub( 128, 128, 128, 255 )),
                colorElevation(192, osg::Vec4ub( 96, 96, 96, 255 )),
                colorElevation(255, osg::Vec4ub( 255, 255, 255, 255 )),
                colorElevation(256, osg::Vec4ub( 255, 255, 255, 255 ))
            };        

        colorMap = new osg::Image();
        colorMap->allocateImage( heightMap->s(), heightMap->t(),1, GL_RGBA, GL_UNSIGNED_BYTE );
        unsigned int margin = 0;
        for( unsigned int r=margin; r< colorMap->t()-margin; ++r)
        {
            for(unsigned int c=margin; c< colorMap->s()-margin; ++c)
            {
                unsigned int h = *heightMap->data(c,r);
                unsigned int i = 0;
                while( h > colorElevationMap[i+1].elevation ) ++i;

                float f0 = float( h - colorElevationMap[i].elevation ) /
                           ( colorElevationMap[i+1].elevation - colorElevationMap[i].elevation );

                float f1 = 1.f - f0;

                               
                *(osg::Vec4ub*)colorMap->data(c,r) = 
                    colorElevationMap[i].color * f1 + colorElevationMap[i+1].color * f0;
            }
        }
    }

    if (colorMap)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(colorMap);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }

    stateset->setAttributeAndModes( new osg::CullFace(), osg::StateAttribute::ON );

    osg::HeightField* grid = new osg::HeightField;
    grid->allocate(heightMap->s(),heightMap->t());
    grid->setOrigin( center - osg::Vec3( radius, radius, 0 ) );
    grid->setXInterval(radius*2.0f/grid->getNumColumns() );
    grid->setYInterval(radius*2.0f/grid->getNumRows());

    for( unsigned int r=0;r<grid->getNumRows()-0;++r)
    {
        for(unsigned int c=0;c<grid->getNumColumns()-0;++c)
        {
            grid->setHeight( c, r, height * exp( *heightMap->data(c,r) / 255.f ) / exp( 1.0 ) );            
        }
    }
    osg::ShapeDrawable * sd  = new osg::ShapeDrawable(grid);
    sd->setColor( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );

    geode->addDrawable(sd);

    osg::Group* group = new osg::Group;
    group->addChild(geode);

    group->addChild( createObjects( grid, 1 ) );

    return group;
}
////////////////////////////////////////////////////////////////////////////////
namespace ModelFour {
    
osg::Node* createModel(osg::ArgumentParser& /*arguments*/)
{
    return createIsland();
}

}
////////////////////////////////////////////////////////////////////////////////
