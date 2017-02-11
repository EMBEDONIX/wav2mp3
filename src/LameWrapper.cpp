/*
This file is part of EMBEDONIX/WAV2MP3.

EMBEDONIX/WAV2MP3 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.

EMBEDONIX/WAV2MP3 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EMBEDONIX/WAV2MP3.  If not, see <http://www.gnu.org/licenses/>.
*/

//
// Created by saeid on 04.02.17.
//

#include "LameWrapper.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>
#include <regex>

using std::cout;
using std::endl;

namespace cinemo {

    LameWrapper::LameWrapper(const string& dir, const string& file)
            : dir(dir), file(file), path(dir + "/" + file), isBusy(false),
              isDone(false), wh(wh::parseWaveHeader(path)) {
    }

    LameWrapper::~LameWrapper() {
        //no need to delete waveFile, its being used by workfiles vector in main
    }

    void LameWrapper::printWaveInfo() {
        if(wh != nullptr) {
            wh::printWaveHeader(getHeader());
        } else {
            cout << getFullPath() << " Appears to be an invalid or "
                    "corrupted WAV file!" << endl;
        }
    }

    int LameWrapper::getLameFlags(lame_t& l) {
        //get information from wave file
        int32_t sample_rate = wh->SampleRate;
        int32_t byte_rate = wh->ByteRate;
        int16_t channels = wh->NumberOfChannels;

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
        lame_set_write_id3tag_automatic(l, 1); //0 manual 1 auto

        return lame_init_params(l);
    }

    void LameWrapper::convertToMp3() {

        if(wh == nullptr) {
            cout << "This file does not appear to be a valid WAV file!" << endl;
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
        //TODO this nesting of switches requires a through rework!
        switch (wh->FormatAudioType) {
            case wh::WaveAudioType::WAT_RAW_PCM:
                switch (lame_get_num_channels(l)) {
                    case 1:
                        switch (wh->BitsPerSample) {
                            case 8:
                                isDone = encodeMono_1_8(getFullPath(),
                                                        changeExt(getFullPath(),
                                                                  "mp3"), l);
                                break; //mono 8bit end

                            case 16:
                                isDone = encodeMono_2_16(getFullPath(),
                                                        changeExt(getFullPath(),
                                                                  "mp3"), l);
                                break; //mono 16bit end
                            default:
                                cout << "Encoding of files with bit depth of "
                                     << wh->BitsPerSample << " is not yet implemented!";
                                break;

                        }
                        break;

                    case 2:
                        switch(wh->BitsPerSample) {
                            case 8:
                                isDone = encodeStereo_2_8(getFullPath(),
                                                      changeExt(getFullPath(),
                                                                "mp3"), l);
                                break;

                            case 16:
                                isDone = encodeStereo_4_16(getFullPath(),
                                            changeExt(getFullPath(), "mp3"), l);
                                break;

                            default:
                                cout << "Encoding of files with bit depth of "
                                     << wh->BitsPerSample << " is not yet implemented!";
                                break;
                        }

                        break;

                    default:
                        cout << "Conversion of wave files with "
                             << wh->NumberOfChannels
                             << " channels is not supported"
                             << endl;
                        break;
                }
                break;

            case wh::WaveAudioType::WAT_MPEG_3: //already MP3 !!!!
                isDone = encodeAlreadyMp3(getFullPath(), changeExt(getFullPath(), "mp3"), l);
                break;

            default: //all other cases
                cout << "Conversion of files with format " << +wh->FormatAudioType
                                                           << " is not yet supported."
                                                           <<endl;
                break;
        }

        if(!isDone) {
            cout << getFullPath() << " Was not converted!" << endl;
        }

        lame_close(l);
        isBusy = false;
    }



    bool LameWrapper::encodeMono_1_8(const string& in, const string& out,
                                 const lame_t& lame) {
        std::ifstream wav(in, std::ios_base::in | std::ios_base::binary);
        std::ofstream mp3(out, std::ios_base::out | std::ios_base::trunc |
                                   std::ios_base::binary);
        //just to make sure...
        if (!wav.is_open() || !mp3.is_open()) {
            return false;
        }

        std::streamsize read = 0, write = 0;

        int WAV_BUFF_SIZE = 8192, MP3_BUFF_SIZE = 8192;
        char wav_buff[WAV_BUFF_SIZE];
        unsigned char mp3_buff[MP3_BUFF_SIZE];
        wav.ignore(wh->DataBegin); //skip to data begin

        do {
            //read from wave file
            wav.read(&wav_buff[0], sizeof(wav_buff[0]) * WAV_BUFF_SIZE) ;
            read = wav.gcount();

            if (read == 0) { //reading from wav file has ended!
                write = lame_encode_flush(lame,
                                  reinterpret_cast<unsigned char*>(&mp3_buff),
                                  MP3_BUFF_SIZE );
            } else { //encoding
                write = lame_encode_buffer(lame,
               (const short*) reinterpret_cast<unsigned  char*>(&wav_buff[0]),
                nullptr, (const int) read / 2, mp3_buff, MP3_BUFF_SIZE);
            }
            mp3.write(reinterpret_cast<char*>(&mp3_buff), write);
        } while (read != 0);

        wav.close();
        mp3.close();

        return true;
    }

    bool LameWrapper::encodeMono_2_16(const string& in, const string& out,
                         const lame_t& lame) {
        std::ifstream wav(in, std::ios_base::in | std::ios_base::binary);
        std::ofstream mp3(out, std::ios_base::out | std::ios_base::trunc |
                                   std::ios_base::binary);
        //just to make sure...
        if (!wav.is_open() || !mp3.is_open()) {
            return false;
        }

        std::streamsize read = 0, write = 0;

        int WAV_BUFF_SIZE = 8192, MP3_BUFF_SIZE = 8192;
        short int wav_buff[WAV_BUFF_SIZE];
        unsigned char mp3_buff[MP3_BUFF_SIZE];
        wav.ignore(wh->DataBegin); //skip to data begin

        do {
            //read from wave file
            wav.read(reinterpret_cast<char*>(&wav_buff[0]),
                     sizeof(wav_buff[0]) * WAV_BUFF_SIZE) ;
            read = wav.gcount();

            if (read == 0) { //reading from wav file has ended!
                write = lame_encode_flush(lame,
                                    reinterpret_cast<unsigned char*>(&mp3_buff),
                                          MP3_BUFF_SIZE );
            } else { //encoding
                write = lame_encode_buffer(lame, &wav_buff[0], nullptr,
                                           (const int) (read / 2), mp3_buff,
                                           MP3_BUFF_SIZE);
            }
            mp3.write(reinterpret_cast<char*>(&mp3_buff), write);
        } while (read != 0);

        wav.close();
        mp3.close();

        return true;
    }


    bool LameWrapper::encodeStereo_2_8(const string& wav_in, const string& mp3_out,
                                 const lame_t& lame) {

        std::ifstream wav(wav_in, std::ios_base::in | std::ios_base::binary);
        std::ofstream mp3(mp3_out, std::ios_base::out | std::ios_base::trunc |
                                   std::ios_base::binary);

        //just to make sure...
        if (!wav.is_open() || !mp3.is_open()) {
            return false;
        }

        std::streamsize read = 0, write = 0;

        int WAV_BUFF_SIZE = 8192, MP3_BUFF_SIZE = 8192;
        short int wav_buff[WAV_BUFF_SIZE]; //all channels
        char wav_buff_l[WAV_BUFF_SIZE / 2]; //left channel
        char wav_buff_r[WAV_BUFF_SIZE / 2]; //right channel
        unsigned char mp3_buff[MP3_BUFF_SIZE];

        wav.ignore(wh->DataBegin); //skip to data

        do {
            //read from wave file
            wav.read(&wav_buff_l[0], sizeof(wav_buff[0]) * WAV_BUFF_SIZE);
            //getChunkAsStereo8(wav_buff, wav_buff_l, wav_buff_r, WAV_BUFF_SIZE);
            read = wav.gcount();

            if (read == 0) { //reading from wav file has ended!
                write = lame_encode_flush(lame,
                                  reinterpret_cast<unsigned char*>(&mp3_buff),
                                  MP3_BUFF_SIZE );
            } else { //encoding
                write = lame_encode_buffer(lame,
                                   reinterpret_cast<short int*>(&wav_buff_l[0]),
                                   reinterpret_cast<short int*>(&wav_buff_r[0]),
                                   (const int) read / 2, mp3_buff,
                                   MP3_BUFF_SIZE);

            }

            //write encoded into mp3 file
            mp3.write(reinterpret_cast<char*>(&mp3_buff), write);
        } while (read != 0);

        wav.close();
        mp3.close();

        return true;
    }

    bool LameWrapper::encodeStereo_4_16(const string& in, const string& out,
                                       const lame_t& lame) {

        std::ifstream wav(in, std::ios_base::in | std::ios_base::binary);
        std::ofstream mp3(out, std::ios_base::out | std::ios_base::trunc |
                                   std::ios_base::binary);

        //just to make sure...
        if (!wav.is_open() || !mp3.is_open()) {
            return false;
        }

        std::streamsize read = 0, write = 0;

        int WAV_BUFF_SIZE = 8192, MP3_BUFF_SIZE = 8192;
        short int wav_buff[WAV_BUFF_SIZE]; //all channels
        short int wav_buff_l[WAV_BUFF_SIZE / 2]; //left channel
        short int wav_buff_r[WAV_BUFF_SIZE / 2]; //right channel
        unsigned char mp3_buff[MP3_BUFF_SIZE];

        wav.ignore(wh->DataBegin); //skip to data

        do {
            wav.read(reinterpret_cast<char*>(&wav_buff_l), sizeof(short int));
            wav.read(reinterpret_cast<char*>(&wav_buff_r), sizeof(short int));
            read = wav.gcount();

            if (read == 0) { //reading from wav file has ended!
                write = lame_encode_flush(lame,
                                          reinterpret_cast<unsigned char*>(&mp3_buff),
                                          MP3_BUFF_SIZE);
            } else { //encoding
                write = lame_encode_buffer(lame,
                                           &wav_buff_l[0],
                                           &wav_buff_r[0],
                                           1,
                                           mp3_buff,
                                           MP3_BUFF_SIZE);

            }
            //write encoded into mp3 file
            mp3.write(reinterpret_cast<char*>(&mp3_buff), write);
        } while (read != 0);

        wav.close();
        mp3.close();

        return true;

    }

    bool LameWrapper::encodeAlreadyMp3(const string& in, const string& out,
                          const lame_t& lame) {
        cout << "File is already encoded as MP3, just change the extension!!"
             << endl;
        return false;
    }
}
