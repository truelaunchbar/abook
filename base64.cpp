#include "globals.h"
#include "base64.h"

char* base64::base64_encode = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t base64::Encode( const base64::uchar* in, size_t length, base64::uchar* outbuf, size_t size, uint option /*= BASE64_STANDARD*/ )
{
	size_t cnt = 0;
	size_t rc  = 0;
	while ( rc < length )
	{
		if ( size < 4 )
		{
			// insufficient buffer
			cnt = (size_t)-1;
			break;
		}
		size_t left = length - rc;
		switch( left )
		{
		case 1:
			*outbuf++ = Encode((in[rc] >> 2));
			*outbuf++ = Encode((in[rc] << 4) & 0x30);
			*outbuf++ = to_uchar(BASE64_PAD);
			*outbuf++ = to_uchar(BASE64_PAD);
			break;
		case 2:
			*outbuf++ = Encode((in[rc] >> 2));
			*outbuf++ = Encode(((in[rc] << 4) & 0x30) | ((in[rc + 1] >> 4) & 0x0f));
			*outbuf++ = Encode((in[rc+1] << 2) & 0x3c);
			*outbuf++ = to_uchar(BASE64_PAD);
			break;
		default:
			*outbuf++ = Encode((in[rc] >> 2));
			*outbuf++ = Encode(((in[rc] << 4) & 0x30) | ((in[rc + 1] >> 4) & 0x0f));
			*outbuf++ = Encode(((in[rc + 1] << 2) & 0x3c) | ((in[rc + 2] >> 6)));
			*outbuf++ = Encode((in[rc + 2]) & 0x3f);
			break;
		}
		rc  += 3;
		cnt += 4;
		size -= 4;
		if ( (option&BASE64_STANDARD) && !(cnt%BASE64_NCHAR))
			*outbuf++ = to_uchar(BASE64_LF);
	}

	if ( cnt != -1 )
		*outbuf = 0;
	return cnt;
}

base64::uchar base64::Decode( uchar c )
{
	if (c == '/')
		return 63;
	if (c == '+')
		return 62;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	return (uchar)-1;
}

size_t base64::Decode( const base64::uchar* in, size_t length, base64::uchar* outbuf, size_t size, uint option /*= BASE64_STANDARD*/ )
{
	size_t cnt = 0;
	size_t rc  = 0;
	while ( (*in) && (rc < length) )
	{
		if ( (in[rc] == BASE64_CR || in[rc] == BASE64_LF || !IsBase64Char(in[rc])))
			rc++;
		else if ( size > 1 )
		{
			uchar b1 = ( Decode(in[rc])<<2 &0xfc ) | ( Decode(in[rc+1])>>4 &0x03 );
			*outbuf++ = b1;
			--size;
			++cnt;

			if ((size>1) && in[rc + 2] != BASE64_PAD)
			{
				uchar b2 = ( Decode(in[rc+1])<<4 &0xf0 ) | ( Decode(in[rc+2])>>2 &0x0f );
				*outbuf++ = b2;
				--size;
				++cnt;
			}
			if ((size>1) && in[rc + 3] != BASE64_PAD)
			{
				uchar b3 = ( Decode(in[rc+2])<<6 &0xc0 ) | ( Decode(in[rc+3] )& 0x3f );
				*outbuf++ = b3;
				--size;
				++cnt;
			}
			rc += 4;
		}
		else
		{
			// insufficient buffer
			cnt = (size_t)-1;
			break;
		}
	}
	if ( cnt != -1 )
		*outbuf = 0;
	return cnt;
}

std::string base64::DecodeToStr( std::string& inStr )
{
	size_t outSize = CalcBufferSize(inStr.length()) + 1;
	char* outBuffer = new char[outSize];
	outSize = Decode((const base64::uchar*) inStr.c_str(), inStr.length(), (base64::uchar*) outBuffer, outSize);
	outBuffer[outSize] = 0;
	std::string ret = outBuffer;
	delete outBuffer;
	return ret;
}

void* base64::DecodeToBin( std::string& inStr, size_t* size )
{
	size_t outSize = inStr.length();
	char* outBuffer = new char[outSize];
	outSize = Decode((base64::uchar*) inStr.c_str(), inStr.length(), (base64::uchar*) outBuffer, outSize);
	*size = outSize;
	return outBuffer;
}
