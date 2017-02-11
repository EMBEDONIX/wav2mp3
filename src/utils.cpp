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

#include "utils.hpp"

#include <fstream>

//Platform specific includes
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>
#include <iostream>
#include <regex>

namespace cinemo {

    /**
     * @brief Valid wave file extension
     */
    const char* extWave = "wav";

    bool isValidWorkDirectory(const string& dir) {
        struct stat st;
        //TODO check for existence of WAV files right here...
        return stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool getWorkingDirectoryFromExec(string& exec) {
        exec = exec.substr(0, string(exec).find_last_of("\\/"));
        return isValidWorkDirectory(exec);
    }

    bool getWaveFiles(const string& dir, vector<LameWrapper>& workFiles) {
        DIR* workDir = opendir(dir.c_str());
        if (workDir == nullptr) {
            return false;
        }

        dirent* dirIt = nullptr;
        while ((dirIt = readdir(workDir)) != nullptr) {
            if (isValidFileType(dirIt->d_name, extWave)) {
                    LameWrapper lw = LameWrapper(dir, dirIt->d_name);
                    workFiles.push_back(lw);
            }
        }

        return workFiles.size() > 0;
    }

    bool isValidFileType(const string& file, const char* ext) {
        bool isValid = false;
        string fileExt;
        //first, check extension
        string::size_type i;
        i = file.rfind('.');
        if (i != string::npos) {
            fileExt = file.substr(i + 1);
            if (!strcasecmp(fileExt.c_str(), ext)) {
                isValid = true;
            }
        } else {
            isValid = false;
        }

        return isValid;
    }

    long getFileSize(const string& file) {
        std::ifstream is;
        is.open(file, std::ios::in | std::ios::binary
                      | std::ios::ate);
        return is.tellg();
    }

    string changeExt(const string& in, const string& ext) {
        //FIXME currently wav is hardcoded
        std::regex regex("^(.*)\\.wav$");
        //FIXME only works with GCC 4.9+ (e.g. not with 4.8)
        return std::regex_replace(in, regex, string("$1." + ext).c_str());
        //return string(in + ".mp3");
    }
}