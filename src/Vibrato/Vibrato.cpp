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

Error_t CVibrato::init (float fMaxDelayTime, float fSampleRate) {
    if (m_bIsInitialized)
        return kFunctionIllegalCallError;
    // TODO I forced it to use sample rate to be the wave length
    auto err = m_Lfo.init(CLfo::kSin, fSampleRate, fSampleRate);
    if (err != kNoError)
        return err;
    m_fSampleRate = fSampleRate;
    m_iMaxVibAmp = ceil(fMaxDelayTime * fSampleRate);
    m_iDelayLineSize = 2 * m_iMaxVibAmp + 2;
    m_delayLine = new CRingBuffer<float> (m_iDelayLineSize);
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

Error_t CVibrato::process (float *pfInputBuffer, float *pfOutputBuffer) {
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
            m_delayLine->getPostInc();
        m_bIsLfoEnabled = true;
    }

    // Based on the MATLAB implementation, write first, read second
    for (int i=0; i<m_iBlockSize; ++i) {
        m_delayLine->putPostInc(pfInputBuffer[i]);
        pfOutputBuffer[i] = m_delayLine->get(m_Lfo.getValue());
        m_delayLine->getPostInc(); // increment the read index
    }
}

Error_t CVibrato::resetIntern (bool bFreeMemory) {
    if (bFreeMemory)
        delete m_delayLine;
    m_delayLine = nullptr;

    auto err = m_Lfo.reset(bFreeMemory);
    if (err != kNoError)
        return err;

    m_iBlockSize = 0;
    m_iDelayLineSize = 0;
    m_fSampleRate = 0;
    m_bIsLfoEnabled = false;
    m_bIsInitialized = false;
    return kNoError;
}
