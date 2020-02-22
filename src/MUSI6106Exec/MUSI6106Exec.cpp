
#include <iostream>
#include <ctime>
#include <cmath>
#include <complex>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
//#include "CombFilterIf.h"
#include "Fft.h"

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

    static int              kBlockSize = 4096;
    static int              kHopSize = 2048;

    clock_t                 time = 0;

    // float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CFft                    *pCFft = nullptr;
    float                   **ppfReadBuffer = nullptr;

    float                   **ppfRingBuffer = nullptr;
    int                     iReadPos = 0;
    int                     iWritePos = 0;

    float                   **ppfBlockBuffer = nullptr;
    bool                    bIsFirstBlock = true;
    CFft::complex_t         **ppcFftOutput = nullptr;
    float                   **ppfFftMag = nullptr;

    //CCombFilterIf   *pInstance = 0;
    //CCombFilterIf::create(pInstance);
    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else if (argc < 3)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
    }
    else if (argc < 4)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        kBlockSize = std::stoi(argv[2]);
    }
    else if (argc < 5)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        kBlockSize = std::stoi(argv[2]);
        kHopSize = std::stoi(argv[3]);
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

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    // ppfAudioData = new float*[stFileSpec.iNumChannels];
    // for (int i = 0; i < stFileSpec.iNumChannels; i++)
    //     ppfAudioData[i] = new float[kBlockSize];

    ppfReadBuffer = new float*[stFileSpec.iNumChannels];
    for (int c = 0; c < stFileSpec.iNumChannels; c++) {
        ppfReadBuffer[c] = new float[kHopSize];
        for (int i = 0; i < kBlockSize; i++)
            ppfReadBuffer[c][i] = 0.f;
    }

    ppfRingBuffer = new float*[stFileSpec.iNumChannels];
    for (int c = 0; c < stFileSpec.iNumChannels; c++) {
        ppfRingBuffer[c] = new float[kBlockSize];
        for (int i = 0; i < kBlockSize; i++)
            ppfRingBuffer[c][i] = 0.f;
    }

    ppfBlockBuffer = new float*[stFileSpec.iNumChannels];
    for (int c = 0; c < stFileSpec.iNumChannels; c++)
        ppfBlockBuffer[c] = new float[kBlockSize];

    ppcFftOutput = new CFft::complex_t*[stFileSpec.iNumChannels];
    for (int c = 0; c < stFileSpec.iNumChannels; c++) {
        ppcFftOutput[c] = new CFft::complex_t[kBlockSize];
    }

    ppfFftMag = new float*[stFileSpec.iNumChannels];
    for (int c = 0; c < stFileSpec.iNumChannels; c++) {
        ppfFftMag[c] = new float[kBlockSize];
    }

    CFft::createInstance(pCFft);
    pCFft->initInstance(kBlockSize);

    iReadPos = 0;
    iWritePos = kBlockSize - kHopSize;

    time = clock();
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFile->isEof())
    {
        long long iNumFrames = kHopSize;
        // iShiftLength = kBlockSize - kHopSize;
        // TODO: last read
        phAudioFile->readData(ppfReadBuffer, iNumFrames);

        for (int i=0; i<iNumFrames; i++) {
            for (int c=0; c<stFileSpec.iNumChannels; c++) {
                ppfRingBuffer[c][iWritePos] = ppfReadBuffer[c][i];
            }
            iWritePos = (iWritePos + 1) % kBlockSize;
        }

        for (int i=0; i<kBlockSize-kHopSize+iNumFrames; i++) {
            for (int c=0; c<stFileSpec.iNumChannels; c++)
                ppfBlockBuffer[c][i] = abs(ppfRingBuffer[c][iReadPos]);
            iReadPos = (iReadPos + 1) % kBlockSize;
        }

        for (int i=kBlockSize-kHopSize+iNumFrames; i<kBlockSize; i++)
            for (int c=0; c<stFileSpec.iNumChannels; c++)
                ppfBlockBuffer[c][i] = 0;

        for (int c=0; c<stFileSpec.iNumChannels; c++) {
            pCFft->doFft(ppcFftOutput[c], ppfBlockBuffer[c]);
            pCFft->getMagnitude(ppfFftMag[c], ppcFftOutput[c]);
        }

        cout << "\r" << "reading and writing";

        //for (int i = kBlockSize/2-1; i < kBlockSize; i++)
        for (int i = 0; i < kBlockSize/2+1; i++)
        {
            hOutputFile << ppfFftMag[0][i] << "\t"; // convert it to amplitude
        }
        hOutputFile << endl;

        //iShiftLength = (iShiftLength + kHopSize) % kBlockSize;
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();

    // for (int i = 0; i < stFileSpec.iNumChannels; i++)
    //     delete[] ppfAudioData[i];
    // delete[] ppfAudioData;
    // ppfAudioData = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfReadBuffer[i];
    delete[] ppfReadBuffer;
    ppfReadBuffer = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfRingBuffer[i];
    delete[] ppfRingBuffer;
    ppfRingBuffer = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfBlockBuffer[i];
    delete[] ppfBlockBuffer;
    ppfBlockBuffer = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfFftMag[i];
    delete[] ppfFftMag;
    ppfFftMag = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppcFftOutput[i];
    delete[] ppcFftOutput;
    ppcFftOutput = 0;

    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

