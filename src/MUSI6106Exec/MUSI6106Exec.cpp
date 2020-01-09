
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
    long long int iNumFrames = 44100 * 3;
    ppfAudioData = (float**) malloc(sizeof(float*)*2);
    *ppfAudioData = (float*) malloc(sizeof(float)*iNumFrames);
    *(ppfAudioData+1) = (float*) malloc(sizeof(float)*iNumFrames);
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    float line[2];
    (*phAudioFile).readData(ppfAudioData, iNumFrames);
    for (int i=0; i<iNumFrames; ++i) {
        // read 2 channels at frame i
        for (int j=0; j<2; ++j) {
            line[j] = ppfAudioData[j][i];
        }
        // write a line into the file
        textFile << line[0] << "\t" << line[1] << endl;
    }

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    textFile.close();
    free(*ppfAudioData);
    free(*(ppfAudioData+1));
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

