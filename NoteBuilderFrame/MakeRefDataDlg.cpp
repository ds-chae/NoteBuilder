// MakeRefDataDlg.cpp : implementation file
//

#include "stdafx.h"

#include "B_Common.h"

#include "CurrencyInfo.h"
#include "NoteBuilderFrame.h"
#include "MakeRefDataDlg.h"

ST_IQDESCRIPTOR		reference[MAX_NUM_CURRENCY][MAX_NUM_SAMPLE][Num_Max_Key];
BOOL				reference_valid[MAX_NUM_CURRENCY][MAX_NUM_SAMPLE][Num_Max_Key];
ST_DATA				merged_data[MAX_SIZE_Y][MAX_SIZE_X][MAX_NUM_CURRENCY][additional_array_num];
BOOL				merged_data_flag[MAX_SIZE_Y][MAX_SIZE_X][MAX_NUM_CURRENCY];
int					added_file_num[MAX_NUM_CURRENCY];
ST_IQDESCRIPTOR		added_data[MAX_NUM_CURRENCY][MAX_NUM_SAMPLE][Num_Max_Key];
int					tf_count[MAX_NUM_SAMPLE];

// CMakeRefDataDlg dialog

IMPLEMENT_DYNAMIC(CMakeRefDataDlg, CDialog)

CMakeRefDataDlg::CMakeRefDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMakeRefDataDlg::IDD, pParent)
{
	m_Num_Currency = 0;
}

CMakeRefDataDlg::~CMakeRefDataDlg()
{
}

void CMakeRefDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DELETED, m_ListDeleted);
}


BEGIN_MESSAGE_MAP(CMakeRefDataDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_KEY_FOLDER, &CMakeRefDataDlg::OnBnClickedButtonKeyFolder)
	ON_BN_CLICKED(IDC_BUTTON_DSP_FOLDER, &CMakeRefDataDlg::OnBnClickedButtonDspFolder)
	ON_BN_CLICKED(IDC_BUTTON_REF_FOLDER, &CMakeRefDataDlg::OnBnClickedButtonRefFolder)
	ON_BN_CLICKED(IDOK, &CMakeRefDataDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMakeRefDataDlg message handlers

BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname, LPARAM lParam, DWORD flag);

void CMakeRefDataDlg::OnBnClickedButtonKeyFolder()
{
	TCHAR tszKeyFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_KEY_FOLDER, tszKeyFolder, MAX_PATH);
	if(BrowseFolder(GetSafeHwnd(), tszKeyFolder, (LPARAM)m_tszKeyFolder1, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		lstrcpy(m_tszKeyFolder1, tszKeyFolder);
		SetDlgItemText(IDC_EDIT_KEY_FOLDER, m_tszKeyFolder1);
	}
}

void CMakeRefDataDlg::OnBnClickedButtonRefFolder()
{
	TCHAR tszFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_REF_FOLDER, tszFolder, MAX_PATH);
	if(BrowseFolder(GetSafeHwnd(), tszFolder, (LPARAM)m_tszRefFolder, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		lstrcpy(m_tszRefFolder, tszFolder);
		SetDlgItemText(IDC_EDIT_REF_FOLDER, m_tszRefFolder);
		RegSetNNRFolder();
	}
}

BOOL CMakeRefDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_EDIT_KEY_FOLDER, m_tszKeyFolder1);
	SetDlgItemText(IDC_EDIT_REF_FOLDER, m_tszRefFolder);

	SetDlgItemInt(IDC_EDIT_XSIZE, 56);
	SetDlgItemInt(IDC_EDIT_YSIZE, 50);

	GetDlgItem(IDC_CHECK_COPY_KEY)->SendMessage(BM_SETCHECK, BST_CHECKED, NULL);

	SetDlgItemInt(IDC_EDIT_CUR_NBR, pWinCurrentLocal->CurrencyNum);
	SetDlgItemText(IDC_EDIT_CUR_CODE, pWinCurrentLocal->tszCurrency);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMakeRefDataDlg::OnBnClickedOk()
{
	TCHAR CurrencyCode[8];
	int CurrencyNum = GetDlgItemInt(IDC_EDIT_CUR_NBR);
	GetDlgItemText(IDC_EDIT_CUR_CODE, CurrencyCode, 8);

	TCHAR tszKeyFolder[MAX_PATH];
	TCHAR tszRefFolder[MAX_PATH];
	TCHAR tszDspFolder[MAX_PATH];
	TCHAR tszPrefix[10];

	GetDlgItemText(IDC_EDIT_KEY_FOLDER, tszKeyFolder, MAX_PATH);
	GetDlgItemText(IDC_EDIT_REF_FOLDER, tszRefFolder, MAX_PATH);
	GetDlgItemText(IDC_EDIT_DSP_FOLDER, tszDspFolder, MAX_PATH);
	GetDlgItemText(IDC_EDIT_PREFIX, tszPrefix, 10);

	m_Img_Size_X = GetDlgItemInt(IDC_EDIT_XSIZE);
	m_Img_Size_Y = GetDlgItemInt(IDC_EDIT_YSIZE);

	m_bContinue = 1;
	if(GetDlgItem(IDC_CHECK_COPY_KEY)->SendMessage(BM_GETCHECK) == BST_CHECKED)
		CopyKeyFolderToRefFolder(L"", tszKeyFolder, tszRefFolder);
	else {
		if(MessageBox(L"Skip copying key?", L"Check", MB_YESNO) != IDYES)
			return;
	}

	if(!m_bContinue)
		return;
	GenerateSrcFromRef(tszRefFolder, tszDspFolder, tszPrefix, CurrencyNum, CurrencyCode);

	SetDlgItemInt(IDC_EDIT_MAX_REF, m_Max_Ref_Buf_Size);
	
	MessageBox(L"Reference generation finished.");
}

void CMakeRefDataDlg::CopyKeyFolderToRefFolder(TCHAR* RefIndex, TCHAR* tszKeyFolder, TCHAR* tszRefFolder)
{
	TCHAR tempPath[MAX_PATH];
	wsprintf(tempPath, L"%s\\DENOM *", tszKeyFolder);
	WIN32_FIND_DATA wfd;
	
	// 폴더 밑에 있는 DENOM ##.txt 파일을 찾는다.
	TCHAR currentRefIndex[10];
	currentRefIndex[0] = 0;
	HANDLE hFind = FindFirstFile(tempPath, &wfd);
	if(hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		TCHAR* cp = wfd.cFileName;
		while(*cp && *cp < '0' || *cp > '9') cp++;
		int i = 0;
		while(*cp >= '0' && *cp <= '9' && i < 3) {
			currentRefIndex[i] = *cp++;
			i++;
		}
		currentRefIndex[i] = 0;
	}
	
	// 못 찾았으면 상위 인덱스를 이용한다.
	if(currentRefIndex[0] == 0) {
		if(RefIndex != NULL)
			lstrcpy(currentRefIndex, RefIndex);
	}

	// 자 이제 파일을 나머지 찾도록 한다.
	wsprintf(tempPath, L"%s\\*", tszKeyFolder);
	hFind = FindFirstFile(tempPath, &wfd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if(StrStr(wfd.cFileName, L"key.txt")) {
			// 아직까지 인덱스가 없다면 오류를 알리고 종료한다.
			if(currentRefIndex[0] == 0) {
				TCHAR msg[MAX_PATH];
				wsprintf(msg, L"Following path requires denom index file, DENOM ##.txt\r\n%s", tszKeyFolder);
				MessageBox(msg);
				m_bContinue = 0;
				break;
			}
			CopyKeyFileToRefFolder(currentRefIndex, tszKeyFolder, wfd.cFileName, tszRefFolder);
		} else {
			if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
				if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					TCHAR subPath[MAX_PATH];
					wsprintf(subPath, L"%s\\%s", tszKeyFolder, wfd.cFileName);
					CopyKeyFolderToRefFolder(currentRefIndex, subPath, tszRefFolder);
				}
			}
		}
		if(!FindNextFile(hFind, &wfd))
			break;
		if(!m_bContinue)
			break;
	}
	FindClose(hFind);
}

BOOL 	FileExist(TCHAR* dstDirName, WIN32_FIND_DATA* wfd);

void CMakeRefDataDlg::CopyKeyFileToRefFolder(TCHAR* currentRefIndex, TCHAR* subPath, TCHAR* subFile, TCHAR* tszRefFolder)
{
	WIN32_FIND_DATA wfd;

	if(currentRefIndex[0] == 0) {
		MessageBox(L"DENOM index is not found.");
		m_bContinue = 0;
		return;
	}

	// 대상 폴더가 없으면 생성한다.
	TCHAR dstDirName[MAX_PATH];
	wsprintf(dstDirName, L"%s\\%s", tszRefFolder, currentRefIndex);
	if(!FileExist(dstDirName, &wfd))
		CreateDirectory(dstDirName, NULL);
	if(!FileExist(dstDirName, &wfd)) {
		TCHAR msg[MAX_PATH];
		wsprintf(msg, L"Cannot create %s", tszRefFolder);
		MessageBox(msg);
		m_bContinue = 0;
		return;
	}
	
	// 이름이 겹치지 않는 이름을 찾는다.
	int seq = 0;
	TCHAR dstFileName[MAX_PATH];
	for(seq = 0; seq < 10000; seq++) {
		wsprintf(dstFileName, L"%s\\%s\\%04d_%s", tszRefFolder, currentRefIndex, seq, subFile);
		if(!FileExist(dstFileName, &wfd))
			break;
	}
	if(seq >= 10000) {
		TCHAR msg[MAX_PATH];
		wsprintf(msg, L"Cannot find subsequent filenae in %s\\%s.", tszRefFolder, currentRefIndex);
		MessageBox(msg);
		m_bContinue = 0;
		return;
	}
	
	// 파일 이름을 찾았으니 사용하자.
	TCHAR srcPath[MAX_PATH];
	wsprintf(srcPath, L"%s\\%s", subPath, subFile);
	CopyFile(srcPath, dstFileName, FALSE);
}

int GetNumberOfImages(TCHAR* path)
{
	int count = -1;

	TCHAR spath[MAX_PATH];
	WIN32_FIND_DATA wfd;
	wsprintf(spath, L"%s\\*", path);
	HANDLE hFind = FindFirstFile(spath, &wfd);
	if(hFind != INVALID_HANDLE_VALUE) {
		while(1) {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
					int t = StrToInt(wfd.cFileName);
					if(t != -1) count = t;
				}
			}
			if(!FindNextFile(hFind, &wfd))
				break;
		}
		FindClose(hFind);
	}

	return count + 1;
}

void CMakeRefDataDlg::GenerateSrcFromRef(TCHAR* tszRefFolder, TCHAR* tszDspFolder, TCHAR* tszPrefix, int CurrencyNum, TCHAR* CurrencyCode)
{
	m_Num_Currency = GetNumberOfImages(tszRefFolder);

	int tag=TRUE;
	CString DataPath = tszRefFolder;
	DataPath += L"\\";
	while(tag) {
		VariablesInitialize();
		for(int i=0;i<m_Num_Currency;i++) {
			LoadKeyData(i, DataPath);
			Merge_1_Data(i);						// 먼저 한 장만 놓고 reference를 만든다
		}
		AddReference();
		tag = RemoveRef(DataPath);
	}
	CheckMaxRefBuf();
	MakeDataFile(CurrencyNum, CurrencyCode); 
}

void CMakeRefDataDlg::VariablesInitialize()
{
	int i,j,k,m;
	for(i=0;i<m_Num_Currency;i++) {
		sample_number[i] = 0;
		for(j=0;j<MAX_NUM_SAMPLE;j++) {
			NumKeys[i][j] = 0;
			for(k=0;k<Num_Max_Key;k++) {
				reference[i][j][k].x = reference[i][j][k].y = 0;
				reference_valid[i][j][k] = FALSE;
				reference[i][j][k].DesVector = 0;
			}
		}
	}
	for(i=0;i<m_Img_Size_Y;i++) {
		for(j=0;j<m_Img_Size_X;j++) {
			for(k=0;k<m_Num_Currency;k++) {
				merged_data_flag[i][j][k] = FALSE;
				for(m=0;m<additional_array_num;m++) {
					merged_data[i][j][k][m].merged_count = 0;
					merged_data[i][j][k][m].valid = FALSE;
					merged_data[i][j][k][m].descriptor = 0;
				}
			}
		}
	}
}

FILE* fopenW(LPCTSTR fn, char* mode)
{
	char szfn[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, fn, lstrlen(fn)+1, szfn, MAX_PATH, NULL, NULL);
	return fopen(szfn, mode);
}


void CMakeRefDataDlg::LoadKeyData(int CurrencyIndex, CString Dir_path)
{
	int j;
	
	CString sample_index;
	if(CurrencyIndex<10) {
		sample_index.Format(L"0%d", CurrencyIndex);
	} else {
		sample_index.Format(L"%d", CurrencyIndex);
	}
	CFileFind ff;
	CString FileName;
	CString DataPath = Dir_path + sample_index + "\\";
	int temp_count = 0;
	if(ff.FindFile(DataPath + _T("*.txt"))){
		while(ff.FindNextFile()) {
			FileName = ff.GetFileName();
			FILE *OpenFile;
			CString tfn = DataPath + FileName;
			OpenFile = fopenW(tfn,"rt");

			fscanf(OpenFile,"SIFT_DN=70\n");
			fscanf(OpenFile,"SIFT_KN=8\n");
			for(j=0;j<Num_Max_Key;j++) {
				fscanf(OpenFile, "%d	%d	",&reference[CurrencyIndex][temp_count][j].x,&reference[CurrencyIndex][temp_count][j].y);
				if(reference[CurrencyIndex][temp_count][j].x==0 && reference[CurrencyIndex][temp_count][j].y==0)	break;
				else if(abs(reference[CurrencyIndex][temp_count][j].x)>100) {
					reference[CurrencyIndex][temp_count][j].x = reference[CurrencyIndex][temp_count][j].y = 0;
					break;
				}
				reference_valid[CurrencyIndex][temp_count][j] = TRUE;
				fscanf(OpenFile, "%d	",&reference[CurrencyIndex][temp_count][j].DesVector);
			}
			NumKeys[CurrencyIndex][temp_count] = j;
			temp_count++;
			fclose(OpenFile);
		}
		sample_number[CurrencyIndex] = temp_count;
	}
}

void CMakeRefDataDlg::Merge_1_Data(int CurrencyIndex)
{
	int i,j;
	int x_index,y_index;

	for(i=0;i<1;i++) {
		for(j=0;j<NumKeys[CurrencyIndex][i];j++) {											// 각각의 keys에 대해서
			x_index = reference[CurrencyIndex][i][j].x;
			y_index = reference[CurrencyIndex][i][j].y;

			merged_data_flag[y_index][x_index][CurrencyIndex] = TRUE;
			merged_data[y_index][x_index][CurrencyIndex][0].valid = TRUE;					// 점유하고 있음을 표시
			merged_data[y_index][x_index][CurrencyIndex][0].merged_count=1;					// merged_count를 증가
			merged_data[y_index][x_index][CurrencyIndex][0].descriptor = reference[CurrencyIndex][i][j].DesVector;
		}
	}
}

void CMakeRefDataDlg::AddReference()
{
	int i,j;

	for(j=m_Num_Currency-1;j>=0;j--) {						// currency 별로 하나씩 넣기로 한다.
		for(i=1;i<sample_number[j];i++) {					// 각각의 파일들을
			Check_Same_And_Add(i,j);							// 같은 데이터와 비교한 후 merged_data에 add한다.
		}
	}
}

int CMakeRefDataDlg::RemoveRef(CString Dir_path)
{
#if 0
	return FALSE;
#else
	int i,j,k,l,CurrencyIndex;
	int current_x[Num_Max_Key],current_y[Num_Max_Key],current_descriptor[Num_Max_Key];
	int x_index,y_index,Check_Count[MAX_NUM_CURRENCY],max_count,second_max_count,max_denom,tt_count=0;
	double t_ratio;
	int diff;

	FILE *SaveFile1;
	SaveFile1 = fopenW(Dir_path + "remove.bat", "w+");
	m_ListDeleted.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	CString str;
//////////////////////////////////////////////////////////////////////////////
//////// Reference를 Load한다.
	for(CurrencyIndex=0;CurrencyIndex<m_Num_Currency;CurrencyIndex++) {
		CString sample_index;
		if(CurrencyIndex<10) {
			sample_index.Format(L"0%d", CurrencyIndex);
		} else {
			sample_index.Format(L"%d", CurrencyIndex);
		}
		CFileFind ff;
		CString FileName;
		CString DataPath = Dir_path+sample_index+"\\";
		int temp_count = 0;
		if(ff.FindFile(DataPath + _T("*.txt"))){		// raw file을 찾으면
			while(ff.FindNextFile()) {
				FileName = ff.GetFileName();
				FILE *OpenFile;
				CString tfn = DataPath + FileName;
				OpenFile = fopenW(tfn,"rt");

				fscanf(OpenFile,"SIFT_DN=70\n");
				fscanf(OpenFile,"SIFT_KN=8\n");
				for(i=0;i<Num_Max_Key;i++) {
					current_x[i]=current_y[i]=0;
					current_descriptor[i]=0;
				}
				for(j=0;j<Num_Max_Key;j++) {
					fscanf(OpenFile, "%d	%d	",&current_x[j],&current_y[j]);
					if(current_x[j]==0 && current_y[j]==0)	break;
					else if(abs(current_x[j])>100) {
						current_x[j] = current_y[j] = 0;
						break;
					}
					fscanf(OpenFile, "%d	",&current_descriptor[j]);
				}
				fclose(OpenFile);
////////////// file을 load했으면
				for(k=0;k<m_Num_Currency;k++) {
					Check_Count[k]=0;
				}
				max_count = second_max_count = 0;
				max_denom = 0;
				t_ratio = 1.0;
				for(k=0;k<Num_Max_Key;k++) {
					if(current_x[k]!=0) {
						x_index=current_x[k];	y_index=current_y[k];
//						for(i=(y_index-1);i<=(y_index+1);i++) {
//							if(i<0)	i=0;	
//							for(j=(x_index-1);j<=(x_index+1);j++) {
//								if(j<0)	j=0;
						for(i=(y_index-2);i<=(y_index+2);i++) {
							if(i<0)	i=0;
							for(j=(x_index-2);j<=(x_index+2);j++) {
								if(j<0)	j=0;
								for(l=0;l<m_Num_Currency;l++) {
									for(int m=0;m<additional_array_num;m++) {
										if(merged_data[i][j][l][m].valid==TRUE) {
											diff = (merged_data[i][j][l][m].descriptor==current_descriptor[k])?1:0;
											if(diff==1) {
												Check_Count[l]++;
											}
										}
									}
								}
								if(j>=m_Img_Size_X-1)	j=x_index+3;
							}
							if(i>=m_Img_Size_Y-1)	i=y_index+3;
						}
					}
				}
				for(k=0;k<m_Num_Currency;k++) {
					if(max_count < Check_Count[k]) {
						max_count = Check_Count[k];			max_denom = k;
					}
				}
				Check_Count[max_denom] = 0;
				for(k=0;k<m_Num_Currency;k++) {
					if(second_max_count < Check_Count[k]) {
						second_max_count = Check_Count[k];
					}
				}
				if(max_count != 0) {
					t_ratio = second_max_count/max_count;
				}
				if((CurrencyIndex!=max_denom && max_count>1)) {
					fprintf(SaveFile1,"del %s\n",DataPath+FileName);
					tt_count ++;
					str=DataPath+FileName;
					m_ListDeleted.InsertItem(i, str);
				}
			}
		}
		fprintf(SaveFile1,"\n");
	}
	fclose(SaveFile1);
	WinExec(Dir_path+"remove.bat",SW_SHOW);
	CString exec;
	exec = L"del "+Dir_path+L"remove.bat";		// Main Data를 넣을 Directory를 만든다
	char szexec[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, NULL, lstrlen(exec)+1, szexec, MAX_PATH, NULL, NULL);
	system(szexec);

	if(tt_count!=0) {
		return TRUE;
	} else {
		return FALSE;
	}
#endif
}

void CMakeRefDataDlg::CheckMaxRefBuf()
{
	int i,j,k,max_n_count=0,n_count;

	for(i=2;i<m_Img_Size_Y-2;i++) {
		for(j=2;j<m_Img_Size_X-2;j++) {
			n_count = 0;
			for(k=0;k<m_Num_Currency;k++) {
				if(merged_data_flag[i][j][k] == TRUE) {
					for(int m=0;m<additional_array_num;m++) {
						if(merged_data[i][j][k][m].valid == TRUE && merged_data[i][j][k][m].merged_count>=min_merged_sum) {
							n_count++;
						}
					}
				}
			}
			if(max_n_count<n_count) {
				max_n_count = n_count;
			}
		}
	}
	m_Max_Ref_Buf_Size = max_n_count;
	UpdateData(FALSE);
}

void CMakeRefDataDlg::Check_Same_And_Add(int num_files, int CurrencyIndex)
{
	int i,j,l,m,n;
	int x_index,y_index;
	int x_m,x_p,y_m,y_p;
	int diff;
	BOOL same_flag;

	for(i=0;i<NumKeys[CurrencyIndex][num_files];i++) {													// 현재 비교할 Currency의 비교 파일의 key 중에서
		if(reference_valid[CurrencyIndex][num_files][i] == TRUE) {										// 다른 데이터와 겹치지 않는 현재 권종만의 데이터라면
			y_index = reference[CurrencyIndex][num_files][i].y;
			x_index = reference[CurrencyIndex][num_files][i].x;
			x_m = ((x_index-merge_margin)<0)?0:x_index-merge_margin;
			y_m = ((y_index-merge_margin)<0)?0:y_index-merge_margin;
			x_p = ((x_index+merge_margin)>m_Img_Size_X)?m_Img_Size_X:x_index+merge_margin;
			y_p = ((y_index+merge_margin)>m_Img_Size_Y)?m_Img_Size_Y:y_index+merge_margin;

			same_flag = FALSE;
			for(l=y_m;l<y_p;l++) {
				for(m=x_m;m<x_p;m++) {
					if(merged_data_flag[l][m][CurrencyIndex] == TRUE) {									// 다른 데이터가 있다면 겹치는 데이터인지 비교한다.
						for(j=0;j<additional_array_num;j++) {											// reference 중에서
							if(merged_data[l][m][CurrencyIndex][j].valid==TRUE && same_flag==FALSE) {	// 유효한 데이터라면
								diff = (reference[CurrencyIndex][num_files][i].DesVector==merged_data[l][m][CurrencyIndex][j].descriptor)?1:0;
								if(diff==1) {
									merged_data[l][m][CurrencyIndex][j].merged_count++;					// 1개가 더해졌으므로 merge_num에 +1
									same_flag = TRUE;													// 이 reference는 사용했으므로 flag 변화
								}
							}
						}
					}
				}
			}																							// (근처에 같은게 있는 지만 검사한거다!!!)
			if(same_flag == FALSE) {																	// 다른 데이터와 겹치는 데이터가 아니라면 데이터를 첨가한다
				if(merged_data_flag[y_index][x_index][CurrencyIndex]==TRUE) {							// 현재 위치에 다른 key값이 있는데
					for(n=0;n<additional_array_num;n++) {
						if(merged_data[y_index][x_index][CurrencyIndex][n].valid == TRUE) {				// 이미 점유하고 있다면
							continue;
						} else {																		// 점유하지 않은 나머지가 있다면
							merged_data[y_index][x_index][CurrencyIndex][n].valid = TRUE;				// 점유하고 있음을 표시
							merged_data[y_index][x_index][CurrencyIndex][n].merged_count++;				// merged_count를 증가
							merged_data[y_index][x_index][CurrencyIndex][n].descriptor = reference[CurrencyIndex][num_files][i].DesVector;	// key를 넣는다
							same_flag = TRUE;															// 변화되었다는 flag를 체크
							break;
						}
					}
				} else {																				// 현재 위치에 key값이 없으면
					merged_data[y_index][x_index][CurrencyIndex][0].valid = TRUE;						// 점유하고 있음을 표시
					merged_data[y_index][x_index][CurrencyIndex][0].merged_count++;						// merged_count를 증가
					merged_data[y_index][x_index][CurrencyIndex][0].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
					merged_data_flag[y_index][x_index][CurrencyIndex] = TRUE;
					same_flag = TRUE;
				}

				if(same_flag == FALSE) {																// 다른 데이터가 점유되지만 현재 데이터가 업뎃이 안됐으면
					x_m = ((x_index-1)<0)?0:x_index-1;
					y_m = ((y_index-1)<0)?0:y_index-1;
					x_p = ((x_index+1)>m_Img_Size_X-1)?m_Img_Size_X-1:x_index+1;
					y_p = ((y_index+1)>m_Img_Size_Y-1)?m_Img_Size_Y-1:y_index+1;

					if(merged_data_flag[y_index][x_m][CurrencyIndex]==TRUE && same_flag==FALSE) {		// 현재 위치에 다른 key값이 있는데
						for(n=0;n<additional_array_num;n++) {
							if(merged_data[y_index][x_m][CurrencyIndex][n].valid == TRUE) {				// 이미 점유하고 있다면
								continue;
							} else {																	// 점유하지 않은 나머지가 있다면
								merged_data[y_index][x_m][CurrencyIndex][n].valid = TRUE;				// 점유하고 있음을 표시
								merged_data[y_index][x_m][CurrencyIndex][n].merged_count++;				// merged_count를 증가
								merged_data[y_index][x_m][CurrencyIndex][n].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
								same_flag = TRUE;														// 변화되었다는 flag를 체크
								break;
							}
						}
					} else if(merged_data_flag[y_index][x_m][CurrencyIndex]==FALSE && same_flag==FALSE) {	// 현재 위치에 key값이 없으면
						merged_data[y_index][x_m][CurrencyIndex][0].valid = TRUE;						// 점유하고 있음을 표시
						merged_data[y_index][x_m][CurrencyIndex][0].merged_count++;						// merged_count를 증가
						merged_data[y_index][x_m][CurrencyIndex][0].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
						merged_data_flag[y_index][x_m][CurrencyIndex] = TRUE;
						same_flag = TRUE;
					}
					if(merged_data_flag[y_index][x_p][CurrencyIndex]==TRUE && same_flag==FALSE) {		// 현재 위치에 다른 key값이 있는데
						for(n=0;n<additional_array_num;n++) {
							if(merged_data[y_index][x_p][CurrencyIndex][n].valid == TRUE) {				// 이미 점유하고 있다면
								continue;
							} else {																	// 점유하지 않은 나머지가 있다면
								merged_data[y_index][x_p][CurrencyIndex][n].valid = TRUE;				// 점유하고 있음을 표시
								merged_data[y_index][x_p][CurrencyIndex][n].merged_count++;				// merged_count를 증가
								merged_data[y_index][x_p][CurrencyIndex][n].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
								same_flag = TRUE;														// 변화되었다는 flag를 체크
								break;
							}
						}
					} else if(merged_data_flag[y_index][x_p][CurrencyIndex]==FALSE && same_flag==FALSE) {	// 현재 위치에 key값이 없으면
						merged_data[y_index][x_p][CurrencyIndex][0].valid = TRUE;						// 점유하고 있음을 표시
						merged_data[y_index][x_p][CurrencyIndex][0].merged_count++;						// merged_count를 증가
						merged_data[y_index][x_p][CurrencyIndex][0].descriptor = reference[CurrencyIndex][num_files][i].DesVector;	// key를 넣는다
						merged_data_flag[y_index][x_p][CurrencyIndex] = TRUE;
						same_flag = TRUE;
					}
/*					if(merged_data_flag[y_m][x_index][CurrencyIndex]==TRUE && same_flag==FALSE) {		// 현재 위치에 다른 key값이 있는데
						for(n=0;n<additional_array_num;n++) {
							if(merged_data[y_m][x_index][CurrencyIndex][n].valid == TRUE) {				// 이미 점유하고 있다면
								continue;
							} else {																	// 점유하지 않은 나머지가 있다면
								merged_data[y_m][x_index][CurrencyIndex][n].valid = TRUE;				// 점유하고 있음을 표시
								merged_data[y_m][x_index][CurrencyIndex][n].merged_count++;				// merged_count를 증가
								merged_data[y_m][x_index][CurrencyIndex][n].descriptor = reference[CurrencyIndex][num_files][i].DesVector;	// key를 넣는다
								same_flag = TRUE;														// 변화되었다는 flag를 체크
								break;
							}
						}
					} else if(merged_data_flag[y_m][x_index][CurrencyIndex]==FALSE && same_flag==FALSE) {	// 현재 위치에 key값이 없으면
						merged_data[y_m][x_index][CurrencyIndex][0].valid = TRUE;						// 점유하고 있음을 표시
						merged_data[y_m][x_index][CurrencyIndex][0].merged_count++;						// merged_count를 증가
						merged_data[y_m][x_index][CurrencyIndex][0].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
						merged_data_flag[y_m][x_index][CurrencyIndex] = TRUE;
						same_flag = TRUE;
					}
					if(merged_data_flag[y_p][x_index][CurrencyIndex]==TRUE && same_flag==FALSE) {		// 현재 위치에 다른 key값이 있는데
						for(n=0;n<additional_array_num;n++) {
							if(merged_data[y_p][x_index][CurrencyIndex][n].valid == TRUE) {				// 이미 점유하고 있다면
								continue;
							} else {																	// 점유하지 않은 나머지가 있다면
								merged_data[y_p][x_index][CurrencyIndex][n].valid = TRUE;				// 점유하고 있음을 표시
								merged_data[y_p][x_index][CurrencyIndex][n].merged_count++;				// merged_count를 증가
								merged_data[y_p][x_index][CurrencyIndex][n].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
								same_flag = TRUE;														// 변화되었다는 flag를 체크
								break;
							}
						}
					} else if(merged_data_flag[y_p][x_index][CurrencyIndex]==FALSE && same_flag==FALSE) {	// 현재 위치에 key값이 없으면
						merged_data[y_p][x_index][CurrencyIndex][0].valid = TRUE;						// 점유하고 있음을 표시
						merged_data[y_p][x_index][CurrencyIndex][0].merged_count++;						// merged_count를 증가
						merged_data[y_p][x_index][CurrencyIndex][0].descriptor = reference[CurrencyIndex][num_files][i].DesVector;
						merged_data_flag[y_p][x_index][CurrencyIndex] = TRUE;
						same_flag = TRUE;
					}
*/				}
			}
		}
	}
}

void WriteWideToMulti(HANDLE h, CString& s)
{
	char *cbuf = (char*)malloc(s.GetLength()+1);
	WideCharToMultiByte(CP_ACP, 0, s, s.GetLength(), cbuf, s.GetLength(), NULL, NULL);
	DWORD dwio = 0;
	WriteFile(h, cbuf, s.GetLength(), &dwio, NULL);
	free(cbuf);
}

void CMakeRefDataDlg::MakeDataFile(int CurrencyNum, TCHAR* CurrencyCode)
{
	int i,j,k,max_n_count=0;

	CString m_Static_DataPath = m_tszRefFolder;
	m_Static_DataPath += L"\\";

	CString SaveFileStr;
	CString SaveCountStr;
	CString SaveCDenomStr;
	
	TCHAR tstr[MAX_PATH];
	wsprintf(tstr, L"#include \"B_Common.h\"\r\n#include \"A_%d_%s.h\"\r\n\r\n", CurrencyNum, CurrencyCode);
	SaveCountStr += tstr;
	wsprintf(tstr, L"#ifndef %s_MAX_DATA\r\n", CurrencyCode);
	SaveCountStr += tstr;
	wsprintf(tstr, L"#define\t%s_MAX_DATA\t%d\r\n", CurrencyCode, m_Max_Ref_Buf_Size);
	SaveCountStr += tstr;
	wsprintf(tstr, L"#endif\r\n");
	SaveCountStr += tstr;

	wsprintf(tstr, L"Uint8 %s_Ref_Cnt[%s_AREA_Y][%s_AREA_X] = {\r\n", CurrencyCode, CurrencyCode, CurrencyCode);
	SaveCountStr += tstr;

	wsprintf(tstr, L"short %s_Tdenom[%s_AREA_Y][%s_AREA_X][%s_MAX_DATA] = {\r\n", CurrencyCode, CurrencyCode, CurrencyCode, CurrencyCode);
	SaveCDenomStr += tstr;

	wsprintf(tstr, L"Uint8 %s_Ref_Key[%s_AREA_Y][%s_AREA_X][%s_MAX_DATA] = {\r\n", CurrencyCode, CurrencyCode, CurrencyCode, CurrencyCode);
	SaveFileStr += tstr;

	FILE *SaveFile = fopenW(m_Static_DataPath+"data.txt", "w+");
	FILE *SaveCount = fopenW(m_Static_DataPath+"count.txt", "w+");
	FILE *SaveCDenom = fopenW(m_Static_DataPath+"cdenom.txt", "w+");
	int n_count;

	for(i=2;i<m_Img_Size_Y-2;i++) {
		for(j=2;j<m_Img_Size_X-2;j++) {
			n_count = 0;
			for(k=0;k<m_Num_Currency;k++) {
				if(merged_data_flag[i][j][k] == TRUE) {
					for(int m=0;m<additional_array_num;m++) {
						if(merged_data[i][j][k][m].valid == TRUE && merged_data[i][j][k][m].merged_count>=min_merged_sum) {
							fprintf(SaveFile, "%d,",merged_data[i][j][k][m].descriptor);
							wsprintf(tstr, L"%d,", merged_data[i][j][k][m].descriptor);
							SaveFileStr += tstr;

							fprintf(SaveCDenom,"%d,",k);
							wsprintf(tstr, L"%d,", k);
							SaveCDenomStr += tstr;

							n_count++;
						}
					}
				}
			}
			if(n_count<m_Max_Ref_Buf_Size) {
				for(k=n_count;k<m_Max_Ref_Buf_Size;k++) {
					fprintf(SaveFile,"0,");
					SaveFileStr += L"0,";

					fprintf(SaveCDenom,"-1,");
					SaveCDenomStr += L"-1,";
				}
			}
			fprintf(SaveFile,"\n");
			SaveFileStr += L"\r\n";

			fprintf(SaveCDenom,"\n");
			SaveCDenomStr += L"\r\n";

			fprintf(SaveCount,"%d,",n_count);
			wsprintf(tstr, L"%d,", n_count);
			SaveCountStr += tstr;
			if(max_n_count<n_count) {
				max_n_count = n_count;
			}
		}
		fprintf(SaveFile,"\n");
		SaveFileStr += L"\r\n";

		fprintf(SaveCount,"\n");
		SaveCountStr += L"\r\n";

		fprintf(SaveCDenom,"\n");
		SaveCDenomStr += L"\r\n";
	}
	fclose(SaveFile);
	fclose(SaveCount);
	fclose(SaveCDenom);

	TCHAR srcFilename[MAX_PATH];
	wsprintf(srcFilename, L"%s\\A_%d_%s_Data.c", m_tszRefFolder, CurrencyNum, CurrencyCode);
	HANDLE h = CreateFile(srcFilename, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		SaveCountStr += L"};\r\n\r\n"; WriteWideToMulti(h, SaveCountStr);
		SaveCDenomStr+= L"};\r\n\r\n"; WriteWideToMulti(h, SaveCDenomStr);
		SaveFileStr  += L"};\r\n";     WriteWideToMulti(h, SaveFileStr);
		CloseHandle(h);
	}

	AfxMessageBox(L"Making Data Completed!");
}

