#include <iostream>
#include <ctime>
#include <fstream>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CAudioFileIf            *phOutputAudioFile = 0;

    float                   **ppfDelayedData = 0;

    CCombFilterIf           *pCombFilter = 0;
    // TODO: Make these command line args?
    float                   fMaxDelayLength = 1;
    float                   fFilterGain = 0.5;
    float                   fFilterDelay = 0.5;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
        {
            sInputFilePath = argv[1];
            sOutputFilePath = sInputFilePath + ".txt";
        }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        return -1;
    }

    // open the output wave file
    CAudioFileIf::create(phOutputAudioFile);
    // TODO: this address is just a placeholder
    phOutputAudioFile->openFile("/Users/yilin/Desktop/test.wav", CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputAudioFile->isOpen())
    {
        cout << "Output Wave file open error!";
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

    ppfDelayedData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfDelayedData[i] = new float[kBlockSize];

    Error_t error = CCombFilterIf::create(pCombFilter);
    pCombFilter->init(CCombFilterIf::kCombFIR, fMaxDelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    pCombFilter->setParam(CCombFilterIf::kParamGain, fFilterGain);
    pCombFilter->setParam(CCombFilterIf::kParamDelay, fFilterDelay);

    time = clock();
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFile->isEof())
    {
        long long iNumFrames = kBlockSize;
        phAudioFile->readData(ppfAudioData, iNumFrames);

        cout << "\r" << "reading and writing";

        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile << ppfAudioData[c][i] << "\t";
            }
            hOutputFile << endl;
        }
        pCombFilter->process(ppfAudioData, ppfDelayedData, iNumFrames);
        phOutputAudioFile->writeData(ppfDelayedData, iNumFrames);
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();
    phOutputAudioFile->closeFile();
    CAudioFileIf::destroy(phOutputAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfDelayedData[i];
    delete[] ppfDelayedData;
    ppfDelayedData = 0;

    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}
