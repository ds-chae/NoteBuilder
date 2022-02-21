// WinDebug.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"
#include "WinDebug.h"
#include "WinDebugDlg.h"

// CWinDebug

IMPLEMENT_DYNAMIC(CWinDebug, CWnd)

CWinDebug::CWinDebug()
{

}

CWinDebug::~CWinDebug()
{
}


BEGIN_MESSAGE_MAP(CWinDebug, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

#define	DEBUG_IPC_SIZE	16

unsigned char* DebugIPCPtr[DEBUG_IPC_SIZE];
DWORD DebugIPCLen[DEBUG_IPC_SIZE];
int DebugIPCHead = 0;
int DebugIPCTail = 0;

int GetDebugIPCPtr(unsigned char** pptr, DWORD* len)
{
	if(DebugIPCHead == DebugIPCTail)
		return 0;

	*pptr = DebugIPCPtr[DebugIPCTail];
	*len = DebugIPCLen[DebugIPCTail];
	DebugIPCTail = (DebugIPCTail+1) % DEBUG_IPC_SIZE;

	return 1;
}

extern TCHAR tszExtToExtract[16];
int GetIPCDataHeader(unsigned char* debugdata, char* caption, int* type, int* width, int* height);
void ConvertRawToTIFF(TCHAR* path);

unsigned int WinDebugWait(void *pData)
{
	// Create the named pipe
	HANDLE hPipe = CreateNamedPipe(L"\\\\.\\pipe\\STMyPipe", PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH,
		PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 10, 10000, 2000, 10000, NULL);

	unsigned char* rbuf = (unsigned char*)LocalAlloc(LPTR, 1728*1000);
	while(1) {
		if(!ConnectNamedPipe(hPipe, 0))
			break;

		// 'Read the data sent by the client over the pipe
		DWORD dwio = 0;
		if(ReadFile(hPipe, rbuf, 1728*1000, &dwio, NULL) && dwio > 0) {
			unsigned char *ptr = (unsigned char*)LocalAlloc(LPTR, dwio);
			memcpy(ptr, rbuf, dwio);

			DebugIPCPtr[DebugIPCHead] = ptr;
			DebugIPCLen[DebugIPCHead] = dwio;
			DebugIPCHead = (DebugIPCHead+1) % DEBUG_IPC_SIZE;

			((CWnd*)pData)->PostMessageW(WM_COMMAND, ID_DEBUG_IPC, 0);

			dwio = 0;
			WriteFile(hPipe, "A", 1, &dwio, NULL);
		}

		// Disconnect the named pipe.
		DisconnectNamedPipe(hPipe);
	}
	LocalFree(rbuf);

	return 0;
}


// CWinDebug message handlers



void CWinDebug::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
}
