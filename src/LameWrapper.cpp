//
// Created by saeid on 04.02.17.
//

#include "LameWrapper.hpp"
#include "utils.hpp"

#include <iostream>
#include <regex>

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
        //TODO change this to return string, not printing on ostream directly
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

        //setup lame struct
        lame_t l;
        l = lame_init();

        //finalize lame params
        if (getLameFlags(l) < 0) {
            cout << "Error in setting lame parameters!" << endl;
            return;
        }

        //all set....go!
        isBusy = true;
        doEncoding(getFullPath(), changeExt(getFullPath(), "mp3"), l);
        lame_close(l);
        isDone = true;
        isBusy = false;
    }

    int LameWrapper::getLameFlags(lame_t& l) {
        //get information from wave file
        int32_t sample_rate = waveFile->get_samples_per_sec();
        int32_t byte_rate = waveFile->get_avg_bytes_per_sec();
        int16_t channels = waveFile->get_channels();

        lame_set_quality(l, 0);
        lame_set_in_samplerate(l, sample_rate);
        lame_set_out_samplerate(l, sample_rate);
        //lame_set_brate(l, 320);
        lame_set_bWriteVbrTag(l, 1);
        lame_set_VBR(l, vbr_mtrh);

        if (channels == 1) {
            lame_set_num_channels(l, 1);
            lame_set_mode(l, MONO);
        } else { //FIXME currently treating anything else as stereo
            lame_set_num_channels(l, channels);
            lame_set_mode(l, STEREO);
        }

        //Init the id3 tag structure
        id3tag_init(l);
        id3tag_v2_only(l);
        id3tag_set_year(l, "");
        id3tag_set_genre(l, "");
        id3tag_set_artist(l, "");
        id3tag_set_album(l, "");
        id3tag_set_title(l, "");
        id3tag_set_track(l, "");
        id3tag_set_comment(l, "Converted by WAV2MP3, www.embedonix.com");
        //no need for album art ;)
        //id3tag_set_albumart(l, nullptr, 0);
        lame_set_write_id3tag_automatic(l, 0); //0 manual 1 auto

        return lame_init_params(l);
    }

    bool LameWrapper::doEncoding(const string& pcm_in, const string& mp3_out,
                                 const lame_t& lame) {

/*      some hints are taken from
        http://stackoverflow.com/questions/16926725/audio-speed-changes-on-converting-wav-to-mp3*/

        FILE* pcm = fopen(pcm_in.c_str(), "rb");
        FILE* mp3 = fopen(mp3_out.c_str(), "wb");

        size_t read = 0;
        int write = 0;

        const int PCM_SIZE = 8192;
        const int MP3_SIZE = 8192;

        short int pcm_buffer[PCM_SIZE * 2];
        unsigned char mp3_buffer[MP3_SIZE];

        int numChannels = lame_get_num_channels(lame);
        cout << "read multiplier = " << numChannels;

        do {
            read = fread(pcm_buffer, numChannels * sizeof(short int), PCM_SIZE,
                         pcm);
            if (read == 0) {
                write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
            } else {
                write = lame_encode_buffer(lame, pcm_buffer, nullptr,
                                           (const int) read, mp3_buffer,
                                           MP3_SIZE);
                //FIXME figure out writing of one channel or two channel correctly
/*                if(numChannels == 1) {
                    write = lame_encode_buffer(lame, pcm_buffer, nullptr,
                                               reinterpret_cast<int>(read), mp3_buffer, MP3_SIZE);
                } else {
                    write = lame_encode_buffer_interleaved(lame, pcm_buffer,
                                                           (int) read, mp3_buffer,
                                                           MP3_SIZE);
                }*/
            }
            fwrite(mp3_buffer, write, 1, mp3);
        } while (read != 0);

        lame_close(lame);
        fclose(mp3);
        fclose(pcm);
        return true;
    }
}
