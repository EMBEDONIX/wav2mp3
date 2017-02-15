//
// Created by saeid on 14.02.17.
//


#ifdef WIN32
//TODO zconf alternative headers for windows systems
#else

#include <zconf.h>

#endif

#include <iostream>

#include "threading.hpp"

using std::cout;
using std::endl;

namespace cinemo {

    bool threading::checkAndPrintFileValidity(const LameWrapper& lw) {
        bool result = true;
        if (!lw.isValidWaveFile()) {
            cout << "\t" << lw.getFileName()
                 << " Appears to be invalid or corrupted."
                 << "Check the Error/Warning flags."
                 << endl;
            result = false;
        }
        return result;
    }

    static void* threading::doWork(void* arg) {
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

    void threading::doMultiThreadedConversion(const vector<LameWrapper*>& lw,
                                              const args::Options& options) {

        long numCores = getCpuCoreCount();
        int numFiles = lw.size();

        pthread_t* t = new pthread_t[numCores];
        int* results = new int[numFiles * 2];

        /*FIXME for single file or last loop where files that does not fit in the numCores range
         this method is inefficent...I know the solution but haven't got time right now
        */
        for (int i = 0; i < numFiles; i += numCores) {
            int k = i; //threads per loop
            int l = 0; //number of jobs per loop
            for (int j = 0; j < numCores && (k < numFiles); ++j) {
                cout << "Thread #" << j << " => " << lw[k]->getFileName()
                     << endl;
                if (options.verbose) {
                    lw[k]->printWaveInfo();
                }
                if (!checkAndPrintFileValidity(*lw[k])) {
                    k++;
                    continue;
                }
                results[i] = pthread_create(&t[j], NULL, threading::doWork,
                                            lw[k++]);
                l++;
            }

            for (int j = 0; j < l; ++j) {
                pthread_join(t[j], NULL);
            }
        }
        //TODO check results!!!
        delete t, results;
    }

    void threading::doSingleThreadedConversion(
            const vector<LameWrapper*>& lw, const args::Options& options) {
        for (auto& work : lw) {
            if (options.verbose) {
                work->printWaveInfo();
            }

            cout << "Thread 0:" << " => " << work->getFileName() << endl;
            if (!checkAndPrintFileValidity(*work)) {
                continue;
            }

            work->convertToMp3(0);
        }

    }
}