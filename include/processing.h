#ifndef PROCESSING_H
#define PROCESSING_H

#include "helpers.h"

std::array<int, 3> process_single_image(const std::string& imageName, const bool& skip, metric mode = IoU, double threshold = 0.6);
void process_directory(const std::string& directory, const bool& skip, metric mode = IoU, double threshold = 0.6);

#endif // PROCESSING_H