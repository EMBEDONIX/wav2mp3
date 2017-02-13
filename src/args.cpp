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
// Created by saeid on 12.02.17.
//

#include "args.hpp"
#include <iostream>

#include "utils.hpp"


using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace cinemo {

    int args::processArgs(int argc, char* argv[], bool& optVerbose,
                          vector<LameWrapper*>& lw) {

        string workDir;
        string arg1;
        string arg2;

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
                    cout << "Invalid option '" << arg1
                         << "'. Use -h for usage." << endl;
                    return 1;
                }
            } else {
                workDir = string(argv[1]);
                getWorkingDirectoryFromExec(workDir);
            }
        }

        if (isValidWorkDirectory(workDir)) {
            cout << "Looking for wave files in " << workDir << endl;
            if (!getWaveFiles(workDir, lw)) {
                cout << "Can not find any WAV file in " << workDir << endl;
            }
        } else {
            cout << workDir << " Is not a directory." << endl;
        }

        return 0;
    }

    void args::printCopyrights() {
        cout
                << "Wave to Mp3 converter v0.1 by Saeid Yazdani, www.embedonix.com | "
                << "Linked against libmp3lame " << get_lame_version() << "\n"
                << "This open source program is licensed under GPL v3.\n"
                << "As a free software, nothing is guaranteed. I wish you a no-crash experience.\n"
                << "When in doubt, Use \"-h\" switch for usage information.\n"
                << "-------------------------------------------------------\n"
                << endl;
    }

    void args::printUsage() {
        cout
                << "Usage:\n"
                << "\"wave2mp3 [path]\"\n\tlook for wav files in the directory "
                << "(current directory if not provided) and converts them to mp3"
                << " under the same name with mp3 extension.\n"
                << "\"wav2mp3 [-v] [current directory/path]\"\n\tsame as above"
                << " but with verbose output messages.\n"
                << "\"wav2mp3 -e\"\n\tprints explanation about error/warning flags.\n"
                << "\"wav2mp3 -h\"\n\tprints this help."
                << endl;
    }

}
