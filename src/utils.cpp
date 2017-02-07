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

    bool getWorkingDirectory(int argc, char* argv[], string& dir) {
        argc == 2 ? dir = string(argv[1]) :
                dir = string(argv[0]).substr(0, string(argv[0]).find_last_of(
                        "\\/"));
        return isValidWorkDirectory(dir);
    }

    bool getWaveFiles(const string& dir, vector<LameWrapper>& workFiles) {
        DIR* workDir = opendir(dir.c_str());
        if (workDir == nullptr) {
            return false;
        }

        dirent* dirIt = nullptr;
        while ((dirIt = readdir(workDir)) != nullptr) {
            if (isValidFileType(dirIt->d_name, extWave)) {
                try {
                    if (getFileSize(string(dir + "/" + dirIt->d_name)) <=
                        4096 * 10) {
                        std::cout << dirIt->d_name
                                  << " is too small for a WAV file "
                                  << " and will be skipped."
                                  << std::endl;
                        continue;
                    }
                    LameWrapper lw = LameWrapper(dir, dirIt->d_name);
                    wave::Wave* waveInfo = new wave::Wave(lw.getFullPath());
                    lw.setWaveInfo(waveInfo);
                    workFiles.push_back(lw);
                } catch (std::exception& e) {
                    //FIXME it only throws if file cannot be opened
                    //so should not rely on throws for validation ...
                    std::cout << dirIt->d_name << " is not a valid WAV file!"
                              << std::endl;
                    continue;
                }

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

    int getFileSize(const string& file) {
        std::ifstream is;
        is.open(file, std::ios::binary);
        is.seekg(0, std::ios::end);
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