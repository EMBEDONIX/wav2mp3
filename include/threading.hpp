//
// Created by saeid on 14.02.17.
//

#ifndef CINEMO_THREADING_HPP
#define CINEMO_THREADING_HPP


#include "LameWrapper.hpp"

namespace cinemo {
    namespace threading {

        /**
        * @brief Function for performing encoding on a thread
        * @param arg Pointer to a LameWrapper object
        * @return nothing
        */
        void* doWork(void* arg);

        /**
         * @brief Tries to get number of available cores of host system
         * @return number of cpu cores. If not successful, will return 2
         */
        long getCpuCoreCount();
    }
}

#endif //CINEMO_THREADING_HPP
