#ifndef _MACROVALUE_H_
#define _MACROVALUE_H_

void ReplaceTagTextInFile(TCHAR* OCR_ConstFileName, char* tag1, char* txt1);
extern WIN_ST_LOCAL WinCurrentLocal;

class CMacroValue {
public:
	char name[128];

private:
	int value;
	int modified;

public:
	CMacroValue(char* n, int initial)
	{
		modified = 1; // 생성시에 들어간 값은 디폴트 값에 불과하므로...
		strcpy_s(name, sizeof(name), n);
		value = initial;
	}

	int Get()
	{
		return value;
	}

	int Set(int v)
	{
		if(value != v) {
			value = v;
			modified = 1;

			return 1;
		}

		return 0;
	}

	int SetIfMatch(char* n, char* vstr)
	{
		if(!name[0]) // 이름이 없다면 세팅할 수 없다.
			return 0;

		if(strstr(n, name)) {
			value = atoi(vstr);
			modified = 0; // 헤더 파일에서 읽어온 것이므로 초기화.
			return 1;
		}

		return 0;
	}
/*
	void SaveToFile(TCHAR* fn)
	{
		char tag1[128];
		char txt1[256];

		if(!modified)
			return;

		sprintf_s(tag1, 128, "#define\t%s%s", pWinCurrentLocal->szCurrency, name);
		sprintf_s(txt1, 256, "#define\t%s%s\t%d\r\n", pWinCurrentLocal->szCurrency, name, value);
		ReplaceTagTextInFile(fn, tag1, txt1);
	}
*/
};

extern CMacroValue OCR_ImageSource;// USD_OCR_IMGSOURCE, 0	= IR Transmit, 1 = W Reflection

extern CMacroValue OCR_HWindow;// USD_OCR_H_WINDOW
extern CMacroValue OCR_VWindow;// USD_OCR_V_WINDOW_1 
extern CMacroValue OCR_VWindow_2;// USD_OCR_V_WINDOW_2	21

extern CMacroValue OCR_maxchar;//OCR_MAXCHAR 12
extern CMacroValue OCR_minchar;//OCR_MINCHAR 10

extern CMacroValue OCR_save_width_height_count; // USD_OCR_SAVE_WIDTH_HEIGHT_COUNT	1

//extern CMacroValue OCR_x_window;// USD_OCR_X_WINDOW	18 삭제, OCR_HWindow중복됨.
extern CMacroValue OCR_f_height;// USD_OCR_F_HEIGHT	14

extern CMacroValue OCR_IR_Percent;// USD_OCR_IR_PCT
extern CMacroValue OCR_W_Percent;// USD_OCR_W_PCT
extern CMacroValue least_pixel;// USD_OCR_MIN_PIXELS
extern CMacroValue OCR_xwindow_method;// USD_OCR_XWIN_METHOD
extern CMacroValue OCR_ThrType;// USD_OCR_THR_TYPE
extern CMacroValue OCR_scaleX;// USD_OCR_SCALE_X	2
extern CMacroValue OCR_scaleY;// USD_OCR_SCALE_Y	1

#endif
