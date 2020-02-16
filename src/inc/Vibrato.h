#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Lfo.h"

class CVibrato {
 public:
    /*! resetable parameters */
    enum VibParam_t {
        kParamAmp,
        kParamFreq,
        kParamBlockSize, // block size can be changed after the init
    };

    CVibrato() {
        resetIntern(false);
    }

    ~CVibrato() {
        resetIntern(true);
    }

    /*! initializes a vibrato filter instance
    \return Error_t
    */
    Error_t init (int iMaxVibAmp, float fSampleRate) {
        // TODO Convert the unit of max_amp to Hz
        if (m_bIsInitialized)
            return kFunctionIllegalCallError;
        // TODO I forced it to use sample rate to be the wave length
        auto err = m_Lfo.init(CLfo::kSin, fSampleRate, fSampleRate);
        if (err != kNoError)
            return err;
        m_iMaxVibAmp = iMaxVibAmp;
        m_iDelayLineSize = 2 * iMaxVibAmp + 2;
        m_delayLine = new CRingBuffer<float> (m_iDelayLineSize);
        m_bIsInitialized = true;

        return kNoError;
    }

    /*! set a paramter for the Vibrato instance
    \param eParam the parameter type
    \param fParam the parameter value
    \return Error_t
    */
    Error_t setParam (VibParam_t eParam, float fParam) {
        if (!m_bIsInitialized)
            return kNotInitializedError;

        Error_t err;

        switch (eParam) {
        case kParamAmp:
            err = m_Lfo.setParam(CLfo::kParamAmp, fParam);
            break;
        case kParamFreq:
            err = m_Lfo.setParam(CLfo::kParamFreq, fParam);
            break;
        case kParamBlockSize:
            m_iBlockSize = int(fParam);
        default:
            return kFunctionIllegalCallError;
        }
        return err;
    }

    /*! get a paramter for the Vibrato instance
      \param eParam the parameter type
      \return float
    */
    float getParam (VibParam_t eParam) const {
        switch (eParam) {
        case kParamAmp:
            return m_Lfo.getParam(CLfo::kParamAmp);
        case kParamFreq:
            return m_Lfo.getParam(CLfo::kParamFreq);
        case kParamBlockSize:
            return m_iBlockSize;
        default:
            return 0;
        }
    }

    /*! resets the internal variables (can only be called when it's not iniialized, requires new call of init)
    \return Error_t
    */
    Error_t reset () {
        if (!m_bIsInitialized)
            return kNotInitializedError;
        else
            return resetIntern(true);
    }


    /*! processes a block of data
    \param pfInputBuffer a pointer to the input buffer
    \param pfOutputBuffer a pointer to the output buffer
    \return Error_t
    */
    Error_t process (float *pfInputBuffer, float *pfOutputBuffer) {
        // TODO maybe remove the if statement in the future
        int offset = 0;
        int lastWriteIdx = 0;
        int bufferWriteIdx = 0; // save the buffer position before LFO is enabled

        if (!m_bIsLfoEnabled) {
            // resume from the last write point
            lastWriteIdx = m_delayLine->getWriteIdx();
            for (int i=0; i<m_iDelayLineSize-lastWriteIdx-1 && i<m_iBlockSize; ++i) {
                m_delayLine->putPostInc(pfInputBuffer[i]);
                pfOutputBuffer[i] = 0;
                bufferWriteIdx++;
            }
            // if the write pointer is pointing to the last cell,
            // then move the read pointer to the middle, and enable
            // the LFO
            if (m_delayLine->getWriteIdx() == m_iDelayLineSize - 1) {
                for (int i=0; i<m_iDelayLineSize-m_iMaxVibAmp-2; ++i)
                    m_delayLine->getPostInc();
                m_bIsLfoEnabled = true;
            }
        }

        if (m_bIsLfoEnabled && bufferWriteIdx<m_iBlockSize) {
            for (int i=bufferWriteIdx; i<m_iBlockSize; ++i) {
                pfOutputBuffer[i] = m_delayLine->get(m_Lfo.getValue());
                m_delayLine->getPostInc(); // increment the read index
                m_delayLine->putPostInc(pfInputBuffer[i]);
            }
        }
    }

private:
    Error_t resetIntern (bool bFreeMemory) {
        if (bFreeMemory)
            delete m_delayLine;
        m_delayLine = nullptr;

        auto err = m_Lfo.reset(bFreeMemory);
        if (err != kNoError)
            return err;

        m_iBlockSize = 0;
        m_iDelayLineSize = 0;
        m_bIsLfoEnabled = false;
        m_bIsInitialized = false;
        return kNoError;
    }

    bool               m_bIsInitialized;
    CRingBuffer<float> *m_delayLine;
    int                m_iDelayLineSize;
    CLfo               m_Lfo;
    bool               m_bIsLfoEnabled; // only enabling LFO when we have enough samples
    int                m_iBlockSize;
    int                m_iMaxVibAmp;
};

#endif // #if !defined(__Vibrato_hdr__)
