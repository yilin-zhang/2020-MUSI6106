
#include <iostream>
#include <ctime>
#include <fstream>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

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

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    (*phAudioFile).openFile(
        sInputFilePath,
        CAudioFileIf::FileIoType_t::kFileRead,
        &stFileSpec
    );
 
    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    std::ofstream textFile(sOutputFilePath);
 
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    const int iNumChannels = 2;
    ppfAudioData = (float**) malloc(sizeof(float*) * iNumChannels);
    *ppfAudioData = (float*) malloc(sizeof(float) * kBlockSize);
    *(ppfAudioData+1) = (float*) malloc(sizeof(float) * kBlockSize);
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    long long int iNumFrames = kBlockSize;
    while(true) {
        // read one block
        (*phAudioFile).readData(ppfAudioData, iNumFrames);
        for (int i=0; i<iNumFrames; ++i) {
            // read frame i and write into file as a line
            for (int j=0; j<iNumChannels; ++j) {
                textFile << ppfAudioData[j][i];
                if (j != iNumChannels-1)
                    textFile << "\t";
                else
                    textFile << endl;
            }
        }
        // break if this is the last block
        if(iNumFrames < kBlockSize)
            break;
    }

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    textFile.close();
    for (int i=0; i<iNumChannels; ++i)
        free(*(ppfAudioData+i));
    free(ppfAudioData);

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

