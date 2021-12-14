#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "processing.h"


std::array<int, 3> process_single_image(const std::string& imageName, const bool& skip, metric mode, double threshold) {
    std::string rawName = imageName.substr(0, imageName.find_last_of("."));
    std::string shortName = rawName.substr(rawName.find_last_of("\\"), rawName.size());
    std::string extension = imageName.substr(imageName.find_last_of("."), imageName.size());
    std::string directory = rawName.substr(0, rawName.find_last_of("\\"));
    std::string textName = directory + "\\parameters" + shortName + ".txt";

    auto img = cv::imread(imageName);
    show_image("Original Image", img);

    int scale = std::min(img.rows, img.cols);
    int filterSizeBox  = scale / 100;

    cv::Mat grayImg, boxImg, biImg;
    cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);

    cv::boxFilter(grayImg, boxImg, -1, cv::Size(filterSizeBox, filterSizeBox));

    std::vector<cv::Vec3f> circlesDetected;
    int minDistance = scale / 20;
	cv::HoughCircles(boxImg, circlesDetected, cv::HOUGH_GRADIENT_ALT, 2, minDistance, 200, 0.7);

    filter_circles(circlesDetected);
    correct_circle_positions(boxImg, circlesDetected);
    draw_circles(img, circlesDetected);

    std::string directoryDetected = directory + "\\detected";
    std::string logDirectory = directoryDetected + "\\log";
    check_directory(directoryDetected);
    check_directory(logDirectory);

    std::string detectedImageName = directoryDetected + shortName + "_detected" + extension;
    std::string logName = logDirectory + shortName + "_log.txt";
    print_circles_info(imageName, logName, circlesDetected);
	show_image("Image With Detected Circles", img);
    cv::imwrite(detectedImageName, img);

    std::array<int, 3> statistics;
    if (!skip) {
        statistics = calculate_statistics(img, textName, logName, circlesDetected, mode, threshold);
    }
   
    return statistics;
}

void process_directory(const std::string& directory, const bool& skip, metric mode, double threshold) {
    std::vector<std::string> imageNamesJPG, imageNamesJPEG, imageNamesPNG, imageNames;

    cv::glob(directory + "\\*.jpg", imageNamesJPG, false);
    cv::glob(directory + "\\*.jpeg", imageNamesJPEG, false);
    cv::glob(directory + "\\*.png", imageNamesPNG, false);

    imageNames.reserve(imageNamesJPG.size() + imageNamesJPEG.size() + imageNamesPNG.size());
    imageNames.insert(imageNames.end(), imageNamesJPG.begin(), imageNamesJPG.end());
    imageNames.insert(imageNames.end(), imageNamesJPEG.begin(), imageNamesJPEG.end());
    imageNames.insert(imageNames.end(), imageNamesPNG.begin(), imageNamesPNG.end());
    std::sort(imageNames.begin(), imageNames.end());

    std::cout << "\nThe following images will be processed:\n";
    for (size_t i = 0; i < imageNames.size(); ++i) {
        std::cout << i + 1 << ") " << imageNames[i] << std::endl;
    }

    int allTruePositives = 0;
    int allFalsePositives = 0;
    int allFalseNegatives = 0;
    std::array<int, 3> tmp;

    for (size_t i = 0; i < imageNames.size(); ++i) {
        std::cout << std::endl << i + 1 << ".";
        tmp = process_single_image(imageNames[i], skip, mode, threshold);
        if (!skip) {
            allTruePositives += tmp[0];
            allFalsePositives += tmp[1];
            allFalseNegatives += tmp[2];
        }
    }

    if (!skip) {
        double precision, recall, f1Score;
        precision = static_cast<double>(allTruePositives) / (allTruePositives + allFalsePositives);
        recall = static_cast<double>(allTruePositives) / (allTruePositives + allFalseNegatives);
        f1Score = 2 * precision * recall / (precision + recall);

        std::cout << "\n\nAll True Positives: " << allTruePositives << ".\n";
        std::cout << "All False Positives: " << allFalsePositives << ".\n";
        std::cout << "All False Negatives: " << allFalseNegatives<< ".\n";

        std::cout << "\nOverall Precision: " << precision << ".\n";
        std::cout << "Overall Recall: " << recall << ".\n";
        std::cout << "Overall F1-score: " << f1Score << ".\n";
    }
}