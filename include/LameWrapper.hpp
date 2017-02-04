//
// Created by saeid on 04.02.17.
//

#ifndef CINEMO_LAMEWRAPPER_HPP
#define CINEMO_LAMEWRAPPER_HPP

#include <string>

using std::string;

namespace cinemo {

    class LameWrapper {
    private:
        string dir;
        string file;
        string path;
        bool isBusy;
        bool isDone;

    public:
        LameWrapper(const string &dir, const string &file);

        ~LameWrapper();

        void convertToMp3();

        string getDir() const { return dir; }

        string getFullPath() const { return path; }

        string getFileName() const { return file; }

        bool isConverting() const { return isBusy; }
    };
}


#endif //CINEMO_LAMEWRAPPER_HPP
