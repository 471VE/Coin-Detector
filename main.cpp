#include <sys/stat.h>
#include <iostream>

#include "helpers.h"
#include "processing.h"

std::string name;
metric type = IoU;
double threshold = 0.6;
bool skip = false;

void parse_cmd_args(int argc, char** argv) {
    if (argc < 2 || argc > 4) {
        std::cerr << "\nUnacceptable number of arguments. Exiting...";
        exit(1);
    }
    name = argv[1];
    if (argc > 2) {
        std::string mode  = argv[2];
        if (mode == "--skip-f1") {
            skip = true;
            if (argc > 3) {
                std::cerr << "\nUnacceptable number of arguments. Exiting...";
                exit(1);
            }
        } else if (mode == "independent") {
            type = INDEPENDENT;
            if (argc > 3) {
                std::cerr << "\nUnacceptable number of arguments. Exiting...";
                exit(1);
            }
        } else if (mode == "IoU") {
            type = IoU;
            if (argc == 4) {
                threshold = std::stod(argv[3]);
                if (threshold < 0 || threshold > 1) {
                    std::cerr << "\nUnacceptable value for threshold to IoU metric. Exiting...";
                    exit(1);
                }
            }
        }
         else {
            std::cerr << "\nUnaccaptable value for third parameter. "
            "Maybe you meant \"--skip-f1\", \"independent\", or \"IoU\"?";
        exit(1);
        }
    }
}


int main(int argc, char* argv[]) {
    parse_cmd_args(argc, argv);

    enum pathType {SINGLE_FILE, DIRECTORY};
    pathType currentPath;
    struct stat s;
    if (stat(name.c_str(), &s) == 0) {
        if( s.st_mode & S_IFDIR ) {
            currentPath = DIRECTORY;
        }
        else if( s.st_mode & S_IFREG ) {
            currentPath = SINGLE_FILE;
        } else {
            std::cerr << "\nThe path is neither a file nor a directory.";
            exit(1);
        }
    } else {
        std::cerr << "\nThe file or directory doesn't exist.";
        exit(1);
    }

    switch (currentPath) {
        case SINGLE_FILE: process_single_image(name, skip, type, threshold); break;
        case DIRECTORY: process_directory(name, skip, type, threshold); break;
    }

    return 0;
}