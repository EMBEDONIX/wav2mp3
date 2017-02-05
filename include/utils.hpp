//
// Created by saeid on 04.02.17.
//

#ifndef CINEMO_UTILS_HPP
#define CINEMO_UTILS_HPP

#include <string>
#include <vector>
#include "LameWrapper.hpp"

using std::string;
using std::vector;

namespace cinemo {

    /**
     * @brief Gets the directory from program arguments.
     * @param argc Number of provided arguments.
     * @param argv Value of arguments.
     * @param dir  The string reference to be populated by path.
     * @return true on success, false on failure.
     */
    bool getWorkingDirectory(int argc, char *argv[], string &dir);

    /**
     * @brief Check to see if a given directory exists.
     * @param dir The path to check.
     * @return true on success, false on failure.
     */
    bool isValidWorkDirectory(const string &dir);

    /**
     * @brief Checks if a given directory contains valid 'wave' files.
     * @param dir The path to check.
     * @return true on success, false on failure.
     */
    bool containsValidWaveFiles(const string &dir);

    bool isValidFileType(const string &file, const char *ext);

    bool getWaveFiles(const string &dir, vector<LameWrapper> &workFiles);

    string changeExt(const string& in, const string& ext);
}

#endif //CINEMO_UTILS_HPP
