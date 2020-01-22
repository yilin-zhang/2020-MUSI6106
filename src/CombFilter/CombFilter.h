#if !defined(__CombFilter_hdr__)
#define __CombFilter_hdr__

#include "CombFilterIf.h"

class CCombFilterBase : public CCombFilterIf
{
public:
    CCombFilterBase ();
    virtual ~CCombFilterBase ();

    //Error_t init (int iDelayLineSize, int iNumChannels);
    Error_t initIntern( CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels ) override;
    Error_t resetIntern() override;

    Error_t processIntern( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames ) override;
    Error_t setParamIntern( FilterParam_t eParam, float fParamValue ) override;
    float getParamIntern( FilterParam_t eParam ) const override;

private:
    CCombFilterBase(const CCombFilterBase& that);
    bool            m_bIsInitializedIntern;   //!< internal bool to check whether the init function has been called
    CombFilterType_t m_eFilterType;
    void (CCombFilterBase::*m_processFunc) (float**, float**, int);

    int   m_iNumChannels;
    float m_fMaxDeleyLength;  //!< max delay length in s
    float m_fSampleRateInHz;
    int   m_iDelayLineSize;
    float **m_pfDelayLine;

    struct DelayLineCounter {
        int   delayLineHead;
        int   delayLineSize;
    } m_delayLineCounter;

    // parameters for users to set after the initialization
    float m_fFilterGain;
    int   m_fDelayTime;

    void FIRProcess (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);
    void IIRProcess (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);

};

#endif // #if !defined(__CombFilter_hdr__)
