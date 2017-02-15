//
// Created by saeid on 14.02.17.
//

#ifndef CINEMO_THREADING_HPP
#define CINEMO_THREADING_HPP


#include <vector>
#include "LameWrapper.hpp"
#include "args.hpp"

using std::vector;

namespace cinemo {
    namespace threading {

        bool checkAndPrintFileValidity(const LameWrapper& lw);

        void doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                       const args::Options& options);

        void doSingleThreadedConversion(const vector<LameWrapper*>& lw,
                                        const args::Options& options);



        /**
        * @brief Function for performing encoding on a thread
        * @param arg Pointer to a LameWrapper object
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
