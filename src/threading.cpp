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
// Created by saeid on 14.02.17.
//

#include <iostream>
#include <algorithm>
#ifdef WIN32
#define HAVE_STRUCT_TIMESPEC
#include "win/pthread.h"
#include "win/dirent.h" //for number of cores
#else
#include <zconf.h>
#endif

#include "threading.hpp"

using std::cout;
using std::endl;
using std::for_each;

namespace cinemo {

    static void* threading::doWork(void* arg) {
        LameWrapper* lw = static_cast<LameWrapper*>(arg);
        lw->convertToMp3();
        return static_cast<void*>(nullptr);
    }

    long threading::getCpuCoreCount() {
        long numCores;

#ifdef WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        numCores = sysinfo.dwNumberOfProcessors;
#else
        numCores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

        if (numCores > 0) {
            return numCores; //2 threads per core
        }

        return 4; //rule of thumb! assume 4 core processor if cant determine
    }

    void threading::doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                              const args::Options& options) {

        long numThreads = getCpuCoreCount() * 2; //2 threads per core
        int numFiles = lw.size();

        //if less jobs than number of threads
        if (numFiles <= numThreads) {
            numThreads = numFiles;
        }

        pthread_t* t = new pthread_t[numThreads];
        int* results = new int[numFiles];
        vector<int> activeThreads(numThreads);

        /*FIXME for single file or last loop where files that does not fit in the numThreads range
         this method is inefficient...a thread pool/queue might perform better
         since if e.g. 2 file go into threads but one is finished much earlier
         the program should wait for the other thread to finish before releasing
         2 new threads...
        */
        for (int i = 0; i < numFiles; i += numThreads) {
            int k = i; //threads per loop
            for (int j = 0; (j < numThreads) && (k < numFiles); ++j) {
                cout << "Thread #" << j << " => " << lw[k]->getFileName()
                     << endl;
                if (options.verbose) {
                    lw[k]->printWaveInfo();
                }
                if (k + 1 == numFiles) { //do last job on main thread
                    lw[k]->convertToMp3();
                    break;
                }
                activeThreads.push_back(j); //to see for which to wait to join
                results[i] = pthread_create(&t[j], nullptr, threading::doWork,
                                            lw[k++]);
            }

            for_each(begin(activeThreads), end(activeThreads),
                     [&](int& activeIndex) {
                         pthread_join(t[activeIndex], nullptr);
                     });
            activeThreads.clear();
            cout << endl;
        }
        //TODO check results!!!
        delete[] t, results;
    }

    void threading::doSingleThreadedConversion(
            const vector<LameWrapper*>& lw, const args::Options& options) {
        for (auto& work : lw) {
            if (options.verbose) {
                work->printWaveInfo();
            }

            cout << "Thread 0:" << " => " << work->getFileName() << endl;

            work->convertToMp3();
        }

    }
}