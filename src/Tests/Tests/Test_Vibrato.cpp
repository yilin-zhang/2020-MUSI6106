#include "MUSI6106Config.h"
#ifdef WITH_TESTS

#include <iostream>
#include "Vibrato.h"

#include "UnitTest++.h"

SUITE(Vibrato) {
    struct VibratoData {
        void initSetGet() {
            vib.init(10, 50);
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
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;

            vib.init(maxDelay, sampleRate);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation
            float *audio = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                audio[i] = 0;
            }

            float *output = new float[audioLength];

            float *inputBuffer = nullptr;
            float *outputBuffer = nullptr;

            // process
            for (int i=0; i<(audioLength/blockSize); ++i) {
                inputBuffer = audio + i*blockSize;
                outputBuffer = output + i*blockSize;
                vib.process(inputBuffer, outputBuffer);
            }

            // check
            for (int i=0; i<audioLength; ++i) {
                CHECK(0.0 == output[i]);
            }

            // clean up
            delete[] audio;
            delete[] output;
            inputBuffer = nullptr;
            outputBuffer = nullptr;
        }

        void testZeroAmp() {
            int blockSize = 20;
            int audioLength = 200;
            float sampleRate = 50;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0;

            vib.init(maxDelay, sampleRate);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation
            float *audio = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                audio[i] = (i % 5) / 10.f;
            }

            float *output = new float[audioLength];

            // Number of the delayed samples should be
            // ceil(maxDelay*sampleRate)
            float *refOutput = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                if (i < ceil(maxDelay*sampleRate))
                    refOutput[i] = 0;
                else
                    refOutput[i] = ((i - int(ceil(maxDelay*sampleRate))) % 5) / 10.f;
            }

            float *inputBuffer = nullptr;
            float *outputBuffer = nullptr;

            // process
            for (int i=0; i<(audioLength/blockSize); ++i) {
                inputBuffer = audio + i*blockSize;
                outputBuffer = output + i*blockSize;
                vib.process(inputBuffer, outputBuffer);
            }

            // check
            for (int i=0; i<audioLength; ++i)
                CHECK(refOutput[i] == output[i]);

            // clean up
            delete[] audio;
            delete[] output;
            delete[] refOutput;
            inputBuffer = nullptr;
            outputBuffer = nullptr;
        }

        void testDC() {
            int blockSize = 20;
            int audioLength = 200;
            float sampleRate = 50;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;
            float valDC = 0.5;

            vib.init(maxDelay, sampleRate);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize);

            // Memory allocation
            float *audio = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                audio[i] = valDC;
            }

            float *output = new float[audioLength];

            float *refOutput = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                refOutput[i] = valDC;
            }

            float *inputBuffer = nullptr;
            float *outputBuffer = nullptr;

            // process
            for (int i=0; i<(audioLength/blockSize); ++i) {
                inputBuffer = audio + i*blockSize;
                outputBuffer = output + i*blockSize;
                vib.process(inputBuffer, outputBuffer);
            }

            // check
            // We cannot guarantee the samples before going through one the delayline
            // size have the DC value.
            // The delayline (ringbuffer) size is:
            // 2*ceil(maxDelay*sampleRate)
            for (int i=(2+2*ceil(maxDelay*sampleRate)); i<audioLength; ++i)
                CHECK(output[i] == refOutput[i]);

            // clean up
            delete[] audio;
            delete[] output;
            delete[] refOutput;
            inputBuffer = nullptr;
            outputBuffer = nullptr;
        }

        void testBlockSize () {
            int blockSize_1 = 7;
            int blockSize_2 = 13;
            int audioLength = 200;
            float sampleRate = 50;
            float maxDelay = 0.2;
            float freq = 2;
            float delay = 0.1;
            int sampleCounter = 0;


            // Memory allocation
            float *audio = new float[audioLength];
            for (int i=0; i<audioLength; ++i) {
                audio[i] = (i % 5) / 10.f;
            }

            float *output_1 = new float[audioLength]; // block size switch between blockSize_1 and blockSize_2
            float *output_2 = new float[audioLength]; // block size is fixed to (blockSize_1 + blockSize_2)

            float *inputBuffer = nullptr;
            float *outputBuffer = nullptr;

            vib.init(maxDelay, sampleRate);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);

            // process 1: block size switch between blockSize_1 and blockSize_2
            for (int i=0; sampleCounter<audioLength; ++i) {
                inputBuffer = audio + sampleCounter;
                outputBuffer = output_1 + sampleCounter;
                if (i%2 == 0) {
                    vib.setParam(CVibrato::kParamBlockSize, blockSize_1);
                    vib.process(inputBuffer, outputBuffer);
                    sampleCounter += blockSize_1;
                } else {
                    vib.setParam(CVibrato::kParamBlockSize, blockSize_2);
                    vib.process(inputBuffer, outputBuffer);
                    sampleCounter += blockSize_2;
                }
            }

            // reset
            sampleCounter = 0;
            vib.reset();

            // process 2: block size is fixed to (blockSize_1 + blockSize_2)
            vib.init(maxDelay, sampleRate);
            vib.setParam(CVibrato::kParamFreq, freq);
            vib.setParam(CVibrato::kParamDelay, delay);
            vib.setParam(CVibrato::kParamBlockSize, blockSize_1 + blockSize_2);

            for (int i=0; sampleCounter<audioLength; ++i) {
                inputBuffer = audio + sampleCounter;
                outputBuffer = output_2 + sampleCounter;
                vib.process(inputBuffer, outputBuffer);
                sampleCounter += blockSize_1 + blockSize_2;
            }

            // check
            for (int i=0; i<audioLength; ++i)
                CHECK(output_1[i] == output_2[i]);

            // clean up
            delete[] audio;
            delete[] output_1;
            delete[] output_2;
            inputBuffer = nullptr;
            outputBuffer = nullptr;

        }

        void testInvalidInputs () {
            Error_t err;
            // init
            err = vib.init(0, 0);
            CHECK(err == kFunctionInvalidArgsError);

            // negative block size
            vib.init(0.2, 100);
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
