#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <string>

#include <opencv2/core.hpp>

enum metric {IoU, INDEPENDENT};

void show_image(std::string window_name, const cv::Mat& img);

std::vector<cv::Vec3i> read_circles_from_file(const std::string& filename);
void draw_circles(cv::Mat& img, std::vector<cv::Vec3f>& circlesVector);
void print_circles_info(std::string filename, std::string logName, std::vector<cv::Vec3f>& circlesVector);

bool are_the_same_circle(const cv::Vec3f& circleTrue, const cv::Vec3f& circleDetected,
                         int spreadX, int spreadY, metric mode, double threshold);

std::array<int, 3> calculate_statistics(const cv::Mat& img, const std::string& filename, std::string logName,
                                        const std::vector<cv::Vec3f>& circlesDetected, metric mode, double threshold);

void LevelCorrection(cv::Mat& src, cv::Mat& dst, int low, int high, double fGamma);

int circle_to_remove(const cv::Vec3f& circleA, const cv::Vec3f& circleB);
void filter_circles(std::vector<cv::Vec3f>& circlesVector);
double InetersectionOverUnion(const cv::Vec3f& circleA, const cv::Vec3f& circleB);
std::array<double, 3> mean_std(const cv::Mat& img, const cv::Vec3f& circle);

void new_circle_position(cv::Vec3f& circle, const cv::Vec3f& circleNew, const int& xMin, const int& yMin);
void correct_circle_positions(const cv::Mat& img, std::vector<cv::Vec3f>& circles);

void check_directory(std::string directoryName);

#endif // HELPERS_H