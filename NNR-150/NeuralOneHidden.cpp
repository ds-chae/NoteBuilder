#include "stdafx.h"
#include "Neural.h"
#include "math.h"

CNeuralOneHidden::CNeuralOneHidden()
{
	m_ConstructionFlag = FALSE;
	m_pSampleData = NULL;
	m_pDesiredOut = NULL;

	m_pInputNode = NULL;
	m_pHiddenNode = NULL;
	m_pOutputNode = NULL;
	rDummy = NULL;

	m_pHiddenWeight = NULL;
	m_pOutputWeight = NULL;

	m_pHiddenMomWeight = NULL;
	m_pOutputMomWeight = NULL;


	m_InputNodeNum = 0;
	m_HiddenNodeNum = 0;
	m_OutputNodeNum = 0;

	m_InputClassNum = 0;
	m_MomentumNum = 0;
	m_MomentumPoint = 0;
	m_MaxError = 0;

	m_SelectActivation = SEL_LOGISTIC;

	m_ETA = LOGIST_ETA;
	m_ALPHA = LOGIST_ALPHA;

	m_a = HYPERBOLIC_A;
	m_b = HYPERBOLIC_B;


}

CNeuralOneHidden::~CNeuralOneHidden()
{
	if(m_ConstructionFlag == TRUE) DestructionNetwork();
}

double CNeuralOneHidden::Activation(double Sum)
{
	double Activation = 0; 

	if(m_SelectActivation == SEL_LOGISTIC){

	Activation = exp(-1 * Sum);
	Activation = 1 / (1 + Activation);

	}else if(m_SelectActivation == SEL_HYPERBOLIC){

		Activation = m_a * tanh(m_b * Sum);
//		Activation -= EPSILON;
	}
	return Activation;
}

void CNeuralOneHidden::OutFunc(NNR_REAL Input[], NNR_REAL Output[])
{
	NNR_REAL Sum = 0;
	int i = 0, j = 0;
	int test = 0;

	Input[m_InputNodeNum] = 1;
	m_pHiddenNode[m_HiddenNodeNum] = 1;

#if USE_CBLAS
	for(i = 0; i < m_HiddenNodeNum; i++)
	{
		NNR_REAL dSum[2];
		dSum[0] = cblas_ddot(m_InputNodeNum+1, Input, 1, m_pHiddenWeight[i], 1);
//		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 1, m_InputNodeNum+1, 1.0, Input, m_InputNodeNum+1, m_pHiddenWeight[i], 1, 0.0, dSum, 1);
		// cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 1, 4, 1.0, A, 4, B, 1, 0.0, C, 1);
		m_pHiddenNode[i] = Activation(dSum[0]);
	}

	for(i = 0; i <m_OutputNodeNum; i++)
	{
		Sum = cblas_ddot(m_HiddenNodeNum + 1, m_pHiddenNode, 1, m_pOutputWeight[i], 1);
		Output[i] = Activation(Sum);
	}
#else
	for(i = 0; i < m_HiddenNodeNum; i++)
	{
		Sum = 0;
		for(j = 0; j < m_InputNodeNum + 1; j++)
		{
			Sum += Input[j] * m_pHiddenWeight[i][j];
		}
		if(Sum > 10){
			test++;
		}
		m_pHiddenNode[i] = Activation(Sum);
	}

	for(i = 0; i <m_OutputNodeNum; i++)
	{
		Sum = 0;
		for(j = 0; j < m_HiddenNodeNum + 1; j++)
		{
			Sum += m_pHiddenNode[j] * m_pOutputWeight[i][j];
		}
		if(Sum < -10){
			test++;
		}
		Output[i] = Activation(Sum);
	}
#endif
}

double CNeuralOneHidden::WeightTranningFunc()
{
	int i = 0, j = 0, k = 0, l = 0;
	double AveragePow = 0, Sum = 0;
	double * pDeltaHidden = new double[m_HiddenNodeNum];
	double * pDeltaOut = new double[m_OutputNodeNum];
	int    * pSelectClass = new int[m_InputClassNum];	
	int     SelectClass = 0;
	int     FindClass = 0;


		for(i = 0; i < m_InputClassNum; i++)//classification에서 input sample은 output의 갯수와 같다.
		{
			do{
				FindClass = 0;
				SelectClass = rand()%m_InputClassNum;

				for(j = 0; j < i; j++){
					if(pSelectClass[j] == SelectClass){
						FindClass = 1;
					}
				}
				if(FindClass == 0){
					pSelectClass[i] = SelectClass;
				}
			}while(FindClass);

			for(j = 0; j < m_InputNodeNum;j++)
			{
				m_pInputNode[j] = m_pSampleData[pSelectClass[i]][j];
			}
			OutFunc(m_pInputNode, m_pOutputNode);
/**************************************Out Weight Update*************************************/
			for(j = 0; j < m_OutputNodeNum; j++)
			{
				pDeltaOut[j] = 0;
				
				if(m_SelectActivation == SEL_LOGISTIC){
					pDeltaOut[j] = (m_pDesiredOut[pSelectClass[i]][j] - m_pOutputNode[j]) * m_pOutputNode[j] * (1 - m_pOutputNode[j]); //sequential update
				}else if(m_SelectActivation == SEL_HYPERBOLIC){
					pDeltaOut[j]  = (m_b/m_a); //sequential update
					pDeltaOut[j] *= (m_pDesiredOut[pSelectClass[i]][j] - m_pOutputNode[j]); //sequential update
					pDeltaOut[j] *= (m_a - m_pOutputNode[j]); //sequential update
					pDeltaOut[j] *= (m_a + m_pOutputNode[j]); //sequential update
//					pDeltaOut[j] = (m_b/m_a) * (m_pDesiredOut[i][j] - m_pOutputNode[j]) * (m_a - m_pOutputNode[j]) * (m_a + m_pOutputNode[j]); //sequential update
				}

				for(k = 0; k < m_HiddenNodeNum + 1; k++) //bias의 weight까지 update
				{
					m_pOutputWeight[j][k] += m_ETA * pDeltaOut[j] * m_pHiddenNode[k];

/*-----------------------------------------------------------------------------------------------------------------*/				
					for(l = 0; l < m_MomentumNum; l++)
					{
						m_pOutputMomWeight[l][j][k] *= m_ALPHA;
						m_pOutputWeight[j][k] += m_pOutputMomWeight[l][j][k];
					}
					m_pOutputMomWeight[m_MomentumPoint][j][k] = m_ETA * pDeltaOut[j] * m_pHiddenNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
				}
			}
/**************************************Out Weight Update*************************************/

/**************************************Hidden Weight Update*************************************/
			for(j = 0; j < m_HiddenNodeNum; j++)
			{
				Sum = 0;
				for(k = 0; k < m_OutputNodeNum; k++)
				{
					Sum += pDeltaOut[k] * m_pOutputWeight[k][j];
				}
				pDeltaHidden[j] = 0;

				if(m_SelectActivation == SEL_LOGISTIC){/********************                logistic update                ************************/

				pDeltaHidden[j] = m_pHiddenNode[j] * (1 - m_pHiddenNode[j] ) * Sum;

				}else if(m_SelectActivation == SEL_HYPERBOLIC){/********************        hyperbolic update              ************************/

					pDeltaHidden[j]  = (m_b/m_a); //sequential update
					pDeltaHidden[j] *= (m_a - m_pHiddenNode[j]); //sequential update
					pDeltaHidden[j] *= (m_a + m_pHiddenNode[j]); //sequential update
					pDeltaHidden[j] *= Sum; //sequential update
//					pDeltaHidden[j]  = (m_b/m_a) * (m_a - m_pHiddenNode[j]) * (m_a + m_pHiddenNode[j]) * Sum; //sequential update
				}

				for(k = 0; k < m_InputNodeNum + 1; k++) //bias의 weight까지 update
				{
					m_pHiddenWeight[j][k] += m_ETA * pDeltaHidden[j] * m_pInputNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
					for(l = 0; l < m_MomentumNum; l++)
					{
						m_pHiddenMomWeight[l][j][k] *= m_ALPHA;
						m_pHiddenWeight[j][k] += m_pHiddenMomWeight[l][j][k];
					}
					m_pHiddenMomWeight[m_MomentumPoint][j][k] = m_ETA * pDeltaHidden[j] * m_pInputNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
				}
			}
/**************************************Hidden Weight Update*************************************/
		}

/*-----------------------------------------------------------------------------------------------------------------*/				
	m_MomentumPoint++;
	if(m_MomentumPoint == m_MomentumNum) m_MomentumPoint = 0;
/*-----------------------------------------------------------------------------------------------------------------*/				

	AveragePow = AveragePowFunc();

	delete []pDeltaOut;
	delete []pDeltaHidden;
	delete []pSelectClass;

	return AveragePow;
}

#if USE_CBLAS
double CNeuralOneHidden::CalculatePowFunc(NNR_REAL DesiredOut[], NNR_REAL Output[], NNR_REAL rDummy[])
#else
double CNeuralOneHidden::CalculatePowFunc(NNR_REAL DesiredOut[], NNR_REAL Output[])
#endif
{
	int i = 0;
	double Pow = 0;

#if USE_CBLAS
	for(i = 0; i < m_OutputNodeNum; i++) rDummy[i] = DesiredOut[i] - Output[i];
	Pow = cblas_ddot(m_OutputNodeNum, rDummy, 1, rDummy, 1);
#else
	for(i = 0; i < m_OutputNodeNum; i++)
	{
		Pow += (DesiredOut[i] - Output[i]) * (DesiredOut[i] - Output[i]);
	}
#endif
	Pow /= 2;

	return Pow;
}

double CNeuralOneHidden::AveragePowFunc()
{
	int i = 0, j = 0;
	double Pow = 0;
	double AveragePow = 0;

	for(i = 0; i < m_InputClassNum; i++)
	{
		for(j = 0; j < m_InputNodeNum; j++)
		{
			m_pInputNode[j] = m_pSampleData[i][j];
		}
		OutFunc(m_pInputNode, m_pOutputNode);
#if USE_CBLAS
		Pow = CalculatePowFunc(m_pDesiredOut[i], m_pOutputNode, rDummy);
#else
		Pow = CalculatePowFunc(m_pDesiredOut[i], m_pOutputNode);
#endif
		AveragePow += Pow;

		if(m_MaxError < Pow){
			m_MaxError    = Pow;
			m_MaxErrorPos = i;
		}
	}
	AveragePow /= m_InputClassNum;

	return AveragePow;
}

void CNeuralOneHidden::InitRunState()
{
	InitDesiredOutFunc();
	InitMomentum();
}

void CNeuralOneHidden::InitDesiredOutFunc()
{
	int i = 0, j = 0;

	for(i = 0; i < m_InputClassNum; i++)
	{
		for(j = 0; j < m_OutputNodeNum; j++)
		{

			if(m_SelectActivation == SEL_LOGISTIC){

			if(i == j) m_pDesiredOut[i][j] = 1;
			else m_pDesiredOut[i][j] = 0;

			}else if(m_SelectActivation == SEL_HYPERBOLIC){

				if(i == j) m_pDesiredOut[i][j] = HYPERBOLIC_A;
				else m_pDesiredOut[i][j] = -HYPERBOLIC_A;
			}
			
		}
	}
}

void CNeuralOneHidden::InitWeightFunc() //차후 좀더 공부하여 고찰하자....
{
	int i = 0, j = 0;

	for(i = 0; i < m_HiddenNodeNum; i++)
		for(j = 0; j < m_InputNodeNum + 1; j++)
			m_pHiddenWeight[i][j] = double(rand()%1000)/10000;

	for(i = 0; i < m_OutputNodeNum; i++)
		for(j = 0; j < m_HiddenNodeNum + 1; j++)
			m_pOutputWeight[i][j] = double(rand()%1000)/10000;
}


void CNeuralOneHidden::InitMomentum()
{
	int i = 0, j = 0, k = 0;

	for(i = 0; i < m_MomentumNum; i++)
		for(j = 0; j < m_HiddenNodeNum; j++) 
			for(k = 0; k < m_InputNodeNum + 1; k++) m_pHiddenMomWeight[i][j][k] = 0;

	for(i = 0; i < m_MomentumNum; i++)
		for(j = 0; j < m_OutputNodeNum; j++)
			for(k = 0; k < m_HiddenNodeNum + 1; k++) m_pOutputMomWeight[i][j][k] = 0;
	
	m_MomentumPoint = 0;

}

void CNeuralOneHidden::LoadSampleData(double **GroupData)
{
	int i = 0, j = 0;
	
	for(i = 0; i < m_InputClassNum; i++)
	{
		for(j = 0; j < m_InputNodeNum; j++)
		{
			m_pSampleData[i][j] = GroupData[i][j];
		}
	}

}


void CNeuralOneHidden::ConstructionNetwork(int InputNode, int HiddenNode, int OutputNode, int InputClass, int Momentum)
{
	int i = 0, j = 0;

	m_pSampleData = new double *[InputClass];
	for(i = 0; i < InputClass; i++) m_pSampleData[i] = new double [InputNode];

	m_pDesiredOut = new double *[InputClass];
	for(i = 0; i < InputClass; i++) m_pDesiredOut[i] = new double [OutputNode];

	m_pInputNode = new NNR_REAL [InputNode + 1];
	m_pHiddenNode = new double [HiddenNode + 1];
	m_pOutputNode = new NNR_REAL [OutputNode];
	rDummy = new NNR_REAL [OutputNode];

	m_pHiddenWeight = new double *[HiddenNode];
	for(i = 0; i < HiddenNode; i++) m_pHiddenWeight[i] = new double [InputNode + 1];

	m_pOutputWeight = new double *[OutputNode];
	for(i = 0; i < OutputNode; i++) m_pOutputWeight[i] = new double [HiddenNode + 1];


	m_pOutputMomWeight = new double **[Momentum];
	for(i = 0; i < Momentum; i++)
	{
		m_pOutputMomWeight[i] = new double *[OutputNode];
		for(j = 0; j < OutputNode; j++) m_pOutputMomWeight[i][j] = new double [HiddenNode + 1];
	}

	m_pHiddenMomWeight = new double **[Momentum];
	for(i = 0; i < Momentum; i++)
	{
		m_pHiddenMomWeight[i] = new double *[HiddenNode];
		for(j = 0; j < HiddenNode; j++) m_pHiddenMomWeight[i][j] = new double [InputNode + 1];
	}
	
	m_InputNodeNum = InputNode;
	m_HiddenNodeNum = HiddenNode;
	m_OutputNodeNum = OutputNode;

	m_InputClassNum = InputClass;
	m_MomentumNum = Momentum;

	m_ConstructionFlag = TRUE;
}

void CNeuralOneHidden::DestructionNetwork()
{
	int i = 0, j = 0;

	for(i = 0; i < m_InputClassNum; i++) delete[] m_pSampleData[i];
	delete[] m_pSampleData;
	m_pSampleData = NULL;

	for(i = 0; i < m_InputClassNum; i++) delete[] m_pDesiredOut[i];
	delete[] m_pDesiredOut;
	m_pDesiredOut = NULL;

	delete[] m_pInputNode;
	m_pInputNode = NULL;
	delete[] m_pHiddenNode;
	m_pHiddenNode = NULL;
	delete[] m_pOutputNode;
	m_pOutputNode = NULL;
	delete rDummy;
	rDummy = NULL;

	for(i = 0; i < m_HiddenNodeNum; i++) delete[] m_pHiddenWeight[i];
	delete[] m_pHiddenWeight;
	m_pHiddenWeight = NULL;

	for(i = 0; i < m_OutputNodeNum; i++) delete[] m_pOutputWeight[i];
	delete[] m_pOutputWeight;
	m_pHiddenWeight = NULL;

	for(i = 0; i < m_MomentumNum; i++)
	{
		for(j = 0; j < m_OutputNodeNum; j++) delete[] m_pOutputMomWeight[i][j];
		delete[] m_pOutputMomWeight[i];
	}
	delete m_pOutputMomWeight;
	m_pOutputMomWeight = NULL;

	for(i = 0; i < m_MomentumNum; i++)
	{
		for(j = 0; j < m_HiddenNodeNum; j++) delete[] m_pHiddenMomWeight[i][j];
		delete[] m_pHiddenMomWeight[i];
	}
	delete m_pHiddenMomWeight;
	m_pOutputMomWeight = NULL;

	m_ConstructionFlag = FALSE;

}


