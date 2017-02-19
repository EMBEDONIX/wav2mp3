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

#ifndef CINEMO_THREADING_HPP
#define CINEMO_THREADING_HPP

#include <vector>
#include "LameWrapper.hpp"
#include "args.hpp"

#define THREADS_PER_CORE    2

using std::vector;

namespace cinemo {
    namespace threading {

        /**
         * @brief Struct for passing works to pthreads
         */
        struct WorkParams {
            /** ID of the thread assigned to process this jobs */
			int threadId;
            /** Copy of command line options parsed at the start of program */
            args::Options options; //will be a copy!
            /** Vector of LameWrapper pointers to be processed by this thread */
            vector<LameWrapper*> works;
		};

        /**
         * @brief Performs the encoding work with multiple threads
         * @param lw vector of {@see LameWrapper} pointers
         * @param options command line options {@see args::Options}
         */
        void doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                       const args::Options& options);

        /**
         * @brief Performs the encoding work in single thread
         * @param lw vector of {@see LameWrapper} pointers
         * @param options command line options {@see args::Options}
         */
        void doSingleThreadedConversion(const vector<LameWrapper*>& lw,
                                        const args::Options& options);

        /**
        * @brief Divide encoding work between threads.
        *   Note: Since there are no shared resources between jobs,
        *   There is no need for fancy pooling/queue systems!
        * @param lw vector of {@see LameWrapper} pointers
        * @param Number of threads
        * @return Array of JobParams (will have 'numThreads' elements)
        */
        static WorkParams* divideWork(const vector<LameWrapper*>& lw,
                                      long numThreads);


        /**
        * @brief Function for performing encoding on a thread
        * @param arg Pointer to a WorkParams object
        * @return nothing
        */
        static void* doWork(void* arg);

        /**
         * @brief Tries to get number of available cores of host system
         * @return number of cpu cores. If not successful, will return 2
         */
        long getCpuCoreCount();
    }
}

#endif //CINEMO_THREADING_HPP
