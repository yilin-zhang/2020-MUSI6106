#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Lfo.h"

class CVibrato {
 public:
    /*! resetable parameters */
    enum VibParam_t {
        kParamDelay,
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
    \param float the maximum delay time
    \param float sample rate
    \return Error_t
    */
    Error_t init (float fMaxDelayTime, float fSampleRate) {
        // TODO Convert the unit of max_amp to Hz
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
        case kParamDelay:
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

    /*! get a paramter for the Vibrato instance
      \param eParam the parameter type
      \return float
    */
    float getParam (VibParam_t eParam) const {
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
        m_fSampleRate = 0;
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
    float              m_fSampleRate;
};

#endif // #if !defined(__Vibrato_hdr__)
