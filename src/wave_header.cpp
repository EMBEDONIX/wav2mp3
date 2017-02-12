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

#include "wave_header.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>

using std::cout;
using std::endl;

namespace cinemo {
namespace wh {

    static const string WaveRIFFString = "RIFF";
    static const string WaveWaveIdString = "WAVE";
    static const string WaveFormatString = "fmt "; //don't date touch that space!
    static const string WaveFactString = "fact";
    static const string WaveDataString = "data";
    static const string WaveListString = "LIST";
    static const int WaveExtNormalGuidLength = 16;
    static const int WaveBufferSize = 256;
    static const int WaveMinLength = 44; //smallest possible wav file, with no audio!

    WaveHeader* parseWaveHeader(const string& file) {

        std::ifstream f(file, std::ios::in | std::ios::binary |
                              std::ios::ate);
        //generate wave header struct
        WaveHeader* const wh = new WaveHeader;

        if (f && !f.good()) {
            wh->ErrorFlags.set(static_cast<int>(ERROR_FileIo), 1);
            return wh;
        }

        //Check for file size requirements
        int64_t fSize = f.tellg();
        wh->File = file;
        wh->FileSize = (uint32_t) fSize;

        if (fSize < WaveMinLength) {
            f.close();
            wh->ErrorFlags.set(static_cast<int>(ERROR_FileTooSmall), 1);
            return wh; //because there is no point in reading rest of the file
        }

        //reset file handle (rewind)
        f.clear();
        f.seekg(0, std::ios::beg);

        char buff[WaveBufferSize]; //buffer for read operations

        //Check if file is RIFF
        f.read(&buff[0], 4); //get first 4 bytes
        if (string(&buff[0], 4) != WaveRIFFString) { //not valid
            f.close();
            wh->ErrorFlags.set(static_cast<int>(ERROR_NoRiffChunk), 1);
            return wh; //because there is no point in reading rest of the file
        }

        //RIFF Size (which should be (total file size - 8)
        //otherwise file might be corrupt
        f.read(reinterpret_cast<char*>(&wh->RiffSize), 4);
        if (wh->FileSize - wh->RiffSize != 8) {
            wh->WarningFlags.set(WARNING_SizeMismatch, 1); //just a warning
        }

        //extract "WAVE"
        f.read(&buff[0], 4);
        wh->WaveId = string(&buff[0], 4);
        if (wh->WaveId != WaveWaveIdString) {
            wh->WarningFlags.set(static_cast<int>(ERROR_NoWaveChunk), 1);
            return wh; //because there is no point in reading rest of the file
        }

        //next might be "fmt " but there is no guarantee!
        long posBeforeHeaderSearch = f.tellg();
        header_pos fmt, fact, data, List;
        f.read(&buff[0], WaveBufferSize);
        size_t read = (size_t) (static_cast<long>(f.tellg()) -
                                posBeforeHeaderSearch);
        //for fmt and fact there is no guranatee which comes first
        //so the search function is called twice
        fmt = searchForHeader(buff, read, WaveFormatString);
        fact = searchForHeader(buff, read, WaveFactString);
        //TODO search for data within fmt or fact search to speed up!
        data = searchForHeader(buff, read, WaveDataString);
        List = searchForHeader(buff, read, WaveListString);

        //fmt and data SHOULD be available, otherwise file is corrupted
        //fact is optional
        if (!fmt.first) {
            wh->ErrorFlags.set(static_cast<int>(ERROR_NoFmtChunk), 1);
        }

        if (!data.first) {
            wh->ErrorFlags.set(static_cast<int>(ERROR_NoDataChunk), 1);
        }

        if (wh->ErrorFlags.any()) {
            f.close();
            return wh; //to much errors! no point in reading rest of the file
        }

        posBeforeHeaderSearch += 4; //since all headers are 4 byte

        if (fact.first || List.first) {
            wh->WarningFlags.set(
                    static_cast<int>(WARNING_NoneStandardExtension), 1);
        }

        if (fact.first) {
            fact.second += posBeforeHeaderSearch;
            wh->HasFact = true; //set the flag
            //fact should be parsed after fmt
        }

        if (fmt.first) {
            fmt.second += posBeforeHeaderSearch;
            getFmtFromFileHandle(wh, f, fmt);
        }

        if(fact.first) {
            getFactFromFileHandle(wh, f, fact);
        }

        if (data.first) {
            data.second += posBeforeHeaderSearch;
            getDataFromFileHandle(wh, f, data);
        }

        if (List.first) {
            wh->hasList = true;
            getListInfoFromFileHandle(wh, f, List);
        }

        f.close();
        return wh;
    }

    void getFmtFromFileHandle(WaveHeader* wh, std::ifstream& file,
                              header_pos dp) {
        file.seekg(dp.second, std::ios::beg);
        char buff[WaveBufferSize];
        //get format size
        file.read(&buff[0], 4);
        wh->FormatSize = convert4CharTo16_BigEndian(&buff[0]);
        file.read(reinterpret_cast<char*>(&wh->FormatAudioType),
                  sizeof(wh->FormatAudioType));
        file.read(reinterpret_cast<char*>(&wh->NumberOfChannels),
                  sizeof(wh->NumberOfChannels));
        file.read(reinterpret_cast<char*>(&wh->SampleRate),
                  sizeof(wh->SampleRate));
        file.read(reinterpret_cast<char*>(&wh->ByteRate),
                  sizeof(wh->ByteRate));
        file.read(reinterpret_cast<char*>(&wh->BlockAlign),
                  sizeof(wh->BlockAlign));
        file.read(reinterpret_cast<char*>(&wh->BitsPerSample),
                  sizeof(wh->BitsPerSample));

        //check if file is standard fmt with 16 fmt bytes or extended!
        long readSoFar = ((long)(file.tellg()) - dp.second - 4); //4 is for "fmt "
        long leftToRead = wh->FormatSize - readSoFar;

        if(leftToRead == 0) { //Standard FMT read is complete
            //double check, if file is standard, now it should have reach to "data"
            file.read(&buff[0], 4);
            if(!wh->HasFact && string(&buff[0], 4) != WaveDataString) { //update error flag
                wh->WarningFlags.set(
                        static_cast<int>(WARNING_NoneStandardExtension), 1);
            }
            return;
        } else if(leftToRead == 2) { //its fmt with 18 bytes
            file.read(reinterpret_cast<char*>(&wh->ExtensionSize),
                      sizeof(wh->ExtensionSize));
            file.read(&buff[0], 4);
            if(!wh->HasFact && string(&buff[0], 4) != WaveDataString) { //update error flag
                wh->WarningFlags.set(
                        static_cast<int>(WARNING_NoneStandardExtension), 1);
            }
            return;
        }

/*        //format is extended, still work to do ;)
        //how many bytes left to read?
        file.read(reinterpret_cast<char*>(&wh->ExtensionSize),
                  sizeof(wh->ExtensionSize));*/

        //standard extension sizes are 0 or 22 (0 is set by default in the struct)
        if(wh->ExtensionSize != 0 && wh->ExtensionSize != 22) {
            wh->WarningFlags.set(
                    static_cast<int>(WARNING_NoneStandardExtension), 1);
        }

        file.read(reinterpret_cast<char*>(&wh->ValidBitsPerSample),
                  sizeof(wh->ValidBitsPerSample));
        file.read(reinterpret_cast<char*>(&wh->ChannelMask),
                  sizeof(wh->ChannelMask));
        file.read(reinterpret_cast<char*>(&wh->SubFormat[0]),
                sizeof(wh->SubFormat[0]) * (wh->ExtensionSize - 6)); //6 = 2 vbps, 4 cm

        //by reaching here the next 4 bytes should be "fact" (if it is available)
        // or "data" (other possibilities might exist!!!!)
        file.read(&buff[0], 4);
        if(!wh->HasFact && string(&buff[0], 4) != WaveDataString) { //update error flag
            wh->WarningFlags.set(
                    static_cast<int>(WARNING_NoneStandardExtension), 1);
        }
    }

    void getFactFromFileHandle(WaveHeader* wh, std::ifstream& file,
                               header_pos dp) {
        file.seekg(dp.second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&wh->FactSize), sizeof(wh->FactSize));
    }

    void getDataFromFileHandle(WaveHeader* wh, std::ifstream& file,
                               header_pos dp) {
        file.seekg(dp.second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&wh->DataSize), sizeof(wh->DataSize));
        wh->DataBegin = (uint32_t) file.tellg();
        //in a correctly formatted wave file, we should reach end of file
        //after "DataSize" bytes
        //TODO check if file ends at where it should end! (mind the pad byte if exists)
    }

    void getListInfoFromFileHandle(WaveHeader* wh, std::ifstream& file,
                                   header_pos dp) {
        //FIXME figure out how to read LIST header chunks...
        //no source for it yet :(

//        file.seekg(dp.second, std::ios::beg);
//        char buff[WaveBufferSize];
//        //get format size
//        file.read(&buff[0], 4);
//        wh->FactSize = convert4CharTo16_BigEndian(&buff[0]);

    }

    uint32_t convert4CharTo16_BigEndian(char* const buff) {
        return (uint32_t) (buff[0] | (buff[1] << 8) | (buff[2] << 16) |
                           (buff[3] << 24));
    }

    void printWaveHeader(const WaveHeader& wh) {

        cout << "-------------------------------------" << endl;
        cout << "Wave Header Information" << endl;
        cout << "File: \t" << wh.File << endl;
        cout << "Total size: " << wh.FileSize << " Bytes" << endl;
        cout << "Riff Size: " << wh.RiffSize << " Bytes" << endl;
        cout << "Wave ID: " << wh.WaveId << endl;
        cout << "Format Size: " << wh.FormatSize << " Bytes" << endl;
        cout << "AudioType: " << wh.FormatAudioType << endl;
        cout << "Channels: " << wh.NumberOfChannels << endl;
        cout << "Sample Rate: " << wh.SampleRate << " Hz" << endl;
        cout << "Byte Rate: " << wh.ByteRate << " Bps" << endl;
        cout << "Block Align: " << wh.BlockAlign << endl;
        cout << "Bits per Sample: \t" << wh.BitsPerSample << endl;
        cout << "Extension Size: \t" << wh.ExtensionSize << endl;
        if(wh.isExtendedFormat()) {
            cout << "\tExt. Valid Bits Per Sample: " << wh.ValidBitsPerSample
                 << endl;
            cout << "\tExt. Channel Mask: " << wh.ChannelMask << endl;
            cout << "\tGUID: ";
            auto flags = cout.flags(); //keep current console flags
            for(int i = 0; i < wh.ExtensionSize - 6; i++) {
                cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << +wh.SubFormat[i] << " ";
            }
            cout << endl;
            cout.flags(flags);
        }
        if(wh.HasFact) {
            cout << "FACT Details: " << endl;
            cout << "\tSize: " << wh.FactSize << endl;
            cout << "\tSample Length: " << wh.FactSampleLength << " Bytes"
                 << endl;
            if(wh.FactSize > 4) { //standard is 4...but there maybe more
                cout << "\tNote: there is information which have been skipped!" << endl;
            }
        }
        cout << "Data Size: " << wh.DataSize << " Bytes" << endl;
        cout << "Data Begins: " << wh.DataBegin << endl;
        if (wh.hasList) {
            cout << "LIST Size: " << wh.ListSize << endl;
        }
        if(wh.WarningFlags.any()) {
            cout << "Warning Flags: " << wh.WarningFlags << endl;
        }
        if(wh.ErrorFlags.any()) {
            cout << "Error Flags: " << wh.ErrorFlags << endl;
        }
        cout << "-------------------------------------" << endl;
    }

    void printFlags() {
        cout << "-------------------------------------" << endl;
        cout << "Error flags are 8 bit (XXXXXXXX). If there is any error, "
                "program can not encode the file. A set bit on each position has "
                "the following meanings (BIT_0 = LSB):\n" << endl;
        cout << "\tBIT_0 I/O error on opening/reading WAV file." << endl;
        cout << "\tBIT_1 WAV File is too small, it should be at least "
             << WaveMinLength << " Bytes (which would not have any audio data!)"
             << endl;
        cout << "\tBIT_2 File does not begin with ASCII characters \"RIFF\"."
             << endl;
        cout << "\tBIT_3 File does not contain ASCII characters \"WAVE\"."
             << endl;
        cout
                << "\tBIT_4 File does not contain ASCII characters \"fmt \" for format specifications."
                << endl;
        cout
                << "\tBIT_5 File does not contain ASCII characters \"data\" for audio."
                << endl;
        cout << "\tBIT_6 [unused in current version]" << endl;
        cout << "\tBIT_7 [unused in current version]" << endl;
        cout << endl;
        cout << "Warning flags (might cause instability or corruption if any "
                "is set):\n" << endl;
        cout
                << "\tBIT_0 Size mismatch between what is declared in the WAV header"
                        " and the actual file's size." << endl;
        cout << "\tBIT_1 WAV File is not standard PCM and its encoding "
                "is not guaranteed to work (yet)." << endl;
        cout << "\tBIT_2 [unused in current version]" << endl;
        cout << "\tBIT_3 [unused in current version]" << endl;
        cout << "\tBIT_4 [unused in current version]" << endl;
        cout << "\tBIT_5 [unused in current version]" << endl;
        cout << "\tBIT_6 [unused in current version]" << endl;
        cout << "\tBIT_7 [unused in current version]" << endl;
        cout << "-------------------------------------" << endl;
    }

    header_pos searchForHeader(char* buff, size_t size, const string& s) {
        header_pos dp(false, -1);
        string b(buff, size);
        int pos = (int) b.find(s);

        if (pos >= 0) {
            dp.first = true;
            dp.second = pos;
        }

        return dp;
    }

}
}