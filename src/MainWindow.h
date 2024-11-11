#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "qcustomplot.h"
#include "qtpropertymanager.h"
#include "qttreepropertybrowser.h"
#include "qteditorfactory.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include "LineScannerInterface.h"
#include "DBSCANCluster.h"
#include "ThreadWorker.h"

#include <QMainWindow>
#include <QThread>

QT_BEGIN_NAMESPACE
class QAction;
class QToolBar;
class QMenuBar;
class QDockWidget;
QT_END_NAMESPACE

// Define a struct to hold adapter information
struct AdapterInfo {
    std::string name;           // Adapter name
    std::string description;    // Adapter description
    std::string ipAddress;      // IP address
    std::string subnetMask;     // Subnet mask
    std::string defaultGateway;  // Default gateway

    // Constructor to initialize adapter information
    AdapterInfo(const std::string &name, const std::string &description,
                const std::string &ipAddress, const std::string &subnetMask,
                const std::string &defaultGateway)
        : name(name), description(description), ipAddress(ipAddress),
          subnetMask(subnetMask), defaultGateway(defaultGateway) {}
};

// Define the enum for filter types
enum class FilterType {
    None = 0,
    PickFilter,
    ROIFilter,
    CircleFilter
};

struct SensorQData {
    QVector<double> x;
    QVector<double> z;
    size_t pointCount;

    // Method to clear all data
    void clear() {
        x.clear();
        z.clear();
        pointCount = 0;
    }

    // Method to append other data
    void append(SensorQData appendDatas) {
        x += appendDatas.x;
        z += appendDatas.z;
        pointCount += appendDatas.pointCount;
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    static void logCallback(const char* message);

private:
    CameraInfo curCamInfo_;      // Store current selected sensor information
    SensorQData curData_;        // Current recorded sensor data 
    FilterType currentFilter_;   // Store the selected filter type
    QTextEdit* logTextEdit_; // QTextEdit to display log messages
    LineScannerInterface sensorApi_; // Make sensorApi a member variable
    bool isPlaying_;

    ThreadWorker *grabWorker_;
    QThread *grabThread_;

    QMenuBar *mainMenuBar_;
    QToolBar *toolbar_;
    QCustomPlot *customPlot_;
    QtTreePropertyBrowser *propertyBrowser_;

    QMap<QCPGraph*, bool> graphStates_;

    ads::CDockManager* dockManager_;
    
    ads::CDockWidget* setupPlot();
    ads::CDockWidget* setupLogViewer();
    ads::CDockWidget* setupPropertyBrowser(); // New method to set up property browser
    void setupToolbar();
    void setupMenubar();

    int getScaledSize(int baseSize); // Function to get scaled size based on screen DPI
    void extractProfilePoints(const Gocator_Data& gocatorData);

    std::vector<AdapterInfo> getNetworkAdapters();
    void executeNetshCommand(const QString &command);

    void QcpPlot(QCustomPlot *customPlot, Gocator_Data data);

    // Update plot from sensor
    void updatePlot();
    void refreshPlot();

    QJsonObject SensorQDataToJson(const SensorQData& data);
    SensorQData JsonToSensorQData(const QJsonObject& jsonObj);

    void resetAllGraphsStatus();

    bool CheckCurData();

    QVector<QColor> generateColors(int numColors);
    QVector<SensorQData> cachePlotData(QCustomPlot* customPlot_);

    QMenu* createCameraMenu();

signals:
    void filterEnableSig(bool conditionMet);
    void logMessageReceived(const QString &message);

private slots:
    void onNewSecTriggered();
    void onIpConnTriggered();

    void onLogMessageReceived(const QString &message);

    void onFilterButtonClicked();
    void onDownloadButtonClicked();
    void onUploadButtonClicked();

    void showScanCameraDialog();    // Slot to show the camera scan dialog
    void selectionChanged();
    void contextMenuRequest(QPoint pos);
    void removeSelectedGraph();
    void mousePress();
    void mouseWheel();
};

#endif // MAINWINDOW_H
