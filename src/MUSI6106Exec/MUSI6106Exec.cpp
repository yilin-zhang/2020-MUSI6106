
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"

using std::cout;
using std::cerr;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputTextPath,
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;
    float                   **ppfOutputData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputText;
    CAudioFileIf::FileSpec_t stFileSpec;

    float                   fDelayTime = 0;
    float                   fVibFreq = 0;
    CAudioFileIf            *phOutputAudioFile = 0;
    CVibrato                vib;

    //CCombFilterIf   *pInstance = 0;
    //CCombFilterIf::create(pInstance);
    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 4)
    {
        cerr << "Missing arugment(s)!" << endl;
        cerr << "arg1: audio file path\narg2: delay time in s\narg3: vibrato frequency in Hz" << endl;
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputTextPath = sInputFilePath + ".txt";
        sOutputFilePath = sInputFilePath.substr(0, sInputFilePath.size()-4) + "_output_cpp.wav";
        fDelayTime = std::stof(argv[2]);
        fVibFreq = std::stof(argv[3]);
        if (fDelayTime <= 0 || fVibFreq <= 0) {
            cerr << "Invalid argument(s)." << endl;
            return -1;
        }
        cout << "Output Path: " << sOutputFilePath << endl;
        cout << "Delay Time: " << fDelayTime << " s" << endl;
        cout << "Vibrato Frequency: " << fVibFreq << " Hz" << endl;
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
    hOutputText.open(sOutputTextPath.c_str(), std::ios::out);
    if (!hOutputText.is_open())
    {
        cout << "Text file open error!";
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

    ppfOutputData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputData[i] = new float[kBlockSize];

    //////////////////////////////////////////////////////////////////////////////
    // open the output wave file
    CAudioFileIf::create(phOutputAudioFile);
    phOutputAudioFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputAudioFile->isOpen())
    {
        cout << "Output Wave file open error!";
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // initialize vibrato
    vib.init(fDelayTime, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    vib.setParam(CVibrato::kParamDelay, fDelayTime);
    vib.setParam(CVibrato::kParamFreq, fVibFreq);
    vib.setParam(CVibrato::kParamBlockSize, kBlockSize);

    time = clock();
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFile->isEof())
    {
        long long iNumFrames = kBlockSize;
        phAudioFile->readData(ppfAudioData, iNumFrames);

        cout << "\r" << "reading and writing";

        vib.process(ppfAudioData, ppfOutputData, iNumFrames);

        phOutputAudioFile->writeData(ppfOutputData, iNumFrames);

        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputText << ppfAudioData[c][i] << "\t";
            }
            hOutputText << endl;
        }
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    hOutputText.close();

    CAudioFileIf::destroy(phOutputAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = nullptr;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfOutputData[i];
    delete[] ppfOutputData;
    ppfOutputData = nullptr;

    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

