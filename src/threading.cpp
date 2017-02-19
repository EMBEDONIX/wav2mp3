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
#ifndef HAVE_STRUCT_TIMESPEC
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

        void doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                       const args::Options& options) {

            long numThreads = getCpuCoreCount() * JOBS_PER_CORE;
			if(lw.size() < numThreads)	{
				numThreads = lw.size();
			}

			JobParams* jp = divideWork(lw, numThreads);
			pthread_t* threads = new pthread_t[numThreads];
			int* tCreateResults = new int[numThreads];
			int* tJoinResults = new int[numThreads];

			cout << "\nConversion is in progress..." << endl;

			//create threads
			for(int i = 0; i < numThreads; ++i)	{
				jp[i].options = options;
				tCreateResults[i] = pthread_create(&threads[i], nullptr,
					doWork, &jp[i]);
			}

			//wait for threads to finish
			for (int i = 0; i < numThreads; ++i) {
				tJoinResults[i] = pthread_join(threads[i], nullptr);
			}

			//TODO check the create and join results

            delete[] threads, tCreateResults, tJoinResults;
        }

        void doSingleThreadedConversion(
                const vector<LameWrapper*>& lw, const args::Options& options) {
            for (auto& work : lw) {
                if (options.verbose) {
                    work->printWaveInfo();
                }

                cout << "Thread 0:" << " => " << work->getFileName() << endl;
                work->convertToMp3();
            }
        }

		static JobParams* divideWork(const vector<LameWrapper*>& lw,
			long numThreads) {
			JobParams* jp = new JobParams[numThreads];
			int ja = 0; //Jobs Assigned

			while (ja < lw.size()) {
				for (int i = 0; i < numThreads && ja < lw.size(); ++i) {
					jp[i].threadId = i;
					jp[i].works.push_back(lw[ja++]);
				}
			}

			for (int i = 0; i < numThreads; ++i) {
				cout << "Jobs assigned to thread #" << i
					<< " => " << jp[i].works.size() << endl;

			}

			return jp;
		}

		static void* doWork(void* arg) {
			JobParams* jp = static_cast<JobParams*>(arg);
			for_each(begin(jp->works), end(jp->works),
				[&](LameWrapper* lw) {
				
				//print some information if verbose option is true
				if(jp->options.verbose)	{
					pthread_mutex_lock(&mutex); //syncs outstream!
					cout << "\nThread #" << jp->threadId
						<< " => " << lw->getFileName() << endl;
					lw->printWaveInfo();
					pthread_mutex_unlock(&mutex);
				}

				lw->convertToMp3();
			});
			return static_cast<void*>(nullptr);
		}
    }
}