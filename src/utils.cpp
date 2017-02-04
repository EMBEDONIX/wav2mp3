//
// Created by saeid on 04.02.17.
//

#include "../include/utils.hpp"
#include "../include/LameWrapper.hpp"

//Platform specific includes
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <strings.h>
#include <iostream>

namespace cinemo {

    /**
     * @brief Valid wave file extension
     */
    const char *extWave = "wav";
    /**
     * @brief Valid mp3 file extension
     */
    const char *extMp3 = "mp3";


    bool isValidWorkDirectory(const string &dir) {
        struct stat st;
        //TODO check for existence of WAV files right here...
        return stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool containsValidWaveFiles(const string &dir) {
        //
    }

    bool getWorkingDirectory(int argc, char *argv[], string &dir) {
        argc == 2 ? dir = string(argv[1]) :
                dir = string(argv[0]).substr(0, string(argv[0]).find_last_of(
                        "\\/"));
        return isValidWorkDirectory(dir);
    }

    bool getWaveFiles(const string &dir, vector<LameWrapper> &workFiles) {
        DIR *workDir = opendir(dir.c_str());
        if (workDir == nullptr) {
            return false;
        }

        dirent *dirIt = nullptr;
        while ((dirIt = readdir(workDir)) != nullptr) {
            if (isValidFileType(dirIt->d_name, extWave)) {
                //TODO check if mp3 file with same name exist or not!
                workFiles.push_back(LameWrapper(dir, dirIt->d_name));
            }
        }

        return workFiles.size() > 0;
    }

    bool isValidFileType(const string &file, const char *ext) {
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
}