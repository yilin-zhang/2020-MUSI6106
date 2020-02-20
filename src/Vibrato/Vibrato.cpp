// project headers
#include "Vibrato.h"
#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Lfo.h"

CVibrato::CVibrato() {
    resetIntern(false);
}

CVibrato::~CVibrato() {
    resetIntern(true);
}

Error_t CVibrato::init (float fMaxDelayTime, float fSampleRate, int iNumChannels = 1) {
    if (m_bIsInitialized)
        return kFunctionIllegalCallError;
    // TODO I forced it to use sample rate to be the wave length
    auto err = m_Lfo.init(CLfo::kSin, fSampleRate, fSampleRate);
    if (err != kNoError)
        return err;
    if (iNumChannels < 1)
        return kFunctionInvalidArgsError;

    m_fSampleRate = fSampleRate;
    m_iNumChannels = iNumChannels;
    m_iMaxVibAmp = ceil(fMaxDelayTime * fSampleRate);
    m_iDelayLineSize = 2 * m_iMaxVibAmp + 2;

    m_ppDelayLine = new CRingBuffer<float>* [iNumChannels];
    for (int i=0; i<iNumChannels; ++i)
        m_ppDelayLine[i] = new CRingBuffer<float> (m_iDelayLineSize);

    m_bIsInitialized = true;

    return kNoError;
}

Error_t CVibrato::setParam (VibParam_t eParam, float fParam) {
    if (!m_bIsInitialized)
        return kNotInitializedError;

    Error_t err;

    switch (eParam) {
    case kParamDelay:
        if (ceil(fParam*m_fSampleRate) > m_iMaxVibAmp)
            return kFunctionInvalidArgsError;
        err = m_Lfo.setParam(CLfo::kParamAmp, fParam*m_fSampleRate);
        break;
    case kParamFreq:
        err = m_Lfo.setParam(CLfo::kParamFreq, fParam);
        break;
    case kParamBlockSize:
        if (fParam <= 0)
            return kFunctionInvalidArgsError;
        m_iBlockSize = int(fParam);
    default:
        return kFunctionIllegalCallError;
    }
    return err;
}

float CVibrato::getParam (VibParam_t eParam) const {
    switch (eParam) {
    case kParamDelay:
        if (m_fSampleRate < 0.000001) {
            return 0;
        }
        return m_Lfo.getParam(CLfo::kParamAmp)/m_fSampleRate;
    case kParamFreq:
        return m_Lfo.getParam(CLfo::kParamFreq);
    case kParamBlockSize:
        return m_iBlockSize;
    default:
        return 0;
    }
}

Error_t CVibrato::reset () {
    if (!m_bIsInitialized)
        return kNotInitializedError;
    else
        return resetIntern(true);
}

Error_t CVibrato::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumFrames) {
    float lfoValue = 0.f;
    // move the read pointer to the middle
    // e.g. m_iDelayLineSize == 12
    // [*------*----]
    //  |      |
    // write  read
    //
    // The read pointer should always be behind, so actually it moves
    // the read pointer back.
    if (!m_bIsLfoEnabled) {
        for (int i=0; i<m_iDelayLineSize-m_iMaxVibAmp; ++i)
            for (int c=0; c<m_iNumChannels; ++c)
                m_ppDelayLine[c]->getPostInc();
        m_bIsLfoEnabled = true;
    }

    // Based on the MATLAB implementation, write first, read second
    for (int i=0; i<m_iBlockSize && i<iNumFrames; ++i) {
        lfoValue = m_Lfo.getValue();
        for (int c=0; c<m_iNumChannels; ++c) {
            m_ppDelayLine[c]->putPostInc(ppfInputBuffer[c][i]);
            ppfOutputBuffer[c][i] = m_ppDelayLine[c]->get(lfoValue);
            m_ppDelayLine[c]->getPostInc(); // increment the read index
        }
    }
}

Error_t CVibrato::resetIntern (bool bFreeMemory) {
    if (bFreeMemory) {
        // for (int i=0; i<m_iNumChannels; ++i)
        //     delete m_ppDelayLine[i];
        delete[] m_ppDelayLine;
    }
    m_ppDelayLine = nullptr;

    auto err = m_Lfo.reset(bFreeMemory);
    if (err != kNoError)
        return err;

    m_iBlockSize = 0;
    m_iDelayLineSize = 0;
    m_iNumChannels = 0;
    m_fSampleRate = 0;
    m_bIsLfoEnabled = false;
    m_bIsInitialized = false;
    return kNoError;
}
