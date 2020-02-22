/*
The design of this interface is generally copied from CombFilterIf.

In order to support varying vibration frequency, width and block size while
processing, I make these parameters to be initialized in `setParam' method,
instead of `init'.

The user has to initialize the maximum delay time and sample rate in order
to create a delay line with a proper size.
*/

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

    CVibrato();

    ~CVibrato();

    /*! initializes a vibrato filter instance
    \param float the maximum delay time
    \param float sample rate
    \param int number of audio channels
    \return Error_t
    */
    Error_t init (float fMaxDelayTime, float fSampleRate, int iNumChannels);

    /*! set a paramter for the Vibrato instance
    \param eParam the parameter type
    \param fParam the parameter value
    \return Error_t
    */
    Error_t setParam (VibParam_t eParam, float fParam);

    /*! get a paramter for the Vibrato instance
    \param eParam the parameter type
    \return float
    */
    float getParam (VibParam_t eParam) const;

    /*! resets the internal variables (can only be called when it's not iniialized, requires new call of init)
    \return Error_t
    */
    Error_t reset ();

    /*! processes a block of data
    \param pfInputBuffer a pointer to the input buffer
    \param pfOutputBuffer a pointer to the output buffer
    \return Error_t
    */
    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumFrames);

private:
    Error_t resetIntern (bool bFreeMemory);

    bool               m_bIsInitialized;
    CRingBuffer<float> **m_ppDelayLine;
    int                m_iDelayLineSize;
    int                m_iNumChannels;
    CLfo               m_Lfo;
    int                m_iBlockSize;
    int                m_iMaxVibAmp;
    float              m_fSampleRate;
};

#endif // #if !defined(__Vibrato_hdr__)
