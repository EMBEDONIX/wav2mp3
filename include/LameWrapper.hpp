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
//NOTES:
//  This header and implementation are minimalistic and in no way are complete and
//  are made only as a solution to a job application task at Cinemo GmbH.
//
//  Use of this code for production/investment is discouraged. Use better open-source
//  solutions which are out there (e.g. libav).
//

#ifndef CINEMO_LAMEWRAPPER_HPP
#define CINEMO_LAMEWRAPPER_HPP

#include <string>
#include <memory>

#include "wave_header.hpp"

#if WIN32
#include "win/lame.h"
#else
#include "lame/lame.h"
#endif

using std::string;
using std::unique_ptr;

namespace cinemo {

    class LameWrapper {
    private:
        string dir;
        string file;
        string path;
		unique_ptr<wh::WaveHeader> wh;
		string message_buffer;
		bool initDone;
        bool isBusy;
        bool isDone;
        int quality = 3;

        int getLameFlags(lame_t l) const;

        /************** ENCODE FUNCTIONS BEG *************/
        // the following private functions are named by following convention
        // encodeChannels_BlockAlign_BitDepthPerSample
        // e.g. stereo, 4byte, 16 bit = encodeStereo_4_16(...)

		//PCM
        bool encodeMono_1_8(const string& in, const string& out,
                            const lame_t& lame);

        bool encodeMono_2_16(const string& in, const string& out,
                             const lame_t& lame);

        bool encodeStereo_2_8(const string& in, const string& out,
                              const lame_t& lame);

        bool encodeStereo_4_16(const string& in, const string& out,
                               const lame_t& lame);

		//IEEE 
		bool encodeMono_IEEE_32(const string& in, const string& out,
			const lame_t& lame);
		bool encodeStereo_IEEE_32(const string& in, const string& out,
			const lame_t& lame);
		bool encodeMono_IEEE_64(const string& in, const string& out,
			const lame_t& lame);
		bool encodeStereo_IEEE_64(const string& in, const string& out,
			const lame_t& lame);

		bool encodeAlreadyMp3 (const string& in, const string& out,
			const lame_t& lame);
        /************** ENCODE FUNCTIONS END *************/

    public:

        //TODO this buffer sizes work OK, but maybe its better to do as formula in lame header
        static const int WAV_BUFF_SIZE = 1024 * 10;
        static const int MP3_BUFF_SIZE = 1024 * 10;

        //FIXME parameters are ambiguous....just use full path instead
		/**
		* @brief Constructs a LameWrapper object
		* @param dir Directory of files 
		* @param file Name fo the file
		*/
        LameWrapper(const string& dir, const string& file);

        ~LameWrapper();

		/**
		* @brief Prepares the wrapper for work. This function must be called before encoding!
		*/
		void init();

        /**
         * @brief Performs the encoding of WAV to MP3.
         * @return true on success, false on failure
         */
        bool convertToMp3();

        /**
         * @brief Returns the wave header object of the associated wave file to this instance.
         * @return wh WaveHeader object parsed from wav file
         */
        wh::WaveHeader getHeader() const { return *wh; }

		/**
		* @brief Get the error messages during encoding.
		*	If error message is empty, be happy!
		* @return string error messages during encoding phase.
		*/
		string getErrorMessage() const { return message_buffer; }

        /**
         * @brief Set the quality of encoding
         * @param quality Quality index (0 = highest, 9 = lowest)
         */
        void setQuality(int quality);

        int getQuality() const { return quality; }

        string getDir() const { return dir; }

        string getFullPath() const { return path; }

        string getFileName() const { return file; }

        bool isConverting() const { return isBusy; }

        bool isFinished() const { return isDone; }

        void printWaveInfo() const;

        //Operator overloads
        //delete copy assignment operator
        LameWrapper& operator=(const LameWrapper&) = delete;
        LameWrapper(const LameWrapper&) = delete;
        LameWrapper() = default;
    };
}


#endif //CINEMO_LAMEWRAPPER_HPP
