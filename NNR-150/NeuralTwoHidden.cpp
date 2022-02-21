#include "stdafx.h"
#include "Neural.h"
#include "math.h"

CNeuralTwoHidden::CNeuralTwoHidden()
{

	m_pSampleData = NULL;
	m_pDesiredOut = NULL;

	m_pInputNode = NULL;
	m_pFirstHiddenNode = NULL;
	m_pSecondHiddenNode = NULL;
	m_pOutputNode = NULL;

	m_pFirstHiddenWeight = NULL;
	m_pSecondHiddenWeight = NULL;
	m_pOutputWeight = NULL;

	m_pFirstHiddenMomWeight = NULL;
	m_pSecondHiddenMomWeight = NULL;
	m_pOutputMomWeight = NULL;

	m_InputNodeNum = 0;
	m_FirstHiddenNodeNum = 0;
	m_SecondHiddenNodeNum = 0;
	m_OutputNodeNum = 0;

	m_InputClassNum = 0;
	m_MomentumNum = 0;
	m_MomentumPoint = 0;

	m_ConstructionFlag = 0;

	m_ETA = LOGIST_ETA;
	m_ALPHA = LOGIST_ALPHA;

}

CNeuralTwoHidden::~CNeuralTwoHidden()
{
	if(m_ConstructionFlag == TRUE) DestructionNetwork();
}


double CNeuralTwoHidden::ActivationExpFunc(double Sum)
{
	double Exp = exp(-1 * Sum);
	return 1 / (1 + Exp);
}

void CNeuralTwoHidden::OutFunc(double Input[], double Output[])
{
	double Sum = 0;
	int i = 0, j = 0;

	Input[m_InputNodeNum] = 1;					  //input bias
	m_pFirstHiddenNode[m_FirstHiddenNodeNum] = 1;      //first hidden bias
	m_pSecondHiddenNode[m_SecondHiddenNodeNum] = 1;    //second hidden bias

	for(i = 0; i < m_FirstHiddenNodeNum; i++)
	{
		Sum = 0;
		for(j = 0; j < m_InputNodeNum + 1; j++)
		{
			Sum += Input[j] * m_pFirstHiddenWeight[i][j];
		}
		m_pFirstHiddenNode[i] = ActivationExpFunc(Sum);
	}

	for(i = 0; i < m_SecondHiddenNodeNum; i++)
	{
		Sum = 0;
		for(j = 0; j < m_FirstHiddenNodeNum + 1; j++)
		{
			Sum += m_pFirstHiddenNode[j] * m_pSecondHiddenWeight[i][j];
		}
		m_pSecondHiddenNode[i] = ActivationExpFunc(Sum);
	}

	for(i = 0; i < m_OutputNodeNum; i++)
	{
		Sum = 0;
		for(j = 0; j < m_SecondHiddenNodeNum + 1; j++)
		{
			Sum += m_pSecondHiddenNode[j] * m_pOutputWeight[i][j];
		}
		Output[i] = ActivationExpFunc(Sum);
	}
}

double CNeuralTwoHidden::WeightTranningFunc()
{
	int i = 0, j = 0, k = 0, l = 0;
	double AveragePow = 0, Sum = 0;
	double * pDeltaFirstHidden = new double [m_FirstHiddenNodeNum];
	double * pDeltaSecondHidden = new double [m_SecondHiddenNodeNum];
	double * pDeltaOut = new double [m_OutputNodeNum];

		for(i = 0; i < m_InputClassNum; i++)//classification에서 input sample은 output의 갯수와 같다.
		{
			for(j = 0; j < m_InputNodeNum;j++)
			{
				m_pInputNode[j] = m_pSampleData[i][j];
			}
			OutFunc(m_pInputNode, m_pOutputNode);
/**************************************Out Weight Update*************************************/
			for(j = 0; j < m_OutputNodeNum; j++)
			{
				pDeltaOut[j] = 0;
				pDeltaOut[j] = (m_pDesiredOut[i][j] - m_pOutputNode[j]) * m_pOutputNode[j] * (1 - m_pOutputNode[j]); //sequential update

				for(k = 0; k < m_SecondHiddenNodeNum + 1; k++) //bias의 weight까지 update
				{
					m_pOutputWeight[j][k] += m_ETA * pDeltaOut[j] * m_pSecondHiddenNode[k];

/*-----------------------------------------------------------------------------------------------------------------*/				
					for(l = 0; l < m_MomentumNum; l++) 
					{
						m_pOutputMomWeight[l][j][k] *= m_ALPHA;
						m_pOutputWeight[j][k] += m_pOutputMomWeight[l][j][k];
					}
					m_pOutputMomWeight[m_MomentumPoint][j][k] = m_ETA * pDeltaOut[j] * m_pSecondHiddenNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				

				}
			}
/**************************************Out Weight Update*************************************/

/**************************************Second Hidden Weight Update*************************************/
			for(j = 0; j < m_SecondHiddenNodeNum; j++)
			{
				Sum = 0;
				for(k = 0; k < m_OutputNodeNum; k++)
				{
					Sum += pDeltaOut[k] * m_pOutputWeight[k][j];
				}
				pDeltaSecondHidden[j] = 0;
				pDeltaSecondHidden[j] = m_pSecondHiddenNode[j] * (1 - m_pSecondHiddenNode[j] ) * Sum;

				for(k = 0; k < m_FirstHiddenNodeNum + 1; k++) //bias의 weight까지 update
				{
					m_pSecondHiddenWeight[j][k] += m_ETA * pDeltaSecondHidden[j] * m_pFirstHiddenNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
					for(l = 0; l < m_MomentumNum; l++)
					{
						m_pSecondHiddenMomWeight[l][j][k] *= m_ALPHA;
						m_pSecondHiddenWeight[j][k] += m_pSecondHiddenMomWeight[l][j][k];
					}
					m_pSecondHiddenMomWeight[m_MomentumPoint][j][k] = m_ETA * pDeltaSecondHidden[j] * m_pFirstHiddenNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
				}
			}
/**************************************Second Hidden Weight Update*************************************/


/**************************************First Hidden Weight Update*************************************/
			for(j = 0; j < m_FirstHiddenNodeNum; j++)
			{
				Sum = 0;
				for(k = 0; k < m_SecondHiddenNodeNum; k++)
				{
					Sum += pDeltaSecondHidden[k] * m_pSecondHiddenWeight[k][j];
				}
				pDeltaFirstHidden[j] = 0;
				pDeltaFirstHidden[j] = m_pFirstHiddenNode[j] * (1 - m_pFirstHiddenNode[j] ) * Sum;

				for(k = 0; k < m_InputNodeNum + 1; k++) //bias의 weight까지 update
				{
					m_pFirstHiddenWeight[j][k] += m_ETA * pDeltaFirstHidden[j] * m_pInputNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
					for(l = 0; l < m_MomentumNum; l++)
					{
						m_pFirstHiddenMomWeight[l][j][k] *= m_ALPHA;
						m_pFirstHiddenWeight[j][k] += m_pFirstHiddenMomWeight[l][j][k];
					}
					m_pFirstHiddenMomWeight[m_MomentumPoint][j][k] = m_ETA * pDeltaFirstHidden[j] * m_pInputNode[k];
/*-----------------------------------------------------------------------------------------------------------------*/				
				}
			}
/**************************************First Hidden Weight Update*************************************/
		}

/*-----------------------------------------------------------------------------------------------------------------*/				
	m_MomentumPoint++;
	if(m_MomentumPoint == m_MomentumNum) m_MomentumPoint = 0;
/*-----------------------------------------------------------------------------------------------------------------*/				

	AveragePow = AveragePowFunc();

	delete []pDeltaFirstHidden;
	delete []pDeltaSecondHidden;
	delete []pDeltaOut;
		
	return AveragePow;
}


double CNeuralTwoHidden::CalculatePowFunc(double DesiredOut[], double Output[])
{
	int i = 0;
	double Pow = 0;

	for(i = 0; i < m_OutputNodeNum; i++)
	{
		Pow += (DesiredOut[i] - Output[i]) * (DesiredOut[i] - Output[i]);
	}
	Pow /= 2;

	return Pow;
}

double CNeuralTwoHidden::AveragePowFunc()
{
	int i = 0, j = 0;
	double AveragePow = 0;

	for(i = 0; i < m_InputClassNum; i++)
	{
		for(j = 0; j < m_InputNodeNum; j++)
		{
			m_pInputNode[j] = m_pSampleData[i][j];
		}
		OutFunc(m_pInputNode, m_pOutputNode);
		AveragePow += CalculatePowFunc(m_pDesiredOut[i], m_pOutputNode);
	}
	AveragePow /= m_InputClassNum;

	return AveragePow;
}

void CNeuralTwoHidden::InitRunState()
{
	InitMomentum();
	InitDesiredOutFunc();
}

void CNeuralTwoHidden::InitDesiredOutFunc()
{
	int i = 0, j = 0;

	for(i = 0; i < m_InputClassNum; i++)
	{
		for(j = 0; j < m_OutputNodeNum; j++)
		{
			if(i == j) m_pDesiredOut[i][j] = 1;
			else m_pDesiredOut[i][j] = 0;
		}
	}
}

void CNeuralTwoHidden::InitWeightFunc() //차후 좀더 공부하여 고찰하자....
{
	int i = 0, j = 0;


	for(i = 0; i < m_FirstHiddenNodeNum; i++)
		for(j = 0; j < m_InputNodeNum + 1; j++)
			m_pFirstHiddenWeight[i][j] = double(rand()%1000)/10000;

	for(i = 0; i < m_SecondHiddenNodeNum; i++)
		for(j = 0; j < m_FirstHiddenNodeNum + 1; j++)
			m_pSecondHiddenWeight[i][j] = double(rand()%1000)/10000;

	for(i = 0; i < m_OutputNodeNum; i++)
		for(j = 0; j < m_SecondHiddenNodeNum + 1; j++)
			m_pOutputWeight[i][j] = double(rand()%1000)/10000;
}

void CNeuralTwoHidden::InitMomentum()
{
	int i = 0, j = 0, k = 0;

	for(i = 0; i < m_MomentumNum; i++)
		for(j = 0; j < m_FirstHiddenNodeNum; j++) 
			for(k = 0; k < m_InputNodeNum + 1; k++) m_pFirstHiddenMomWeight[i][j][k] = 0;

	for(i = 0; i < m_MomentumNum; i++)
		for(j = 0; j < m_SecondHiddenNodeNum; j++) 
			for(k = 0; k < m_FirstHiddenNodeNum + 1; k++) m_pSecondHiddenMomWeight[i][j][k] = 0;


	for(i = 0; i < m_MomentumNum; i++)
		for(j = 0; j < m_OutputNodeNum; j++)
			for(k = 0; k < m_SecondHiddenNodeNum + 1; k++) m_pOutputMomWeight[i][j][k] = 0;
	
	m_MomentumPoint = 0;
}

void CNeuralTwoHidden::LoadSampleGroupData(double **GroupData)
{
	int i = 0, j = 0;
	
	for(i = 0; i < m_InputClassNum; i++){
		for(j = 0; j < m_InputNodeNum; j++){
			m_pSampleData[i][j] = GroupData[i][j];
		}
	}
}

void CNeuralTwoHidden::ConstructionNetwork(int InputNode, int FirstHiddenNode,  int SecondHiddenNode, int OutputNode, int InputClass, int Momentum)
{
	int i = 0, j = 0;

	m_pSampleData = new double *[InputClass];
	for(i = 0; i < InputClass; i++) m_pSampleData[i] = new double [InputNode];

	m_pDesiredOut = new double *[InputClass];
	for(i = 0; i < InputClass; i++) m_pDesiredOut[i] = new double [OutputNode];

	m_pInputNode        = new double [InputNode + 1];
	m_pFirstHiddenNode  = new double [FirstHiddenNode + 1];
	m_pSecondHiddenNode = new double [SecondHiddenNode + 1];
	m_pOutputNode       = new double [OutputNode];

	m_pFirstHiddenWeight = new double *[FirstHiddenNode];
	for(i = 0; i < FirstHiddenNode; i++) m_pFirstHiddenWeight[i] = new double [InputNode + 1];

	m_pSecondHiddenWeight = new double *[SecondHiddenNode];
	for(i = 0; i < SecondHiddenNode; i++) m_pSecondHiddenWeight[i] = new double [FirstHiddenNode + 1];

	m_pOutputWeight = new double *[OutputNode];
	for(i = 0; i < OutputNode; i++) m_pOutputWeight[i] = new double [SecondHiddenNode + 1];

	m_pFirstHiddenMomWeight = new double **[Momentum];
	for(i = 0; i < Momentum; i++){
		m_pFirstHiddenMomWeight[i] = new double *[FirstHiddenNode];
		for(j = 0; j < FirstHiddenNode; j++) m_pFirstHiddenMomWeight[i][j] = new double [InputNode + 1];
	}

	m_pSecondHiddenMomWeight = new double **[Momentum];
	for(i = 0; i < Momentum; i++){
		m_pSecondHiddenMomWeight[i] = new double *[SecondHiddenNode];
		for(j = 0; j < SecondHiddenNode; j++) m_pSecondHiddenMomWeight[i][j] = new double [FirstHiddenNode + 1];
	}
	
	m_pOutputMomWeight = new double **[Momentum];
	for(i = 0; i < Momentum; i++){
		m_pOutputMomWeight[i] = new double *[OutputNode];
		for(j = 0; j < OutputNode; j++) m_pOutputMomWeight[i][j] = new double [SecondHiddenNode + 1];
	}

	m_InputNodeNum        = InputNode;
	m_FirstHiddenNodeNum  = FirstHiddenNode;
	m_SecondHiddenNodeNum = SecondHiddenNode;
	m_OutputNodeNum       = OutputNode;

	m_InputClassNum       = InputClass;
	m_MomentumNum         = Momentum;

	m_ConstructionFlag = TRUE;
}

void CNeuralTwoHidden::DestructionNetwork()
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
	delete[] m_pFirstHiddenNode;
	m_pFirstHiddenNode = NULL;
	delete[] m_pSecondHiddenNode;
	m_pSecondHiddenNode = NULL;
	delete[] m_pOutputNode;
	m_pOutputNode = NULL;

	for(i = 0; i < m_FirstHiddenNodeNum; i++) delete[] m_pFirstHiddenWeight[i];
	delete[] m_pFirstHiddenWeight;
	m_pFirstHiddenWeight = NULL;

	for(i = 0; i < m_SecondHiddenNodeNum; i++) delete[] m_pSecondHiddenWeight[i];
	delete[] m_pSecondHiddenWeight;
	m_pSecondHiddenWeight = NULL;

	for(i = 0; i < m_OutputNodeNum; i++) delete[] m_pOutputWeight[i];
	delete[] m_pOutputWeight;
	m_pOutputWeight = NULL;

	for(i = 0; i < m_MomentumNum; i++){
		for(j = 0; j < m_FirstHiddenNodeNum; j++) delete[] m_pFirstHiddenMomWeight[i][j];
		delete[] m_pFirstHiddenMomWeight[i];
	}
	delete m_pFirstHiddenMomWeight;
	m_pFirstHiddenMomWeight = NULL;

	for(i = 0; i < m_MomentumNum; i++){
		for(j = 0; j < m_SecondHiddenNodeNum; j++) delete[] m_pSecondHiddenMomWeight[i][j];
		delete[] m_pSecondHiddenMomWeight[i];
	}
	delete m_pSecondHiddenMomWeight;
	m_pSecondHiddenMomWeight = NULL;

	for(i = 0; i < m_MomentumNum; i++){
		for(j = 0; j < m_OutputNodeNum; j++) delete[] m_pOutputMomWeight[i][j];
		delete[] m_pOutputMomWeight[i];
	}
	delete m_pOutputMomWeight;
	m_pOutputMomWeight = NULL;

	m_ConstructionFlag = FALSE;
}

