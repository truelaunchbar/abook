#include "globals.h"
#include "CropPhotoDlg.h"

CCropPhotoDlg::CCropPhotoDlg(CXUIEngine* engine, LPCWSTR fileName) : CXUIDialog(L"res:cropphoto.xml", engine)
{
	m_fileName	= NULL;
	m_img		= NULL;
	m_imgSize	= NULL;
	MAKE_STR(m_fileName, fileName);
}

CCropPhotoDlg::~CCropPhotoDlg(void)
{
	if(m_img)
	{
		delete m_img;
	}
}

void CCropPhotoDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();
	
	m_ctlCrop->setImage(m_fileName);

}

BOOL CCropPhotoDlg::OnEndDialog( UINT code )
{
	if(code == IDOK)
	{
		WCHAR tempPath[MAX_PATH];
		WCHAR tempFile[MAX_PATH];
		GetTempPath(MAX_PATH, tempPath);
		GetTempFileName(tempPath, L"abthumb", 0, tempFile);
		if(m_ctlCrop->save(tempFile))
		{
			HANDLE hFile = CreateFile(tempFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				m_imgSize = GetFileSize(hFile, NULL);
				if(m_imgSize)
				{
					m_img = new BYTE[m_imgSize];
					ReadFile(hFile, m_img, m_imgSize, (LPDWORD) &m_imgSize, NULL);
					CloseHandle(hFile);
				}
			}
			DeleteFile(tempFile);
		}
	}
	return TRUE;
}

BOOL CCropPhotoDlg::OnZoomChanged( CXUIElement* el, LPARAM lParam )
{
	m_ctlCrop->setZoom(el->value_INT());
	return TRUE;
}