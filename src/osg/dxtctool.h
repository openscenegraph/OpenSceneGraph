// dxtctool.h: interface for the DXTC tools.
//
//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002 Tanguy Fautré.
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Tanguy Fautré
//  softdev@telenet.be
//
//////////////////////////////////////////////////////////////////////
//
//						DXTC Tools: Vertical Flip.
//						*************************
//
// Current version: 1.00 BETA 1 (27/08/2002)
//
// Comment: Only works with DXTC mode supported by OpenGL.
//          (currently: DXT1/DXT3/DXT5) 
//
// History: -
//
//////////////////////////////////////////////////////////////////////

#ifndef DXTCTOOL_H
#define DXTCTOOL_H 

#include <osg/GL>
#include <osg/Texture>

#if !defined(_MSC_VER)
typedef char        __int8;
typedef short       __int16;
typedef int         __int32;
typedef long long   __int64;
#endif

namespace dxtc_tool {




// C-like function wrappers
bool VerticalFlip(size_t Width, size_t Height, GLenum Format, void * pPixels);



// Class holding reference to DXTC image pixels 
class dxtc_pixels
{
public:
	dxtc_pixels(size_t Width, size_t Height, GLenum Format, void * pPixels);

	// Vertically flip the whole picture
	bool VFlip() const;

protected:

	// Limitation check functions
	bool DXT1() const;
	bool DXT3() const;
	bool DXT5() const;
	bool OpenGLSize() const;
	bool SupportedFormat() const;

	// Vertical flipping functions
	void VFlip_DXT1() const;	
	void VFlip_DXT3() const;
	void VFlip_DXT5() const;

	// Block vertical flipping functions
	void BVF_Color_H2(void * const pBlock) const;							// V. flip one color block with its virtual height == 2
	void BVF_Color_H4(void * const pBlock) const;							// V. flip one color block with its virtual height == 4
	void BVF_Color(void * const pBlock1, void * const pBlock2) const;		// V. flip and swap two color blocks, with their virtual height == 4
	void BVF_Alpha_DXT3_H2(void * const pBlock) const;						// V. flip one alpha (DXT3) block with its virtual height == 2
	void BVF_Alpha_DXT3_H4(void * const pBlock) const;						// V. flip one alpha (DXT3) block with its virtual height == 4
	void BVF_Alpha_DXT3(void * const pBlock1, void * const pBlock2) const;	// V. flip and swap two alpha (DXT3) blocks, with their virtual height == 4
	void BVF_Alpha_DXT5_H2(void * const pBlock) const;						// V. flip one alpha (DXT5) block with its virtual height == 2
	void BVF_Alpha_DXT5_H4(void * const pBlock) const;						// V. flip one alpha (DXT5) block with its virtual height == 4
	void BVF_Alpha_DXT5(void * const pBlock1, void * const pBlock2) const;	// V. flip and swap two alpha (DXT5) blocks, with their virtual height == 4
	
	// Block localization functions
	void * GetBlock(size_t i, size_t j, size_t BlockSize) const;

	// mighty const and var
	static const size_t BSIZE_DXT1 = 8;
	static const size_t BSIZE_DXT3 = 16;
	static const size_t BSIZE_DXT5 = 16;
	static const size_t BSIZE_ALPHA_DXT3 = 8;
	static const size_t BSIZE_ALPHA_DXT5 = 8;

	const size_t	m_Width;
	const size_t	m_Height;
	const GLenum	m_Format;
	void * const	m_pPixels;
};




//////////////////////////////////////////////////////////////////////
// C-Like Function Wrappers
//////////////////////////////////////////////////////////////////////

inline bool VerticalFlip(size_t Width, size_t Height, GLenum Format, void * pPixels) {
	return (dxtc_pixels(Width, Height, Format, pPixels)).VFlip(); 
}



//////////////////////////////////////////////////////////////////////
// dxtc_pixels Inline Functions
//////////////////////////////////////////////////////////////////////

inline dxtc_pixels::dxtc_pixels(size_t Width, size_t Height, GLenum Format, void * pPixels) :
	m_Width(Width), m_Height(Height), m_Format(Format), m_pPixels(pPixels) { }


inline bool dxtc_pixels::DXT1() const {
	return ((m_Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) || (m_Format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT));
}


inline bool dxtc_pixels::DXT3() const {
	return (m_Format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
}


inline bool dxtc_pixels::DXT5() const {
	return (m_Format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}


inline bool dxtc_pixels::SupportedFormat() const {
	return (DXT1() || DXT3() || DXT5());
}


inline void dxtc_pixels::BVF_Color_H2(void * const pBlock) const {
	// Swap the two first row of pixels
	__int8 * const pP = ((__int8 * const) pBlock) + 4;

	std::swap(pP[0], pP[1]);
}


inline void dxtc_pixels::BVF_Color_H4(void * const pBlock) const {
	// Swap the the first row of pixels with the last one, then the 2 middle row of pixels
	__int8 * const pP = ((__int8 * const) pBlock) + 4;

	std::swap(pP[0], pP[3]);
	std::swap(pP[1], pP[2]);
}


inline void dxtc_pixels::BVF_Color(void * const pBlock1, void * const pBlock2) const {
	// Swap the "2 colors" header (32bits each header)
	__int32 * const pHdr1 = (__int32 * const) pBlock1;
	__int32 * const pHdr2 = (__int32 * const) pBlock2;

	std::swap(* pHdr1, * pHdr2);

	// Now swap the pixel values
	__int8 * const pP1 = ((__int8 * const) pBlock1) + 4;
	__int8 * const pP2 = ((__int8 * const) pBlock2) + 4;

	std::swap(pP1[0], pP2[3]);
	std::swap(pP1[1], pP2[2]);
	std::swap(pP1[2], pP2[1]);
	std::swap(pP1[3], pP2[0]);
}


inline void dxtc_pixels::BVF_Alpha_DXT3_H2(void * const pBlock) const {
	// Swap the two first row of pixels
	__int16 * const pP = (__int16 * const) pBlock;

	std::swap(pP[0], pP[1]);
}


inline void dxtc_pixels::BVF_Alpha_DXT3_H4(void * const pBlock) const {
	// Swap the the first row of pixels with the last one, then the 2 middle row of pixels
	__int16 * const pP = (__int16 * const) pBlock;

	std::swap(pP[0], pP[3]);
	std::swap(pP[1], pP[2]);
}


inline void dxtc_pixels::BVF_Alpha_DXT3(void * const pBlock1, void * const pBlock2) const {
	// Swap all the pixel values
	__int16 * const pP1 = (__int16 * const) pBlock1;
	__int16 * const pP2 = (__int16 * const) pBlock2;

	std::swap(pP1[0], pP2[3]);
	std::swap(pP1[1], pP2[2]);
	std::swap(pP1[2], pP2[1]);
	std::swap(pP1[3], pP2[0]);
}


inline void dxtc_pixels::BVF_Alpha_DXT5_H2(void * const pBlock) const {
	// Swap the two first row of pixels (kinda tricky with DXT5 unaligned encoding)
	__int32 * const pP = (__int32 * const) (((__int8 * const) pBlock) + 2);

	__int32 TmpDWord = (pP[0] & 0xFF000000);
	TmpDWord |=	(pP[0] & 0x00000FFF) << 12;
	TmpDWord |= (pP[0] & 0x00FFF000) >> 12;
	pP[0] = TmpDWord;
}


inline void dxtc_pixels::BVF_Alpha_DXT5_H4(void * const pBlock) const {
	// Swap the the first row of pixels with the last one, then the 2 middle row of pixels (tricky again)
	__int64 * const pB = (__int64 * const) pBlock;

	__int64 TmpQWord = (pB[0] & 0x000000000000FFFFll);
	TmpQWord |= (pB[0] & 0x000000000FFF0000ll) << 36;
	TmpQWord |= (pB[0] & 0x000000FFF0000000ll) << 12;
	TmpQWord |= (pB[0] & 0x000FFF0000000000ll) >> 12;
	TmpQWord |= (pB[0] & 0xFFF0000000000000ll) >> 36;
	pB[0] = TmpQWord;
}


inline void dxtc_pixels::BVF_Alpha_DXT5(void * const pBlock1, void * const pBlock2) const {
	// Swap all the pixel values (same trick for DXT5)
	__int64 * const pB1 = (__int64 * const) pBlock1;
	__int64 * const pB2 = (__int64 * const) pBlock2;

	__int64 TmpQWord1 = (pB1[0] & 0x000000000000FFFFll);
	TmpQWord1 |= (pB1[0] & 0x000000000FFF0000ll) << 36;
	TmpQWord1 |= (pB1[0] & 0x000000FFF0000000ll) << 12;
	TmpQWord1 |= (pB1[0] & 0x000FFF0000000000ll) >> 12;
	TmpQWord1 |= (pB1[0] & 0xFFF0000000000000ll) >> 36;

	__int64 TmpQWord2 = (pB2[0] & 0x000000000000FFFFll);
	TmpQWord2 |= (pB2[0] & 0x000000000FFF0000ll) << 36;
	TmpQWord2 |= (pB2[0] & 0x000000FFF0000000ll) << 12;
	TmpQWord2 |= (pB2[0] & 0x000FFF0000000000ll) >> 12;
	TmpQWord2 |= (pB2[0] & 0xFFF0000000000000ll) >> 36;

	pB1[0] = TmpQWord2;
	pB2[0] = TmpQWord1;
}


inline void * dxtc_pixels::GetBlock(size_t i, size_t j, size_t BlockSize) const {
	const __int8 * pPixels = (const __int8 *) m_pPixels;

	return (void *) (pPixels + i * ((m_Width + 3) / 4) * BlockSize + j * BlockSize);
}




} // namespace dxtc_tool

#endif // DXTCTOOL_H
