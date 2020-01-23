#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using namespace std;

// local function declarations
void    showClInfo ();

// Tests
void testFIRInputFreqFeedforward ();
void testIIRMagnitudeChange ();
void testVaryingBlockSize ();
void testZeroInput ();
void testReset ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    // testFIRInputFreqFeedforward();
    // testIIRMagnitudeChange();
    // testVaryingBlockSize();
    testZeroInput();
    // testReset();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    // if (argc < 2)
    //     {
    //         cout << "Missing audio input path!";
    //         return -1;
    //     }
    // else
    //     {
    //         sInputFilePath = argv[1];
    //         sOutputFilePath = sInputFilePath + ".txt";
    //     }
    return 0;
}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

//////////////////////////////////////////////////////////////////////////////
// Main logic
typedef struct {
    std::string inputAudioPath;
    std::string outputAudioPath;
    int blockSize;
    CCombFilterIf::CombFilterType_t filterType;
    float delayGain;
    float delayTime;
} Args;

int mainProcess (Args argCollection)
{
    // input/output file paths
    std::string             sInputFilePath = argCollection.inputAudioPath;
    std::string             sOutputFilePath = argCollection.outputAudioPath;
    std::string             sOutputTxtPath = sOutputFilePath + ".txt";

    // block size
    static const int        kBlockSize = argCollection.blockSize;

    // input file/buffer pointers, spec
    float                   **ppfAudioData = 0;
    CAudioFileIf            *phAudioFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;

    // output text file
    std::fstream            hOutputTxt;

    // output file/buffer pointers
    float                   **ppfDelayedData = 0;
    CAudioFileIf            *phOutputAudioFile = 0;

    // filter
    CCombFilterIf           *pCombFilter = 0;
    float                   fMaxDelayLength = argCollection.delayTime;
    float                   fFilterGain = argCollection.delayGain;
    float                   fFilterDelay = argCollection.delayTime;

    //clock
    clock_t                 time = 0;

    showClInfo();

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
    hOutputTxt.open(sOutputTxtPath.c_str(), std::ios::out);
    if (!hOutputTxt.is_open())
    {
        cout << "Text file open error!";
        return -1;
    }

    // open the output wave file
    CAudioFileIf::create(phOutputAudioFile);
    phOutputAudioFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
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

        pCombFilter->process(ppfAudioData, ppfDelayedData, iNumFrames);
        phOutputAudioFile->writeData(ppfDelayedData, iNumFrames);

        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputTxt << ppfDelayedData[c][i] << "\t";
            }
            hOutputTxt << endl;
        }
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    hOutputTxt.close();
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

//////////////////////////////////////////////////////////////////////////////
// Tests

// Test Case 1
void testFIRInputFreqFeedforward () {
    cout << "Test 1 Starts" << endl;

    Args args;
    args.inputAudioPath = "/Users/yilin/Desktop/sine_256.wav";
    args.outputAudioPath = "/Users/yilin/Desktop/sine_256_fir_output.wav";
    args.blockSize = 1024;
    args.filterType = CCombFilterIf::kCombFIR;
    args.delayGain = -1;
    args.delayTime = 1;
    mainProcess(args);

}

// Test Case 2
void testIIRMagnitudeChange () {
    cout << "Test 2 Starts" << endl;

    Args args;
    args.inputAudioPath = "/Users/yilin/Desktop/sine_256.wav";
    args.outputAudioPath = "/Users/yilin/Desktop/sine_256_iir_output.wav";
    args.blockSize = 1024;
    args.filterType = CCombFilterIf::kCombIIR;
    args.delayGain = -1;
    args.delayTime = 1;
    mainProcess(args);
}

// Test Case 3
void testVaryingBlockSize () {
    cout << "Test 3 Starts" << endl;
    // initialization
    string inputAudioPath = "/Users/yilin/Desktop/sweep.wav";
    vector<string> outputAudioPathsFIR;
    outputAudioPathsFIR.push_back("/Users/yilin/Desktop/sweep_output_fir_1.wav");
    outputAudioPathsFIR.push_back("/Users/yilin/Desktop/sweep_output_fir_2.wav");
    outputAudioPathsFIR.push_back("/Users/yilin/Desktop/sweep_output_fir_3.wav");
    vector<string> outputAudioPathsIIR;
    outputAudioPathsIIR.push_back("/Users/yilin/Desktop/sweep_output_iir_1.wav");
    outputAudioPathsIIR.push_back("/Users/yilin/Desktop/sweep_output_iir_2.wav");
    outputAudioPathsIIR.push_back("/Users/yilin/Desktop/sweep_output_iir_3.wav");
    vector<int> blockSizes;
    blockSizes.push_back(256);
    blockSizes.push_back(512);
    blockSizes.push_back(1024);
    float delayGain = 0.5;
    float delayTime = 0.5;

    vector<Args> vArgsFIR;
    vector<Args> vArgsIIR;
    Args a;

    for (int i=0; i<3; ++i) {
        a.inputAudioPath = inputAudioPath;
        a.outputAudioPath = outputAudioPathsFIR[i];
        a.blockSize = blockSizes[i];
        a.filterType = CCombFilterIf::kCombFIR;
        a.delayGain = delayGain;
        a.delayTime = delayTime;
        vArgsFIR.push_back(a);
    }
    for (int i=0; i<3; ++i) {
        a.inputAudioPath = inputAudioPath;
        a.outputAudioPath = outputAudioPathsIIR[i];
        a.blockSize = blockSizes[i];
        a.filterType = CCombFilterIf::kCombIIR;
        a.delayGain = delayGain;
        a.delayTime = delayTime;
        vArgsIIR.push_back(a);
    }

    mainProcess(vArgsFIR[0]);
    mainProcess(vArgsFIR[1]);
    mainProcess(vArgsFIR[2]);
}

// Test Case 4
void testZeroInput () {
    cout << "Test 4 Starts" << endl;
    Args args;
    args.inputAudioPath = "/Users/yilin/Desktop/silence.wav";
    args.outputAudioPath = "/Users/yilin/Desktop/silence_output.wav";
    args.blockSize = 1024;
    args.filterType = CCombFilterIf::kCombIIR;
    args.delayGain = 0.5;
    args.delayTime = 0.5;
    mainProcess(args);
}

// Test Case 5
void testReset () {
    cout << "Test 5 Starts" << endl;

    float initGain = 1;
    float initDelay = 0.8;
    float resetGain = 0.5;
    float resetDelay = 0.2;

    CCombFilterIf *pCombFilter = 0;

    Error_t error = CCombFilterIf::create(pCombFilter);
    pCombFilter->init(CCombFilterIf::kCombFIR, 1, 44100, 2);
    pCombFilter->setParam(CCombFilterIf::kParamGain, initGain);
    pCombFilter->setParam(CCombFilterIf::kParamDelay, initDelay);

    float gain_1 = pCombFilter->getParam(CCombFilterIf::kParamGain);
    float delay_1 = pCombFilter->getParam(CCombFilterIf::kParamDelay);

    if (gain_1 != initGain) {
        cout << "Got wrong gain value!" << endl;
        cout << "Got gain: " << gain_1 << endl;
    }

    if (delay_1 != initDelay) {
        cout << "Got wrong delay value!" << endl;
        cout << "Got delay: " << delay_1 << endl;
    }

    pCombFilter->setParam(CCombFilterIf::kParamGain, resetGain);
    pCombFilter->setParam(CCombFilterIf::kParamDelay, resetDelay);

    float gain_2 = pCombFilter->getParam(CCombFilterIf::kParamGain);
    float delay_2 = pCombFilter->getParam(CCombFilterIf::kParamDelay);

    if (gain_2 != resetGain) {
        cout << "Got wrong gain value after reset!" << endl;
        cout << "Got gain: " << gain_2 << endl;
    }
    if (delay_2 != resetDelay) {
        cout << "Got wrong delay value!" << endl;
        cout << "Got delay: " << delay_2 << endl;
    }

}
