//
// Created by saeid on 14.02.17.
//


#ifdef WIN32
//TODO zconf alternative headers for windows systems
#else

#include <zconf.h>

#endif

#include "threading.hpp"

namespace cinemo {

    void* threading::doWork(void* arg) {
        LameWrapper* lw = static_cast<LameWrapper*>(arg);
        lw->convertToMp3(0);
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

        if (numCores > 2) {
            return numCores;
        }

        return 2;
    }
}