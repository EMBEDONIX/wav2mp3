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
    static const int WaveExtNormalGuidLength = 16;
    static const int WaveBufferSize = 256;
    static const int WaveMinLength = 32;

    WaveHeader* parseWaveHeader(const string& file) {

        std::ifstream f(file, std::ios::in | std::ios::binary |
                              std::ios::ate);
        if (f == nullptr && !f.good()) {
            return nullptr;
        }

        //Check for file size requirements
        int64_t fSize = f.tellg();
        if (fSize < WaveMinLength) {
            f.close();
            return nullptr;
        }

        //reset file handle (rewind)
        f.clear();
        f.seekg(0, std::ios::beg);

        char buff[WaveBufferSize]; //buffer for read operations

        //Check if file is RIFF
        f.read(&buff[0], 4); //get first 4 bytes
        if (string(&buff[0], 4) != WaveRIFFString) { //not valid
            f.close();
            return nullptr;
        }

        //generate wave header struct
        WaveHeader* const wh = new WaveHeader;
        wh->File = file;
        wh->FileSize = (uint32_t) fSize;

        //RIFF Size (which should be (total file size - 8)
        //otherwise file might be corrupt
        f.read(reinterpret_cast<char*>(&wh->RiffSize), 4);
        if (wh->FileSize - wh->RiffSize != 8) {
            wh->WarningFlags.set(WARNING_SizeMismatch, 1);
        }

        //extract "WAVE"
        f.read(&buff[0], 4);
        wh->WaveId = string(&buff[0], 4);
        if (wh->WaveId != WaveWaveIdString) {
            wh->WarningFlags.set(static_cast<int>(WARNING_InvalidWaveId), 1);
        }

        //next might be "fmt " but there is no guarantee!
        long int posBeforeHeaderSearch = f.tellg();
        data_pos fmt, fact, data;
        f.read(&buff[0], WaveBufferSize);
        size_t read = (size_t) (f.tellg() - posBeforeHeaderSearch);
        //for fmt and fact there is no guranatee which comes first
        //so the search function is called twice
        fmt = searchForHeader(buff, read, WaveFormatString);
        fact = searchForHeader(buff, read, WaveFactString);
        //TODO search for data within fmt or fact search to speed up!
        data = searchForHeader(buff, read, WaveDataString);

        //fmt and data SHOULD be available, otherwise file is corrupted
        //fact is optional
        if (!fmt.first && !data.first) { //file is bad!
            f.close();
            return nullptr;
        }

        posBeforeHeaderSearch += 4; //since all headers are 4 byte

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
            //TODO parse fact
        }

        if (data.first) {
            data.second += posBeforeHeaderSearch;
            getDataFromFileHandle(wh, f, data);
        }

        f.close();
        return wh;
    }

    void getFmtFromFileHandle(WaveHeader* wh, std::ifstream& file,
                              data_pos dp) {
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
                wh->ErrorFlags.set(static_cast<int>(ERROR_NoFact_DataNotAfterFmt), 1);
            }
            return;
        } else if(leftToRead == 2) { //its fmt with 18 bytes
            file.read(reinterpret_cast<char*>(&wh->ExtensionSize),
                      sizeof(wh->ExtensionSize));
            file.read(&buff[0], 4);
            if(!wh->HasFact && string(&buff[0], 4) != WaveDataString) { //update error flag
                wh->ErrorFlags.set(static_cast<int>(ERROR_NoFact_DataNotAfterFmt), 1);
            }
            return;
        }

/*        //format is extended, still work to do ;)
        //how many bytes left to read?
        file.read(reinterpret_cast<char*>(&wh->ExtensionSize),
                  sizeof(wh->ExtensionSize));*/

        //standard extension sizes are 0 or 22 (0 is set by default in the struct)
        if(wh->ExtensionSize != 0 && wh->ExtensionSize != 22) {
            wh->WarningFlags.set(static_cast<int>(WARNING_NotStandardExtension), 1);
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
            wh->ErrorFlags.set(static_cast<int>(ERROR_NoFact_DataNotAfterFmt), 1);
        }
    }

    void getFactFromFileHandle(WaveHeader* wh, std::ifstream& file,
                               data_pos dp) {
        file.seekg(dp.second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&wh->FactSize), sizeof(wh->FactSize));
    }

    void getDataFromFileHandle(WaveHeader* wh, std::ifstream& file,
                               data_pos dp) {
        file.seekg(dp.second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&wh->DataSize), sizeof(wh->DataSize));
        wh->DataBegin = (uint32_t) file.tellg();
        //in a correctly formatted wave file, we should reach end of file
        //after "DataSize" bytes
        //TODO check if file ends at where it should end! (mind the pad byte if exists)
    }

    uint32_t convert4CharTo16_BigEndian(char* const buff) {
        return (uint32_t) (buff[0] | (buff[1] << 8) | (buff[2] << 16) |
                           (buff[3] << 24));
    }

    void printWaveHeader(const WaveHeader& wh) {

        cout << "-------------------------------------" << endl;
        cout << "Wave Header Information" << endl;
        cout << "File: \t\t\t" << wh.File << endl;
        cout << "Total size: \t\t" << wh.FileSize << " Bytes" << endl;
        cout << "Riff Size: \t\t" << wh.RiffSize << " Bytes" << endl;
        cout << "Wave ID: \t\t" << wh.WaveId << endl;
        cout << "Format Size: \t\t" << wh.FormatSize << " Bytes" << endl;
        cout << "AudioType: \t\t" << wh.FormatAudioType << endl;
        cout << "Channels: \t\t" << wh.NumberOfChannels << endl;
        cout << "Sample Rate: \t\t" << wh.SampleRate << " Hz" << endl;
        cout << "Byte Rate: \t\t" << wh.ByteRate << " Bps" << endl;
        cout << "Block Align: \t\t" << wh.BlockAlign << endl;
        cout << "Bits per Sample: \t" << wh.BitsPerSample << endl;
        cout << "Extension Size: \t" << wh.ExtensionSize << endl;
        if(wh.isExtendedFormat()) {
            cout << "\tExt. Valid Bits Per Sample: \t\t" << wh.ValidBitsPerSample
                    << endl;
            cout << "\tExt. Channel Mask: \t\t" << wh.ChannelMask << endl;
            cout << "\tGUID: ";
            auto flags = cout.flags(); //keep current console flags
            for(int i = 0; i < wh.ExtensionSize - 6; i++) {
                cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << +wh.SubFormat[i] << " ";
            }
            cout << endl;
            cout.flags(flags);
        }
        if(wh.HasFact) {
            cout << "FACT Details: \t\t" << endl;
            cout << "\tSize: \t\t" << wh.FactSize << endl;
            cout << "\tSample Length: \t\t" << wh.FactSampleLength << " Bytes" << endl;
            if(wh.FactSize > 4) { //standard is 4...but there maybe more
                cout << "\tNote: there is information which have been skipped!" << endl;
            }
        }
        cout << "Data Size: \t\t" << wh.DataSize << " Bytes" << endl;
        cout << "Data Begins: \t\t" << wh.DataBegin << endl;
        if(wh.WarningFlags.any()) {
            cout << "Warning Flags: \t\t" << wh.WarningFlags << endl;
        }
        if(wh.ErrorFlags.any()) {
            cout << "Error Flags: \t\t" << wh.ErrorFlags << endl;
        }
        cout << "-------------------------------------" << endl;
    }

    data_pos searchForHeader(char* buff, size_t size, const string& s) {
        data_pos dp(false, -1);
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