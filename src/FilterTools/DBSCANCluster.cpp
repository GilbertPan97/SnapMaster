#include "DBSCANCluster.h"
#include <cmath>
#include <queue>

DBSCANCluster::DBSCANCluster(double eps, int minPoints)
    : eps_(eps), minPoints_(minPoints) {}

void DBSCANCluster::setData(const std::vector<double>& x, const std::vector<double>& z) {
    data_.clear();
    labels_.clear();
    for (size_t i = 0; i < x.size(); ++i) {
        data_.push_back({x[i], z[i]});
        labels_.push_back(-1);  // Initialize labels as -1 (unclassified)
    }
    // std::cout << "Info: successful set data: " << data_.size() << std::endl;
}

double DBSCANCluster::calculateDistance(const std::vector<double>& point1, const std::vector<double>& point2) const {
    double sum = 0.0;
    for (size_t i = 0; i < point1.size(); ++i) {
        sum += std::pow(point1[i] - point2[i], 2);
    }
    return std::sqrt(sum);
}

std::vector<int> DBSCANCluster::regionQuery(int pointIndex) const {
    std::vector<int> neighbors;
    for (size_t i = 0; i < data_.size(); ++i) {
        if (calculateDistance(data_[pointIndex], data_[i]) <= eps_) {
            neighbors.push_back(i);
        }
    }
    return neighbors;
}

bool DBSCANCluster::expandCluster(int pointIndex, int clusterId) {
    std::vector<int> seeds = regionQuery(pointIndex);
    if (seeds.size() < minPoints_) {
        labels_[pointIndex] = -1;  // Mark as noise if not enough neighbors
        return false;
    }

    // Assign the seeds to the current cluster
    for (int seed : seeds) {
        labels_[seed] = clusterId;
    }

    // Expand the cluster from each seed point
    for (size_t i = 0; i < seeds.size(); ++i) {
        int currentPoint = seeds[i];
        std::vector<int> result = regionQuery(currentPoint);
        if (result.size() >= minPoints_) {
            for (int neighbor : result) {
                if (labels_[neighbor] == -1) {  // Add unclassified points to the cluster
                    labels_[neighbor] = clusterId;
                    seeds.push_back(neighbor);
                }
            }
        }
    }
    return true;
}

std::vector<int> DBSCANCluster::runClustering() {
    int clusterId = 0;
    for (size_t i = 0; i < data_.size(); ++i) {
        if (labels_[i] != -1) continue;  // Skip already classified points

        // Try to expand a new cluster
        if (expandCluster(i, clusterId)) {
            clusterId++;
        }
    }
    return labels_;
}

std::vector<std::vector<double>> DBSCANCluster::getClusteredData() const {
    std::vector<std::vector<double>> clusteredData(2);
    for (size_t i = 0; i < data_.size(); ++i) {
        if (labels_[i] != -1) {  // Return only classified points
            clusteredData[0].push_back(data_[i][0]);
            clusteredData[1].push_back(data_[i][1]);
        }
    }
    return clusteredData;
}

std::vector<int> DBSCANCluster::getClusteredLabels() const {
    std::vector<int> filteredLabels;
    filteredLabels.reserve(labels_.size());  // 预留空间，优化性能
    
    for (int label : labels_) {
        if (label != -1) {
            filteredLabels.push_back(label);
        }
    }
    
    return filteredLabels;
}

double DBSCANCluster::calculateNoiseRatio() const {
    int noiseCount = 0;
    for (int label : labels_) {
        if (label == -1) {
            noiseCount++;
        }
    }
    return static_cast<double>(noiseCount) / labels_.size();
}

int DBSCANCluster::getOptimalClusterCount(double noiseRatioThreshold) {
    int bestClusterCount = 0;
    double initialEps = eps_;

    // Loop to find optimal clusters by adjusting eps value
    for (double currentEps = initialEps; currentEps < initialEps * 3; currentEps += 0.1) {
        eps_ = currentEps;
        runClustering();
        double noiseRatio = calculateNoiseRatio();

        if (noiseRatio < noiseRatioThreshold) {
            bestClusterCount = *max_element(labels_.begin(), labels_.end()) + 1;
            break;
        }
    }
    // Restore original eps value
    eps_ = initialEps;
    return bestClusterCount;
}
