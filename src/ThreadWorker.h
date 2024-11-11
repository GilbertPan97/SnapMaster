#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QObject>
#include <QThread>
#include "qcustomplot.h"
#include "LineScannerInterface.h"  // Include the SensorApi header file

class ThreadWorker : public QObject {
    Q_OBJECT

public:
    explicit ThreadWorker(LineScannerInterface *sensorApi, QObject *parent = nullptr);

    explicit ThreadWorker();

public slots:
    void ContinueGrabPlot();
    void startGrabbing();
    void stopGrabbing();

signals:
    void updatePlot(); 

private:
    LineScannerInterface *sensorApi;
    // Gocator_Data *plotData;
    bool isGrabing = false;

    // void qcpPlot(QCustomPlot *customPlot, Gocator_Data data);
};

#endif // THREADWORKER_H
