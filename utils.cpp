#include "globals.h"

void Version2Str(DWORD versionMS, DWORD versionLS, LPTSTR str)
{
	DWORD ver[4];
	ver[0] = HIWORD(versionMS);
	ver[1] = LOWORD(versionMS);
	ver[2] = HIWORD(versionLS);
	ver[3] = LOWORD(versionLS);
	int lastNum = 3;
	for(int i=3; i >=0; i--)
	{
		if(ver[i])
		{
			lastNum = i;
			break;
		}
	}
	if(lastNum < 1) lastNum = 1;
	str[0] = 0;
	for(int i=0; i <= lastNum; i++)
	{
		TCHAR num[100];
		_itot(ver[i], num, 10);
		lstrcat(str, num);
		if(i != lastNum)
		{
			lstrcat(str, TEXT("."));
		}
	}
}

void GetFileVersion(LPCTSTR fileName, DWORD& versionMS, DWORD& versionLS)
{
	versionMS = 0;
	versionLS = 0;
	DWORD dw;
	DWORD verSize = GetFileVersionInfoSize((LPTSTR) fileName, &dw);
	if(verSize)
	{
		void* verData = malloc(verSize);
		if(GetFileVersionInfo((LPTSTR) fileName, NULL, verSize, verData))
		{
			VS_FIXEDFILEINFO* verInfo;
			UINT len = 0;
			if(VerQueryValue(verData, TEXT("\\"), (void**) &verInfo, &len))
			{
				versionMS = verInfo->dwFileVersionMS;
				versionLS = verInfo->dwFileVersionLS;
			}
		}
		free(verData);
	}
}

void DrawVGradient(HDC dc, const RECT& rectClient, COLORREF m_clrStart, COLORREF m_clrEnd, BYTE alpha) 
{ 
	int nMaxWidth = rectClient.bottom - rectClient.top;
	RECT rectFill; // Rectangle for filling band 
	float fStep; // How wide is each band? 
	HBRUSH brush = NULL; // Brush to fill in the bar 
	//First find out the largest color distance between the start and 
	//end colors.This distance will determine how many steps we use to 
	//carve up the client region and the size of each gradient rect. 

	int r, g, b; // First distance, then starting value 
	float rStep, gStep, bStep; // Step size for each color 
	BOOL bSameColor = FALSE; 
	int nSteps; 
	// Get the color differences 
	r = (GetRValue(m_clrEnd) - GetRValue(m_clrStart)); 
	g = (GetGValue(m_clrEnd) - GetGValue(m_clrStart)); 
	b = (GetBValue(m_clrEnd) - GetBValue(m_clrStart)); 

	if((r == 0) && (g == 0) && (b == 0)) 
	{ 
		bSameColor = TRUE; 
		//{{ADDED BY RAJESH 
		//Added the three lines below to fix the drawing 
		//problem which used to occur when both the start 
		//and end colors are same. 
		r = GetRValue(m_clrStart); 
		g = GetGValue(m_clrStart); 
		b = GetBValue(m_clrStart); 
		//}}ADDED BY RAJESH 
	} 

	// Make the number of steps equal to the greatest distance 
	//{{MODIFIED BY RAJESH 
	//To fix the drawing problem which used to occur when 
	//both the start and end colors are RGB(0,0,0). 
	if(bSameColor && m_clrStart == 0) 
	{
		nSteps = 255;//Select max. possible value for nSteps 
	}
	else 
	{
		nSteps = max(abs(r), max(abs(g), abs(b))); 
	}
	//Determine how large each band should be in order to cover the 
	//client with nSteps bands (one for every color intensity level) 
	//}}MODIFIED BY RAJESH 
	fStep = (float)(rectClient.bottom - rectClient.top) / (float)nSteps; 

	//Calculate the step size for each color 
	rStep = r/(float)nSteps; 
	gStep = g/(float)nSteps; 
	bStep = b/(float)nSteps; 

	//Reset the colors to the starting position 
	r = GetRValue(m_clrStart); 
	g = GetGValue(m_clrStart); 
	b = GetBValue(m_clrStart); 

	// Start filling bands 
	for (int iOnBand = 0; iOnBand < nSteps; iOnBand++) 
	{ 
		SetRect(&rectFill, 
			rectClient.left, 
			rectClient.top + (int)(iOnBand * fStep), 
			rectClient.right, 
			rectClient.top + (int)((iOnBand+1) * fStep));

		if(iOnBand+1 == nSteps)
		{
			rectFill.bottom = rectClient.bottom;
		}

		if(rectFill.bottom > rectClient.bottom) 
		{ 
			rectFill.bottom = rectClient.bottom;
		}

		//CDC::FillSolidRect is faster, but it does not handle 
		//8-bit color depth 
		if(bSameColor) 
		{
			brush = CreateSolidBrush(m_clrStart);
		}
		else 
		{
			brush = CreateSolidBrush(RGB(r+rStep * iOnBand, 
				g + gStep * iOnBand, b + bStep * iOnBand)); 
		}
		FillRect(dc, &rectFill, brush); 
		DeleteObject(brush);

		//If we are past the maximum for the current position we 
		//need to get out of the loop.Before we leave,we repaint the 
		//remainder of the client area with the background color. 
	} 
} 

void replace_str(std::wstring& dst, std::wstring& substr, std::wstring& replaceWith)
{
	size_t off = dst.find(substr);
	while (off != std::wstring::npos)
	{
		dst.replace(off, substr.size(), replaceWith);
		off = dst.find(substr);
	}
}

LPWSTR load_utf8_file(LPCWSTR fileName)
{
	LPWSTR ret = NULL;

	if (PathFileExists(fileName))
	{
		HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD szHigh;
			DWORD szLow = GetFileSize(hFile, &szHigh);
			HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, szHigh, szLow, NULL);
			if (hMapping)
			{
				SIZE_T memSize = szLow;
				if (szHigh)
				{
					memSize = MAXDWORD;
				}
				else
				{
					memSize = szLow;
				}
				LPVOID data = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, memSize);

				LPCSTR str_data = (LPCSTR)data;
				if (szLow >= 3 && str_data[0] == '\xEF' && str_data[1] == '\xBB' && str_data[2] == '\xBF')
				{
					str_data += 3;
				}

				int sz = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_data, (int)memSize, NULL, 0);
				ret = new WCHAR[sz + 1];
				sz = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_data, (int)memSize, ret, sz + 1);
				ret[sz] = 0;

				UnmapViewOfFile(data);
				CloseHandle(hMapping);
			}
			CloseHandle(hFile);
		}
	}
	return ret;
}

std::wstring load_text_from_resource(LPCWSTR id, LPCWSTR group)
{
	std::wstring ret;
	HRSRC hResource = ::FindResource(g_hInst, id, group);
	if (hResource)
	{
		DWORD imageSize = ::SizeofResource(g_hInst, hResource);
		if (imageSize)
		{
			LPCSTR pResourceData = (LPCSTR) ::LockResource(::LoadResource(g_hInst, hResource));
			if (pResourceData)
			{
				LPWSTR css = new WCHAR[imageSize * 3];
				int sz = MultiByteToWideChar(CP_UTF8, 0, pResourceData, imageSize, css, imageSize * 3);
				css[sz] = 0;
				ret = css;
				delete css;
			}
		}
	}
	return ret;
}