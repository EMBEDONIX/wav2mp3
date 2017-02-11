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
// Created by saeid on 03.02.17.
//

#include <iostream>
#include <vector>

#define NUM_THREADS 4

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

    //Prepare working files
    string workDir;
    vector<LameWrapper> lw;

    if (getWorkingDirectory(argc, argv, workDir)) {
        cout << "Looking for wave files in " << workDir << endl;
        if (!getWaveFiles(workDir, lw)) {
            cout << "Can not find any WAV file in " << workDir << endl;
            return 0;
        }
    } else {
        cout << workDir << " Does not exist" << endl;
        return 0;
    }

    //begin conversion
    for (auto work : lw) {
        if(!work.hasValidWaveHeader()) {
            cout << work.getFileName() << " Appears to be invalid or corrupt" << endl;
            continue;
        }
        cout << "\nStarting conversion of " << work.getFileName() << endl;
        work.printWaveInfo();
        cout << "Converting..." << endl;
        work.convertToMp3();
        cout << "Done." << endl;
    }

    return 0;
}