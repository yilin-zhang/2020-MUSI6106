#include "MUSI6106Config.h"
#ifdef WITH_TESTS

#include <iostream>
#include "Vibrato.h"

#include "UnitTest++.h"

SUITE(Vibrato) {
    struct VibratoData {
        void allocMemory(float **&ppfBuffer, int iNumChannels, int iLength) {
            ppfBuffer = new float* [iNumChannels];
            for (int c=0; c<iNumChannels; ++c)
                ppfBuffer[c] = new float[iLength];
        }

        void deallocMemory(float **&ppfBuffer, int iNumChannels) {
            for (int c=0; c<iNumChannels; ++c)
                delete[] ppfBuffer[c];
            delete[] ppfBuffer;
            ppfBuffer = nullptr;
        }

        void audioProcess(float **audio, float **output, int numChannels, int blockSize, int audioLength) {
            float **inputBuffer = nullptr;
            float **outputBuffer = nullptr;

            inputBuffer = new float*[numChannels];
            outputBuffer = new float*[numChannels];

            for (int c=0; c<numChannels; c++) {
                inputBuffer[c]  = audio[c];
                outputBuffer[c] = output[c];
            }

            int startPoint = 0;
            int numFrames = 0;
            // process
            while (startPoint < audioLength) {
                for (int c=0; c<numChannels; ++c) {
                    inputBuffer[c] = audio[c] + startPoint;
                    outputBuffer[c] = output[c] + startPoint;
                }

                // startPoint + blockSize == audioLength in the perfect case
                if (audioLength - startPoint < blockSize)
                    numFrames = audioLength - startPoint;
                else
                    numFrames = blockSize;
                vib.process(inputBuffer, outputBuffer, numFrames);
                startPoint += blockSize;
            }
            delete[] inputBuffer;
            delete[] outputBuffer;
        }

        void initSetGet() {
            vib.init(10, 50, 2);
            vib.setParam(CVibrato::kParamFreq, 5.5);
            vib.setParam(CVibrato::kParamDelay, 0.2);
            vib.setParam(CVibrato::kParamBlockSize, 10.3);
            CHECK_CLOSE(5.5, vib.getParam(CVibrato::kParamFreq), 0.000001);
            CHECK_CLOSE(0.2, vib.getParam(CVibrato::kParamDelay), 0.000001);
            CHECK(10 == vib.getParam(CVibrato::kParamBlockSize));
        }

        void reset() {
            vib.reset();
            CHECK(0.0 == vib.getParam(CVibrato::kParamFreq));
            CHECK(0.0 == vib.getParam(CVibrato::kParamDelay));
            CHECK(0 == vib.getParam(CVibrato::kParamBlockSize));
        }

        void testZeroInput() {
            int blockSize = 20;
            int audioLength = 200;
            float sampleRate = 50;
            int numChannels = 2;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;

            vib.init(maxDelay, sampleRate, numChannels);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation and initialization
            float **audio = nullptr;
            allocMemory(audio, numChannels, audioLength);
            for (int i=0; i<audioLength; i++)
                for (int c=0; c<numChannels; c++)
                    audio[c][i] = 0;

            float **output = nullptr;
            allocMemory(output, numChannels, audioLength);
            for (int c=0; c<numChannels; c++)
                output[c] = new float[audioLength];

            // process
            audioProcess(audio, output, numChannels, blockSize, audioLength);

            // check
            for (int c=0; c<numChannels; c++)
                for (int i=0; i<audioLength; i++)
                    CHECK(0.0 == output[c][i]);

            // clean up
            deallocMemory(audio, numChannels);
            deallocMemory(output, numChannels);
        }

        void testZeroAmp() {
            int blockSize = 20;
            int audioLength = 200;
            float sampleRate = 50;
            int numChannels = 2;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0;

            vib.init(maxDelay, sampleRate, numChannels);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation and initialization
            float **audio = nullptr;
            allocMemory(audio, numChannels, audioLength);
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    audio[c][i] = (i % 5) / 10.f;

            float **output = nullptr;
            allocMemory(output, numChannels, audioLength);

            // Number of the delayed samples should be
            // ceil(maxDelay*sampleRate)
            float **refOutput = nullptr;
            allocMemory(refOutput, numChannels, audioLength);
            for (int i=0; i<audioLength; ++i) {
                for (int c=0; c<numChannels; ++c) {
                    if (i < ceil(maxDelay*sampleRate))
                        refOutput[c][i] = 0;
                    else
                        refOutput[c][i] = ((i - int(ceil(maxDelay*sampleRate))) % 5) / 10.f;
                }
            }

            // process
            audioProcess(audio, output, numChannels, blockSize, audioLength);

            // check
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    CHECK(refOutput[c][i] == output[c][i]);

            // clean up
            deallocMemory(audio, numChannels);
            deallocMemory(output, numChannels);
            deallocMemory(refOutput, numChannels);
        }

        void testDC() {
            int blockSize = 20;
            int audioLength = 200;
            float sampleRate = 50;
            int numChannels = 2;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;
            float valDC = 0.5;

            vib.init(maxDelay, sampleRate, numChannels);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation
            float **audio = nullptr;
            allocMemory(audio, numChannels, audioLength);
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    audio[c][i] = valDC;

            float **output = nullptr;
            allocMemory(output, numChannels, audioLength);

            float **refOutput = nullptr;
            allocMemory(refOutput, numChannels, audioLength);
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    refOutput[c][i] = valDC;

            // process
            audioProcess(audio, output, numChannels, blockSize, audioLength);

            // check
            // We cannot guarantee the samples before going through one the delayline
            // size have the DC value.
            // The delayline (ringbuffer) size is:
            // 2*ceil(maxDelay*sampleRate)
            for (int i=(2+2*ceil(maxDelay*sampleRate)); i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    CHECK(output[c][i] == refOutput[c][i]);

            // clean up
            deallocMemory(audio, numChannels);
            deallocMemory(output, numChannels);
            deallocMemory(refOutput, numChannels);
        }

        void testBlockSize () {
            int blockSize_1 = 7;
            int blockSize_2 = 13;
            int audioLength = 200;
            float sampleRate = 50;
            int numChannels = 1;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;
            int sampleCounter = 0;

            // Memory allocation
            float **audio = nullptr;
            allocMemory(audio, numChannels, audioLength);
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    audio[c][i] = (i % 5) / 10.f;

            float **output_1 = nullptr;
            allocMemory(output_1, numChannels, audioLength);

            float **output_2 = nullptr;
            allocMemory(output_2, numChannels, audioLength);

            float **inputBuffer = nullptr;
            float **outputBuffer = nullptr;
            inputBuffer = new float*[numChannels];
            outputBuffer = new float*[numChannels];

            vib.init(maxDelay, sampleRate, numChannels);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);

            // process 1: block size switch between blockSize_1 and blockSize_2
            for (int i=0; sampleCounter<audioLength; ++i) {
                inputBuffer[0] = audio[0] + sampleCounter;
                outputBuffer[0] = output_1[0] + sampleCounter;
                if (i%2 == 0) {
                    vib.setParam(CVibrato::kParamBlockSize, blockSize_1);
                    vib.process(inputBuffer, outputBuffer, blockSize_1);
                    sampleCounter += blockSize_1;
                } else {
                    vib.setParam(CVibrato::kParamBlockSize, blockSize_2);
                    vib.process(inputBuffer, outputBuffer, blockSize_2);
                    sampleCounter += blockSize_2;
                }
            }

            // reset
            sampleCounter = 0;
            vib.reset();

            // process 2: block size is fixed to (blockSize_1 + blockSize_2)
            vib.init(maxDelay, sampleRate, numChannels);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize_1 + blockSize_2);

            for (int i=0; sampleCounter<audioLength; ++i) {
                inputBuffer[0] = audio[0] + sampleCounter;
                outputBuffer[0] = output_2[0] + sampleCounter;
                vib.process(inputBuffer, outputBuffer, blockSize_1 + blockSize_2);
                sampleCounter += blockSize_1 + blockSize_2;
            }

            // check
            for (int i=0; i<audioLength; ++i)
                for (int c=0; c<numChannels; ++c)
                    CHECK(output_1[c][i] == output_2[c][i]);

            // clean up
            deallocMemory(audio, numChannels);
            deallocMemory(output_1, numChannels);
            deallocMemory(output_2, numChannels);
            delete[] inputBuffer;
            delete[] outputBuffer;
        }

        void testInvalidInputs () {
            Error_t err;
            // init
            err = vib.init(0, 0, 0);
            CHECK(err == kFunctionInvalidArgsError);

            // negative block size
            vib.init(0.2, 100, 1);
            err = vib.setParam(CVibrato::kParamBlockSize, -1);
            CHECK(err == kFunctionInvalidArgsError);

            // delay time is bigger than max delay time
            err = vib.setParam(CVibrato::kParamDelay, 0.2);
            CHECK(err == kNoError);
            err = vib.setParam(CVibrato::kParamDelay, 0.3);
            CHECK(err == kFunctionInvalidArgsError);

            // double-reset
            vib.reset();
            err = vib.reset();
            CHECK(err == kNotInitializedError);

            // set param without init
            err = vib.setParam(CVibrato::kParamBlockSize, 10);
            CHECK(err == kNotInitializedError);
        }
        CVibrato vib;
    };

    // Methods
    TEST_FIXTURE(VibratoData, InitTest) {
        VibratoData vibdata;
        vibdata.initSetGet();
        vibdata.reset();
    }

    TEST_FIXTURE(VibratoData, ZeroInputTest) {
        VibratoData vibdata;
        vibdata.testZeroInput();
    }

    // Output equals delayed input when modulation amplitude is 0.
    TEST_FIXTURE(VibratoData, ZeroAmpTest) {
        VibratoData vibdata;
        vibdata.testZeroAmp();
    }

    // DC input stays DC ouput regardless of parametrization.
    TEST_FIXTURE(VibratoData, DCInputTest) {
        VibratoData vibdata;
        vibdata.testDC();
    }

    TEST_FIXTURE(VibratoData, BlockSizeTest) {
        VibratoData vibdata;
        vibdata.testBlockSize();
    }

    TEST_FIXTURE(VibratoData, InvalidInputsTest) {
        VibratoData vibdata;
        vibdata.testInvalidInputs();
    }
}
#endif //WITH_TESTS
