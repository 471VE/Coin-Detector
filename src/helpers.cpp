#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <algorithm>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "helpers.h"

const double pi = 3.14159265358979323846;

void show_image(std::string window_name, const cv::Mat& img) {
    auto img_to_show = img.clone();
    if (img.rows > 850) {
        cv::resize(img, img_to_show, cv::Size(), 850.0/img.rows, 850.0/img.rows);
    } 
    cv::imshow(window_name, img_to_show);
    cv::waitKey(0);
}


std::vector<cv::Vec3i> read_circles_from_file(const std::string& filename) {
    std::vector<cv::Vec3i> circlesVector;
    std::string oneCircle, circleParameter;
    cv::Vec3i circleParameters;
    std::ifstream circlesFile(filename);

    if (!circlesFile) {
        std::cerr << "\nUnable to open file \"" + filename + "\". Is it present in the same directory?\n";
        exit(1);
    }

    while (std::getline(circlesFile, oneCircle)) {
        std::istringstream input;
        input.str(oneCircle);
        for (int i = 0; i < 3; ++i) {
            std::getline(input, circleParameter, ' ');
            circleParameters[i] = std::stoi(circleParameter);
        }
        circlesVector.push_back(circleParameters);
    }
    circlesFile.close();
    return circlesVector;
}


void draw_circles(cv::Mat& img, std::vector<cv::Vec3f>& circlesVector) {
    for (size_t i = 0; i < circlesVector.size(); ++i) {
        cv::circle(img, {static_cast<int>(circlesVector[i][0]), static_cast<int>(circlesVector[i][1])},
                         static_cast<int>(circlesVector[i][2]), CV_RGB(255, 0, 0), 2);
        cv::circle(img, {static_cast<int>(circlesVector[i][0]), static_cast<int>(circlesVector[i][1])},
                         3, CV_RGB(0, 255, 0), -3);
    }
}


void print_circles_info(std::string filename, std::string logName, std::vector<cv::Vec3f>& circlesVector) {
    std::ostringstream circlesInfoStream;
    circlesInfoStream << "\nImage file: \"" + filename + "\".\n\n";
    circlesInfoStream << "Detected coins information:\n";
    for (size_t i = 0; i < circlesVector.size(); ++i) {
        circlesInfoStream << i + 1 << ") x = " << static_cast<int>(circlesVector[i][0]) << ", y = " << static_cast<int>(circlesVector[i][1])
            << ", radius = " << static_cast<int>(circlesVector[i][2]) << ".\n";
    }
    circlesInfoStream << "\nTotal number of detected coins: " << circlesVector.size() << ".\n";
    std::string circlesInfo = circlesInfoStream.str();
    std::cout << circlesInfo;

    std::ofstream logFile;
    logFile.open(logName);
    logFile << circlesInfo;
    logFile.close();
}


bool are_the_same_circle(const cv::Vec3f& circleTrue, const cv::Vec3f& circleDetected,
                         int spreadX, int spreadY, metric mode, double threshold) {
    if (mode == IoU) {
        double IoU = InetersectionOverUnion(circleTrue, circleDetected);
        if (IoU > threshold) {
            return true;
        }
        return false;
    }
    else if (mode == INDEPENDENT) {
        double radiiRatio = circleDetected[2] / circleTrue[2];
        if ((circleTrue[0] - spreadX <= circleDetected[0]) && (circleDetected[0] <= circleTrue[0] + spreadX)
            && (circleTrue[1] - spreadY <= circleDetected[1]) && (circleDetected[1] <= circleTrue[1] + spreadY)
            && (0.85 <= radiiRatio) && (radiiRatio <= 1.15)) {
                return true;
            }
        return false;
    }
    else {
        std::cerr << "\nUnacceptable value for metric type.";
        exit(1);
    }
}


std::array<int, 3> calculate_statistics(const cv::Mat& img, const std::string& filename, std::string logName,
                                        const std::vector<cv::Vec3f>& circlesDetected, metric mode, double threshold) {
    auto circlesTrue = read_circles_from_file(filename);
    int spreadX = img.cols / 80;
    int spreadY = img.rows / 80;

    int truePositive = 0;
    int falsePositive = static_cast<int>(circlesDetected.size());
    int falseNegative = static_cast<int>(circlesTrue.size());
    bool skip = false;

    for (int i = 0; i < circlesDetected.size(); ++i) {
        for (int j = 0; j < circlesTrue.size(); ++j) {
            if (are_the_same_circle(circlesTrue[j], circlesDetected[i], spreadX, spreadY, mode, threshold)) {
                truePositive++;
                falseNegative--;
                falsePositive--;
                circlesTrue.erase(circlesTrue.begin() + j);
                skip = true;
                break;
            }
        }
        if (skip) {
            skip = false;
            continue;
        }
    }
    std::ostringstream f1Stream;

    f1Stream << "\nTrue Positives: " << truePositive << ".\n";
    f1Stream << "False Positives: " << falsePositive << ".\n";
    f1Stream << "False Negatives: " << falseNegative << ".\n";

    double precision = static_cast<double>(truePositive) / (truePositive + falsePositive);
    double recall = static_cast<double>(truePositive) / (truePositive + falseNegative);
    double f1Score = 2 * precision * recall / (precision + recall);

    f1Stream << "\nPrecision: " << precision << ".\n";
    f1Stream << "Recall: " << recall << ".\n";
    f1Stream << "\nF1-score: " << f1Score << ".\n";

    std::string f1Info = f1Stream.str();
    std::cout << f1Info;

    std::ofstream logFile;
    logFile.open(logName, std::ios_base::app);
    logFile << f1Info;
    logFile.close();

    std::array<int, 3> statistics = {truePositive, falsePositive, falseNegative};
    return statistics;
}


void LevelCorrection(cv::Mat& src, cv::Mat& dst, int low, int high, double fGamma){
	unsigned char lut[256];

    for (int i = 0; i < low; ++i)
        lut[i] = 0;

    for (int i = low; i < high; ++i)
        lut[i] = (i - low) * 255 / (high - low);

    for (int i = high; i < 256; ++i)
        lut[i] = 255;

	for (int i = 0; i < 256; ++i)
		lut[i] = cv::saturate_cast<uchar>(pow((float)(lut[i] / 255.0), fGamma) * 255.0f);

	dst = src.clone();
    cv::MatIterator_<uchar> it, end;
    for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; ++it)
        *it = lut[(*it)];
}


int circle_to_remove(const cv::Vec3f& circleA, const cv::Vec3f& circleB) {
    if (circleA[2] > 3 * circleB[2]) {
        return 1;
    } else if (circleB[2] > 3 * circleA[2]) {
        return 0;
    }

    int deltaX = static_cast<int>(circleA[0] - circleB[0]);
    int deltaY = static_cast<int>(circleA[1] - circleB[1]);
    int distanceSquared = deltaX * deltaX + deltaY * deltaY;

    int radiusSum = static_cast<int>(circleA[2] + circleB[2]);    
    if (distanceSquared >= radiusSum * radiusSum) {
        return -1;
    }

    int deltaR = static_cast<int>(circleA[2] - circleB[2]);
    if (distanceSquared <= deltaR * deltaR) {
        return static_cast<int>(deltaR > 0);
    }

    double distance = std::sqrt(distanceSquared);
    double ARadiusSquared = circleA[2] * circleA[2];
    double BRadiusSquared = circleB[2] * circleB[2];

    double phi = 2 * std::acos(std::clamp((ARadiusSquared + distanceSquared - BRadiusSquared) / (2 * circleA[2] * distance), -1., 1.));
    double theta = 2 * std::acos(std::clamp((BRadiusSquared + distanceSquared - ARadiusSquared) / (2 * circleB[2] * distance), -1., 1.));
    double overlapArea = 0.5 * (BRadiusSquared * (theta - std::sin(theta)) + ARadiusSquared * (phi - std::sin(phi)));

    double smallestArea;
    int chosenCircle;
    if (ARadiusSquared <= BRadiusSquared) {
        smallestArea = pi * ARadiusSquared;
        chosenCircle = 0;
    } else {
        smallestArea = pi * BRadiusSquared;
        chosenCircle = 1;
    }

    if (overlapArea > 0.7 * smallestArea) {
        return chosenCircle;
    } else {
        return -1;
    }
}

double InetersectionOverUnion(const cv::Vec3f& circleA, const cv::Vec3f& circleB) {
    int deltaX = static_cast<int>(circleA[0] - circleB[0]);
    int deltaY = static_cast<int>(circleA[1] - circleB[1]);
    int distanceSquared = deltaX * deltaX + deltaY * deltaY;
    int radiusSum = static_cast<int>(circleA[2] + circleB[2]);
    int deltaR = static_cast<int>(circleA[2] - circleB[2]);

    double distance = std::sqrt(distanceSquared);
    double minRadius = std::min(circleA[2], circleB[2]);
    double maxRadius = std::max(circleA[2], circleB[2]);
    double minRadiusSquared = minRadius * minRadius;
    double maxRadiusSquared = maxRadius * maxRadius;
 
    if (distanceSquared >= radiusSum * radiusSum) {
        return 0;
    }

    if (distanceSquared <= deltaR * deltaR) {
        return minRadiusSquared / maxRadiusSquared;
    }

    double phi = 2 * std::acos(std::clamp((minRadiusSquared + distanceSquared - maxRadiusSquared) / (2 * minRadius * distance), -1., 1.));
    double theta = 2 * std::acos(std::clamp((maxRadiusSquared + distanceSquared - minRadiusSquared) / (2 * maxRadius * distance), -1., 1.));
    double overlapArea = 0.5 * (maxRadiusSquared * (theta - std::sin(theta)) + minRadiusSquared * (phi - std::sin(phi)));
    return overlapArea / (pi * (minRadiusSquared + maxRadiusSquared) - overlapArea);
}


void filter_circles(std::vector<cv::Vec3f>& circlesVector) {
    std::vector<size_t> toDelete;
    for (size_t i = 1; i < circlesVector.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            int circleToRemove = circle_to_remove(circlesVector[i], circlesVector[j]);
            switch (circleToRemove) {
                case -1: break;
                case 0: toDelete.push_back(i); break;
                case 1: toDelete.push_back(j); break;
                default: std::cerr << "Something went wrong. Exiting...\n"; exit(1);
            }
        }
    }
    
    std::sort(toDelete.begin(), toDelete.end(), std::greater<size_t>());
    auto last = std::unique(toDelete.begin(), toDelete.end());
    toDelete.erase(last, toDelete.end());

    for (size_t i = 0; i < toDelete.size(); ++i) {
        circlesVector.erase(circlesVector.begin() + toDelete[i]);
    }
}

std::array<double, 3> mean_std(const cv::Mat& img, const cv::Vec3f& circle) {
    long long int N = 0;
    long long int NOutside = 0;

    long long int sum = 0;
    long long int sumOutside = 0;

    long long int sumOfSquares = 0;

    int xCenter = static_cast<int>(circle[0]);
    int yCenter = static_cast<int>(circle[1]);
    int radius = static_cast<int>(circle[2]);
    int radiusSquared = radius * radius;
    long long int pixelValue;

    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            if (yCenter + y >= img.rows || yCenter + y < 0 || xCenter + x >= img.cols || xCenter + x < 0) {
                continue;
            } else if (x * x + y * y <= radiusSquared) {
                N++;
                pixelValue = static_cast<long long int>(img.at<uchar>(yCenter + y, xCenter + x));
                sum += pixelValue;
                sumOfSquares += (pixelValue * pixelValue);
            } else {
                NOutside++;
                sumOutside += static_cast<long long int>(img.at<uchar>(yCenter + y, xCenter + x));
            }
        }
    }
    double standardDevSquared = static_cast<double>(N * sumOfSquares - sum * sum) / static_cast<double>(N * N * (N - 1));
    double mean = static_cast<double>(sum) / static_cast<double>(N);
    double meanOutside = static_cast<double>(sumOutside) / static_cast<double>(NOutside);
    std::array<double, 3> result = {standardDevSquared, mean, meanOutside};
    return result;
}


void new_circle_position(cv::Vec3f& circle, const cv::Vec3f& circleNew, const int& xMin, const int& yMin) {
    circle[0] = xMin + circleNew[0];
    circle[1] = yMin + circleNew[1];
    circle[2] = circleNew[2];
}


void correct_circle_positions(const cv::Mat& img, std::vector<cv::Vec3f>& circles) {
    std::vector<cv::Vec3f> circlesCropped;
    for (size_t i = 0; i < circles.size(); ++i) {
        auto [stdOriginal, meanOriginal, meanOutsideOriginal] = mean_std(img, circles[i]);

        int yMin = std::max(0, static_cast<int>(circles[i][1] - 1.5 * circles[i][2]));
        int yMax = std::min(img.rows, static_cast<int>(circles[i][1] + 1.5 * circles[i][2]));
        int xMin = std::max(0, static_cast<int>(circles[i][0] - 1.5 * circles[i][2]));
        int xMax = std::min(img.cols, static_cast<int>(circles[i][0] + 1.5 * circles[i][2]));

        cv::Mat imgCropped = img(cv::Range(yMin, yMax), cv::Range(xMin, xMax)).clone();                                              
        cv::HoughCircles(imgCropped, circlesCropped, cv::HOUGH_GRADIENT, 1.5,
                         2*circles[i][2], 100, 120,
                         static_cast<int>(0.4*circles[i][2]), static_cast<int>(1.1*circles[i][2]));

        if (circlesCropped.size() != 0) {
            auto [stdNew, meanNew, meanOutsideNew] = mean_std(imgCropped, circlesCropped[0]);
            if (stdNew < stdOriginal) {
                if (meanNew > meanOutsideNew && meanNew > meanOriginal) {
                    new_circle_position(circles[i], circlesCropped[0], xMin, yMin);
                } else if (meanNew < meanOutsideNew && meanNew < meanOriginal) {
                    new_circle_position(circles[i], circlesCropped[0], xMin, yMin);
                }
            }
        }
    }
}


void check_directory(std::string directoryName) {
    if (!std::filesystem::is_directory(directoryName) || !std::filesystem::exists(directoryName)) {
        std::filesystem::create_directory(directoryName); 
    }
}