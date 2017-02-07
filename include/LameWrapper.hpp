//
// Created by saeid on 04.02.17.
//

#ifndef CINEMO_LAMEWRAPPER_HPP
#define CINEMO_LAMEWRAPPER_HPP

#include <string>
//TODO forward declare
#include "wave.h"

#include "lame/lame.h"

using std::string;
using namespace wave;

namespace cinemo {

    class LameWrapper {
    private:
        string dir;
        string file;
        string path;
        //TODO use std unique ptr
        Wave* waveFile;
        bool isBusy;
        bool isDone;

        int getLameFlags(lame_t& l);

        bool doEncoding(const string& in, const string& out,
                        const lame_t& lame);

    public:
        LameWrapper(const string &dir, const string &file);

        ~LameWrapper();

        void convertToMp3();

        string getDir() const { return dir; }

        string getFullPath() const { return path; }

        string getFileName() const { return file; }

        bool isConverting() const { return isBusy; }

        Wave* getWaveInfo() const { return waveFile; }

        void setWaveInfo(Wave* wave);

        void printWaveInfo();
    };
}


#endif //CINEMO_LAMEWRAPPER_HPP
