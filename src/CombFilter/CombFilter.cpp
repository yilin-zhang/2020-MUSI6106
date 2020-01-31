#include "CombFilter.h"
#include "ErrorDef.h"

CCombFilterBase::CCombFilterBase() : CCombFilterIf()
{
    //reset();
}

CCombFilterBase::~CCombFilterBase ()
{
    reset();
}

Error_t CCombFilterBase::initIntern( CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels )
{
    // init file type and process function
    m_eFilterType = eFilterType;
    switch (m_eFilterType){
    case CCombFilterIf::kCombFIR:
        m_processFunc = &CCombFilterBase::FIRProcess;
        break;
    case CCombFilterIf::kCombIIR:
        m_processFunc = &CCombFilterBase::IIRProcess;
        break;
    }

    // init delay line
    m_fMaxDeleyLength = fMaxDelayLengthInS;
    m_fSampleRateInHz = fSampleRateInHz;
    m_iNumChannels = iNumChannels;
    m_iDelayLineSize = m_fSampleRateInHz * m_fMaxDeleyLength;
    // init delay line counter
    m_delayLineCounter.delayLineHead = 0;
    m_delayLineCounter.delayLineSize = 0;

    m_pfDelayLine = new float*[m_iNumChannels];
    for (int i=0; i<m_iNumChannels; ++i)
        m_pfDelayLine[i] = new float[m_iDelayLineSize];

    m_bIsInitializedIntern = true;

    return kNoError;
}

Error_t CCombFilterBase::resetIntern ()
{
    // set member variables 0
    m_iNumChannels = 0;
    m_fMaxDeleyLength = 0;
    m_fSampleRateInHz = 0;
    m_iDelayLineSize = 0;
    m_fFilterGain = 0;

    // reset the delayLine and its counter
    if (m_bIsInitializedIntern) {
        for (int i=0; i<m_iNumChannels; ++i)
            delete[] m_pfDelayLine[i];
        delete[] m_pfDelayLine;
    }

    m_pfDelayLine = 0;
    m_delayLineCounter.delayLineHead = 0;
    m_delayLineCounter.delayLineSize = 0;

    return kNoError;
}


Error_t CCombFilterBase::setParamIntern( CCombFilterBase::FilterParam_t eParam, float fParamValue )
{
    switch (eParam) {
    case CCombFilterBase::kParamGain:
        m_fFilterGain = fParamValue;
        break;
    case CCombFilterBase::kParamDelay:
        if (fParamValue > m_fMaxDeleyLength || int(fParamValue * m_fSampleRateInHz) == 0)
            return kFunctionInvalidArgsError;
        m_fDelayTime = fParamValue;
        m_delayLineCounter.delayLineSize = fParamValue * m_fSampleRateInHz;
        break;
    default:
        return kFunctionInvalidArgsError;
    }
    return kNoError;
}

float CCombFilterBase::getParamIntern( FilterParam_t eParam ) const {
    switch (eParam) {
    case kParamGain:
        return m_fFilterGain;
    case kParamDelay:
        return m_fDelayTime;
    case kNumFilterParams:
        return 2;
    }
    return 0;
}

Error_t CCombFilterBase::processIntern( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
    (this->*m_processFunc)(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
    return kNoError;
}

void CCombFilterBase::FIRProcess (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    for (int i=0; i<iNumberOfFrames; ++i) {
        for (int c=0; c<m_iNumChannels; ++c) {
            ppfOutputBuffer[c][i] = ppfInputBuffer[c][i] + m_fFilterGain * m_pfDelayLine[c][m_delayLineCounter.delayLineHead];
            m_pfDelayLine[c][m_delayLineCounter.delayLineHead] = ppfInputBuffer[c][i];
        }
        m_delayLineCounter.delayLineHead = (m_delayLineCounter.delayLineHead + 1) % m_delayLineCounter.delayLineSize;
    }
}

void CCombFilterBase::IIRProcess (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    for (int i=0; i<iNumberOfFrames; ++i) {
        for (int c=0; c<m_iNumChannels; ++c) {
            ppfOutputBuffer[c][i] = ppfInputBuffer[c][i] + m_fFilterGain * m_pfDelayLine[c][m_delayLineCounter.delayLineHead];
            m_pfDelayLine[c][m_delayLineCounter.delayLineHead] = ppfOutputBuffer[c][i];
        }
        m_delayLineCounter.delayLineHead = (m_delayLineCounter.delayLineHead + 1) % m_delayLineCounter.delayLineSize;
    }
}

