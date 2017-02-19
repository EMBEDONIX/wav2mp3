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
// Created by saeid on 08.02.17.
//
//NOTES:
//  This header and implementation are minimalistic and in no way are complete and
//  are made only as a solution to a job application task at Cinemo GmbH.
//
//  Use of this code for production/investment is discouraged. Use better open-source
//  solutions which are out there (e.g. libav).
//
//REFERENCES:
//  http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
//  http://soundfile.sapp.org/doc/WaveFormat/
//  http://jhove.sourceforge.net/wave-hul.html
//  http://www.topherlee.com/software/pcm-tut-wavformat.html
//  http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/RIFF.html#Info
//  http://www.onicos.com/staff/iz/formats/wav.html
//  http://wavefilegem.com/how_wave_files_work.html
//
//TOOLS USED:
//  "jhove" a command line tool to validate RIFF headers
//      e.g. "$jhove file.wav"
//  "xxd" to visualise RIFF header in binary and ASCII
//      e.g. "$xxd -b -c 4 -l 100 file.wav"
//  "ffmpeg" to produce test files (ffmpeg appends lots of extra headers!!!)
//      e.g. "$ffmpeg -i input.wav -ar 8000 -ac 1 -acodec pcm_u8 output.wav"
//
//  Refer to man pages of the above commands for more info

#ifndef CINEMO_WAVE_HEADER_HPP
#define CINEMO_WAVE_HEADER_HPP

#include <bitset>
#include <string>

using std::string;

/**
 * @brief Indicates exsistance and position of a header chunk.
 *  first:  boolean, true if header exists
 *  second: position of the last character of header + 1, relative to beggining
 *  of the file.
 */
typedef std::pair<bool, int> header_pos;

namespace cinemo {
    namespace wh {
        /**
         * @brief Flags for any warnings during parsing of the wave file's RIFF header
         */
        enum WaveParseWarnings {
            /** No warnings during header parsing */
                    WARNING_SizeMismatch = 0,
            /** Happens if extension size is not 22 or data is not immediately followed by fmt */
                    WARNING_NoneStandardExtension = 1
        };

        /**
         * @brief Flags for any errors during parsing of the wave file's RIFF header
         */
        enum WaveErrorFlags {
            /** Error if can not open and seek through the file */
                    ERROR_FileIo = 0,
            /** Wave file should be at least 32 byte */
                    ERROR_FileTooSmall = 1,
            /** RIFF chunk header does not exist */
                    ERROR_NoRiffChunk = 2,
            /** WAVE chunk header does not exist */
                    ERROR_NoWaveChunk = 3,
            /** fmt chunk header does not exist */
                    ERROR_NoFmtChunk = 4,
            /** Data chunk header does not exist */
                    ERROR_NoDataChunk = 5
        };

        /**
         * @brief Compression method of a wave file
         * As far as I know, must formats are obsolete, but they can be find in
         * MMREG.H which is provided by microsoft MSVC compiler.
         *
         * NOTE: This enumeration is incomplete. Further formats can be also found
         * in {@link http://jhove.sourceforge.net/wave-hul.html}
         */
        enum WaveAudioType {
            WAT_INVALID = 0,
            WAT_RAW_PCM = 1,
            WAT_IEEE_FP = 3,
            WAT_MULAW = 6,
            WAT_ALAW = 7,
            WAT_IMAADPCM = 17,
            WAT_YAMAHA_ADPCM = 22,
            WAT_GSM610 = 49,
            WAT_ITUG_ADPCM = 64,
            WAT_MPEG = 80,
            WAT_MPEG_3 = 85,
            WAT_IBM_MULAW = 257,
            WAT_IBM_ALAW = 258,
            WAT_ADPCM = 259,
            /** Must be an extension */
                    WAT_EXTENSION = 65534,
            WAT_EXPIRIMENTAL = 65535
        };

        //FIXME convert to a nicely aligned C struct (e.g. for embedded systems)
        /**
         * @brief Minimalistic representation of a wave file header.
         */
        struct WaveHeader {
            /** Path to the file */
            string File;
            /** Wave ID */
            string WaveId;
            /** If file contains "fact" chunk */
            bool HasFact = false;
            /** Possible problems and warnings during parsing the wave file */
            std::bitset<8> WarningFlags = 0;
            /** Flags for errors during parsing if it is not 0, file is invalid */
            std::bitset<8> ErrorFlags = 0;
            /** Total size of the file in bytes (including header and data) */
            uint32_t FileSize = 0;
            /** Size of file in bytes, 8 bytes less than actual file size */
            uint32_t RiffSize = 0;
            /** FMT Chunk Size */
            uint32_t FormatSize = 0;
            /** Audio Type (compression method) */
            uint16_t FormatAudioType = 0;
            /** Number of channels (1 = mono, 2 = stereo, etc.) */
            uint16_t NumberOfChannels = 0;
            /** Sample rate of the audio */
            uint32_t SampleRate = 0;
            /** Byte rate of the audio */
            uint32_t ByteRate = 0;
            /** Block align */
            uint16_t BlockAlign = 0;
            /** Bits per sample or bit depth */
            uint16_t BitsPerSample = 0;
            /** Size of the extension (0 or 22, other possibilities might exist!) */
            uint16_t ExtensionSize = 0;
            uint16_t ValidBitsPerSample = 0;
            /** Bit fields indicating the speakers to use (e.g. left, rear, top, etc.) */
            uint32_t ChannelMask = 0;
            /** GUID, including the data format code */
            uint8_t SubFormat[16] = {0};
            /** Number of bytes to be parsed in "fact" section */
            uint32_t FactSize = 0;
            /** Number of samples (per channel) */
            uint32_t FactSampleLength = 0;
            /** The position where actual sound data begins */
            uint32_t DataBegin = 0;
            /** Size of bytes that contain actual audio content */
            uint32_t DataSize = 0;

            //Extra, possibly harmless headers
            bool hasList = false;
            uint32_t ListSize = 0;


            //FUNCTIONS
            /**
             * @brief Check if the file is in extended format
             * @return true if is extended, false if its standard PCM
             */
            bool isExtendedFormat() const { return ExtensionSize > 0; };

            /**
             * @brief Check if the last byte of the file is not important.
             *  Note: If number of data bytes is odd, then one byte at the end
             *  of the file is the pad byte.
             * @return true if has, false if has not!
             */
            bool hasPadByte() const { return (DataSize % 2 != 0); }
        };

        /**
         * @brief Tries to parse header out of (an assumed) wave file.
         * The result will contain must important information in regards to using them
         * for converting the wave file to mp3 format.
         * @param file Path to the file
         * @return A populated {@link WaveHeader} pointer. nullptr if something goes wrong.
         */
        WaveHeader* parseWaveHeader(const string& file);

        void getFmtFromFileHandle(WaveHeader* wh, std::ifstream& file,
                                  header_pos dp);

        void getFactFromFileHandle(WaveHeader* wh, std::ifstream& file,
                                   header_pos dp);

        void getDataFromFileHandle(WaveHeader* wh, std::ifstream& file,
                                   header_pos dp);

        void getListInfoFromFileHandle(WaveHeader* wh, std::ifstream& file,
                                       header_pos dp);

        void printWaveHeader(const WaveHeader& wh);

        uint32_t convert4CharTo16_BigEndian(char* const buff);

        header_pos searchForHeader(char* buff, size_t size, const string& s);

        void printFlags();

    }
}


#endif //CINEMO_WAVE_HEADER_HPP
