// NNRView.cpp : implementation of the CNNRView class
//

#include "stdafx.h"
#include "NNR.h"

#include "NNRDoc.h"
#include "NNRView.h"
#include "RawFormatConvertDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNNRView

IMPLEMENT_DYNCREATE(CNNRView, CFormView)

BEGIN_MESSAGE_MAP(CNNRView, CFormView)
	//{{AFX_MSG_MAP(CNNRView)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SAMPLELOAD, OnButtonSampleload)
	ON_BN_CLICKED(IDC_BUTTON_WEIGHTHOAD, OnButtonWeighthoad)
	ON_BN_CLICKED(IDC_BUTTON_RUN, OnButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_WEIGHTSAVE, OnButtonWeightsave)
	ON_BN_CLICKED(IDC_BUTTON_CONVIQ, OnButtonConviq)
	ON_BN_CLICKED(IDC_BUTTON_PATH_SELECT, OnButtonPathSelect)
	ON_BN_CLICKED(IDC_BUTTON_MAKE_NTF, OnButtonMakeNtf)
	ON_BN_CLICKED(IDC_BUTTON_PATH_RESULT, OnButtonPathResult)
	ON_BN_CLICKED(IDC_RADIO_LOGISTIC, OnRadioLogistic)
	ON_BN_CLICKED(IDC_RADIO_HYPERBOLIC, OnRadioHyperbolic)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
	ON_EN_CHANGE(IDC_EDIT_AVERAGE_LIMIT, &CNNRView::OnEnChangeEditAverageLimit)

	ON_EN_CHANGE(IDC_EDIT_EXT, &CNNRView::OnEnChangeEditExt)
	ON_BN_CLICKED(IDC_BUTTON_REMAKE, &CNNRView::OnBnClickedButtonRemake)
	ON_BN_CLICKED(IDC_BUTTON_FIND, &CNNRView::OnBnClickedButtonFind)
END_MESSAGE_MAP()

TCHAR RegNNRFolder[MAX_PATH];
CString NNRFolder;
int		m_Edit_FirstHiddenNode;

double m_averageLimit;
double RegAverageLimit;

int m_RaxW;
int m_RaxH;

/////////////////////////////////////////////////////////////////////////////
// CNNRView construction/destruction

CNNRView::CNNRView()
	: CFormView(CNNRView::IDD)
	, m_foundGroups(0)
	, m_foundFiles(0)
	, m_elapsedSeconds(0)
	, m_EditInputNode(0)
{
	//{{AFX_DATA_INIT(CNNRView)
	m_Edit_EpochNum = 0;
	m_Edit_AverageError = 0.0;
	m_Combo_SelectLayer = _T("One Hidden Layer");
	m_Edit_FirstHiddenNode = 24;
	m_Edit_SecondHiddenNode = 0;
	m_Edit_InputNodeNum = 0;
	m_Edit_ConversionErrorRate = DCONVERSIONRATE;
	m_Edit_Momentum = LOGIST_ALPHA;
	m_Edit_ETA = LOGIST_ETA;
	m_Edit_GroupNum = 0;
	m_Edit_ClassNum = 0;

	m_Edit_Sel_GroupNum = 0;
	m_Edit_MatchMax = 1000;
	m_Edit_MatchMin = 0;
	m_Edit_MaximumError = 0.0;
	m_averageLimit = 1.0e-6;
	RegAverageLimit = 1.0e-6;
	//}}AFX_DATA_INIT
	// TODO: add construction code here

	m_pTrainData = NULL; //학습할 data를 load하는 point
	m_pNeuralOneHidden = NULL;
	m_pNeuralTwoHidden = NULL;
	m_SelectLayer = 0;   //layer에 따른 class호출
	m_WeightLoadFlag = 0;  //weight를 load하였을 경우.
	m_GroupNum = 0;       //몇 번째 sample를 보내는지 확인을 위해..

	m_InputMax = 0;         //정규화를 위한 input의 max range
	m_InputMin = 0;         //정규화를 위한 input의 min range
	m_ActiveFlag = 0;

	m_pLoad_FirstHiddenWeight = NULL;
	m_pLoad_SecondHiddenWeight = NULL;
	m_pLoad_OutputWeight = NULL;
	m_Load_Buffer = NULL;

	m_Ext = "raw";
	m_bPathSet = FALSE;

	RegNNRFolder[0] = 0;
	NNRFolder = "";

	m_RaxW = 14;
	m_RaxH = 11;

	RegGetNNRFolder();

	NNRFolder = RegNNRFolder;
}

CNNRView::~CNNRView()
{
	DeleteTrainData();
	if(m_pNeuralOneHidden != NULL)
	{
		if(m_pNeuralOneHidden->m_ConstructionFlag == TRUE) m_pNeuralOneHidden->DestructionNetwork();
		delete m_pNeuralOneHidden;
		m_pNeuralOneHidden = NULL;
	}

	if(m_pNeuralTwoHidden != NULL)
	{
		if(m_pNeuralTwoHidden->m_ConstructionFlag == TRUE) m_pNeuralTwoHidden->DestructionNetwork();
		delete m_pNeuralTwoHidden;
		m_pNeuralTwoHidden = NULL;
	}

}

void CNNRView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNNRView)
	DDX_Control(pDX, IDC_LIST_RESULT, m_List_Result);
	DDX_Text(pDX, IDC_EDIT_EPOCHNUM, m_Edit_EpochNum);
	DDX_Text(pDX, IDC_EDIT_ERRORRATE, m_Edit_AverageError);
	DDX_CBString(pDX, IDC_COMBO_SELECTLAYER, m_Combo_SelectLayer);
	DDX_Text(pDX, IDC_EDIT_FIRSTHIDDENNODE, m_Edit_FirstHiddenNode);
	DDX_Text(pDX, IDC_EDIT_SECONDHIDDENNODE, m_Edit_SecondHiddenNode);
	DDX_Text(pDX, IDC_EDIT_INPUTNODENUM, m_Edit_InputNodeNum);
	DDX_Text(pDX, IDC_EDIT_INPUT_NODE, m_EditInputNode);
	DDX_Text(pDX, IDC_EDIT_CONVERSIONRATE, m_Edit_ConversionErrorRate);
	DDX_Text(pDX, IDC_EDIT_MOMENTUM, m_Edit_Momentum);
	DDX_Text(pDX, IDC_EDIT_ETA, m_Edit_ETA);
	DDX_Text(pDX, IDC_EDIT_CLASSGROUPNUM, m_Edit_GroupNum);
	DDX_Text(pDX, IDC_EDIT_CLASSNUM, m_Edit_ClassNum);
	DDX_Text(pDX, IDC_STATIC_PATH, NNRFolder);
	DDX_Text(pDX, IDC_EDIT_MATCH_MAX, m_Edit_MatchMax);
	DDX_Text(pDX, IDC_EDIT_MATCH_MIN, m_Edit_MatchMin);
	DDX_Text(pDX, IDC_EDIT_MAXIMUM_ERROR, m_Edit_MaximumError);
	DDX_Text(pDX, IDC_EDIT_FOUND_GROUP, m_foundGroups);
	DDX_Text(pDX, IDC_EDIT_FOUND_FILES, m_foundFiles);
	DDX_Text(pDX, IDC_EDIT_AVERAGE_LIMIT, m_averageLimit);
	DDX_Text(pDX, IDC_EDIT_ELAPSED_SECONDS, m_elapsedSeconds);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_EDIT_EXT, m_EditExt);
}

BOOL CNNRView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CNNRView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	
	m_List_Result.InsertColumn(0,"Path");
	m_List_Result.InsertColumn(1,"Result");
	m_List_Result.InsertColumn(2,"Rate");
	m_List_Result.InsertColumn(3,"Warning");

	m_List_Result.SetColumnWidth(1,20);
	m_List_Result.SetColumnWidth(2,20);
	m_List_Result.SetColumnWidth(3,20);
	RECT r;
	m_List_Result.GetClientRect(&r);
	m_List_Result.SetColumnWidth(0, (r.right-r.left)-60);

	CButton * pOption = (CButton*)GetDlgItem(IDC_RADIO_LOGISTIC);// logistic로 초기화 하자.
	pOption->SetCheck(1);
	m_SelectActivation = SEL_LOGISTIC;

	m_EditExt.SetWindowText(m_Ext);

	char szRestriction[256];
	sprintf_s(szRestriction, "Restriction : Group 10000, Class : 1000, Node : %d", MAX_INPUT_NODE);
	SetDlgItemText(IDC_EDIT_RESTRICTION, szRestriction);
}

/////////////////////////////////////////////////////////////////////////////
// CNNRView printing

BOOL CNNRView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CNNRView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CNNRView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CNNRView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CNNRView diagnostics

#ifdef _DEBUG
void CNNRView::AssertValid() const
{
	CFormView::AssertValid();
}

void CNNRView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CNNRDoc* CNNRView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNNRDoc)));
	return (CNNRDoc*)m_pDocument;
}
#endif //_DEBUG

int CalculateElapsedSeconds(SYSTEMTIME& st)
{
	SYSTEMTIME et;
	GetSystemTime(&et);

	FILETIME sf, ef;
	SystemTimeToFileTime(&st, &sf);
	SystemTimeToFileTime(&et, &ef);

	ULARGE_INTEGER su, eu;
	su.HighPart = sf.dwHighDateTime; su.LowPart = sf.dwLowDateTime;
	eu.HighPart = ef.dwHighDateTime; eu.LowPart = ef.dwLowDateTime;

	return (int)((eu.QuadPart-su.QuadPart)/10000000);
}

CString GroupPath[MAX_CLASS][MAX_GROUP]; // Maximum 50 Classes and 1000 Groups(Samples)
CString ClassName[MAX_CLASS];

/////////////////////////////////////////////////////////////////////////////
// CNNRView message handlers
void CNNRView::OnTimer(UINT nIDEvent) 
{
	CString DisplayError;

	switch(nIDEvent){
	case SELECTONEHIDDEN:
#if 1
		for(m_GroupNum = 0; m_GroupNum < m_Edit_GroupNum; m_GroupNum++) {
			m_pNeuralOneHidden->LoadSampleData(m_pTrainData[m_GroupNum]);
			m_Edit_AverageError += m_pNeuralOneHidden->WeightTranningFunc();

			if(m_Edit_MaximumError < m_pNeuralOneHidden->m_MaxError){
				m_Edit_MaximumError = m_pNeuralOneHidden->m_MaxError;
				m_MaxErrorClass = m_pNeuralOneHidden->m_MaxErrorPos;
				m_MaxErrorGroup = m_GroupNum;
			}

			m_Edit_EpochNum++;
		}

		if(m_GroupNum == m_Edit_GroupNum)
		{
			m_Edit_AverageError /= m_GroupNum;
		
			char msgBuf[128];
			sprintf_s(msgBuf, 128, "MaxEr  G: %d, C: %d %s", m_MaxErrorGroup, m_MaxErrorClass, GroupPath[m_MaxErrorClass][m_MaxErrorGroup]);
			m_List_Result.InsertItem(0, msgBuf);

			sprintf_s(msgBuf, "%d", int(m_Edit_MaximumError*100));
			m_List_Result.SetItem(0, 2, LVIF_TEXT, msgBuf, 0, 0, 0, 0);

			m_elapsedSeconds = CalculateElapsedSeconds(m_startTime);
			UpdateData(false);

			m_Edit_MaximumError = 0;
			m_pNeuralOneHidden->m_MaxError = 0;

			if(m_Edit_AverageError < m_Edit_ConversionErrorRate)
			{
				KillTimer(SELECTONEHIDDEN);
				OneHiddenWeightSave();
				AfxMessageBox("Running Complete!!");
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
				break;
			}

			if(m_Edit_AverageError < m_averageLimit)
			{
				KillTimer(SELECTONEHIDDEN);
				CString FileName;
				SYSTEMTIME st;
				GetLocalTime(&st);
				FileName.Format("%04d%02d%02d%02d%02d_%e.iq", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, m_Edit_AverageError);
				SaveIqFile(FileName);
				AfxMessageBox("Limit Exceeded!!");
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
				break;
			}
			m_GroupNum = 0;
			m_Edit_AverageError = 0;

			if(m_ActiveFlag == FALSE){
				KillTimer(SELECTONEHIDDEN);
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
			}
		}
#else
		m_pNeuralOneHidden->LoadSampleData(m_pTrainData[m_GroupNum]);
		m_Edit_AverageError += m_pNeuralOneHidden->WeightTranningFunc();

		if(m_Edit_MaximumError < m_pNeuralOneHidden->m_MaxError){
			m_Edit_MaximumError = m_pNeuralOneHidden->m_MaxError;
			m_MaxErrorClass = m_pNeuralOneHidden->m_MaxErrorPos;
			m_MaxErrorGroup = m_GroupNum;
		}

		m_Edit_EpochNum++;
		m_GroupNum++;

		if(m_GroupNum == m_Edit_GroupNum)
		{
			m_Edit_AverageError /= m_GroupNum;
		
			char msgBuf[128];
			sprintf_s(msgBuf, 128, "MaxEr  G: %d, C: %d %s", m_MaxErrorGroup + 1, m_MaxErrorClass + 1, GroupPath[m_MaxErrorClass+1][m_MaxErrorGroup+1]);
			m_List_Result.InsertItem(0, msgBuf);

			sprintf_s(msgBuf, "%d", int(m_Edit_MaximumError*100));
			m_List_Result.SetItem(0, 2, LVIF_TEXT, msgBuf, 0, 0, 0, 0);

			m_elapsedSeconds = CalculateElapsedSeconds(m_startTime);
			UpdateData(false);

			m_Edit_MaximumError = 0;
			m_pNeuralOneHidden->m_MaxError = 0;

			if(m_Edit_AverageError < m_Edit_ConversionErrorRate)
			{
				KillTimer(SELECTONEHIDDEN);
				OneHiddenWeightSave();
				AfxMessageBox("Running Complete!!");
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
				break;
			}

			if(m_Edit_AverageError < m_averageLimit)
			{
				KillTimer(SELECTONEHIDDEN);
				CString FileName;
				FileName.Format("%e.iq", m_Edit_AverageError);
				SaveIqFile(FileName);
				AfxMessageBox("Limit Exceeded!!");
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
				break;
			}
			m_GroupNum = 0;
			m_Edit_AverageError = 0;

			if(m_ActiveFlag == FALSE){
				KillTimer(SELECTONEHIDDEN);
				GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
			}
		}
#endif
		break;
	case SELECTTWOHIDDEN:
		{
			m_pNeuralTwoHidden->LoadSampleGroupData(m_pTrainData[m_GroupNum]);
			m_Edit_AverageError += m_pNeuralTwoHidden->WeightTranningFunc();
			m_Edit_EpochNum++;
			m_GroupNum++;

			if(m_GroupNum == m_Edit_GroupNum)
			{
				m_Edit_AverageError /= m_GroupNum;
				m_elapsedSeconds = CalculateElapsedSeconds(m_startTime);
				UpdateData(false);

				if(m_Edit_AverageError < m_Edit_ConversionErrorRate)
				{
					KillTimer(SELECTTWOHIDDEN);
					AfxMessageBox("Running Complete!!");
					TwoHiddenWeightSave();
				}
				if(m_Edit_AverageError < m_averageLimit)
				{
					KillTimer(SELECTONEHIDDEN);
					AfxMessageBox("Limit Exceeded!!");
					CString FileName;
					SYSTEMTIME st;
					GetLocalTime(&st);
					FileName.Format("%04d$02d%02d%02d%02d_%e.iq", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, m_Edit_AverageError);
					SaveIqFile(FileName);
				}
				m_GroupNum = 0;
				m_Edit_AverageError = 0;

			
				if(m_ActiveFlag == FALSE){
					KillTimer(SELECTTWOHIDDEN);
					GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
				}
			}
		}
		break;
	}

	CFormView::OnTimer(nIDEvent);
}

void CNNRView::OnButtonSampleload() 
{
	if(m_ActiveFlag == TRUE) { AfxMessageBox("동작중입니다. 더이상 사용할수 없습니다."); return;}
	LoadTrainData();
}

void CNNRView::OnButtonRun() 
{
	if(m_ActiveFlag == TRUE) {
		AfxMessageBox("동작중입니다. 더이상 사용할수 없습니다.");
		return;}

	UpdateData(true);
	RegSetNNRFolder();
	
	if(m_Combo_SelectLayer == "") 
	{
		AfxMessageBox("우선 layer를 선택 해주세요.");
		return;
	}

	if(m_Combo_SelectLayer == "Single Layer") { AfxMessageBox("아직 미구현중입니다. 다른 layer를 선택하세요."); return; }//m_SelectLayer = SELECTSINGLE;
	else if(m_Combo_SelectLayer == "One Hidden Layer") m_SelectLayer = SELECTONEHIDDEN;
	else if(m_Combo_SelectLayer == "Two Hidden Layer") m_SelectLayer = SELECTTWOHIDDEN;

	InitNNR();

////////////////////////////////////////////////////////
/*	net = AfxBeginThread(StartNetThread, this,
		THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	net->ResumeThread();
		
	m_bStop = false;

	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);
	
	DWORD retval;
	while(::GetExitCodeThread(net->m_hThread, &retval) &&
		  retval == STILL_ACTIVE) {
		if (!AfxGetThread()->PumpMessage()) m_bStop = true;
		if (m_bStop) ::TerminateThread(net->m_hThread, 0);
	}

	delete net;

	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);
*/
////////////////////////////////////////////////////////
}

void CNNRView::OnButtonStop() 
{
//	m_bStop = true;
	m_ActiveFlag = FALSE;
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

}

void CNNRView::InitNNR()
{
	GetSystemTime(&m_startTime);

	switch(m_SelectLayer)
	{
	case SELECTSINGLE:
		{
		}
		break;
	case SELECTONEHIDDEN:
		{
			if(m_Edit_ConversionErrorRate <= 0 || m_Edit_ConversionErrorRate > 0.5) { AfxMessageBox("Error범위가 적절하지 않습니다."); return;}
			if(m_Edit_Momentum <= 0 || m_Edit_Momentum > 1) { AfxMessageBox("Momentum범위가 적절하지 않습니다."); return;}
			if(m_Edit_ETA <= 0 || m_Edit_ETA > 1) { AfxMessageBox("ETA범위가 적절하지 않습니다."); return;}
			if(m_WeightLoadFlag == FALSE)
			{
				if(m_Edit_FirstHiddenNode == 0) {
					AfxMessageBox(" HiddenNode의 갯수를 정하십시요");
					return;
				}
				InitOneHiddenLayer();
				m_pNeuralOneHidden->InitWeightFunc();
				m_WeightLoadFlag = TRUE;
			}
			SetTimer(SELECTONEHIDDEN, 1, NULL);
			m_Edit_EpochNum = 0;
			m_GroupNum = 0;
		}
		break;
	case SELECTTWOHIDDEN:
		{
			if(m_Edit_ConversionErrorRate <= 0 || m_Edit_ConversionErrorRate > 0.5) { AfxMessageBox("Error범위가 적절하지 않습니다."); return;}
			if(m_Edit_Momentum <= 0 || m_Edit_Momentum > 1) { AfxMessageBox("Momentum범위가 적절하지 않습니다."); return;}
			if(m_Edit_ETA <= 0 || m_Edit_ETA > 1) AfxMessageBox("ETA범위가 적절하지 않습니다.");
			if( m_WeightLoadFlag == FALSE)
			{
				if(m_Edit_FirstHiddenNode == NULL) {
					AfxMessageBox(" 첫번째 HiddenNode의 갯수를 정하십시요");
					return;
				}
				if(m_Edit_SecondHiddenNode == NULL) { AfxMessageBox(" 두번째 HiddenNode의 갯수를 정하십시요"); return;}
				InitTwoHiddenLayer();
				m_pNeuralTwoHidden->InitWeightFunc();
				m_WeightLoadFlag = TRUE;
			}
			SetTimer(SELECTTWOHIDDEN, 1, NULL);
			m_Edit_EpochNum = 0;
			m_GroupNum = 0;
		}
		break;
	}
	m_ActiveFlag = TRUE;
	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow(false);

}


void CNNRView::InitOneHiddenLayer()
{
	m_pNeuralOneHidden = new CNeuralOneHidden;
	m_pNeuralOneHidden->ConstructionNetwork(m_Edit_InputNodeNum, m_Edit_FirstHiddenNode, m_Edit_ClassNum, m_Edit_ClassNum, 20);

	m_pNeuralOneHidden->m_SelectActivation = m_SelectActivation;
	m_pNeuralOneHidden->m_ALPHA = m_Edit_Momentum;
	m_pNeuralOneHidden->m_ETA = m_Edit_ETA;

	m_pNeuralOneHidden->InitRunState();

}

void CNNRView::InitTwoHiddenLayer()
{
	m_pNeuralTwoHidden = new CNeuralTwoHidden;
	m_pNeuralTwoHidden->ConstructionNetwork(m_Edit_InputNodeNum, m_Edit_FirstHiddenNode, m_Edit_SecondHiddenNode, m_Edit_ClassNum, m_Edit_ClassNum, 20);
	m_pNeuralTwoHidden->InitRunState();
	m_pNeuralTwoHidden->m_ALPHA = m_Edit_Momentum;
	m_pNeuralTwoHidden->m_ETA = m_Edit_ETA;
}

CString SampleFileName ="";
CString SampleFileExt = "";

UINT CNNRView::FileDataLoad(CString FileFormat)
{
	CFile file;

	SampleFileName ="";

	CString str = FileFormat + " 파일(" + FileFormat + ")|" + FileFormat+ "| 모든파일(*.*)|*.*|";
	CFileDialog Dlg(TRUE, FileFormat, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
	if(Dlg.DoModal() == IDOK)
	{
		SampleFileName =  Dlg.GetFileName();
		SampleFileExt = Dlg.GetFileExt();
		NNRFolder = Dlg.GetPathName();
		NNRFolder.Replace(Dlg.GetFileName(), "");

		UpdateData(false);

	}
	else return 0;
	
	if(!file.Open(SampleFileName, CFile::modeRead))
	{
		AfxMessageBox("해당파일을 찾을수 없어여~");
		return 0;
	}

	UINT nCount_Tile = file.GetLength();

	m_Load_Buffer = new unsigned char[nCount_Tile];

	file.Read(m_Load_Buffer, nCount_Tile);
	file.Close();

	return nCount_Tile;
}

void CNNRView::LoadTrainData()
{
	int i = 0, j = 0, k = 0;
	double DataValue = 0;
//	UINT Char_Tile = 0;
	CString FileFormat = "*.ntf;*.ntfx";
	CString data_Tile = "";
	UINT nCount_Tile = FileDataLoad(FileFormat);
	if(nCount_Tile == 0)
		return;
	if(SampleFileExt.CompareNoCase("NTFX") == 0) {
		TCHAR buf[MAX_PATH];
		memcpy_s(buf, sizeof(buf), m_Load_Buffer, sizeof(buf));
		NNRFolder = buf;

		for(int i = 0; i < nCount_Tile-sizeof(buf); i++)
			m_Load_Buffer[i] = m_Load_Buffer[i+sizeof(buf)];
		nCount_Tile -= sizeof(buf);
	}

	DeleteTrainData();
/*------------------------------------------------------------------------------------------*/

	m_Edit_ClassNum   = (m_Load_Buffer[nCount_Tile-6] << 8) & 0xff00; 
	m_Edit_ClassNum  |=  m_Load_Buffer[nCount_Tile-5]       & 0x00ff; 
	
	m_Edit_GroupNum  = (m_Load_Buffer[nCount_Tile-4] << 8) & 0xff00; 
	m_Edit_GroupNum |=  m_Load_Buffer[nCount_Tile-3]       & 0x00ff; 

	m_Edit_InputNodeNum  = (m_Load_Buffer[nCount_Tile-2] << 8) & 0xff00; 
	m_Edit_InputNodeNum |=  m_Load_Buffer[nCount_Tile-1]       & 0x00ff; 

	if( m_Edit_GroupNum > MAX_GROUP ) {
		AfxMessageBox("너무 많은 Group을 load하였습니다"); return;}
	if( m_Edit_ClassNum > MAX_CLASS ) {
		AfxMessageBox("너무 많은 class를 구분하였습니다"); return;}
	if( m_Edit_InputNodeNum > MAX_INPUT_NODE ) {
		AfxMessageBox("너무 많은 InputNode을 입력하였습니다"); return;}

	m_pTrainData = new double **[m_Edit_GroupNum];
	for(i = 0; i < m_Edit_GroupNum; i++)
	{
		m_pTrainData[i] = new double *[m_Edit_ClassNum];
		for(j = 0; j < m_Edit_ClassNum; j++) m_pTrainData[i][j] = new double [m_Edit_InputNodeNum];
	}

	int CntFile = 0;
	int CntGroup = 0;
	int CntSample = 0;
	int CntSampleSize = 0;

	for(i = 0; i < m_Edit_ClassNum; i++){
		CntSample  = (m_Load_Buffer[CntFile++] << 8) & 0xff00; 
		CntSample |=  m_Load_Buffer[CntFile++]       & 0x00ff; 
		
		if(i + 1 != CntSample){
			DeleteTrainData();
			AfxMessageBox("Not Matched Sample Count!");
			return;
		}
		
		for(j = 0 ; j < m_Edit_GroupNum; j++){

			CntGroup  = (m_Load_Buffer[CntFile++] << 8) & 0xff00; 
			CntGroup |=  m_Load_Buffer[CntFile++]       & 0x00ff; 

			if(j + 1 != CntGroup){
				DeleteTrainData();
				AfxMessageBox("Not Matched Group Count!");
				return;
			}

			CntSampleSize  = (m_Load_Buffer[CntFile++] << 8) & 0xff00; 
			CntSampleSize |=  m_Load_Buffer[CntFile++]       & 0x00ff; 

			if(m_Edit_InputNodeNum != CntSampleSize){
				DeleteTrainData();
				AfxMessageBox("Not Matched SampleSize Count!");
				return;
			}

			for(k = 0; k < m_Edit_InputNodeNum; k++){
				m_pTrainData[j][i][k] = m_Load_Buffer[CntFile++];
			}

			if(SampleFileExt.CompareNoCase("NTFX") == 0) {
				TCHAR buf[MAX_PATH];
				memcpy_s(buf, sizeof(buf), m_Load_Buffer+CntFile, sizeof(buf));
				CntFile += sizeof(buf);
				GroupPath[i][j] = buf;
			}
		}
	}

	if(SampleFileExt.CompareNoCase("NTFX") == 0) {
		for(i = 0; i < m_Edit_ClassNum; i++){
			TCHAR buf[64];
			memcpy_s(buf, sizeof(buf), m_Load_Buffer+CntFile, sizeof(buf));
			CntFile += sizeof(buf);
			ClassName[i] = buf;
		}
	}

	AfxMessageBox("Data Load Complete!!"); 
	
	for(i = 0; i < m_Edit_ClassNum; i++){
		for(j = 0; j < m_Edit_GroupNum; j++){
			for(k = 0; k < m_Edit_InputNodeNum; k++){
				m_pTrainData[j][i][k] = (m_pTrainData[j][i][k] - 128)/128;
//				m_pTrainData[j][i][k] = m_pTrainData[j][i][k]/255;
				if(m_pTrainData[j][i][k] > 1 || m_pTrainData[j][i][k] < -1){
					AfxMessageBox("Input Data의 정규화 error입니다."); delete []m_Load_Buffer; DeleteTrainData(); return; 
				}
			}
		}
	}

	UpdateData(false);

	delete []m_Load_Buffer;	
}

void CNNRView::DeleteTrainData()
{
	int i = 0, j = 0;

	if(m_pTrainData != NULL)
	{
		for(i = 0; i < m_Edit_GroupNum; i++)
		{
			for(j = 0; j < m_Edit_ClassNum; j++) delete[] m_pTrainData[i][j];
			delete[] m_pTrainData[i];
		}
		delete m_pTrainData;
		m_pTrainData = NULL;
	}
}

void CNNRView::OnButtonWeighthoad() 
{
	if(m_ActiveFlag == TRUE) {
		AfxMessageBox("동작중입니다. 더이상 사용할수 없습니다.");
		return;
	}

	m_Edit_FirstHiddenNode = 0;
	m_Edit_SecondHiddenNode = 0;

	m_Edit_Momentum = 0;
	m_Edit_ETA = 0;


	if(m_pNeuralOneHidden != NULL)
	{
		if(m_pNeuralOneHidden->m_ConstructionFlag == TRUE) m_pNeuralOneHidden->DestructionNetwork();
		delete m_pNeuralOneHidden;
		m_pNeuralOneHidden = NULL;
	}

	if(m_pNeuralTwoHidden != NULL)
	{
		if(m_pNeuralTwoHidden->m_ConstructionFlag == TRUE) m_pNeuralTwoHidden->DestructionNetwork();
		delete m_pNeuralTwoHidden;
		m_pNeuralTwoHidden = NULL;
	}

	WeightLoad();			
	m_WeightLoadFlag = TRUE;
}

void CNNRView::OnButtonWeightsave() 
{
	switch(m_SelectLayer)
	{
	case SELECTSINGLE:
		{
			SingleWeightSave();
		}
		break;
	case SELECTONEHIDDEN:
		{
			OneHiddenWeightSave();
		}
		break;
	case SELECTTWOHIDDEN:
		{
			TwoHiddenWeightSave();
		}
		break;
	}
	m_WeightLoadFlag = TRUE;//다시 재시작할때 학습된걸로 계속가기 위해..
	
}

void CNNRView::WeightLoad()
{
	UINT Char_Tile = 0;
	CString FileFormat = "*.wtf";
	CString data_Tile = "";
	UINT nCount_Tile = FileDataLoad(FileFormat);
	if(nCount_Tile == NULL) return;

	while(m_Load_Buffer[Char_Tile] != '\n')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); delete []m_Load_Buffer; return; }
	}
	m_Edit_ETA = atof(data_Tile);

	Char_Tile++;
	data_Tile = "";
	while(m_Load_Buffer[Char_Tile] != '\n')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); delete []m_Load_Buffer; return; }
	}
	m_Edit_Momentum = atof(data_Tile);

	Char_Tile++;
	data_Tile = "";
	while(m_Load_Buffer[Char_Tile] != '\n')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); delete []m_Load_Buffer; return; }
	}
	m_SelectActivation = atoi(data_Tile);

	CButton * pOption1 = (CButton*)GetDlgItem(IDC_RADIO_LOGISTIC);// logistic로 초기화 하자.
	CButton * pOption2 = (CButton*)GetDlgItem(IDC_RADIO_HYPERBOLIC);// logistic로 초기화 하자.
	if(m_SelectActivation == SEL_LOGISTIC){
		pOption1->SetCheck(1);
		pOption2->SetCheck(0);
	}else{
		pOption1->SetCheck(0);
		pOption2->SetCheck(1);
	}

	Char_Tile++;
	data_Tile = "";
	while(m_Load_Buffer[Char_Tile] != 'c')
	{
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); delete []m_Load_Buffer; return; }
	}

/*---------------------------------------------Select How maney Hidden Layer load---------------------------------------------------*/
	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); delete []m_Load_Buffer; return; }
	}

	if(data_Tile == "const double HiddenWeight"){Char_Tile++; LoadOneHiddenWeight(Char_Tile, nCount_Tile);}  //file format을 살펴본다..
	else if(data_Tile == "const double FirstHiddenWeight"){Char_Tile++; LoadTwoHiddenWeight(Char_Tile, nCount_Tile); }
	else if(data_Tile == "const double OutputWeight"){Char_Tile++; LoadSingleWeight(Char_Tile, nCount_Tile); }
	else{ AfxMessageBox("Wrong Format Data!!");	delete []m_Load_Buffer; return; }
}


void CNNRView::OneHiddenWeightSave()
{
	int Position = 0;
	int Number = 0;
	int i = 0, j = 0;
	FILE *stream_Tile;
	CString FileName ="";

	if(m_SelectActivation == SEL_LOGISTIC){        FileName = "Logistic_";}
	else if(m_SelectActivation == SEL_HYPERBOLIC){ FileName = "Hyperbolic_";}

	CString str = "WTF 파일(*.wtf)|*.wtf| 모든파일(*.*)|*.*|";
	CFileDialog Dlg(FALSE, "wtf", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
	if(Dlg.DoModal() == IDOK)
	{
		FileName += Dlg.GetFileName();
	}else return;

	fopen_s(&stream_Tile, FileName,"w+");
	if( stream_Tile == NULL )
	{
		AfxMessageBox("파일 열기 실패내~");
		return;
	}

	FileName = FileName.Left(FileName.GetLength()-4);
	
	fprintf(stream_Tile, "%f\n%f\n%d\n", m_Edit_ETA, m_Edit_Momentum, m_SelectActivation);//eta와 momentum을 초기에 저장한다.
									
/*------------------------------------------ First weight save----------------------------------------*/
	fprintf(stream_Tile, "const double HiddenWeight[%d][%d + 1] = {", m_Edit_FirstHiddenNode, m_Edit_InputNodeNum);

	for(i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_InputNodeNum + 1 ; j++)
		{
			fprintf(stream_Tile, " %f", m_pNeuralOneHidden->m_pHiddenWeight[i][j]);
			if(i == m_Edit_FirstHiddenNode -1 && j == m_Edit_InputNodeNum );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ First weight save----------------------------------------*/

/*------------------------------------------ Out weight save----------------------------------------*/
	fprintf(stream_Tile, "const double OutputWeight[%d][%d + 1] = {", m_Edit_ClassNum, m_Edit_FirstHiddenNode);

	for(i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1 ; j++)
		{
			fprintf(stream_Tile, " %f", m_pNeuralOneHidden->m_pOutputWeight[i][j]);
			if(i == m_Edit_ClassNum - 1 && j == m_Edit_FirstHiddenNode );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ Out weight save----------------------------------------*/

	fclose(stream_Tile);

}

void CNNRView::TwoHiddenWeightSave()
{
	int Position = 0;
	int Number = 0;
	int i = 0, j = 0;
	FILE *stream_Tile;
	CString FileName ="";

	CString str = "WTF 파일(*.wtf)|*.wtf| 모든파일(*.*)|*.*|";
	CFileDialog Dlg(FALSE, "wtf", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
	if(Dlg.DoModal() == IDOK)
	{
		FileName =  Dlg.GetFileName();
	}else return;

	fopen_s(&stream_Tile, FileName,"w+");
	if( stream_Tile == NULL )
	{
		AfxMessageBox("파일 열기 실패내~");
		return;
	}

	FileName = FileName.Left(FileName.GetLength()-4);

	fprintf(stream_Tile, "%f\n%f\n", m_Edit_ETA, m_Edit_Momentum);//eta와 momentum을 초기에 저장한다.
	
/*------------------------------------------ First weight save----------------------------------------*/
	fprintf(stream_Tile, "const double FirstHiddenWeight[%d][%d + 1] = {", m_Edit_FirstHiddenNode, m_Edit_InputNodeNum);

	for(i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_InputNodeNum + 1 ; j++)
		{
			fprintf(stream_Tile, " %f", m_pNeuralTwoHidden->m_pFirstHiddenWeight[i][j]);
			if(i == m_Edit_FirstHiddenNode -1 && j == m_Edit_ClassNum );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ First weight save----------------------------------------*/

/*------------------------------------------ Second weight save----------------------------------------*/
	fprintf(stream_Tile, "const double SecondHiddenWeight[%d][%d + 1] = {", m_Edit_SecondHiddenNode, m_Edit_FirstHiddenNode);

	for(i = 0; i < m_Edit_SecondHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1 ; j++)
		{
			fprintf(stream_Tile, " %f", m_pNeuralTwoHidden->m_pSecondHiddenWeight[i][j]);
			if(i == m_Edit_SecondHiddenNode -1 && j == m_Edit_FirstHiddenNode );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ Second weight save----------------------------------------*/

/*------------------------------------------ Out weight save----------------------------------------*/
	fprintf(stream_Tile, "const double OutputWeight[%d][%d + 1] = {", m_Edit_ClassNum, m_Edit_SecondHiddenNode);

	for(i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_SecondHiddenNode + 1 ; j++)
		{
			fprintf(stream_Tile, " %f", m_pNeuralTwoHidden->m_pOutputWeight[i][j]);
			if(i == m_Edit_ClassNum - 1 && j == m_Edit_SecondHiddenNode );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ Out weight save----------------------------------------*/

	fclose(stream_Tile);

}

void CNNRView::SingleWeightSave()
{
/*
	int ***a = new int**[10];
    for( int i = 0; i < 10; ++i )
    {
        a[i] = new int*[10];

        for( int j = 0; j < 10; ++j )
        {
            a[i][j] = new int[10];
        }
    }

    a[1][1][1] = 1;



   for( i = 0; i < 10; ++i )
    {
        for( int j = 0; j < 10; ++j )
        {
            delete [] a[i][j];
        }

        delete [] a[i];
    }

    delete [] a;
    a = NULL;
*/
 
}

void CNNRView::LoadOneHiddenWeight(UINT Char_Tile, UINT nCount_Tile)
{
	int i = 0, j = 0;
	double DataValue = 0;
	CString data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //Hidden node수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	m_Edit_FirstHiddenNode = int(DataValue);
	if( m_Edit_FirstHiddenNode > 200 ) { AfxMessageBox("Load된 Hidden Node수가 범위를 초과하였습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //InputSample수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_InputNodeNum != DataValue ) { AfxMessageBox("Load된 sample의 Node수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != '{')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";
///////////////////////////////////////////
	m_pLoad_FirstHiddenWeight = new double *[m_Edit_FirstHiddenNode];
	for(i = 0; i < m_Edit_FirstHiddenNode; i++) m_pLoad_FirstHiddenWeight[i] = new double [m_Edit_InputNodeNum + 1];


	for(i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j <m_Edit_InputNodeNum + 1; j++)
		{
			while(m_Load_Buffer[Char_Tile] != ',' && m_Load_Buffer[Char_Tile] != '}')
			{
				data_Tile += m_Load_Buffer[Char_Tile];
				Char_Tile++;
				if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
			}
			DataValue = atof( data_Tile );
			m_pLoad_FirstHiddenWeight[i][j] = DataValue;
			Char_Tile++;
			data_Tile = "";
			if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
		}
	}

	if(m_Load_Buffer[Char_Tile] != ';'){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
/*---------------------------------------------Hidden weight load---------------------------------------------------*/

/*---------------------------------------------Out weight load---------------------------------------------------*/
	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != 'c')
	{
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY;} 
	}

	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}

	if(data_Tile == "const double OutputWeight");  //file format을 살펴본다..
	else{ AfxMessageBox("Wrong Format Data!!"); goto DELETE_MEMORY; }
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //InputSample수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_ClassNum != DataValue ){ AfxMessageBox("Load된 sample의 Output수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //Hidden node수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_FirstHiddenNode != DataValue ){ AfxMessageBox("Load된 sample의 Output수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '{')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); return; goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";
//////////////////////////////////////
	m_pLoad_OutputWeight = new double *[m_Edit_ClassNum];
	for(i = 0; i < m_Edit_ClassNum; i++) m_pLoad_OutputWeight[i] = new double [m_Edit_FirstHiddenNode + 1];


	for(i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1; j++)
		{
			while(m_Load_Buffer[Char_Tile] != ',' && m_Load_Buffer[Char_Tile] != '}')
			{
				data_Tile +=m_Load_Buffer[Char_Tile];
				Char_Tile++;
				if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
			}

			DataValue = atof( data_Tile );
			m_pLoad_OutputWeight[i][j] = DataValue;
			Char_Tile++;
			data_Tile = "";
			if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
		}
	}

	if(m_Load_Buffer[Char_Tile] != ';'){ AfxMessageBox("Data Load Fail !!");  goto DELETE_MEMORY; }
	else AfxMessageBox("Weight Data Load Complete !!");
/*---------------------------------------------Out weight load---------------------------------------------------*/
	
	m_Combo_SelectLayer = "One Hidden Layer";
	m_SelectLayer = SELECTONEHIDDEN;

	UpdateData(false);

	InitOneHiddenLayer();
	for( i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_InputNodeNum + 1; j++)
		{
			m_pNeuralOneHidden->m_pHiddenWeight[i][j] = m_pLoad_FirstHiddenWeight[i][j];
		}
	}

	for( i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1; j++)
		{
			m_pNeuralOneHidden->m_pOutputWeight[i][j] = m_pLoad_OutputWeight[i][j];
		}
	}


DELETE_MEMORY:
	if(	m_pLoad_FirstHiddenWeight != NULL)
	{
		for(i = 0; i < m_Edit_FirstHiddenNode; i++) delete[] m_pLoad_FirstHiddenWeight[i];
		delete[] m_pLoad_FirstHiddenWeight;
		m_pLoad_FirstHiddenWeight = NULL;
	}

	if( m_pLoad_OutputWeight != NULL)
	{
		for(i = 0; i < m_Edit_ClassNum; i++) delete[] m_pLoad_OutputWeight[i];
		delete[] m_pLoad_OutputWeight;
		m_pLoad_OutputWeight = NULL;
	}


	delete []m_Load_Buffer;	

}

void CNNRView::LoadTwoHiddenWeight(UINT Char_Tile, UINT nCount_Tile)
{
	int i = 0, j = 0;
	double DataValue = 0;
	CString data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //Hidden node수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	m_Edit_FirstHiddenNode = int(DataValue);
	if( m_Edit_FirstHiddenNode > 200 ) { AfxMessageBox("Load된 Hidden Node수가 범위를 초과하였습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //InputSample수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_InputNodeNum != DataValue ) { AfxMessageBox("Load된 sample의 Node수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";




	while(m_Load_Buffer[Char_Tile] != '{')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";
///////////////////////////////////////////
	m_pLoad_FirstHiddenWeight = new double *[m_Edit_FirstHiddenNode];
	for(i = 0; i < m_Edit_FirstHiddenNode; i++) m_pLoad_FirstHiddenWeight[i] = new double [m_Edit_InputNodeNum + 1];


	for(i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j <m_Edit_InputNodeNum + 1; j++)
		{
			while(m_Load_Buffer[Char_Tile] != ',' && m_Load_Buffer[Char_Tile] != '}')
			{
				data_Tile += m_Load_Buffer[Char_Tile];
				Char_Tile++;
				if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
			}
			DataValue = atof( data_Tile );
			m_pLoad_FirstHiddenWeight[i][j] = DataValue;
			Char_Tile++;
			data_Tile = "";
			if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
		}
	}

	if(m_Load_Buffer[Char_Tile] != ';'){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
/*---------------------------------------------FirstHidden weight load---------------------------------------------------*/
/*---------------------------------------------SecondHidden weight load---------------------------------------------------*/
	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != 'c')
	{
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}

	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	if(data_Tile == "const double SecondHiddenWeight");  //file format을 살펴본다..
	else{ AfxMessageBox("Wrong Format Data!!"); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //Hidden node수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	m_Edit_SecondHiddenNode = int(DataValue);
	if( m_Edit_SecondHiddenNode > 200 ) { AfxMessageBox("Load된 Hidden Node수가 범위를 초과하였습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //InputSample수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_FirstHiddenNode != DataValue ) { AfxMessageBox("Load된 첫번째 HiddenNode수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '{')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile) { AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";
///////////////////////////////////////////
	m_pLoad_SecondHiddenWeight = new double *[m_Edit_SecondHiddenNode];
	for(i = 0; i < m_Edit_SecondHiddenNode; i++) m_pLoad_SecondHiddenWeight[i] = new double [m_Edit_FirstHiddenNode + 1];

	for(i = 0; i < m_Edit_SecondHiddenNode; i++)
	{
		for(j = 0; j <m_Edit_FirstHiddenNode + 1; j++)
		{
			while(m_Load_Buffer[Char_Tile] != ',' && m_Load_Buffer[Char_Tile] != '}')
			{
				data_Tile += m_Load_Buffer[Char_Tile];
				Char_Tile++;
				if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
			}
			DataValue = atof( data_Tile );
			m_pLoad_SecondHiddenWeight[i][j] = DataValue;
			Char_Tile++;
			data_Tile = "";
			if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
		}
	}

	if(m_Load_Buffer[Char_Tile] != ';'){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }

/*---------------------------------------------Out weight load---------------------------------------------------*/
	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != 'c')
	{
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}


	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}

	if(data_Tile == "const double OutputWeight");  //file format을 살펴본다..
	else{ AfxMessageBox("Wrong Format Data!!"); goto DELETE_MEMORY; }
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //InputSample수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_ClassNum != DataValue ){ AfxMessageBox("Load된 sample의 Output수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";

	while(m_Load_Buffer[Char_Tile] != '[')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != ']' && m_Load_Buffer[Char_Tile] != '+') //Hidden node수를 load한다.
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); goto DELETE_MEMORY; }
	}
	DataValue = atoi(data_Tile); 
	if( m_Edit_SecondHiddenNode != DataValue ){ AfxMessageBox("Load된 sample의 Output수와 일치하지 않습니다."); goto DELETE_MEMORY; }

	Char_Tile++;
	data_Tile = "";


	while(m_Load_Buffer[Char_Tile] != '{')
	{
		data_Tile += m_Load_Buffer[Char_Tile];
		Char_Tile++;
		if(Char_Tile > nCount_Tile){ AfxMessageBox("Read CheckSum error"); return; goto DELETE_MEMORY; }
	}
	Char_Tile++;
	data_Tile = "";
//////////////////////////////////////
	m_pLoad_OutputWeight = new double *[m_Edit_ClassNum];
	for(i = 0; i < m_Edit_ClassNum; i++) m_pLoad_OutputWeight[i] = new double [m_Edit_SecondHiddenNode + 1];


	for(i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_SecondHiddenNode + 1; j++)
		{
			while(m_Load_Buffer[Char_Tile] != ',' && m_Load_Buffer[Char_Tile] != '}')
			{
				data_Tile +=m_Load_Buffer[Char_Tile];
				Char_Tile++;
				if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
			}

			DataValue = atof( data_Tile );
			m_pLoad_OutputWeight[i][j] = DataValue;
			Char_Tile++;
			data_Tile = "";
			if(Char_Tile == nCount_Tile){ AfxMessageBox("Data Load Fail !!"); goto DELETE_MEMORY; }
		}
	}

	if(m_Load_Buffer[Char_Tile] != ';'){ AfxMessageBox("Data Load Fail !!");  goto DELETE_MEMORY; }
	else AfxMessageBox("Weight Data Load Complete !!");
/*---------------------------------------------Out weight load---------------------------------------------------*/
	
	m_Combo_SelectLayer = "Two Hidden Layer";
	m_SelectLayer = SELECTTWOHIDDEN;
	UpdateData(false);

	InitTwoHiddenLayer();
	for( i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_InputNodeNum + 1; j++)
		{
			m_pNeuralTwoHidden->m_pFirstHiddenWeight[i][j] = m_pLoad_FirstHiddenWeight[i][j];
		}
	}

	for( i = 0; i < m_Edit_SecondHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1; j++)
		{
			m_pNeuralTwoHidden->m_pSecondHiddenWeight[i][j] = m_pLoad_SecondHiddenWeight[i][j];
		}
	}

	for( i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_SecondHiddenNode + 1; j++)
		{
			m_pNeuralTwoHidden->m_pOutputWeight[i][j] = m_pLoad_OutputWeight[i][j];
		}
	}


DELETE_MEMORY:
	if(	m_pLoad_FirstHiddenWeight != NULL)
	{
		for(i = 0; i < m_Edit_FirstHiddenNode; i++) delete[] m_pLoad_FirstHiddenWeight[i];
		delete[] m_pLoad_FirstHiddenWeight;
		m_pLoad_FirstHiddenWeight = NULL;
	}


	if(	m_pLoad_SecondHiddenWeight != NULL)
	{
		for(i = 0; i < m_Edit_SecondHiddenNode; i++) delete[] m_pLoad_SecondHiddenWeight[i];
		delete[] m_pLoad_SecondHiddenWeight;
		m_pLoad_SecondHiddenWeight = NULL;
	}

	if( m_pLoad_OutputWeight != NULL)
	{
		for(i = 0; i < m_Edit_ClassNum; i++) delete[] m_pLoad_OutputWeight[i];
		delete[] m_pLoad_OutputWeight;
		m_pLoad_OutputWeight = NULL;
	}


	delete []m_Load_Buffer;	

}

void CNNRView::LoadSingleWeight(UINT Char_Tile, UINT nCount_Tile)
{

}

void CNNRView::SaveIqFile(CString& FileName)
{
	int Position = 0;
	int Number = 0;
	int i = 0, j = 0;
	FILE *stream_Tile = NULL;
	TCHAR head[MAX_PATH];

	TCHAR tpath[MAX_PATH];
	lstrcpy(tpath, (LPCTSTR)NNRFolder);
	i = lstrlen(tpath);
	// 우측 분리자 제거
	if(tpath[i-1] == '\\') { i--; tpath[i] = 0; }
	// 맨 처음 분리자 찾기
	while(i > 0 && tpath[i-1] != '\\') i--;
	// 복사 개시
	j = 0;
	while(tpath[i])
		head[j++] = tpath[i++];
	head[j] = 0;
	// head에 맨 마지막 폴더 명이 들어간다. C:\ABC\DEF 라면, DEF

	SetCurrentDirectory((LPCTSTR)NNRFolder);
	fopen_s(&stream_Tile, FileName,"w+");
	if( stream_Tile == NULL )
	{
		AfxMessageBox("파일 열기 실패");
		return;
	}

	FileName = FileName.Left(FileName.GetLength()-4);

	fprintf(stream_Tile, "#define %s_NNR_FHIDDEN_SIZE %d\n", head, m_Edit_FirstHiddenNode);
	fprintf(stream_Tile, "#define %s_NNR_INPUTNODE_SIZE %d\n", head, m_Edit_InputNodeNum);

/*------------------------------------------ First weight save----------------------------------------*/
	long IQData = 0;
	fprintf(stream_Tile, "const long %s_HiddenWeight[%s_NNR_FHIDDEN_SIZE * (%s_NNR_INPUTNODE_SIZE + 1)] = {", head, head, head);

	for(i = 0; i < m_Edit_FirstHiddenNode; i++)
	{
		for(j = 0; j < m_Edit_InputNodeNum + 1 ; j++)
		{
			IQData = long(1048576 * m_pNeuralOneHidden->m_pHiddenWeight[i][j]);
			fprintf(stream_Tile, " %d", IQData);

			if(i == m_Edit_FirstHiddenNode -1 && j == m_Edit_InputNodeNum );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ First weight save----------------------------------------*/

	fprintf(stream_Tile, "#define %s_NNR_CLASS_NUM %d\n\n", head, m_Edit_ClassNum);

/*------------------------------------------ Out weight save----------------------------------------*/
	fprintf(stream_Tile, "const long %s_OutputWeight[%s_NNR_CLASS_NUM * (%s_NNR_FHIDDEN_SIZE + 1)] = {", head, head, head);

	for(i = 0; i < m_Edit_ClassNum; i++)
	{
		for(j = 0; j < m_Edit_FirstHiddenNode + 1 ; j++)
		{
			IQData = long(1048576 * m_pNeuralOneHidden->m_pOutputWeight[i][j]);
			fprintf(stream_Tile, " %d", IQData);

			if(i == m_Edit_ClassNum - 1 && j == m_Edit_FirstHiddenNode );
			else fprintf(stream_Tile, ",");
		}
		fprintf(stream_Tile, "\n");
	}
	fprintf(stream_Tile, "};\n");
/*------------------------------------------ Out weight save----------------------------------------*/

/*-------------- save class name ---------------*/
	fprintf(stream_Tile, "char *%s_ClassName[%s_NNR_CLASS_NUM] = {\n", head, head);
	for(i = 0; i < m_Edit_ClassNum; i++) {
		fprintf(stream_Tile, "\t\"%s\",\n", ClassName[i]);
	}
	fprintf(stream_Tile, "};\n");

	fclose(stream_Tile);
}

void CNNRView::OnButtonConviq() 
{
	CString FileName ="";

	CString str = "iq 파일(*.iq)|*.iq| 모든파일(*.*)|*.*|";
	CFileDialog Dlg(FALSE, "iq", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
	if(Dlg.DoModal() == IDOK)
	{
		FileName =  Dlg.GetFileName();
	}else return;


	SaveIqFile(FileName);
}

int CNNRView::CountFiles(CString& pattern)
{
	int count = 0;
	CFileFind ff;
	if(ff.FindFile(pattern)) {
		int benext = TRUE;
		while(benext) {
			benext = ff.FindNextFile();
			if(ff.IsDots() || ff.IsDirectory())
				continue;
			m_EditInputNode = ff.GetLength();
			count++;
		}
	}
	return count;
}

// 초기 셋팅 시 '경로'를 무조건 나오게 할려면
int CALLBACK BICallBackFunc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
 switch(uMsg)
 {
 case BFFM_INITIALIZED:
  if(lpData)
   ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);      // lpData 가 경로
  break;
 }

 return 0;
}


BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname)
{
	ITEMIDLIST*  pildBrowse;
	BROWSEINFO  bInfo;

	memset(&bInfo, 0, sizeof(bInfo));

	bInfo.hwndOwner   = hWnd;
	bInfo.pidlRoot   = NULL;
	bInfo.pszDisplayName = pszPathname;
	bInfo.lpszTitle   = _T("디렉토리를 선택하세요");             // dialog title
	bInfo.ulFlags   = BIF_RETURNONLYFSDIRS|BIF_VALIDATE;                 // 여러가지 옵션이 존재합니다.. msdn을 참고해보세요.
	bInfo.lpfn    = BICallBackFunc;                        // 옵션? 입니다.. 초기 경로 설정등을 할때는 여기에 추가하세요.
	bInfo.lParam  = (LPARAM)(LPCTSTR)NNRFolder;

	pildBrowse    = ::SHBrowseForFolder(&bInfo);

	if(!pildBrowse)
		return FALSE;

	SHGetPathFromIDList(pildBrowse, pszPathname);
	return TRUE;
}

/*--------------------------------------------------------------------------------------------------*/
void CNNRView::OnButtonPathSelect() 
{
	UpdateData(true);

	TCHAR   pszPathname[MAX_PATH];
	if(BrowseFolder(GetSafeHwnd(), pszPathname)) {
		NNRFolder = pszPathname;                                // => 실제 선택한 폴더가 전체 경로로 나옵니다.
		RegSetNNRFolder();

		UpdateData(false);
	}
}

int LoadFileData(LPCTSTR fn, unsigned char* buf, int fsize)
{
	HANDLE h = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return 0;
	
	if(GetFileSize(h, NULL) != fsize) {
		CloseHandle(h);
		return 0;
	}
	
	DWORD dwio = 0;
	ReadFile(h, buf, fsize, &dwio, NULL);
	CloseHandle(h);
	
	return dwio;
}

void WriteCStringToFile(HANDLE h, CString& s, int size)
{
	TCHAR buf[MAX_PATH];
	memset(buf, 0, sizeof(buf));
	lstrcpy(buf, (LPCTSTR)s);
	DWORD dwio = 0;
	WriteFile(h, buf, size, &dwio, NULL);
}

void WriteNumber2Bytes(HANDLE h, unsigned int num)
{
	unsigned char buf[4];
	buf[0] = (num >> 8) & 0x00ff;
	buf[1] = num & 0x00ff;//file Num는 최대 65535까지 입력 가능하다.
	DWORD dwio = 0;
	WriteFile(h, buf, 2, &dwio, NULL);
}

unsigned char **DataBuf = NULL;
unsigned char *Data = NULL;

int WriteSubPathClassToSample(HANDLE hMakeFile, WIN32_FIND_DATA &ff, unsigned int ClassNum, unsigned int PrvGroupNum, int inputsize, LPCTSTR m_Ext)
{
	WIN32_FIND_DATA dirf;
	TCHAR SubPath_Class[MAX_PATH]; //denomination
	TCHAR FileName[MAX_PATH];

	unsigned int GroupNum  = 0;

	wsprintf(SubPath_Class, "%s\\%s", (LPCTSTR)NNRFolder, ff.cFileName);
	TCHAR findPath[MAX_PATH];
	wsprintf(findPath, "%s\\*.%s", SubPath_Class, m_Ext); // ."raw")

	HANDLE hdirf = FindFirstFile(findPath, &dirf);
	while(hdirf != INVALID_HANDLE_VALUE) {
		if(!(dirf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(GroupNum >= PrvGroupNum)
				break;

			wsprintf(FileName, "%s\\%s", SubPath_Class, dirf.cFileName);
			if(!LoadFileData(FileName, DataBuf[GroupNum], inputsize)) {
				MessageBox(NULL, FileName, "Cannot load ", MB_OK);
				break;
			}

			GroupNum++;
			GroupPath[ClassNum-1][GroupNum-1] = FileName;

			WriteNumber2Bytes(hMakeFile, GroupNum);
			WriteNumber2Bytes(hMakeFile, inputsize);

			DWORD dwio = 0;
			WriteFile(hMakeFile, DataBuf[GroupNum - 1], inputsize, &dwio, NULL);
			WriteCStringToFile(hMakeFile, GroupPath[ClassNum-1][GroupNum-1], sizeof(TCHAR)*MAX_PATH);
		}
		if(!FindNextFile(hdirf, &dirf))
			break;
	}
	if(hdirf != INVALID_HANDLE_VALUE)
		FindClose(hdirf);

	if(GroupNum <= 0) {
		MessageBox(NULL, SubPath_Class, "Empty Folder", MB_OK);
		return 0;
	}
				
	for(int i = GroupNum; i < PrvGroupNum; i++){
		WriteNumber2Bytes(hMakeFile, i+1);
		WriteNumber2Bytes(hMakeFile, inputsize);

		int test = (i - GroupNum)%GroupNum;
		DWORD dwio = 0;
		WriteFile(hMakeFile, DataBuf[test], inputsize, &dwio, NULL);
		GroupPath[ClassNum-1][i] = GroupPath[ClassNum-1][test];
		WriteCStringToFile(hMakeFile, GroupPath[ClassNum-1][i], sizeof(TCHAR)*MAX_PATH);
//								FileSize = MakeFile.GetLength();
	}

	return PrvGroupNum;
}

void CNNRView::OnButtonMakeNtf() 
{
	int bError = 0;
	unsigned int i = 0, j = 0, FileCnt = 0;

	HANDLE hMakeFile;
	WIN32_FIND_DATA ff;

	CString MakeFileName = "";   //make file

	unsigned int ClassNum = 0;

	unsigned char ClassNumBuf[3];
	unsigned char FileNumBuf[3];
	unsigned char FileSizeBuf[3];

	ClassNumBuf[2] = '\0';
	FileNumBuf[2]  = '\0';
	FileSizeBuf[2] = '\0';

	if(NNRFolder == ""){
		AfxMessageBox("Select Directory!!");
		return;
	}

	UpdateData(true);

	if(m_Edit_Sel_GroupNum <= 0){
		MessageBox("Group should be greater than 0");
		return;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	MakeFileName.Format("%s\\%04d%02d%02d%02d%02d_Feature.ntfx", (LPCTSTR)NNRFolder, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	hMakeFile = CreateFile(MakeFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwio = 0;
	WriteCStringToFile(hMakeFile, NNRFolder, sizeof(TCHAR)*MAX_PATH);

	int PrvGroupNum = m_Edit_Sel_GroupNum;
	DataBuf = new unsigned char *[PrvGroupNum];
	for(i = 0; i < PrvGroupNum; i++) DataBuf[i] = new unsigned char[m_EditInputNode];

	CString fpath = NNRFolder + _T("\\*.*");
	HANDLE hff = FindFirstFile(fpath, &ff);
	while(hff != INVALID_HANDLE_VALUE) {
		if((ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && lstrcmp(ff.cFileName, ".") && lstrcmp(ff.cFileName, "..")) {
			ClassName[ClassNum] = ff.cFileName;

			ClassNum++;
			WriteNumber2Bytes(hMakeFile, ClassNum);

			if(WriteSubPathClassToSample(hMakeFile, ff, ClassNum, PrvGroupNum, m_EditInputNode, m_Ext) != PrvGroupNum) {
				bError = 1;
				break;
			}
		}
		if(!FindNextFile(hff, &ff)) {
			break;
		}
	}

	if(hff != INVALID_HANDLE_VALUE)
		FindClose(hff);

	for(i = 0; i < PrvGroupNum; i++) delete [] DataBuf[i];
	delete [] DataBuf;

	for(i = 0; i < ClassNum; i++)
		WriteCStringToFile(hMakeFile, ClassName[i], 64*sizeof(TCHAR));

	WriteNumber2Bytes(hMakeFile, ClassNum);
	WriteNumber2Bytes(hMakeFile, PrvGroupNum);
	WriteNumber2Bytes(hMakeFile, m_EditInputNode);
	CloseHandle(hMakeFile);

	m_bPathSet = TRUE;
//	FileSize = MakeFile.GetLength();
	if(bError) {
		AfxMessageBox("Creating ntf failed!!!");
	} else {
		AfxMessageBox("Finished creating ntf File!!!");
	}
}

void CNNRView::OnButtonPathResult() 
{
	int i = 0, j = 0, FileCnt = 0;

	CFile File;
	CFileFind ff;
	CFileFind denomf;
	CFileFind dirf;
	DWORD	FileSize = 0;
//	FILE *stream_Tile; //FILE은 사용 배제하자 읽어 들일때 오류 있음.

	CString FileName = "";
	CString SubPath_Class = ""; //denomination
	CString Display = "";

	unsigned int GroupNum  = 0;
	int SampleSize = 0;

	char Char[3];


	if(NNRFolder == ""){
		AfxMessageBox("Select Directory!!");
		return;
	}
	UpdateData(true);
	BOOL bFlag = TRUE;

	m_List_Result.InsertItem(0, "................Calculation Start...");
	m_List_Result.InsertItem(0, "");
	m_pNeuralOneHidden->m_SelectActivation = m_SelectActivation;

	if(ff.FindFile(NNRFolder + _T("\\*.*"))){

		while(bFlag){

			bFlag = ff.FindNextFile();
			if(ff.IsDots()) continue;//처음 directory에서 .과 ..이 읽히므로(*.*의 경우) 다시 while로 돌아가 아래루틴을 무시한다.

			if(ff.IsDirectory()){//권종에서 denomination으로 내려가는 단계
				SubPath_Class = ff.GetFilePath();
				SubPath_Class += '/';

/********************************************************************************/				
							BOOL bFlagSubDir = TRUE;
							if(dirf.FindFile(SubPath_Class + _T("*.raw"))){

								while(bFlagSubDir){

									bFlagSubDir = dirf.FindNextFile();
									if(dirf.IsDots()) continue;//처음 directory에서 .과 ..이 읽히므로(*.*의 경우) 다시 while로 돌아가 아래루틴을 무시한다.

									FileName = dirf.GetFileName();

									if(!File.Open(SubPath_Class + FileName, CFile::modeRead)){
										ff.Close(); denomf.Close(); dirf.Close();
										AfxMessageBox("Lost File!"); return;
									}

									GroupNum++;

									SampleSize = File.GetLength();

									unsigned char * Data = new unsigned char[SampleSize];
									File.Read(Data, SampleSize);
									File.Close();

									if(SampleSize == m_Edit_InputNodeNum){
										
										FileName = FileName.Left(FileName.GetLength()-4);
										int FileNum = atoi(FileName);

										if(m_Edit_MatchMax >= FileNum && m_Edit_MatchMin <= FileNum){
											NNR_REAL * Input  = new NNR_REAL[m_Edit_InputNodeNum+1];
											NNR_REAL * Output = new NNR_REAL[m_Edit_ClassNum];

											for(i = 0; i < SampleSize; i++){
												Input[i] = Data[i];
												Input[i] = (Input[i] - 128)/128;
											}
											m_pNeuralOneHidden->OutFunc(Input,Output);
										
											int Pos = 0;
											double MaxValue = 0;
											for(i = 0; i < m_Edit_ClassNum; i++){
												if(MaxValue < Output[i]){
													MaxValue = Output[i];
													Pos = i;
												}
											}
											MaxValue *= 100;

											Display = SubPath_Class;
											Display.Replace(NNRFolder, "");
											Display += FileName;

											m_List_Result.InsertItem(0, Display);

											sprintf_s(Char, 10, "%d", Pos);
											m_List_Result.SetItem(0, 1, LVIF_TEXT, Char, 0, 0, 0, 0);
											sprintf_s(Char, 10, "%d", int(MaxValue));
											m_List_Result.SetItem(0, 2, LVIF_TEXT, Char, 0, 0, 0, 0);
											if(MaxValue < 90) m_List_Result.SetItem(0, 3, LVIF_TEXT, "Check", 0, 0, 0, 0);

											delete []Input;
											delete []Output;
										}

										delete []Data;
//										if(m_Edit_Sel_GroupNum == GroupNum) break;
									}
								}
							}
							GroupNum = 0;
/********************************************************************************/				
			}
		}
	}
	m_List_Result.InsertItem(0, "");
	m_List_Result.InsertItem(0, "................Calculation End...");

	ff.Close(); denomf.Close(); dirf.Close();
}

void CNNRView::OnRadioLogistic() 
{
	m_SelectActivation = SEL_LOGISTIC;
	m_Edit_Momentum = LOGIST_ALPHA;
	m_Edit_ETA = LOGIST_ETA;
	UpdateData(false);

}

void CNNRView::OnRadioHyperbolic() 
{
	m_SelectActivation = SEL_HYPERBOLIC;
	m_Edit_Momentum = HYPER_ALPHA;
	m_Edit_ETA = HYPER_ETA;
	UpdateData(false);
}


/*
UINT CNNRView::StartNetThread(void *pointer)
{
	CNNRView *View = reinterpret_cast<CNNRView *>(pointer);
	View->InitThreadNNR();
	return 0;
}

void CNNRView::InitThreadNNR()
{
	if(m_Edit_ConversionErrorRate <= 0 || m_Edit_ConversionErrorRate > 0.5) { AfxMessageBox("Error범위가 적절하지 않습니다."); return;}
	if(m_Edit_Momentum <= 0 || m_Edit_Momentum > 1) { AfxMessageBox("Momentum범위가 적절하지 않습니다."); return;}
	if(m_Edit_ETA <= 0 || m_Edit_ETA > 1) { AfxMessageBox("ETA범위가 적절하지 않습니다."); return;}
	if(m_WeightLoadFlag == FALSE)
	{
		if(m_Edit_FirstHiddenNode == NULL) { AfxMessageBox(" HiddenNode의 갯수를 정하십시요"); return;}
		InitOneHiddenLayer();
		m_pNeuralOneHidden->InitWeightFunc();
	}
	m_Edit_EpochNum = 0;
	m_GroupNum = 0;
	m_ActiveFlag = TRUE;

	m_Edit_AverageError = 1;

	CString Rate = "";
	while(m_Edit_AverageError > m_Edit_ConversionErrorRate){
		
		m_pNeuralOneHidden->LoadSampleData(m_pTrainData[m_GroupNum]);
		m_Edit_AverageError += m_pNeuralOneHidden->WeightTranningFunc();
		m_Edit_EpochNum++;
		m_GroupNum++;

		if(m_GroupNum == m_Edit_GroupNum)
		{
			m_Edit_AverageError /= m_GroupNum;

//			UpdateData(FALSE);

//			printf(Rate, " %f", m_Edit_AverageError);
//			SetWindowText("Kohonen - Phase 1");
//			SendMessage(WM_PAINT);

			m_GroupNum = 0;
			m_Edit_AverageError = 1;
		}
	}

	AfxMessageBox("Running Complete!!");
	OneHiddenWeightSave();
}
*/

void CNNRView::OnEnChangeEditAverageLimit()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	UpdateData(true);
}

#define	APP_NAME	"SOFTWARE\\SEETECH\\ST150DataMaker"
#define	NNR_FOLDER	"NNRFolder"
#define	AGERAGE_LIMIT	"AverageLimit"
#define	FHIDDEN_NODE	"FistHiddenNode"
#define	RAX_W	"RaxW"
#define	RAX_H	"RaxH"

void RegSetNNRFolder()
{
	DWORD dwDisp = 0;
	HKEY hkey = NULL;
	LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, APP_NAME, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
	if(err != ERROR_SUCCESS)
		return;

	if(lstrcmp(RegNNRFolder, NNRFolder)) {
		lstrcpy(RegNNRFolder, NNRFolder);
		RegSetValueEx(hkey, TEXT(NNR_FOLDER), 0, REG_SZ, (LPBYTE)RegNNRFolder, (lstrlen(RegNNRFolder)+1)*sizeof(TCHAR));
	}

	RegSetValueEx(hkey, TEXT(AGERAGE_LIMIT), 0, REG_BINARY, (LPBYTE)&m_averageLimit, sizeof(m_averageLimit));
	RegSetValueEx(hkey, TEXT(FHIDDEN_NODE), 0, REG_BINARY, (LPBYTE)&m_Edit_FirstHiddenNode, sizeof(m_Edit_FirstHiddenNode));
	RegSetValueEx(hkey, TEXT(RAX_W), 0, REG_BINARY, (LPBYTE)&m_RaxW, sizeof(m_RaxW));
	RegSetValueEx(hkey, TEXT(RAX_H), 0, REG_BINARY, (LPBYTE)&m_RaxH, sizeof(m_RaxH));
	
	RegCloseKey(hkey);
}

void RegGetNNRFolder()
{
	lstrcpy(RegNNRFolder, TEXT("."));
	HKEY hkey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(APP_NAME), 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
		return;

	DWORD dwType, dwSize;
	dwType = 0;
	dwSize = MAX_PATH;
	RegQueryValueEx(hkey, TEXT(NNR_FOLDER), NULL, &dwType, (LPBYTE)RegNNRFolder, &dwSize);

	dwType = 0;
	dwSize = sizeof(RegAverageLimit);
	if(RegQueryValueEx(hkey, TEXT(AGERAGE_LIMIT), NULL, &dwType, (LPBYTE)&RegAverageLimit, &dwSize) == ERROR_SUCCESS)
		m_averageLimit = RegAverageLimit;

	int tint = 24;
	dwType = 0;
	dwSize = sizeof(tint);
	if(RegQueryValueEx(hkey, TEXT(FHIDDEN_NODE), NULL, &dwType, (LPBYTE)&tint, &dwSize) == ERROR_SUCCESS)
		m_Edit_FirstHiddenNode = tint;

	dwType = 0;
	dwSize = sizeof(m_RaxW);
	RegQueryValueEx(hkey, TEXT(RAX_W), NULL, &dwType, (LPBYTE)&m_RaxW, &dwSize);

	dwType = 0;
	dwSize = sizeof(m_RaxH);
	RegQueryValueEx(hkey, TEXT(RAX_H), NULL, &dwType, (LPBYTE)&m_RaxH, &dwSize);

	dwSize = MAX_PATH;
	RegCloseKey(hkey);
}


void CNNRView::OnEnChangeEditExt()
{
	m_EditExt.GetWindowText(m_Ext);
}

extern "C" {
int GetLengthInBytesForNNROCR(int width, int height);
int MakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height);
void RemoveUpperBlank(unsigned char* image, int width, int height);
void RemoveSidePixels(unsigned char* srcbuf, int old_w, int old_h, int new_w, int new_h);
};

void SaveToBitmap(TCHAR* path, unsigned char* buf, int x_size, int y_size)
{
	BITMAPINFOHEADER bi;
	int x_byte_len = ((x_size * 3 + 3)/4)*4;

	//*------------------------- 비트멥 헤더를 기록함 -------------------------

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = x_size;
	bi.biHeight = -y_size;
	bi.biPlanes = 1;
	bi.biBitCount = 24;       
	bi.biCompression = BI_RGB;
	bi.biSizeImage = y_size * x_byte_len;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	BITMAPFILEHEADER hdr;

	hdr.bfType = ((WORD)('M' << 8) | 'B');        
	hdr.bfSize = bi.biSizeImage + sizeof(bi) + sizeof(hdr);   
	hdr.bfReserved1 = 0;                                 
	hdr.bfReserved2 = 0;                                 
	int nColors = 0; // Color Index 없음
	hdr.bfOffBits=(DWORD)(sizeof(hdr) + bi.biSize + nColors * sizeof(RGBQUAD));

	/*--------------------- 실제 파일에 기록함 --------------------------*/
	HANDLE pFile = NULL;

	TCHAR bmpFileName[MAX_PATH];
	lstrcpy(bmpFileName, path);
	lstrcpy(bmpFileName+lstrlen(bmpFileName)-3, "bmp");
	pFile = CreateFile(bmpFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	bool bResult = false;
	if (pFile == INVALID_HANDLE_VALUE)
		return;

	DWORD dwio = 0;
	WriteFile(pFile, &hdr, sizeof(hdr), &dwio, NULL);
	WriteFile(pFile, &bi, sizeof(bi), &dwio, NULL);

	unsigned char *rowBuffer = (unsigned char*)malloc(x_byte_len);
	int x, y;

	for(y = 0; y < y_size; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < x_size; x++) {
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
		}
		WriteFile(pFile, rowBuffer, x_byte_len, &dwio, NULL);
	}
	free(rowBuffer);

	CloseHandle(pFile);
}


void CNNRView::OnBnClickedButtonRemake()
{
	CRawFormatConvertDlg *pDlg = new CRawFormatConvertDlg();
	pDlg->DoModal();
	delete pDlg;
}

void CNNRView::OnBnClickedButtonFind()
{
	TCHAR pszPathname[MAX_PATH];
	lstrcpy(pszPathname, (LPCTSTR)NNRFolder);
	lstrcat(pszPathname, TEXT("\\*"));

	m_foundGroups = 0;
	CFileFind ff;
	if(ff.FindFile(pszPathname)) {
		int benext = TRUE;
		while(benext) {
			benext = ff.FindNextFile();
			if(ff.IsDots())
				continue;
			int foundRaws = CountFiles(ff.GetFilePath() + "/*."+m_Ext);
			m_foundFiles += foundRaws;
			if(foundRaws > m_foundGroups)
				m_foundGroups = foundRaws;
		}
	}
	m_Edit_Sel_GroupNum = m_foundGroups;

	UpdateData(false);

	CString msg;
	msg = "Find all ." + m_Ext + " files";
	MessageBox(msg, "Finished");
}
