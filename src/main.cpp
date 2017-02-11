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

namespace cinemo {
    namespace args {
        void printCopyrights() {
            cout
                    << "Wave to Mp3 converter v0.1 by Saeid Yazdani, www.embedonix.com\n"
                            "This open source program is licensed under GPL v3.\n"
                    << endl;
        }

        void printUsage() {
            //TODO clean up and better (standard) formatting
            cout << "Usage:" << endl;
            cout
                    << "\"wave2mp3 [path]\"\n\tlook for wav files in the directory "
                            "(current directory if not provided) and converts them to mp3"
                            " under the same name with mp3 extension." << endl;
            cout << "\"wav2mp3 [-v] [current directory/path]\"\n\tsame as above"
                    " but with verbose output messages." << endl;
            cout
                    << "\"wav2mp3 -e\"\n\tprints explanation about error/warning flags."
                    << endl;
            cout << "\"wav2mp3 -h\"\n\tprints this help." << endl;
        }
    }
}

int main(int argc, char *argv[]) {

    args::printCopyrights();

    bool optVerbose(false);
    string workDir;
    vector<LameWrapper> lw;

    //only upto 2 arguments are considered valid, arg0 is exec itself
    string arg1;
    string arg2;

    //some simple argument check
    //FIXME I know I might be blamed for this mess, but later I'll use getopt header
    workDir = argv[0];
    if (argc == 1) {
        getWorkingDirectoryFromExec(workDir);
    } else if (argc == 2 || argc == 3) {
        arg1 = string(argv[1]);
        if (arg1.find("-") == 0) {
            if (arg1.find("h") == 1) {
                args::printUsage();
                return 0;
            } else if (arg1.find("e") == 1) {
                wh::printFlags();
                return 0;
            } else if (arg1.find("v") == 1) {
                optVerbose = true;
                if (argc < 3) {
                    getWorkingDirectoryFromExec(workDir);
                } else if (argc == 3) {
                    workDir = string(argv[2]);
                }
            } else {
                cout << "Invalid option. Use -h for usage." << endl;
                return -1;
            }
        } else {
            workDir = string(argv[1]);
            getWorkingDirectoryFromExec(workDir);
        }
    }

#ifdef WIN32
    //    cout << "We are on windows!" << endl;
#endif
#ifdef LINUX
//    cout << "We are on linux!" << endl;
#endif


    //dir = string(argv[0]).substr(0, string(argv[0]).find_last_of("\\/"));

    if (isValidWorkDirectory(workDir)) {
        cout << "Looking for wave files in " << workDir << endl;
        if (!getWaveFiles(workDir, lw)) {
            cout << "Can not find any WAV file in " << workDir << endl;
            return 0;
        }
    } else {
        cout << workDir << " Is not a directory." << endl;
        return 0;
    }

    //begin conversion
    for (auto work : lw) {
        if (!work.isValidWaveFile()) {
            cout << work.getFileName() << " Appears to be invalid or corrupted."
                 << endl;
            continue;
        }
        cout << "\nStarting conversion of " << work.getFileName() << endl;
        if (optVerbose) {
            work.printWaveInfo();
        }
        cout << "Converting...";
        if (optVerbose) {
            cout << endl;
        }
        if (work.convertToMp3()) {
            cout << "Done." << endl;
        } else {
            cout << "canceled." << endl;
        }
    }

    lw.clear();

    return 0;
}