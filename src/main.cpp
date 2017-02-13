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
// Created by saeid on 03.02.17.
//

#include <iostream>
#include <vector>

#ifdef WIN32
#else

#include <zconf.h>

#endif

#include "utils.hpp"
#include "args.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using namespace cinemo;

/**
 * @brief Number of threads for encoding
 */

int main(int argc, char *argv[]) {

    args::printCopyrights();

    bool optVerbose(false);
    string workDir;
    vector<LameWrapper*> lw;

    int retVal = args::processArgs(argc, argv, optVerbose, lw);

    if (retVal > 0) {
        return retVal;
    }

    if (lw.size() == 0) {
        return 0;
    }

    long numCores = -1;
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    numCores = sysinfo.dwNumberOfProcessors;
#else
    numCores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

    if (numCores <= 0) {
        numCores = 2;
    }

    cout << "There are " << lw.size()
         << " wave files to be encoded to mp3.";
    cout << endl;

    for (auto& work : lw) {
        if (!work->isValidWaveFile()) {
            cout << work->getFileName()
                 << " Appears to be invalid or corrupted."
                 << endl;
            continue;
        }
        cout << "\nStarting conversion of " << work->getFileName() << endl;
        if (optVerbose) {
            work->printWaveInfo();
        }
        cout << "Converting...";
        if (optVerbose) {
            cout << endl;
        }
        if (work->convertToMp3()) {
            cout << "Done." << endl;
        } else {
            cout << "canceled." << endl;
        }
    }

    lw.clear();

    return 0;
}