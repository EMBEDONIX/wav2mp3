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

#ifndef CINEMO_ARGS_HPP
#define CINEMO_ARGS_HPP

#include <vector>

#include "LameWrapper.hpp"
#include "args.hpp"

using std::vector;

namespace cinemo {
    /**
     * @brief Contains functions for parsing command line options
     */
    namespace args {

        /**
         * @brief Structure for command line options parsing
         */
        struct Options {
            /** -h was passed? then show help*/
            bool help = false;
            /** -v was passed? then details should be printed when converting */
            bool verbose = false;
            /** -n was passed? then run jobs on single (main) thread */
            bool noThread = false;
            //TODO get quality from arguments as well!
            /** -q[0..9] was passed? then pass this as the quality factor to lame */
            int quality = 3;
        };

        /**
         * @brief Validates input arguments of the program.
         * @param argc Number of arguments.
         * @param argv Array of char as arguments.
         * @param options To set the provided options
         * @param lw vector of {@see LameWrapper} pointers to fill
         * @return in case of wrong arguments -1, otherwise 0
         */
        int processArgs(int argc, char* argv[], Options& options,
                        vector<LameWrapper*>& lw);

        /**
         * @brief Prints the programs copyright and disclaimers.
         */
        void printCopyrights();

        /**
         * @brief Prints the usage of the program (help)
         */
        void printUsage();
    }
}


#endif //CINEMO_ARGS_HPP
