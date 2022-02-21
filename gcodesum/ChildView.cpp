
// ChildView.cpp : CChildView 클래스의 구현
//

#include "stdafx.h"
#include "gcodesum.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct _tGcodeData {
	TCHAR cFileName[MAX_PATH];
	TCHAR cFilePath[MAX_PATH];
	float xPos;
	float yPos;
	float Width;
	float Height;
	float Depth;
	int   size;
	char* data;
} GCodeDataType;

#define	MAX_GCODE_FILES	20

GCodeDataType GCodeDataTable[MAX_GCODE_FILES];
int GcodeDataCount = 0;

TCHAR tszSaveFilePath[MAX_PATH];

// CChildView

CChildView::CChildView()
{
	tszSaveFilePath[0] = 0;
	GcodeDataCount = 0;
	memset(GCodeDataTable, sizeof(GCodeDataTable), 0);

	x_min = 10000000, x_max = -10000000;
	y_min = 10000000, y_max = -10000000;

	x_cur = 0;
	y_cur = 0;

	z_min = 10000000, z_max = -10000000;
	z_cur = 0;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CChildView::OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_ADD, &CChildView::OnUpdateFileAdd)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CChildView::OnUpdateFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CChildView::OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CChildView::OnUpdateFileSave)
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
END_MESSAGE_MAP()



// CChildView 메시지 처리기

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.
	
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	
	// 그리기 메시지에 대해서는 CWnd::OnPaint()를 호출하지 마십시오.
}



void CChildView::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void CChildView::OnUpdateFileAdd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void CChildView::OnUpdateFileNew(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void CChildView::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void CChildView::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


char* srcgets(char** srcbuffer, char* dstbuffer, int dstsize)
{
	char* sp = *srcbuffer;
	if(!*sp) {
		*dstbuffer = 0;
		return NULL;
	}

	int dstlen = 0;
	dstsize--;

	char c;
	while(1) {
		c = *sp++;
		if(c == '\n' || c == 0)
			break;
		dstbuffer[dstlen++] = c;
		if(dstlen >= dstsize)
			break;
	}
	dstbuffer[dstlen] = 0;
	if(c == 0)
		sp--;
	*srcbuffer = sp;

	return dstbuffer;
}

int strtovalue(char* tstr)
{
	int value = 0;
	int sign = 1;
	int decount = -1;
	while(*tstr) {
		if(*tstr == '-')
			sign = -1;
		else if(*tstr == '.')
			decount = 0;
		else if(*tstr >= '0' && *tstr <= '9') {
			value = value * 10 + (*tstr - '0');
			if(decount != -1) decount++;
		}
		if(decount >= 3)
			break;
		tstr++;
	}
	if(decount == -1)
		decount = 0;
	while(decount < 3) {
		value = value * 10;
		decount++;
	}

	return value * sign;
}

void CChildView::parseLine(char* linebuffer)
{
	char code = 0;
	int  state = 0;
	int  value = 0;
		
	char parabuf[128];
	int  paralen = 0;
	int  dot_pos = -1;

	for(char *cp = linebuffer; *cp; cp++) {
			
		switch(state) {
		case 0 :
			if(*cp >= 'A' && *cp <= 'Z') {
				code = *cp;
				state = 1;
				dot_pos = -1;
			} else if(*cp == '(') {
				state = 2;
			}
			break;
		case 1 : // wait number
			if(*cp >= '0' && *cp <= '9' || *cp == '.') {
				parabuf[paralen++] = *cp;
			} else {
				cp--;
				state = 0;
				parabuf[paralen] = 0;
				paralen = 0;

				value = strtovalue(parabuf);
				switch(code){
				case 'X' :
					x_cur = value;
					if(x_cur < x_min) x_min = x_cur;
					if(y_cur > x_max) x_max = x_cur;
					break;
				case 'Y' :
					y_cur = value;
					if(y_cur < y_min) y_min = y_cur;
					if(y_cur > y_max) y_max = y_cur;
					break;
				case 'Z' :
					z_cur = value;
					if(z_cur < z_min) z_min = z_cur;
					if(z_cur > z_max) z_max = z_cur;
				}
			}
			break;
		case 2 :
			if(*cp == ')')
				state = 0;
		}
	}
}

void CChildView::OnFileOpen()
{
	x_min = 10000000, x_max = -10000000;
	y_min = 10000000, y_max = -10000000;

//	TCHAR szFilter[] = L"GCode (*.BMP,*.GIF) | *.BMP;*.GIF;*.JPG | All Files(*.*)|*.*||";
	TCHAR szFilter[] = L"GCode (*.TAP) | *.TAP | All Files(*.*)|*.*||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK != dlg.DoModal())
		return;

	CString strPathName = dlg.GetPathName();
	HANDLE h = CreateFile(strPathName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;
	
	int fsize = GetFileSize(h, NULL);
	char *fullsrc = (char*)malloc(fsize+1);
	if(fullsrc == NULL) {
		CloseHandle(h);
		MessageBox(L"Not enough memory");
		return;
	}

	DWORD dwio = 0;
	ReadFile(h, fullsrc, fsize, &dwio, NULL);
	CloseHandle(h);
	if(dwio != fsize) {
		MessageBox(L"Reading failed.");
		return;
	}
	fullsrc[fsize] = 0;

	char linebuffer[256];
	char linebuflen = 0;
	char* srcptr = fullsrc;

/*
	// test signature
	srcgets( &srcptr, linebuffer, sizeof(linebuffer));
	if(strncmp(linebuffer, "(GCODESUM)", strlen("(GCODESUM)")) {
		MessageBox("No GCODESUM signature");
		free(fullsrc);
		return;
	}
*/

	while(srcgets( &srcptr, linebuffer, sizeof(linebuffer))) {
		parseLine(linebuffer);
	}
	free(fullsrc);


}
