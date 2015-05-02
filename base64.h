#pragma once

#include <string>

namespace base64
{
	typedef unsigned char uchar;
	typedef unsigned int  uint;

	extern char* base64_encode;

	enum {
		BASE64_PAD      = '=',	///< Pad character
		BASE64_CR       = '\r',	///< Carriage return
		BASE64_LF       = '\n',	///< New line separator
		BASE64_NCHAR    = 76,	///< Number of characters per line when encoding
		BASE64_STANDARD = 0x01	///< Multi line support
	};

	inline uchar		to_uchar (char c)				{ return (uchar) c;												}
	inline size_t		CalcBufferSize(size_t length)	{ return ((length + 2) / 3) * 4 + (length / BASE64_NCHAR) + 1;	};
	inline uchar		Encode(uchar c)					{ return ( c < 64 ? base64_encode[ c ] : (uchar)-1 );			}
	size_t		Encode(const uchar* in, size_t length, uchar* outbuf, size_t size, uint option = BASE64_STANDARD);
	uchar		Decode(uchar c);
	size_t		Decode(const uchar* in, size_t length, uchar* outbuf, size_t size, uint option = BASE64_STANDARD);
	inline bool		IsBase64Char(uchar c)			{ return ( Decode( c ) != (uchar)-1 );							}
	std::string EncodeToStr(const uchar* in, size_t length);
	std::string EncodeToStr(std::string& inStr);
	std::string DecodeToStr(std::string& inStr);
	void*		DecodeToBin(std::string& inStr, size_t* size);
};