
// standard headers

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

static const char*  kCMyProjectBuildDate             = __DATE__;


CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_fSampleRate(0)
{
    // this never hurts
    this->reset ();
}


CCombFilterIf::~CCombFilterIf ()
{
    this->reset ();
}

const int  CCombFilterIf::getVersion (const Version_t eVersionIdx)
{
    int iVersion = 0;

    switch (eVersionIdx)
    {
    case kMajor:
        iVersion    = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion    = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion    = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion    = -1;
        break;
    }

    return iVersion;
}
const char*  CCombFilterIf::getBuildDate ()
{
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create( CCombFilterIf*& pCCombFilter)
{
    pCCombFilter = new CCombFilterBase();

    if (!pCCombFilter)
        return kMemError;

    return kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilter)
{
    delete pCCombFilter;
    pCCombFilter = 0;

    return kNoError;
}

Error_t CCombFilterIf::init( CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels )
{

    Error_t error_init = initIntern(eFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    if (error_init == kNoError)
        m_bIsInitialized = true;

    return error_init;
}

Error_t CCombFilterIf::reset ()
{
    if (m_bIsInitialized) {
        Error_t error = resetIntern();
        if (error == kNoError)
            m_bIsInitialized = false;
        return error;
    }
    return kNoError;
}

Error_t CCombFilterIf::process( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
    Error_t error = processIntern(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
    return error;
}

Error_t CCombFilterIf::setParam( FilterParam_t eParam, float fParamValue )
{
    // TODO: Error checking
    // maxdelay
    Error_t error = setParamIntern(eParam, fParamValue);
    return error;
}

float CCombFilterIf::getParam( FilterParam_t eParam ) const
{
    float param = getParamIntern(eParam);
    return param;
}
