#ifndef DBSCANCLUSTER_H
#define DBSCANCLUSTER_H

#include <vector>
#include <cmath>
#include <iostream>

class DBSCANCluster {
public:
    explicit DBSCANCluster(double eps, int minPoints);

    // Set the data points
    void setData(const std::vector<double>& x, const std::vector<double>& z);

    // Run the DBSCAN clustering algorithm and return cluster labels
    std::vector<int> runClustering();

    // Get the clustered data points
    std::vector<std::vector<double>> getClusteredData() const;

    // Get the clustered labels
    std::vector<int> getClusteredLabels() const;

    // Determine the optimal cluster count by adjusting eps and minPoints
    int getOptimalClusterCount(double noiseRatioThreshold);

private:
    double eps_;                             // Radius for neighborhood search
    int minPoints_;                          // Minimum points to form a cluster
    std::vector<std::vector<double>> data_;  // Data points (x, z)
    std::vector<int> labels_;                // Cluster labels for each point (-1 indicates noise)

    // Calculate Euclidean distance between two points
    double calculateDistance(const std::vector<double>& point1, const std::vector<double>& point2) const;

    // Find all neighbors of a point within eps radius
    std::vector<int> regionQuery(int pointIndex) const;

    // Expand cluster from a given point
    bool expandCluster(int pointIndex, int clusterId);

    // Calculate the noise ratio (percentage of points labeled as noise)
    double calculateNoiseRatio() const;
};

#endif // DBSCANCLUSTER_H
