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

#ifndef CINEMO_UTILS_HPP
#define CINEMO_UTILS_HPP

#include <string>
#include <vector>
#include "LameWrapper.hpp"
#include "args.hpp"

#ifdef WIN32
#define PATH_SEPARATOR  '\\'
#else
#define PATH_SEPARATOR  '/'
#endif

using std::string;
using std::vector;

namespace cinemo {

    /**
     * @brief Evaluates working directory for finding wave files.
     * @param exec The argv[0] of the program (executable file)
     * @return true on success, false on failure.
     */
    bool getWorkingDirectoryFromExec(string& exec);

    /**
     * @brief Check to see if a given directory exists.
     * @param dir The path to check.
     * @return true on success, false on failure.
     */
    bool isValidWorkDirectory(const string& dir);

    /**
     * @brief Checks if a given directory contains valid 'wave' files.
     * @param dir The path to check.
     * @return true on success, false on failure.
     */
    bool containsValidWaveFiles(const string& dir);

    /**
     * @brief Check if a file has correct extension
     * @param file the file to check
     * @param ext the extension to check
     * @return true on success, false on failure
     */
    bool isValidFileType(const string& file, const char* ext);

    /**
     * @brief Get all valid wave files from a directory
     * @param dir the directory to get wave files from
     * @param workFiles the vector of {@see LameWrapper} to populate
     * @return true on success, false if no wave files were found
     */
    bool getWaveFiles(const string& dir, vector<LameWrapper*>& workFiles);

    /**
     * @brief Changes extension of a file
     * @param in the file to change its extension
     * @param ext the extension to change too
     * @return a string with the original extension is changed
     */
    string changeExt(const string& in, const string& ext);

    /**
     * @brief Checks a {@link LameWrapper} object for errors and warnings.
     *  It also writes on output stream!
     * @param lw The LameWrapper object to be checked.
     * @return true if no error, false if has errors
     */
    bool checkAndPrintFileValidity(const LameWrapper& lw,
                                   const args::Options& options);
}

#endif //CINEMO_UTILS_HPP
