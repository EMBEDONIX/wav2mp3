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
#ifndef HAVE_STRUCT_TIMESPEC //fix for redefinition in time.h of windows SDK
#define HAVE_STRUCT_TIMESPEC
#endif
#include "win/pthread.h"
#include "win/dirent.h" //for number of cores
#else

#include <zconf.h>

#endif

#include "threading.hpp"

using std::cout;
using std::endl;
using std::for_each;
using std::begin;
using std::end;

namespace cinemo {
    namespace threading {

        //used only for organizing cout calls
        static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

        long getCpuCoreCount() {
            long numCores = 0;
#ifdef WIN32
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            numCores = sysinfo.dwNumberOfProcessors;
#else
            numCores = sysconf(_SC_NPROCESSORS_ONLN);
#endif
            if (numCores > 0) { //check if num of cores could be detected
                return numCores;
            }

            return 4; //rule of thumb! assume quad core processor
        }

        void doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                       const args::Options& options) {

            long numThreads = getCpuCoreCount() * THREADS_PER_CORE;
            if (lw.size() < numThreads) { //prevents overkill ;)
                numThreads = lw.size();
            }
            
			//TODO use vector instead of new[]
			WorkParams* wp = divideWork(lw, numThreads);
			pthread_t* threads = new pthread_t[numThreads];
			int* tCreateResults = new int[numThreads];
			int* tJoinResults = new int[numThreads];

			cout << "\nMulti-threaded Conversion is in progress..." << endl;

			//create threads
			for (int i = 0; i < numThreads; ++i) {
				wp[i].options = options;
				wp[i].threadId = i;
				tCreateResults[i] = pthread_create(&threads[i], nullptr,
					doWork, &wp[i]);
			}

			//wait for threads to finish
			//since files were sorted by size before, waiting for last threads to first
			//is going to be faster (at least theoritically!)
			for (int i = numThreads - 1; i >= 0; --i) {
				tJoinResults[i] = pthread_join(threads[i], nullptr);
			}


			for (int i = 0; i < numThreads; i++) {
				wp[i].works.clear(); //clear work vector of thread

										//check for errors
				if (tCreateResults[i] != 0 || tJoinResults[i] != 0) {
					cout << "There was a problem with Thread #" << i << endl;
				}
			}

            //free the allocated memory
            delete[] threads, tCreateResults, tJoinResults, wp;
        }

        void doSingleThreadedConversion(const vector<LameWrapper*>& lw,
                                        const args::Options& options) {
            cout << "\nSingle-threaded conversion is in progress..." << endl;

            for (auto& work : lw) {
				cout << "Thread 0" << " => " << work->getFileName()
					<< endl;
                if (options.verbose) {
                    work->printWaveInfo();
                }
                work->convertToMp3();
            }
        }

        static WorkParams* divideWork(const vector<LameWrapper*>& lw,
                                      long numThreads) {
            WorkParams* wp = new WorkParams[numThreads];
            int ja = 0; //Jobs Assigned

            while (ja < lw.size()) {
                for (int i = 0; i < numThreads && ja < lw.size(); ++i) {
                    wp[i].works.push_back(lw[ja++]);
                }
            }

            return wp;
        }

        static void* doWork(void* arg) {
            WorkParams* wp = static_cast<WorkParams*>(arg);
            for_each(begin(wp->works), end(wp->works),
                     [&](LameWrapper* lw) {
						pthread_mutex_lock(&mutex); //syncs outstream!
						cout << "Thread #" << wp->threadId
							<< " => " << lw->getFileName() << endl;
                        if (wp->options.verbose) {
							lw->printWaveInfo();
                         }
						pthread_mutex_unlock(&mutex);

                        lw->convertToMp3();
                     });

			return static_cast<void*>(nullptr);
        }
    }
}