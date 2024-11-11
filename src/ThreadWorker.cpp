#include "threadworker.h"

ThreadWorker::ThreadWorker(LineScannerInterface *sensorApi, QObject *parent)
    : QObject(parent), sensorApi(sensorApi) {

}

ThreadWorker::ThreadWorker(){

}

void ThreadWorker::ContinueGrabPlot() {
    while (isGrabing) {
        if (!isGrabing)
            break;
        sensorApi->GrabOnce();  // Perform the grabbing operation
        // qcpPlot(customPlot, sensorApi->RetriveData());

        emit updatePlot();                    // Emit signal when data is ready

        QThread::msleep(100);   // Simulate delay without blocking the UI thread
    }
}

void ThreadWorker::startGrabbing() {
    isGrabing = true;
    ContinueGrabPlot();     // Start grabbing frames
}

void ThreadWorker::stopGrabbing() {
    isGrabing = false;      // Stop the grabbing process
}

// void ThreadWorker::qcpPlot(QCustomPlot *customPlot, Gocator_Data data) {
//     // Check if the profileBuffer is valid and pointCount is greater than 0
//     if (data.profileBuffer == nullptr || data.pointCount == 0) {
//         qWarning("Gocator_Data has no valid profile points.");
//         customPlot->graph(0)->data()->clear();      // Clear data for graph 0
//         customPlot->replot();
//         return;
//     }

//     QVector<double> x(data.pointCount); // Create vector for x coordinates
//     QVector<double> z(data.pointCount); // Create vector for z coordinates

//     // Extract the profile points into x and z vectors
//     for (size_t i = 0; i < data.pointCount; ++i) {
//         x[i] = data.profileBuffer[i].x; // Extract x coordinate
//         z[i] = data.profileBuffer[i].z; // Extract z coordinate
//         // qDebug() << "Point" << i << ": x =" << x[i] << ", z =" << z[i]; // Print values for debugging
//     }

//     // Create test data for x and z within the range -700 to 700
//     QVector<double> testX, testZ;
//     for (int i = -14; i <= 14; ++i) {  // Generates 29 points across the range
//         testX.append(i * 50);       // X-coordinates from -700 to 700
//         testZ.append(-i * i + 700); // Z-coordinates from 700 down to 0 in a parabolic shape
//     }

//     // Assign the input data to the graph
//     customPlot->graph(0)->setData(x, z);

//     // Rescale axes to fit the data
//     customPlot->xAxis->setRange(-1000, 1000); // Set x-axis range (adjust as needed)
//     customPlot->yAxis->setRange(-1000, 1000); // Set y-axis range (adjust as needed)
//     customPlot->replot();
// }
