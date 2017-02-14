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
#include <chrono>
#include <iomanip>

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
 * @brief Function for performing encoding on a thread
 * @param arg Pointer to a LameWrapper object
 * @return void???
 */
static void* doWork(void* arg) {
    LameWrapper* lw = static_cast<LameWrapper*>(arg);
    lw->convertToMp3(0);
}


bool checkAndPrintFileValidity(const LameWrapper& lw) {
    bool result = true;
    if (!lw.isValidWaveFile()) {
        cout << lw.getFileName()
             << " Appears to be invalid or corrupted. Check the Error/Warning flags."
             << endl;
        result = false;
    }
    return result;
}


int main(int argc, char* argv[]) {

    args::printCopyrights();
    args::Options options;
    string workDir;
    vector<LameWrapper*> lw;

    int retVal = args::processArgs(argc, argv, options, lw);

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
        numCores = 2; //Rule of thumb!!!!!!
    }

    cout << "There are " << lw.size()
         << " wave files to be encoded to mp3.";
    cout << endl;

    //Time measurement
    auto startTime = std::chrono::system_clock::now();

    int numFiles = 0;
    for (auto& file : lw) {
        numFiles++;
    }

    if (!options.noThread || numFiles != 1) {

        pthread_t* t = new pthread_t[numCores];
        int* results = new int[numFiles * 2];

        /*FIXME for single file or last loop where files that does not fit in the numCores range
         this method is inefficent...I know the solution but haven't got time right now
        */
        for (int i = 0; i < numFiles; i += numCores) {
            int k = i; //threads per loop
            int l = 0; //number of jobs per loop
            for (int j = 0; j < numCores && (k < numFiles); ++j) {
                cout << "File #" << k << " => " << lw[k]->getFileName() << endl;
                if (!checkAndPrintFileValidity(*lw[k])) {
                    continue;
                }
                if (options.verbose) {
                    lw[k]->printWaveInfo();
                }
                results[i] = pthread_create(&t[j], NULL, doWork, lw[k++]);
                l++;
            }
            for (int j = 0; j < l; ++j) {
                pthread_join(t[j], NULL);
            }
        }
        //TODO check results!!!
        delete t, results;
    } else { //single thread mode for comparison with -n switch
        int i = 0; //TODO start from 1?
        for (auto& work : lw) {
            if (!checkAndPrintFileValidity(*work)) {
                continue;
            }
            cout << "File #" << i++ << " => " << work->getFileName() << endl;
            if (options.verbose) {
                work->printWaveInfo();
            }
            work->convertToMp3(0);
        }
    }

    //Calculate and print duration
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now() - startTime);
    cout << "Encode time: "
         << std::fixed << std::setprecision(3) << elapsed.count() / 1000.0
         << " seconds." << endl;

    lw.clear();
    return 0;
}