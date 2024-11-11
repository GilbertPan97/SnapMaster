#include "KMeansCluster.h"
#include <vector>
#include <iostream>
#include <random>

int main() {
    // Generate some sample 2D data points
    std::vector<double> x;
    std::vector<double> z;

    // Random number generator for generating clusters of points
    std::random_device rd;
    std::mt19937 gen(rd());

    // Three normal distributions to simulate different clusters
    std::normal_distribution<> dist1(50.0, 10.0);  // Cluster 1 around (50, 50)
    std::normal_distribution<> dist2(-50.0, 10.0); // Cluster 2 around (-50, -50)
    std::normal_distribution<> dist3(0.0, 10.0);   // Cluster 3 around (0, 0)

    // Add points for each cluster to the x and z vectors
    for (int i = 0; i < 50; ++i) {
        x.push_back(dist1(gen));
        z.push_back(dist1(gen));
    }
    for (int i = 0; i < 50; ++i) {
        x.push_back(dist2(gen));
        z.push_back(dist2(gen));
    }
    for (int i = 0; i < 50; ++i) {
        x.push_back(dist3(gen));
        z.push_back(dist3(gen));
    }

    // Create a KMeansCluster object and set the data
    KMeansCluster kmeans;
    kmeans.setData(x, z);

    // Run the clustering algorithm
    std::vector<int> labels = kmeans.runClustering();

    // Retrieve the clustered data as vectors of vectors
    std::vector<std::vector<double>> clusters = kmeans.getClusteredData();

    // Print each cluster's points
    for (size_t i = 0; i < clusters.size(); ++i) {
        std::cout << "Cluster " << i << " points:\n";
        for (size_t j = 0; j < clusters[i].size(); j += 2) {
            std::cout << "(" << clusters[i][j] << ", " << clusters[i][j + 1] << ") ";
        }
        std::cout << std::endl;
    }

    return 0;
}
