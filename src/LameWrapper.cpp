//
// Created by saeid on 04.02.17.
//

#include "LameWrapper.hpp"
#include "utils.hpp"

#include <lame/lame.h>
#include <iostream>
#include <regex>
#include <fstream>

using std::cout;
using std::endl;

namespace cinemo {

    LameWrapper::LameWrapper(const string& dir, const string& file)
            : dir(dir), file(file), path(dir + "/" + file), isBusy(false),
              isDone(false), waveFile(nullptr) {
    }

    LameWrapper::~LameWrapper() {
        //no need to delete waveFile, its being used by workfiles vector in main
    }

    void LameWrapper::setWaveInfo(Wave* wave) {
        if (wave != nullptr) {
            waveFile = wave;
        }
    }

    void LameWrapper::printWaveInfo() {
        //TODO change this to return string, not print on ostream directly
        cout << "Sample Rate:\t" << waveFile->get_samples_per_sec() << " Hz"
             << endl;
        cout << "Bit Rate:\t" << waveFile->get_avg_bytes_per_sec() << " bps"
             << endl;
        cout << "Channels:\t" << waveFile->get_channels() << endl;
    }

    void LameWrapper::convertToMp3() {

        if (waveFile == nullptr) {
            cout << "Problem with underlying WAV file...skipping conversion"
                 << endl;
            return;
        }

        isBusy = true;

        int32_t sample_rate = waveFile->get_samples_per_sec();
        int32_t byte_rate = waveFile->get_avg_bytes_per_sec();
        int16_t channels = waveFile->get_channels();

        std::ofstream mp3(changeExt(getFullPath(), "mp3").c_str(),
                          std::ios_base::binary | std::ios_base::out);

        const int PCM_SIZE = 8192;
        const int MP3_SIZE = 8192;
        unsigned offset = 0;
        std::vector<char> pcm_buffer(sizeof(short int) * PCM_SIZE * 2);
        //FIXME following reservation causes sigsev using gcc 4.9
        //pcm_buffer.reserve(sizeof(short int) * PCM_SIZE * 2);

        unsigned char mp3_buffer[MP3_SIZE];

        lame_t l = lame_init();
        lame_set_in_samplerate(l, sample_rate);
        lame_set_brate(l, byte_rate);

        if (channels == 1) {
            lame_set_num_channels(l, 1);
            lame_set_mode(l, MONO);
        } else {
            lame_set_num_channels(l, channels);
        }

        lame_set_VBR(l, vbr_default);
        lame_init_params(l);

        while (true) {
            int k = (channels == 1) ? 1 : 2;
            unsigned size = PCM_SIZE * k * sizeof(short int);
            waveFile->get_samples(offset, size, pcm_buffer);
            unsigned read = pcm_buffer.size();
            offset += read;

            if (read > 0) {
                unsigned read_shorts =
                        read / 2;  // need number of 'short int' read
                int write = 0;

                if (channels == 1) {
                    write = lame_encode_buffer(l,
                                               reinterpret_cast<short int*>( &pcm_buffer[0] ),
                                               NULL, read_shorts, mp3_buffer,
                                               MP3_SIZE);
                } else {
                    write = lame_encode_buffer_interleaved(l,
                                                           reinterpret_cast<short int*>( &pcm_buffer[0] ),
                                                           read_shorts,
                                                           mp3_buffer,
                                                           MP3_SIZE);
                }

                pcm_buffer.clear();

                mp3.write(reinterpret_cast<char*>( mp3_buffer ), write);
            }

            if (read < size) {
                int write = lame_encode_flush(l, mp3_buffer, MP3_SIZE);

                mp3.write(reinterpret_cast<char*>( mp3_buffer ), write);

                break;
            }
        };

        lame_close(l);

        isDone = true;
        isBusy = false;
    }
}
