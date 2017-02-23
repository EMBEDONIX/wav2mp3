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
#include <chrono>
#include <iomanip>
#include <algorithm>

#include "utils.hpp"
#include "threading.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using namespace cinemo;

int main(int argc, char* argv[]) {

    args::printCopyrights();
    args::Options options;
    string workDir;
    vector<LameWrapper*> lw;

    int retVal = args::processArgs(argc, argv, options, lw);

    if (retVal > 0) {
        return retVal;
    }

    if (lw.size() == 0) {
        return 0;
    }

    //Filter out files which will not be converted due to errors
    int lwSizeBeforeFilter = lw.size();
    cout << "Checking for incompatible files...\n" << endl;
    lw.erase(std::remove_if(begin(lw), end(lw),
                            [&](const LameWrapper* file) {
                                return checkAndPrintFileValidity(*file,
                                                                 options);
                            }), end(lw));

    int removed = lwSizeBeforeFilter - lw.size();
    if (removed > 0) {
        cout << "\n" << removed << " Files removed from encoding queue due to"
             << " incompatibility reasons!"
             << endl;
    }

    //sort files by size, descending, "may help with performance ;)"
    std::sort(begin(lw), end(lw),
              [](const LameWrapper* w1, const LameWrapper* w2) {
                  return w1->getHeader().FileSize > w2->getHeader().FileSize;
              });

    cout << "\nThere are " << lw.size()
         << " wave files to be encoded to mp3.";
    cout << endl;

    //Time measurement
    auto startTime = std::chrono::system_clock::now();

    //Do the work
    if (!options.noThread) {
        threading::doMultiThreadedConversion(lw, options);
    } else { //single thread mode for comparison with -n switch
        threading::doSingleThreadedConversion(lw, options);
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now() - startTime);

    cout << "\nTotal conversion time: "
         << std::fixed << std::setprecision(3) << elapsed.count() / 1000.0
         << " seconds." << endl;

	//write encoding results
	int success = 0, failure = 0;
	for_each(begin(lw), end(lw), [&](const LameWrapper* item) {
		if (item->isFinished())
			success++;
		else
			failure++;
	});

	cout << "\nConverted " << success << " files and skipped "
		<< failure << "." << endl;

    lw.clear();

    return 0;
}