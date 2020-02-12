#if !defined(__Lfo_hdr__)
#define __Lfo_hdr__

#include <cmath>
#include "ErrorDef.h"

class CLfo {
 public:
    /*! waveshapes of LFO*/
    enum LfoType_t {
        kSin,
        kSaw,
        kPulse
    };

    /*! resetable parameters */
    enum LfoParam_t {
        kParamAmp,
        kParamFreq,
    };

    /*! creates a new Lfo instance
    \param pCLfo pointer to the new class
    \return Error_t
    */
    static Error_t create (CLfo*& pCLfo) {
        pCLfo = new CLfo ();

        if (!pCLfo)
            return kUnknownError;

        return kNoError;
    }

    /*! destroys an Lfo instance
    \param pCLfo pointer to the class to be destroyed
    \return Error_t
    */
    static Error_t destroy (CLfo*& pCLfo) {
        if (!pCLfo)
            return kUnknownError;

        delete pCLfo;
        pCLfo = nullptr;

        return kNoError;
    }

    /*! initializes an Lfo instance
    \param eType the type of the LFO
    \param fMinFreq minimum frequency of the LFO
    \param fSampleRate sample rate in Hz of audio (should match the processed signal)
    \return Error_t
    */
    Error_t init (LfoType_t eType, float fMinFreq, float fSampleRate) {
        if (fMinFreq <= 0 || fSampleRate <= 0)
            return kFunctionInvalidArgsError;

        m_fSampleRate = fSampleRate;
        m_fMinFreq = fMinFreq;
        m_kLfoType = eType;

        // allocate memory
        m_iWavetableLength = ceil(fSampleRate / fMinFreq);
        m_pfWavetable = new float[m_iWavetableLength];

        m_bIsInitialized = true;

        return kNoError;
    }

    /*! set a paramter for the Lfo instance
    \param eParam the parameter type
    \param fParam the parameter value
    \return Error_t
    */
    Error_t setParam (LfoParam_t eParam, float fParam) {
        if (!m_bIsInitialized)
            return kNotInitializedError;

        switch (eParam) {
        case kParamAmp:
            m_fAmp = fParam;
            initWavetable(m_pfWavetable, fParam);
            break;
        case kParamFreq:
            if (fParam < m_fMinFreq)
                return kFunctionIllegalCallError;
            m_fFreq = fParam;
            m_iHop = round(m_fFreq / m_fMinFreq);
            break;
        default:
            return kFunctionIllegalCallError;
        }
        return kNoError;
    }

    /*! get a paramter for the Lfo instance
    \param eParam the parameter type
    \return float
    */
    float getParam (LfoParam_t eParam) const {
        if (!m_bIsInitialized) // indicates illeagal argument
            return -1;

        switch (eParam) {
        case kParamAmp:
            return m_fAmp;
        case kParamFreq:
            return m_fFreq;
        default:
            return -1; // indicates illeagal argument
        }
    }

    /*! resets the internal variables (requires new call of init)
    \return Error_t
    */
    Error_t reset (bool bFreeMemory) {
        if (bFreeMemory)
            delete[] m_pfWavetable;
        m_pfWavetable = nullptr;

        m_iWavetableLength = 0;
        m_iWavetablePos = 0;
        m_fSampleRate = 0;

        m_fMinFreq = 0;
        m_fAmp = 0;
        m_fFreq = 0;
        m_iHop = 0;

        m_bIsInitialized = false;

        return kNoError;
    }

    /*! resets the LFO pointer position to 0
    \return Error_t
    */
    Error_t resetPos() {
        m_iWavetablePos = 0;
        return kNoError;
    }

    /*! get the current LFO value
      \return float
    */
    float getValue() {
        int idx = m_iWavetablePos;
        m_iWavetablePos = (m_iWavetablePos + 1) % m_iWavetableLength;
        return m_pfWavetable[idx];
    }

protected:
    CLfo () {
        reset(false);
    };
    ~CLfo () {
        reset(true);
    };

private:
    static float sinFunc(int ind, int bufferLength) {
        return sin(float(ind) / float(bufferLength) * 2 * M_PI);
    }
    static float sawFunc(int ind, int bufferLength) {
        return (float(ind) / float(bufferLength)) * 2 - 1;
    }
    // TODO only support square wave for now
    static float pulseFunc(int ind, int bufferLength) {
        if (float(ind) / float(bufferLength) < 0.5)
            return -1;
        else
            return 1;
    }

    void initWavetable(float* pfWavetable, float amp) {
        // TODO try using lambda in the future
        float (*func)(int, int) = nullptr;
        switch (m_kLfoType) {
        case kSin:
            func = &sinFunc;
            break;
        case kSaw:
            func = &sawFunc;
            break;
        case kPulse:
            func = &pulseFunc;
            break;
        default:
            return;
        }

        for (int i=0; i<m_iWavetableLength; ++i) {
            pfWavetable[i] = func(i, m_iWavetableLength) * amp;
        }
    }

    bool m_bIsInitialized;
    LfoType_t m_kLfoType;
    float m_fSampleRate;

    float *m_pfWavetable;
    int m_iWavetableLength;
    int m_iWavetablePos;

    float m_fMinFreq; // minimum frequency in Hz
    float m_fAmp;
    float m_fFreq;    // current frequency in Hz
    int   m_iHop;
};

#endif // #if !defined(__Lfo_hdr__)
