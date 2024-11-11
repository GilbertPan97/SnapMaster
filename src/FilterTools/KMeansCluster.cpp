#include "KMeansCluster.h"
#include <cmath>
#include <iostream>

KMeansCluster::KMeansCluster(int maxClusters)
    : maxClusters_(maxClusters), optimalClusterCount_(0) {}

void KMeansCluster::setData(const std::vector<double>& x, const std::vector<double>& z) {
    data_ = convertToMat(x, z);
}

std::vector<int> KMeansCluster::runClustering(double eps, int count) {
    if (data_.empty()) {
        std::cerr << "Data is empty. Set data points before clustering." << std::endl;
        return {};
    }

    // Determine optimal cluster count and perform K-means clustering
    optimalClusterCount_ = determineOptimalClusterCount(eps, count);
    return performKMeansClustering(optimalClusterCount_, eps, count);
}

int KMeansCluster::getOptimalClusterCount() const {
    return optimalClusterCount_;
}

cv::Mat KMeansCluster::convertToMat(const std::vector<double>& x, const std::vector<double>& z) const {
    cv::Mat matData(x.size(), 2, CV_32F);
    for (size_t i = 0; i < x.size(); ++i) {
        matData.at<float>(i, 0) = static_cast<float>(x[i]);
        matData.at<float>(i, 1) = static_cast<float>(z[i]);
    }
    return matData;
}

float KMeansCluster::calculateWCSS(int clusterCount, cv::Mat& labels, double eps, int count) const {
    cv::Mat centers;
    double compactness = cv::kmeans(data_, clusterCount, labels,
                                    cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, count, eps),
                                    10, cv::KMEANS_PP_CENTERS, centers);
    return static_cast<float>(compactness);
}

int KMeansCluster::determineOptimalClusterCount(double eps, int count) const {
    std::vector<float> wcssValues;

    for (int k = 1; k <= maxClusters_; ++k) {
        cv::Mat tempLabels;
        float wcss = calculateWCSS(k, tempLabels, eps, count);
        wcssValues.push_back(wcss);
    }

    // Find "elbow" in WCSS values
    float maxSlopeDiff = 0;
    int bestClusterCount = 1;
    for (int i = 1; i < maxClusters_ - 1; ++i) {
        float slope1 = wcssValues[i - 1] - wcssValues[i];
        float slope2 = wcssValues[i] - wcssValues[i + 1];
        float slopeDiff = std::abs(slope1 - slope2);

        if (slopeDiff > maxSlopeDiff) {
            maxSlopeDiff = slopeDiff;
            bestClusterCount = i + 1;
        }
    }
    return bestClusterCount;
}

std::vector<int> KMeansCluster::performKMeansClustering(int clusterCount, double eps, int count) {
    cv::kmeans(data_, clusterCount, labels_,
               cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, count, eps),
               10, cv::KMEANS_PP_CENTERS);

    // Convert labels to a vector<int> for easier handling
    std::vector<int> clusterLabels(labels_.rows);
    for (int i = 0; i < labels_.rows; ++i) {
        clusterLabels[i] = labels_.at<int>(i);
    }
    return clusterLabels;
}

std::vector<std::vector<double>> KMeansCluster::getClusteredData() const {
    std::vector<std::vector<double>> clusteredData(2);

    for (int i = 0; i < labels_.rows; ++i) {
        clusteredData[0].push_back(data_.at<float>(i, 0));     // x coordinate
        clusteredData[1].push_back(data_.at<float>(i, 1));     // z coordinate
    }
    return clusteredData;
}

std::vector<int> KMeansCluster::getClusteredLabels() const {
    // Convert labels to a vector<int> for easier handling
    std::vector<int> clusterLabels(labels_.rows);
    for (int i = 0; i < labels_.rows; ++i) {
        clusterLabels[i] = labels_.at<int>(i);
    }
    return clusterLabels;
}
