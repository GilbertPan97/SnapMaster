#ifndef KMEANSCLUSTER_H
#define KMEANSCLUSTER_H

#include <opencv2/opencv.hpp>
#include <vector>

class KMeansCluster {
public:
    // Constructor with optional max cluster count
    explicit KMeansCluster(int maxClusters = 15);

    // Set data points (x and z coordinates)
    void setData(const std::vector<double>& x, const std::vector<double>& z);

    // Run clustering with optional eps and count parameters and return cluster labels
    std::vector<int> runClustering(double eps = 0.5, int count = 100);

    // Get the optimal cluster count
    int getOptimalClusterCount() const;

    // Get clustered data points as std::vector<double> groups
    std::vector<std::vector<double>> getClusteredData() const;

    std::vector<int> getClusteredLabels() const;

private:
    int maxClusters_;                       // Maximum number of clusters to consider
    int optimalClusterCount_;               // Best number of clusters determined
    cv::Mat data_;                          // Data points
    cv::Mat labels_;                        // Labels for each data point

    // Convert std::vector data to OpenCV Mat format
    cv::Mat convertToMat(const std::vector<double>& x, const std::vector<double>& z) const;

    // Calculate WCSS for a given cluster count with eps and count parameters
    float calculateWCSS(int clusterCount, cv::Mat& labels, double eps, int count) const;

    // Determine the optimal cluster count using the Elbow method with eps and count parameters
    int determineOptimalClusterCount(double eps, int count) const;

    // Perform K-means clustering with specified cluster count, eps, and count parameters
    std::vector<int> performKMeansClustering(int clusterCount, double eps, int count);
};

#endif // KMEANSCLUSTER_H
