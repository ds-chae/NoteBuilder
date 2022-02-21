#ifndef NEURAL_H
#define NEURAL_H

#define	MAX_INPUT_NODE	1500
#define	MAX_CLASS	300
#define	MAX_GROUP	1000

#define DBIAS				1
#define LOGIST_ETA			0.5  //running rate
#define LOGIST_ALPHA		0.8
#define HYPER_ETA			0.03  //running rate
#define HYPER_ALPHA			0.05

#define DCONVERSIONRATE		0.0000001


#define SEL_LOGISTIC		0
#define SEL_HYPERBOLIC		1

//double g_ConversionThread; 
//#define HYPERBOLIC_A		1.7159
//#define EPSILON				0.7159
#define HYPERBOLIC_A		1.0
#define EPSILON				0.0
#define HYPERBOLIC_B		2.0/3.0

#define	NNR_REAL	double
//#define	NNR_REAL	float

#define	USE_CBLAS	0

class CNeuralOneHidden
{
public:
	double WeightTranningFuncRand();
	void LoadSampleData(double ** GroupData);
	void DestructionNetwork();
	void ConstructionNetwork(int InputNode, int HiddenNode, int OutputNode, int InputClass, int Momentum);
	void InitRunState();
	void InitMomentum();
	double AveragePowFunc();
#if USE_CBLAS
	double CalculatePowFunc(NNR_REAL DesiredOut[], NNR_REAL Output[], NNR_REAL rDummy[]);
#else
	double CalculatePowFunc(NNR_REAL DesiredOut[], NNR_REAL Output[]);
#endif
	void InitDesiredOutFunc();
	double WeightTranningFunc();
	void InitWeightFunc();

	double Activation(double Sum);
 	void OutFunc(NNR_REAL Input[], NNR_REAL Output[]);


	CNeuralOneHidden();
	virtual ~CNeuralOneHidden();

	double **m_pSampleData;
	double **m_pDesiredOut;

	NNR_REAL *m_pInputNode;
	double *m_pHiddenNode;
	NNR_REAL *m_pOutputNode;
	NNR_REAL *rDummy;

	double **m_pHiddenWeight;
	double **m_pOutputWeight;

	double ***m_pHiddenMomWeight;
	double ***m_pOutputMomWeight;

	int m_InputNodeNum;
	int m_HiddenNodeNum;
	int m_OutputNodeNum;

	int m_InputClassNum;
	int m_MomentumNum;
	int m_MomentumPoint;

	int m_SelectActivation;
	int m_ConstructionFlag;

	double  m_MaxError;
	int     m_MaxErrorPos;

	double m_a;
	double m_b;

	double m_ETA;
	double m_ALPHA;
}; 

class CNeuralTwoHidden
{
public:
	void LoadSampleGroupData(double ** GroupData);
	void DestructionNetwork();
	void ConstructionNetwork(int InputNode, int FirstHiddenNode,  int SecondHiddenNode, int OutputNode, int InputClass, int Momentum);
	void InitRunState();
	void InitMomentum();
	double AveragePowFunc();
	double CalculatePowFunc(double DesiredOut[], double Output[]);
	void InitDesiredOutFunc();
	double WeightTranningFunc();
	void InitWeightFunc();

	CNeuralTwoHidden();
	virtual ~CNeuralTwoHidden();

	double **m_pSampleData;
	double **m_pDesiredOut;

	double *m_pInputNode;
	double *m_pFirstHiddenNode;
	double *m_pSecondHiddenNode;
	NNR_REAL *m_pOutputNode;

	double **m_pFirstHiddenWeight;
	double **m_pSecondHiddenWeight;
	double **m_pOutputWeight;

	double ***m_pFirstHiddenMomWeight;
	double ***m_pSecondHiddenMomWeight;
	double ***m_pOutputMomWeight;

	int m_InputNodeNum;
	int m_FirstHiddenNodeNum;
	int m_SecondHiddenNodeNum;
	int m_OutputNodeNum;

	int m_InputClassNum;
	int m_MomentumNum;
	int m_MomentumPoint;

	int m_ConstructionFlag;

	double m_ETA;
	double m_ALPHA;
	
	double ActivationExpFunc(double Sum);
	void OutFunc(double Input[], double Output[]);
}; 

#endif  // end of NEURALMG_H definition   