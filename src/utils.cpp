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
#include <sys/stat.h>
#include <iostream>
#include <regex>

//Platform specific includes
#ifdef WIN32
#include "win/dirent.h"
#define PATH_SEPARATOR  "\\"
#else
#define PATH_SEPARATOR  "/"
#include <dirent.h>
#endif

#ifdef _MSC_VER //for string compare ignore case, if on MSVC
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif


namespace cinemo {

    /**
     * @brief Valid wave file extension
     */
    static const char* extWave = "wav";

    bool isValidWorkDirectory(const string& dir) {
        struct stat st;
        return stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool getWorkingDirectoryFromExec(string& exec) {
        exec = exec.substr(0, string(exec).find_last_of(PATH_SEPARATOR));
        return isValidWorkDirectory(exec);
    }

    bool getWaveFiles(const string& dir, vector<LameWrapper*>& workFiles) {
        DIR* workDir = opendir(dir.c_str());
        if (workDir == nullptr) {
            return false;
        }

        dirent* dirIt = nullptr;
        while ((dirIt = readdir(workDir)) != nullptr) {
            if (isValidFileType(dirIt->d_name, extWave)) {
                workFiles.push_back(new LameWrapper(dir, dirIt->d_name));
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

    string changeExt(const string& in, const string& ext) {
        //FIXME currently wav is hardcoded
        std::regex regex("^(.*)\\.wav$");
        //FIXME only works with GCC 4.9+ (e.g. not with 4.8)
        return std::regex_replace(in, regex, string("$1." + ext).c_str());
    }

    //This function is here because it shouldnt  be called within a thread!
    bool checkAndPrintFileValidity(const LameWrapper& lw,
                                   const args::Options& options) {

        if (!lw.isValidWaveFile()) {
            std::cout << lw.getFileName()
                      << " Appears to be invalid or corrupted.";
            if (options.verbose) {
                std::cout << " Flags = "
                          << "Errors: " << lw.getHeader().ErrorFlags
                          << ", Warnings: " << lw.getHeader().WarningFlags;
            }
            std::cout << std::endl;
            return true;
        }

        return false;
    }
}
