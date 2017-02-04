//
// Created by saeid on 04.02.17.
//

#include "../include/LameWrapper.hpp"

#include <lame/lame.h>
#include <stdio.h>
#include <iostream>

namespace cinemo {

    LameWrapper::LameWrapper(const string &dir, const string &file)
            : dir(dir), file(file), path(dir + "/" + file) {
    }

    LameWrapper::~LameWrapper() {
        //TODO clean up any mess
    }

    void LameWrapper::convertToMp3() {
        if (!isDone) {
            //for whatever reason the file is already been converted
            return;
        }

        isBusy = true;
        int read = 0, write = 0;
        FILE *wav = fopen(getFullPath().c_str(), "rb");
        FILE *mp3 = fopen(string(getDir() + ".mp3").c_str(), "wb");

        const int wsize = 8192;
        const int msize = 8192;

        short int w_buf[wsize * 2];
        unsigned char m_buf[msize * 2];

        lame_t lame = lame_init();
        lame_set_in_samplerate(lame, 44100);
        lame_set_VBR(lame, vbr_default);
        lame_init_params(lame);

        do {
            read = fread(w_buf, 2 * sizeof(short int), wsize, wav);
            if (read == 0)
                write = lame_encode_flush(lame, m_buf, msize);
            else
                write = lame_encode_buffer_interleaved(lame, w_buf, read, m_buf,
                                                       msize);
            fwrite(m_buf, write, 1, mp3);
        } while (read != 0);

        lame_close(lame);
        fclose(mp3);
        fclose(wav);

        isDone = true;
        isBusy = false;
    }
}
