#include <iostream>
#include <vector>

#include "../include/utils.hpp"
#include "../include/LameWrapper.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;

using namespace cinemo;

int main(int argc, char *argv[]) {

    string workDir;
    if (getWorkingDirectory(argc, argv, workDir)) {
        cout << "Looking for wave files in " << workDir << endl;
        vector<LameWrapper> lw;
        if (!getWaveFiles(workDir, lw)) {
            cout << "Can not find any WAV file in " << workDir << endl;
            return 0;
        }
        for (auto n : lw) {
            cout << " starting conversion of " << n.getFileName() << endl;
            n.convertToMp3();
        }
    } else {
        cout << workDir << " Does not exist" << endl;
        return 0;
    }


    return 0;
}