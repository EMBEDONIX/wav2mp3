#include <iostream>
#include <vector>

#include "utils.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;

using namespace cinemo;

int main(int argc, char *argv[]) {

#ifdef WIN32
    cout << "We are on windows!" << endl;
#endif
#ifdef LINUX
    cout << "We are on linux!" << endl;
#endif

    string workDir;
    if (getWorkingDirectory(argc, argv, workDir)) {
        cout << "Looking for wave files in " << workDir << endl;
        vector<LameWrapper> lw;

        if (!getWaveFiles(workDir, lw)) {
            cout << "Can not find any WAV file in " << workDir << endl;
            return 0;
        }

        for (auto work : lw) {
            cout << " starting conversion of " << work.getFileName() << endl;
            work.printWaveInfo();
            cout << "Converting...";
            work.convertToMp3();
            cout << "done" << endl;

        }
    } else {
        cout << workDir << " Does not exist" << endl;
        return 0;
    }

    return 0;
}