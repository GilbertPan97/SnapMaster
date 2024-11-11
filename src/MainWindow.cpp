#include "MainWindow.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include "qcustomplot.h"

#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"
#include "qtbuttonpropertybrowser.h"
#include "qtgroupboxpropertybrowser.h"

#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTimer>
#include <QSettings>

#include <Windows.h>
#include <iphlpapi.h>

static MainWindow* instance = nullptr;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), isPlaying_(false) {
    // Load style file
    QFile file(":/styles/style.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = file.readAll();
        this->setStyleSheet(styleSheet);
    } else {
        qDebug() << "Trying to open style file:" << file.fileName();
    }

    // Set the application window icon and central widget
    setWindowIcon(QIcon(":/icons/app-logo.ico"));
    QWidget *centralWidget = new QWidget(this);

    // Setup the menuBar
    mainMenuBar_ = this->menuBar();
    setMenuBar(mainMenuBar_);            // Set the menu bar to the main window
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);  // Set to horizontal line
    line->setFrameShadow(QFrame::Sunken); // Set the shadow style

    // Create a layout for the central widget
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addWidget(mainMenuBar_); // Add menu bar to the layout
    centralLayout->addWidget(line);         // Add the line below the menu bar

    // Setup the toolbar
    setupToolbar();
    addToolBar(Qt::LeftToolBarArea, toolbar_);

    // Setup the menubar
    setupMenubar();

    // Initialize QCustomPlot
    customPlot_ = new QCustomPlot(this);
    ads::CDockWidget* plotDockWidget = setupPlot();

    // Initialize the property browser
    ads::CDockWidget* PropDockWidget = setupPropertyBrowser();

    // Initialize the log viewer
    ads::CDockWidget* logDockWidget = setupLogViewer();

    // Set DockWidget area and initial size
    dockManager_ = new ads::CDockManager(this);
    auto TopDockArea = dockManager_->addDockWidget(ads::TopDockWidgetArea, plotDockWidget);
    auto RightDockArea = dockManager_->addDockWidget(ads::RightDockWidgetArea, PropDockWidget);
    auto bottomDockArea = dockManager_->addDockWidget(ads::BottomDockWidgetArea, logDockWidget);

    dockManager_->setSplitterSizes(RightDockArea, {1280/10 * 7, 1280/10 * 3});
    dockManager_->setSplitterSizes(bottomDockArea, {1280/10 * 8, 1280/10 * 2});
    centralLayout->addWidget(dockManager_);

    setCentralWidget(centralWidget);  // Set the central widget with the layout

    // Create mainwindow pointer for log call back in C function
    instance = this;
    setLogCallback(MainWindow::logCallback);

    // Create a new thread for camera grabbing
    grabThread_ = new QThread(this); 
    grabWorker_ = new ThreadWorker(&sensorApi_);
    grabWorker_->moveToThread(grabThread_);
    connect(grabThread_, &QThread::started, grabWorker_, &ThreadWorker::startGrabbing);
    // connect(grabThread, &QThread::finished, grabWorker, &ThreadWorker::stopGrabbing);
    connect(grabWorker_, &ThreadWorker::updatePlot, this, &MainWindow::updatePlot);
}

void MainWindow::setupToolbar() {
    toolbar_ = addToolBar("Main Toolbar");  // Initialize the private toolbar
    toolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly); // Show only icons

    // Initialize actions
    QAction *new_section = new QAction(QIcon(":/icons/new-project.png"), "New", this);
    new_section->setToolTip("New Section");
    new_section->setText("New Sec");

    QAction *open_section = new QAction(QIcon(":/icons/open.png"), "Open", this);
    open_section->setToolTip("Open Section");
    open_section->setText("Open Sec");

    QAction *ip_conn = new QAction(QIcon(":/icons/connect.png"), "Connect", this);
    ip_conn->setText("System IP");

    QAction *sel_camBrand = new QAction(QIcon(":/icons/camera.png"), "Branch", this);
    sel_camBrand->setToolTip("Select Camera Branch");
    sel_camBrand->setText("Camera Branch");

    QAction *scan_camera = new QAction(QIcon(":/icons/global-search.png"), "Device", this);
    scan_camera->setToolTip("Scan Camera");
    scan_camera->setText("Camera Scan");

    // Create the camera menu using the encapsulated function
    QMenu *cameraMenu = createCameraMenu();

    // Connect signals and slots
    connect(sel_camBrand, &QAction::triggered, [this, cameraMenu]() {
        cameraMenu->exec(QCursor::pos()); // Show the menu at the cursor position
    });
    
    connect(new_section, &QAction::triggered, this, &MainWindow::onNewSecTriggered);
    connect(ip_conn, &QAction::triggered, this, &MainWindow::onIpConnTriggered);
    connect(scan_camera, &QAction::triggered, this, &MainWindow::showScanCameraDialog);

    // Add actions to the toolbar
    toolbar_->addAction(new_section);
    toolbar_->addAction(open_section);
    toolbar_->addSeparator();
    toolbar_->addAction(ip_conn);
    toolbar_->addAction(sel_camBrand);
    toolbar_->addAction(scan_camera);
    toolbar_->addSeparator();

}

void MainWindow::setupMenubar() {
    // Create menus and add actions
    QMenu *fileMenu = mainMenuBar_->addMenu("File");
    QMenu *deviceMenu = mainMenuBar_->addMenu("Device");
    QMenu *toolMenu = mainMenuBar_->addMenu("Tool");
    QMenu *viewerMenu = mainMenuBar_->addMenu("Viewer");
    QMenu *winMenu = mainMenuBar_->addMenu("Window");
    QMenu *settingMenu = mainMenuBar_->addMenu("Setting");
    QMenu *helperMenu = mainMenuBar_->addMenu("Help");

    // Add three options to the Tool menu
    QAction *fil_none = new QAction("None", this);
    QAction *fil_pick = new QAction("Pick Filter", this);
    QAction *fil_roi = new QAction("ROI Filter", this);
    QAction *fil_circle = new QAction("Circle Filter", this);

    // Make actions checkable
    fil_none->setCheckable(true);
    fil_pick->setCheckable(true);
    fil_roi->setCheckable(true);
    fil_circle->setCheckable(true);

    // Create an action group to enforce mutual exclusivity
    QActionGroup *filterGroup = new QActionGroup(this);
    filterGroup->addAction(fil_none);
    filterGroup->addAction(fil_pick);
    filterGroup->addAction(fil_roi);
    filterGroup->addAction(fil_circle);

    // Allow only one action to be checked at a time
    filterGroup->setExclusive(true);

    // Set "None" as the default selected option
    fil_none->setChecked(true);
    currentFilter_ = FilterType::None;  // Initialize member variable

    // Connect each action's triggered signal to update the member variable
    connect(fil_none, &QAction::triggered, this, [this]() { 
        currentFilter_ = FilterType::None;
        emit filterEnableSig(false);
    });
    connect(fil_pick, &QAction::triggered, this, [this]() { 
        currentFilter_ = FilterType::PickFilter; 
        emit filterEnableSig(true);
    });
    connect(fil_roi, &QAction::triggered, this, [this]() { 
        currentFilter_ = FilterType::ROIFilter; 
        emit filterEnableSig(true);
    });
    connect(fil_circle, &QAction::triggered, this, [this]() { 
        currentFilter_ = FilterType::CircleFilter; 
        emit filterEnableSig(true);
    });

    // Add actions to the Tool menu
    toolMenu->addAction(fil_none);
    toolMenu->addAction(fil_pick);
    toolMenu->addAction(fil_roi);
    toolMenu->addAction(fil_circle);
}

// Function to setup the plot
ads::CDockWidget* MainWindow::setupPlot() {
    // Create a dock widget for the plot
    ads::CDockWidget* dockWidget = new ads::CDockWidget("Data Viewer", this);

    // Set external border with QSS
    dockWidget->setStyleSheet("QDockWidget { border: 2px solid black; }");

    // Load the QSS style file
    QFile styleFile(":/styles/style_qcp.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = styleFile.readAll();
        customPlot_->setStyleSheet(styleSheet);
    } else {
        qDebug() << "Error: Could not read the QSS style file:" << styleFile.fileName();
    }

    customPlot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                 QCP::iSelectLegend | QCP::iSelectPlottables);

    // Set axis labels
    customPlot_->xAxis->setLabel("X-axis");
    customPlot_->yAxis->setLabel("Z-axis");

    // Set axis ranges to create a closed box
    customPlot_->xAxis->setRange(-1000, 1000);   // Set x-axis range
    customPlot_->yAxis->setRange(-1000, 1000);     // Set z-axis range

    // Enable grid
    customPlot_->xAxis->grid()->setVisible(true);
    customPlot_->yAxis->grid()->setVisible(true);

    // Show all axes
    customPlot_->xAxis->setVisible(true);
    customPlot_->yAxis->setVisible(true);
    customPlot_->xAxis2->setVisible(true); // Keep top x-axis visible
    customPlot_->yAxis2->setVisible(true); // Keep right y-axis visible

    // Hide ticks and labels for the top and right axes
    customPlot_->xAxis2->setTickLabels(false); // Hide tick labels for top x-axis
    customPlot_->yAxis2->setTickLabels(false); // Hide tick labels for right y-axis

    // Set pen for the axes to be bold
    QPen axisPen(Qt::black, 2); // Create a pen with a width of 2
    customPlot_->xAxis->setBasePen(axisPen);
    customPlot_->yAxis->setBasePen(axisPen);
    customPlot_->xAxis2->setBasePen(axisPen);
    customPlot_->yAxis2->setBasePen(axisPen);

    // Set tick pen for the visible axes
    customPlot_->xAxis->setTickPen(axisPen);
    customPlot_->yAxis->setTickPen(axisPen);
    customPlot_->xAxis2->setTickPen(axisPen); // Set tick pen for top x-axis
    customPlot_->yAxis2->setTickPen(axisPen); // Set tick pen for right y-axis

    // Add a graph to the custom plot
    customPlot_->addGraph();
    customPlot_->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8)); // Use disc markers
    customPlot_->graph(0)->setPen(QPen(Qt::blue, 2)); // Set line color and width

    // Disable lines by setting the line style to none
    customPlot_->graph(0)->setLineStyle(QCPGraph::lsNone);

    // Connect QCP graph selection change signal
    connect(customPlot_, &QCustomPlot::selectionChangedByUser, this, &MainWindow::selectionChanged);
    
    // Create a QWidget to hold the buttons
    QWidget *buttonContainer = new QWidget(dockWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(10, 10, 0, 0); // Add margins for button layout

    // Helper lambda function to create buttons
    auto createButton = [&](const QString& iconPath, const QString& toolTip,
                            const QSize& btnSize = QSize(40, 40), 
                            const QSize& iconSize = QSize(25, 25)) -> QPushButton* 
    {
        QPushButton* button = new QPushButton(QIcon(iconPath), "", dockWidget);
        button->setToolTip(toolTip);
        button->setFixedSize(btnSize); // Set custom button size
        button->setIconSize(iconSize); // Set custom icon size
        return button;
    };

    // Create buttons for controlling the display functionality
    QPushButton *ctlBtn_expand = createButton(":/icons/expand-arrows.png", "Expand Data Plot");
    QPushButton *ctlBtn_play = createButton(":/icons/play.png", "Run and pause camera grab");
    QPushButton *ctlBtn_capture = createButton(":/icons/photo-capture.png", "Capture current view");
    QPushButton *ctlBtn_download = createButton(":/icons/download.png", "Download data");
    QPushButton *ctlBtn_upload = createButton(":/icons/upload.png", "Upload data");
    QPushButton *ctlBtn_filter = createButton(":/icons/filter.png", "Filter data");
    QPushButton *ctlBtn_refresh = createButton(":/icons/refresh.png", "Refresh the view");
    QPushButton *ctlBtn_trash = createButton(":/icons/trash.png", "Clear data");

    // Set the button to checkable mode, allowing it to stay pressed after being clicked
    ctlBtn_filter->setCheckable(true);
    ctlBtn_filter->setEnabled(false);
    connect(this, &MainWindow::filterEnableSig, this, [=](bool status) {
        ctlBtn_filter->setEnabled(status); 
    });

    // Connect buttons to empty slots
    connect(ctlBtn_expand, &QPushButton::clicked, [=]() {
        // Slot for expanding the data plot (currently empty)
    });
    
    connect(ctlBtn_play, &QPushButton::clicked, [=]() {
        // Check sensor connect first
        if (!curCamInfo_.isConnected) {
            QMessageBox::warning(this, "Input Error", "Please connect sensor before open it.");
            return;
        }

        // Check the current icon and toggle between play and pause icons
        if (!isPlaying_) {
            // Switch to pause icon
            ctlBtn_play->setIcon(QIcon(":/icons/pause.png"));
            ctlBtn_play->setToolTip("Pause camera grab");
            isPlaying_ = true;

            // Start the thread, triggering ContinueGrabPlot, sensor shouble open first
            sensorApi_.SetStatus(true);
            grabThread_->start();
        } 
        else {
            // Switch to play icon
            ctlBtn_play->setIcon(QIcon(":/icons/play.png"));
            ctlBtn_play->setToolTip("Run camera grab");

            // isPlay set to false to stop thread loop
            isPlaying_ = false;
            grabWorker_->stopGrabbing();
            grabThread_->quit();
            sensorApi_.SetStatus(false);  // Stop the camera
        }
    });

    connect(ctlBtn_capture, &QPushButton::clicked, [=]() {
        if (curCamInfo_.isConnected) {
            sensorApi_.SetStatus(true);
            sensorApi_.GrabOnce();
            sensorApi_.SetStatus(false);
            extractProfilePoints(sensorApi_.RetriveData());
            QcpPlot(customPlot_, sensorApi_.RetriveData());
            emit logMessageReceived("Info: Open camera and grab once.");
        } 
        else {
            QMessageBox::warning(this, "Input Error", "Please connect sensor before open it.");
            return;
        }      
    });

    connect(ctlBtn_download, &QPushButton::clicked, this, &MainWindow::onDownloadButtonClicked);

    // Connect the upload button's clicked signal to the slot function
    connect(ctlBtn_upload, &QPushButton::clicked, this, &MainWindow::onUploadButtonClicked);

    connect(ctlBtn_filter, &QPushButton::clicked, this, &MainWindow::onFilterButtonClicked);

    connect(ctlBtn_refresh, &QPushButton::clicked, this, &MainWindow::refreshPlot);

    connect(ctlBtn_trash, &QPushButton::clicked, [=]() {
        // Clear curData_
        curData_.pointCount = 0; // Set pointCount to 0
        curData_.x.clear();      // Clear the QVector<double> x
        curData_.z.clear();      // Clear the QVector<double> z

        // Clear QCP graph
        customPlot_->clearGraphs();
        customPlot_->replot();
    });

    // Add buttons to the container layout
    buttonLayout->addWidget(ctlBtn_expand);
    buttonLayout->addWidget(ctlBtn_play);
    buttonLayout->addWidget(ctlBtn_capture);
    buttonLayout->addWidget(ctlBtn_download);
    buttonLayout->addWidget(ctlBtn_upload);
    buttonLayout->addWidget(ctlBtn_filter);
    buttonLayout->addWidget(ctlBtn_refresh);
    buttonLayout->addWidget(ctlBtn_trash);

    // Create a QWidget to hold the main layout
    QWidget *widget = new QWidget(dockWidget);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0); // Remove margins for layout

    // Add button container to the main layout
    layout->addWidget(buttonContainer, 0, Qt::AlignLeft | Qt::AlignTop); // Align buttons to top-left
    layout->addWidget(customPlot_, 1); // Add the custom plot below the buttons with stretch factor

    widget->setLayout(layout); // Set the layout to the widget
    dockWidget->setWidget(widget); // Set the widget with the layout to the dock widget
    dockWidget->show();

    // Return the dock widget
    return dockWidget;
}

// Function to setup the log viewer
ads::CDockWidget* MainWindow::setupLogViewer() {
    // Create a dock widget for the log viewer
    ads::CDockWidget* logDockWidget = new ads::CDockWidget("Log Viewer", this);

    // Set external border with QSS
    logDockWidget->setStyleSheet("QDockWidget { border: 2px solid black; }");

    // Create a QTextEdit for the log messages
    logTextEdit_ = new QTextEdit(logDockWidget); // Make logTextEdit a member variable
    logTextEdit_->setReadOnly(true); // Make it read-only
    logTextEdit_->setStyleSheet("QTextEdit { background-color: white; }");

    // Add the log text edit to the dock widget
    logDockWidget->setWidget(logTextEdit_);

    // Connect the log message signal to the slot
    connect(this, &MainWindow::logMessageReceived, this, &MainWindow::onLogMessageReceived);

    // Show the dock widget
    logDockWidget->show();

    // Return the dock widget
    return logDockWidget;
}

ads::CDockWidget* MainWindow::setupPropertyBrowser()
{
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Property Browser"));
    
    QWidget* w = new QWidget();

    QtStringPropertyManager* fileName = new QtStringPropertyManager(w);
    QtPathPropertyManager* filePath = new QtPathPropertyManager(w);
    QtStringPropertyManager* objName = new QtStringPropertyManager(w);
    QtIntPropertyManager* pntsNumber = new QtIntPropertyManager(w);
    QtGroupPropertyManager* filter = new QtGroupPropertyManager(w);
    QtBoolPropertyManager* filterEnable = new QtBoolPropertyManager(w);
    QtEnumPropertyManager* filterAlgor = new QtEnumPropertyManager(w);
    QtEnumPropertyManager* color = new QtEnumPropertyManager(w);

    QtProperty* item0 = fileName->addProperty("File Name");
    QtProperty* item1 = filePath->addProperty("Path");
    QtProperty* item2 = objName->addProperty("Objection");
    QtProperty* item3 = pntsNumber->addProperty("Points Number");
    QtProperty* item4 = filter->addProperty("Filter");
    QtProperty* item5 = filterEnable->addProperty("Filter Enable");
    QtProperty* item6 = filterAlgor->addProperty("Method");
    QtProperty* item7 = color->addProperty("Color");

    item4->addSubProperty(item5);
    item4->addSubProperty(item6);

    QtAbstractPropertyBrowser *editor = new QtTreePropertyBrowser(w);
    editor->addProperty(item0);
    editor->addProperty(item1);
    editor->addProperty(item2);
    editor->addProperty(item3);
    editor->addProperty(item4);
    editor->addProperty(item7);

    // build layout to DockWidget
    QVBoxLayout* v_layout = new QVBoxLayout(w);

    v_layout->addWidget(editor);

    DockWidget->setWidget(w);
    DockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);

    return DockWidget;
}

MainWindow::~MainWindow() {
    grabThread_->quit();    // Stop the thread when MainWindow is destroyed
    grabThread_->wait();

    delete customPlot_;     // Clean up
}

void MainWindow::logCallback(const char* message) {
    if (instance) {
        QString log = QString::fromUtf8(message);
        emit instance->logMessageReceived(log);
    }
}

// Function to get scaled size based on screen DPI
int MainWindow::getScaledSize(int baseSize) {
    int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    return dpi / 96 * baseSize; // Scale the size based on DPI
}

void MainWindow::extractProfilePoints(const Gocator_Data& gocatorData) {
    // Resize the vectors to match the number of profile points
    curData_.x.resize(static_cast<int>(gocatorData.pointCount));
    curData_.z.resize(static_cast<int>(gocatorData.pointCount));
    curData_.pointCount = gocatorData.pointCount;

    // Iterate over each ProfilePoint in profileBuffer and populate x and z vectors
    for (int i = 0; i < gocatorData.pointCount; ++i) {
        curData_.x[i] = gocatorData.profileBuffer[i].x;
        curData_.z[i] = gocatorData.profileBuffer[i].z;
    }
}

// Slot implementations
void MainWindow::onNewSecTriggered() {
    QMessageBox::information(this, "NewSection", "NewSection triggered!");
}

// Slot function to trigger the IP connection dialog
void MainWindow::onIpConnTriggered() {
    QDialog dialog(this);
    dialog.setWindowTitle("System IP Address");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // Get the list of adapters
    std::vector<AdapterInfo> adapters = getNetworkAdapters();
    
    // ComboBox to select an adapter
    QComboBox *adapterComboBox = new QComboBox;
    // Iterate through the adapters and add them to the ComboBox
    for (const auto &adapter : adapters) {
        // Convert the adapter name from std::string (UTF-8) to QString (UTF-16)
        QString adapterDesc = QString::fromStdString(adapter.description); // Assuming adapter.first is in UTF-8
        QString displayText = adapterDesc + " (IP: " + QString::fromStdString(adapter.ipAddress) + ")";
        
        // Add the formatted string to the ComboBox
        adapterComboBox->addItem(displayText);
    }

    // Form layout and input fields for IP settings
    QFormLayout *formLayout = new QFormLayout;
    
    QLineEdit *ipLineEdit = new QLineEdit;
    QLineEdit *subnetLineEdit = new QLineEdit;
    QLineEdit *gatewayLineEdit = new QLineEdit;

    // Regular expression for IP address validation
    QRegExp ipRegex("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);

    ipLineEdit->setValidator(ipValidator);
    subnetLineEdit->setValidator(ipValidator);
    gatewayLineEdit->setValidator(ipValidator);

    formLayout->addRow("IP Address:", ipLineEdit);
    formLayout->addRow("Subnet Mask:", subnetLineEdit);
    formLayout->addRow("Default Gateway:", gatewayLineEdit);

    // Initialize with the selected adapter's IP address
    if (!adapters.empty()) {
        ipLineEdit->setText(QString::fromStdString(adapters[0].ipAddress));
    }

    // Update IP related address when a different adapter is selected
    connect(adapterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index >= 0 && index < adapters.size()) {
            ipLineEdit->setText(QString::fromStdString(adapters[index].ipAddress));
            subnetLineEdit->setText(QString::fromStdString(adapters[index].subnetMask));
            gatewayLineEdit->setText(QString::fromStdString(adapters[index].defaultGateway));
        }
    });

    // OK and Cancel buttons
    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Add components to main layout
    mainLayout->addWidget(adapterComboBox);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // Connect the Cancel button to close the dialog
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Connect the OK button to validate input and set IP configuration
    connect(okButton, &QPushButton::clicked, [&]() {
        QString ip = ipLineEdit->text();
        QString subnet = subnetLineEdit->text();
        QString gateway = gatewayLineEdit->text();

        // Validate inputs
        if (ipLineEdit->hasAcceptableInput() && subnetLineEdit->hasAcceptableInput() && gatewayLineEdit->hasAcceptableInput()) {
            // Construct the netsh command
            QString selectedAdapterName = QString::fromStdString(adapters[adapterComboBox->currentIndex()].name).trimmed();
            QString command = QStringLiteral("C:\\Windows\\System32\\netsh.exe interface ip set address name=\"%1\" static %2 %3 %4")
                            .arg(selectedAdapterName, ip, subnet, gateway);

            // Print the command and parameters for logger
            QStringList logList;
            logList << "========= Setting System Ip Address... ========="
                << "Selected Adapter Name: " + selectedAdapterName
                << "IP Address: " + ip
                << "Subnet Mask: " + subnet
                << "Default Gateway: " + gateway
                << "Constructed Command: " + command;
            QString log = logList.join("\n");
            emit logMessageReceived(log);

            // Execute the command
            executeNetshCommand(command);

            dialog.accept();            // Close the dialog
        } else {
            // Show warning if input is invalid
            QMessageBox::warning(this, "Input Error", "Please enter valid IP address, subnet mask, and gateway.");
            emit logMessageReceived("Input Error: Invalid IP Address");
        }
    });

    // Show the dialog
    dialog.exec();
}

// Slot to handle received log messages
void MainWindow::onLogMessageReceived(const QString &message) {
    // Get the current date and time and format it
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"); // Format can be adjusted as needed

    // Prepend the current time and ">> " to the message for console-like output
    QString formattedMessage = QString("[%1] >> %2").arg(currentTime, message);
    
    // Append the formatted message to the log text edit
    logTextEdit_->append(formattedMessage);     // Display the log message in the log viewer
}

// Slot function to handle the filter button click event
void MainWindow::onFilterButtonClicked() {
    // Get the button that sent the signal
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;        // If the cast fails, exit the function

    // Check CurData_ before activate filter
    if (!CheckCurData()) {
        button->setChecked(false);
        return;
    }  

    // Check the button's state, if it is checked (pressed)
    if (button->isChecked()) {
        // Define a list of colors for the clusters
        QVector<QColor> colors = generateColors(100);
        
        if (currentFilter_ == FilterType::PickFilter) {
            // TODO: function to set dbscan data
            DBSCANCluster dbscan(5, 3);         // Create a DBSCANCluster object
            std::vector<double> x_stdv(curData_.x.begin(), curData_.x.end());   // Get x coordinates
            std::vector<double> z_stdv(curData_.z.begin(), curData_.z.end());   // Get z coordinates
            dbscan.setData(x_stdv, z_stdv);     // Set data for clustering
            dbscan.runClustering();             // Perform clustering

            double noiseRatioThreshold = 0.2;
            int countGraph = dbscan.getOptimalClusterCount(noiseRatioThreshold); // Get the optimal number of clusters
            emit logMessageReceived(QString("Info: The number of clusters in the point set is: %1").arg(countGraph));
            std::vector<std::vector<double>> xz_stdvs = dbscan.getClusteredData(); // Get clustered data
            std::vector<int> label_idx = dbscan.getClusteredLabels();   // Get cluster labels

            // Create QVector<double> to hold x and z coordinates
            QVector<double> xCoordinates;
            QVector<double> zCoordinates;

            // Clear previous graphs from the plot
            customPlot_->clearGraphs();

            // Iterate through each graph's data
            for (int i = 0; i < countGraph; i++) {
                // Clear previous graph coordinates
                xCoordinates.clear();
                zCoordinates.clear();

                // Collect x and z coordinates for the current graph
                for (int j = 0; j < label_idx.size(); j += 2) {
                    if (i == label_idx[j]) {
                        xCoordinates.append(xz_stdvs[0][j]);       // Add x coordinate
                        zCoordinates.append(xz_stdvs[1][j]);       // Add z coordinate
                    }
                }

                // Add a new graph to customPlot
                customPlot_->addGraph();
                customPlot_->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8)); // Use disc markers
                customPlot_->graph(i)->setPen(QPen(colors[i % colors.size()], 2)); // Set color and width
                customPlot_->graph(i)->setLineStyle(QCPGraph::lsNone); // No connecting line

                // Set the data for the current graph
                customPlot_->graph(i)->setData(xCoordinates, zCoordinates);
            }

            // Replot customPlot to update the view
            customPlot_->replot();
            emit logMessageReceived(QString("Info: The number of plotted graphs: %1").arg(customPlot_->graphCount()));

            // Setup policy and connect slot for context menu popup:
            resetAllGraphsStatus();     // Reset all graphs status as negative
            customPlot_->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(customPlot_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
        } 
        // TODO: Add others filter tool implements
        else {
            
        }
    } 
    else {
        disconnect(customPlot_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
        // // If the button is not checked, clear the plot
        // customPlot_->clearGraphs();
        // customPlot_->replot(); // Refresh the plot
        // emit logMessageReceived("Info: Filter deactivated and plot cleared.");
    }
}

void MainWindow::onDownloadButtonClicked() {
    // Create a QSettings object to store the last used path
    QSettings settings("Shanghai Fanuc", "SnapMaster"); // Replace with your organization and application name
    QString lastPath = settings.value("lastSavePath", "").toString(); // Load the last used path

    // Open file dialog to select save path
    QString filePath = QFileDialog::getSaveFileName(nullptr, "Select Save Location", lastPath, "JSON Files (*.json);;Text Files (*.txt);;All Files (*)");

    // Check if the path is empty, if so, show a warning and return
    if (filePath.isEmpty()) {
        QMessageBox::warning(nullptr, "Save Error", "No path selected, download canceled.");
        return;
    }

    // Check if curData is empty
    if (curData_.pointCount == 0 || curData_.x.isEmpty() || curData_.z.isEmpty()) {
        QMessageBox::warning(nullptr, "Data Error", "No data available to save.");
        return;  // Exit if there's no data to save
    }

    // Determine the file format based on the extension
    QFile file(filePath);
    if (filePath.endsWith(".json")) {
        // Convert to JSON and save to file
        QJsonObject jsonObj = SensorQDataToJson(curData_);
        QJsonDocument jsonDoc(jsonObj);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(jsonDoc.toJson(QJsonDocument::Indented));  // Indented for readability
            file.close();
        }

        // Save the last used path
        settings.setValue("lastSavePath", QFileInfo(filePath).absolutePath());

        // Display success message
        QMessageBox::information(nullptr, "Download Complete", "Data successfully saved to:\n " + filePath);
    } 
    else if (filePath.endsWith(".txt")) {
        // Save as TXT format
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < curData_.pointCount; ++i) {
                // Write x, 0, z for each point
                out << curData_.x[i] << ", 0, " << curData_.z[i] << "\n";
            }
            file.close();
        }

        // Save the last used path
        settings.setValue("lastSavePath", QFileInfo(filePath).absolutePath());

        // Display success message
        QMessageBox::information(nullptr, "Download Complete", "Data successfully saved to:\n " + filePath);
    } 
    else {
        QMessageBox::warning(nullptr, "Save Error", "Unsupported file format. Please select a .json or .txt file.");
    }
}

// Define the slot function
void MainWindow::onUploadButtonClicked() {
    // Open a file dialog to select a JSON or TXT file to upload
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Select File to Upload", "", "All Files (*);;JSON Files (*.json);;Text Files (*.txt)");

    // Check if the file path is empty
    if (filePath.isEmpty()) {
        QMessageBox::warning(nullptr, "Upload Error", "No file selected, upload canceled.");
        return;
    }

    // Initialize curData_ as a SensorQData structure
    curData_.clear();  // Clear any existing data

    if (filePath.endsWith(".json")) {
        // Handle JSON file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(nullptr, "File Error", "Failed to open the file.");
            return;
        }

        QByteArray fileData = file.readAll();       // Read the entire file into a QByteArray
        file.close();       // Close the file after reading

        // Parse JSON data
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            QMessageBox::warning(nullptr, "Format Error", "Invalid JSON format in the file.");
            return;
        }

        // Convert JSON data to curData_
        QJsonObject jsonObj = jsonDoc.object();
        curData_ = JsonToSensorQData(jsonObj);  // Assuming this function is defined to convert JSON to SensorQData
    }
    else if (filePath.endsWith(".txt")) {
        // Handle TXT file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(nullptr, "File Error", "Failed to open the file.");
            return;
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList values = line.split(',');

            if (values.size() != 3) {
                QMessageBox::warning(nullptr, "Format Error", "Invalid TXT format in the file.");
                return;
            }

            bool okX, okZ;
            double x = values[0].toDouble(&okX);
            double z = values[2].toDouble(&okZ);

            if (!okX || !okZ) {
                QMessageBox::warning(nullptr, "Conversion Error", "Failed to parse number values in the TXT file.");
                return;
            }

            curData_.x.append(x);
            curData_.z.append(z);
            curData_.pointCount++;
        }
        file.close();
    }
    else {
        QMessageBox::warning(nullptr, "File Error", "Unsupported file format.");
        return;
    }

    refreshPlot();

    // Display success message
    QMessageBox::information(nullptr, "Upload Complete",
        "Data successfully uploaded from:\n " + filePath);
}

void MainWindow::showScanCameraDialog() {
    QDialog dialog(this); // Create a dialog
    dialog.setWindowTitle("Scan Cameras");
    dialog.setFixedSize(600, 400); // Set fixed size

    QVBoxLayout *layout = new QVBoxLayout(&dialog); // Create a vertical layout for the dialog
    layout->setContentsMargins(10, 10, 10, 10); // Set margins for the layout

    QLabel *infoLabel = new QLabel("Select Camera:", &dialog); // Label for selecting a camera
    layout->addWidget(infoLabel);

    // Create a combo box for selecting camera IDs
    QComboBox *cameraComboBox = new QComboBox(&dialog);
    layout->addWidget(cameraComboBox); // Add combo box to layout
    if (curCamInfo_.id !=-1){
        cameraComboBox->addItem(QString("%1 - %2").arg(curCamInfo_.id)
                    .arg(QString::fromStdString(curCamInfo_.ipAddress)));
    }

    QPushButton *scanButton = new QPushButton("Scan", &dialog); // Scan button
    layout->addWidget(scanButton);

    // Create a group box for camera status
    QGroupBox *statusGroupBox = new QGroupBox("Camera Status", &dialog); // Create a group box
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroupBox); // Create a vertical layout for the group box

    // Create a label for displaying current camera brand
    QLabel *brandLabel = new QLabel("Camera Brand: " + 
        QString::fromStdString(curCamInfo_.brand), statusGroupBox); // Initial brand label
    statusLayout->addWidget(brandLabel);        // Add brand label to group box layout

    // Create a horizontal layout for status and camera ID
    QString id_s = (curCamInfo_.id == -1) ? "N/A" : QString::number(curCamInfo_.id);
    QString conn_s = (curCamInfo_.isConnected) ? "Connected" : "Not Connected";
    QHBoxLayout *statusIdLayout = new QHBoxLayout();
    QLabel *statusLabel = new QLabel("Status: " + conn_s, statusGroupBox); // Initial status
    QLabel *cameraIdLabel = new QLabel("Camera ID: " + id_s, statusGroupBox); // Initial camera ID

    statusIdLayout->addWidget(statusLabel); // Add status label to horizontal layout
    statusIdLayout->addWidget(cameraIdLabel); // Add camera ID label to horizontal layout
    statusLayout->addLayout(statusIdLayout); // Add horizontal layout to group box layout

    layout->addWidget(statusGroupBox); // Add group box to main layout

    // Create a horizontal layout for connect and disconnect buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *connectButton = new QPushButton("Connect", &dialog); // Connect button
    buttonLayout->addWidget(connectButton); // Add connect button to layout

    QPushButton *disconnectButton = new QPushButton("Disconnect", &dialog); // Disconnect button
    buttonLayout->addWidget(disconnectButton); // Add disconnect button to layout

    layout->addLayout(buttonLayout); // Add horizontal layout to main layout

    // Create a label for displaying camera information
    QLabel *cameraInfoLabel = new QLabel(&dialog); // Define camera info label
    layout->addWidget(cameraInfoLabel); // Add to layout

    connect(scanButton, &QPushButton::clicked, [=]() {
        std::vector<CameraInfo> sensorList; // Vector to hold camera info
        // Scan the cameras and update the sensor list
        CameraStatus status = sensorApi_.Scan(sensorList);
        if (status == CameraStatus::Go_OK) {
            // Populate the combo box with camera info if scan is successful
            cameraComboBox->clear(); // Clear previous entries
            for (const auto& camera : sensorList) {
                cameraComboBox->addItem(QString("%1 - %2").arg(camera.id)
                    .arg(QString::fromStdString(camera.ipAddress)));
            }

            // Update curCamInfo with the first camera's information
            if (!sensorList.empty()) {
                curCamInfo_.ipAddress = sensorList[0].ipAddress; // Update IP address
                curCamInfo_.id = sensorList[0].id;               // Update camera ID
                // Optionally, you can set curCamInfo.brand if brand information is available
                curCamInfo_.brand = sensorList[0].brand;         // Update camera brand if needed
                cameraIdLabel->setText(QString("Camera ID: %1").arg(curCamInfo_.id)); // Update with the first camera ID
            } else {
                QMessageBox::warning(this, "Warning", "No device detected.");
                return;
            }

            // Update status and camera ID labels
            cameraIdLabel->setText(QString("Camera ID: %1").arg(sensorList[0].id)); // Update with the first camera ID
            cameraInfoLabel->setText("Cameras scanned successfully."); // Update camera info label

            // Connect the combo box selection change to update curCamInfo
            connect(cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, sensorList](int index) {
                if (index >= 0 && index < sensorList.size()) {
                    // Update curCamInfo based on the selected camera
                    curCamInfo_.ipAddress = sensorList[index].ipAddress; // Update IP address
                    curCamInfo_.id = sensorList[index].id;               // Update camera ID
                    curCamInfo_.brand = sensorList[index].brand;         // Update camera brand if needed
                    // Optionally, update labels or other UI elements here
                }
            });
        } else {
            cameraInfoLabel->setText("Failed to scan cameras."); // Display error message
        }
    });

    // Connect the buttons to slots
    connect(connectButton, &QPushButton::clicked, [=]() {
        // Check if the IP address is valid (not empty)
        if (curCamInfo_.ipAddress.empty() || curCamInfo_.ipAddress == "0.0.0.0") {
            // Show warning message if IP address is not set
            QMessageBox::warning(this, "Warning", "Please select a valid camera IP address before connecting.");
            return; // Exit the function to prevent connecting
        }

        // Attempt to connect to the camera
        CameraStatus status = sensorApi_.Connect(curCamInfo_.ipAddress);
        if (status == CameraStatus::Go_OK) {
            // Update connection status and brand label if successful
            curCamInfo_.isConnected = true;
            QString log = QString("Info: Connected to camera: %1").arg(QString::fromStdString(curCamInfo_.ipAddress));
            emit logMessageReceived(log);
            statusLabel->setText("Status: Connected"); // Update status label
        } else {
            // Show error message if connection fails
            QMessageBox::critical(this, "Error", "Failed to connect to the camera.");
        }
    });

    connect(disconnectButton, &QPushButton::clicked, [=]() {
        // Attempt to disconnect to the camera
        CameraStatus status = sensorApi_.Disconnect(curCamInfo_.ipAddress);
        if (status == CameraStatus::Go_NOT_CONNECTED) {
            // Update connection status and brand label if disconnect successful
            curCamInfo_.isConnected = false;
            QString log = QString("Info: Disconnected to camera: %1").arg(QString::fromStdString(curCamInfo_.ipAddress));
            emit logMessageReceived(log);
            statusLabel->setText("Status: Disconnected"); // Update status label
        } else {
            // Show error message if connection fails
            QMessageBox::critical(this, "Error", "Failed to disconnect to the camera.");
        }
    });

    dialog.exec(); // Show the dialog as a modal window
}

void MainWindow::selectionChanged()
{
    /*
    * This function synchronizes the selection state of various elements in the plot.
    * - Ensures that the x-axes and y-axes (bottom/top and left/right) are selected synchronously.
    * - Ties together the axis and tick label selections to act as a single selectable object.
    * - Synchronizes the selection of graphs with their respective legend items.
    */
    
    // Synchronize top and bottom x-axis selection
    if (customPlot_->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot_->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        customPlot_->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot_->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        customPlot_->xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        customPlot_->xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // Synchronize left and right y-axis selection
    if (customPlot_->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot_->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        customPlot_->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot_->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        customPlot_->yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        customPlot_->yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // Synchronize graph selection with legend item selection
    for (int i = 0; i < customPlot_->graphCount(); ++i) {
        QCPGraph *graph = customPlot_->graph(i);
        QCPPlottableLegendItem *item = customPlot_->legend->itemWithPlottable(graph);

        if (item && (item->selected() || graph->selected())) {
            item->setSelected(true); // Select the legend item
            graph->setSelection(QCPDataSelection(graph->data()->dataRange())); // Select all data points in the graph
        }
    }
}

void MainWindow::contextMenuRequest(QPoint pos)
{
    // Create a new context menu and set it to delete itself when closed
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Add an option to remove selected graphs if any are selected
    if (customPlot_->selectedGraphs().size() > 0) {

        // Define the action for marking the selected graph as positive
        QAction *markAsPositiveAction = new QAction("Mark selected graph as positive[+]", this);
        markAsPositiveAction->setCheckable(true);

        // Add the action to the menu
        menu->addAction(markAsPositiveAction);

        // Write graphs status to context menu
        QCPGraph *selectedGraph = customPlot_->selectedGraphs().first();
        bool isPositive = graphStates_[selectedGraph];
        markAsPositiveAction->setChecked(isPositive);

        // Map to store the state of each graph (positive or negative)
        connect(markAsPositiveAction, &QAction::triggered, this, [=]() {
            // Toggle the check state and the graph's marking status
            if (isPositive) {
                markAsPositiveAction->setChecked(false);
                graphStates_[selectedGraph] = false;
            } else {
                markAsPositiveAction->setChecked(true);
                graphStates_[selectedGraph] = true;
            }
        });
    }

    // Add an option to remove non-positive graphs if there are any on the plot
    if (customPlot_->graphCount() > 0) {
        QAction *rmAllNonPositiveGraphsAction = new QAction("Remove all non-positive graphs", this);

        connect(rmAllNonPositiveGraphsAction, &QAction::triggered, this, [=]() {
            // Iterate over all graphs in customPlot_
            for (int i = customPlot_->graphCount() - 1; i >= 0; --i) {
                QCPGraph *graph = customPlot_->graph(i);    // Get the graph at index i
                
                // Check if the graph is marked as non-positive
                if (!graphStates_[graph]) { 
                    // false indicates negative or non-positive
                    customPlot_->removeGraph(graph);    // Remove the non-positive graph
                }
            }
            
            // Replot the customPlot_ to update the view
            customPlot_->replot(); 

            // Cache the current plot data after replot
            QVector<SensorQData> cacheData = cachePlotData(customPlot_);

            // Merge each SensorQData in cacheData into curData_
            curData_.clear();
            for (const SensorQData &data : cacheData)
                curData_.append(data);
        });
        menu->addAction(rmAllNonPositiveGraphsAction);
    }
  
    // Display the context menu at the global position mapped from the custom plot position
    menu->popup(customPlot_->mapToGlobal(pos));
}

void MainWindow::removeSelectedGraph()
{
    if (customPlot_->selectedGraphs().size() > 0) {
        customPlot_->removeGraph(customPlot_->selectedGraphs().first());
        customPlot_->replot();
    }
}

void MainWindow::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    
    if (customPlot_->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        customPlot_->axisRect()->setRangeDrag(customPlot_->xAxis->orientation());
    else if (customPlot_->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        customPlot_->axisRect()->setRangeDrag(customPlot_->yAxis->orientation());
    else
        customPlot_->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed
    
    if (customPlot_->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        customPlot_->axisRect()->setRangeZoom(customPlot_->xAxis->orientation());
    else if (customPlot_->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        customPlot_->axisRect()->setRangeZoom(customPlot_->yAxis->orientation());
    else
        customPlot_->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

// Private member function to get the list of network adapters and their IP addresses
std::vector<AdapterInfo> MainWindow::getNetworkAdapters() {
    std::vector<AdapterInfo> adapters; // Vector to hold adapter information

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);

    // Handle buffer overflow
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    }

    // Get adapter information
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            // Create an instance of AdapterInfo and populate it
            AdapterInfo info(pAdapter->AdapterName,        // Adapter name
                             pAdapter->Description,      // Adapter description
                             pAdapter->IpAddressList.IpAddress.String, // IP address
                             pAdapter->IpAddressList.IpMask.String,    // Subnet mask
                             pAdapter->GatewayList.IpAddress.String);  // Default gateway

            adapters.push_back(info); // Add the adapter info to the vector
            pAdapter = pAdapter->Next; // Move to the next adapter
        }
    }

    // Free the allocated memory
    if (pAdapterInfo) {
        free(pAdapterInfo);
    }

    return adapters; // Return the vector of adapter information
}

// Private member function to execute a netsh command
void MainWindow::executeNetshCommand(const QString &command) {
    // Start the process to execute the netsh command
    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(); // Wait for the process to finish

    // // Read the output and error messages
    // QString output = process.readAllStandardOutput();
    // QString errorOutput = process.readAllStandardError();

    // // Handle the output
    // if (!errorOutput.isEmpty()) {
    //     // Display the error output in a message box
    //     QMessageBox::warning(this, "Execution Error", errorOutput);
    // } else {
    //     // Display the success output in a message box
    //     QMessageBox::information(this, "Success", "Command executed successfully:\n" + output);
    // }

    // Read the output and error messages
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(process.readAllStandardError());

    // Handle the output
    if (!errorOutput.isEmpty()) {
        // Display the error output in a message box
        QMessageBox::warning(this, "执行错误", errorOutput);
    } else {
        // Display the success output in a message box
        QMessageBox::information(this, "成功", "命令执行成功:\n" + output);
    }
}

void MainWindow::QcpPlot(QCustomPlot *customPlot, Gocator_Data data) {
    // Check if the profileBuffer is valid and pointCount is greater than 0
    if (data.profileBuffer == nullptr || data.pointCount == 0) {
        qWarning("Gocator_Data has no valid profile points.");
        customPlot->graph(0)->data()->clear();      // Clear data for graph 0
        customPlot->replot();
        return;
    }

    QVector<double> x(static_cast<int>(data.pointCount)); // Create vector for x coordinates
    QVector<double> z(static_cast<int>(data.pointCount)); // Create vector for z coordinates

    // Extract the profile points into x and z vectors
    for (int i = 0; i < data.pointCount; ++i) {
        x[i] = data.profileBuffer[i].x; // Extract x coordinate
        z[i] = data.profileBuffer[i].z; // Extract z coordinate
        // qDebug() << "Point" << i << ": x =" << x[i] << ", z =" << z[i]; // Print values for debugging
    }

    // Create test data for x and z within the range -700 to 700
    QVector<double> testX, testZ;
    for (int i = -14; i <= 14; ++i) {  // Generates 29 points across the range
        testX.append(i * 50);       // X-coordinates from -700 to 700
        testZ.append(-i * i + 700); // Z-coordinates from 700 down to 0 in a parabolic shape
    }

    // Assign the input data to the graph
    customPlot->graph(0)->setData(x, z);

    // Rescale axes to fit the data
    customPlot->xAxis->setRange(-1000, 1000); // Set x-axis range (adjust as needed)
    customPlot->yAxis->setRange(-1000, 1000); // Set y-axis range (adjust as needed)
    customPlot->replot();
}

void MainWindow::updatePlot() {
    // Retrive sensor data from sensor handle
    Gocator_Data data = sensorApi_.RetriveData();

    // Check if the profileBuffer is valid and pointCount is greater than 0
    if (data.profileBuffer == nullptr || data.pointCount == 0) {
        qWarning("Gocator_Data has no valid profile points.");
        customPlot_->clearGraphs();      // Clear all graphs
        customPlot_->addGraph();
        customPlot_->replot();
        return;
    } else {
        extractProfilePoints(data);
    }

    // QVector<double> x(static_cast<int>(data.pointCount)); // Create vector for x coordinates
    // QVector<double> z(static_cast<int>(data.pointCount)); // Create vector for z coordinates

    // // Extract the profile points into x and z vectors
    // for (int i = 0; i < data.pointCount; ++i) {
    //     x[i] = data.profileBuffer[i].x; // Extract x coordinate
    //     z[i] = data.profileBuffer[i].z; // Extract z coordinate
    //     // qDebug() << "Point" << i << ": x =" << x[i] << ", z =" << z[i]; // Print values for debugging
    // }

    // Create test data for x and z within the range -700 to 700
    // QVector<double> testX, testZ;
    // for (int i = -14; i <= 14; ++i) {  // Generates 29 points across the range
    //     testX.append(i * 50);       // X-coordinates from -700 to 700
    //     testZ.append(-i * i + 700); // Z-coordinates from 700 down to 0 in a parabolic shape
    // }

    refreshPlot();
}

void MainWindow::refreshPlot() {
    // Check if curData_ is empty
    if (curData_.pointCount == 0 || curData_.x.isEmpty() || curData_.z.isEmpty()) {
        QMessageBox::warning(this, "Plot Error", "No data available for plotting.");
        return;  // Exit the function if there is no data to plot
    }

    // Check if there are any existing graphs, if not, add the first graph
    if (customPlot_->graphCount() == 0) {
        customPlot_->addGraph();
    }
    else {
        customPlot_->clearGraphs();
        customPlot_->addGraph();
    }

    // Reset graph style
    customPlot_->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8)); // Use disc markers
    customPlot_->graph(0)->setPen(QPen(Qt::blue, 2));   // Set line color and width
    customPlot_->graph(0)->setLineStyle(QCPGraph::lsNone);

    // Assign the input data to the graph
    customPlot_->graph(0)->setData(curData_.x, curData_.z);

    // Rescale axes to fit the data
    customPlot_->xAxis->setRange(-1000, 1000); // Set x-axis range (adjust as needed)
    customPlot_->yAxis->setRange(-1000, 1000); // Set y-axis range (adjust as needed)
    customPlot_->replot();
}

QMenu* MainWindow::createCameraMenu() {
    // Create a menu for camera selection
    QMenu *cameraMenu = new QMenu(this);
    
    // Hardcode available camera options
    QStringList availableCameras = { "SSZN", "LMI" };

    // Add camera options to the menu
    for (const QString &camera : availableCameras) {
        QAction *cameraAction = new QAction(camera, this);
        cameraAction->setCheckable(true); // Make it checkable

        // Connect to handle selection
        connect(cameraAction, &QAction::triggered, [this, cameraAction, cameraMenu]() {
            // Check if the current camera is connected
            if (curCamInfo_.isConnected) {
                // Show warning message if the camera is connected
                QMessageBox::warning(this, "Warning", "Please disconnect the camera before switching brands.");
                return; // Exit the function to prevent switching
            }

            // Logic to handle camera selection
            for (QAction *action : cameraMenu->actions()) {
                if (action != cameraAction) {
                    action->setChecked(false); // Uncheck all other actions
                } else {
                    action->setChecked(true); // Check the selected action
                    // Update current camera info and log the change
                    curCamInfo_.brand = cameraAction->text().toStdString(); // Update brand in curCamInfo
                }
            }

            // Update current camera info and log the change
            curCamInfo_.brand = cameraAction->text().toStdString(); // Update brand in currentCameraInfo
            QString log = "Info: Selected Camera Brand: " + cameraAction->text(); // Log selected camera
            emit logMessageReceived(log);
        });

        cameraMenu->addAction(cameraAction); // Add action to camera menu
    }

    return cameraMenu;  // Return the created camera menu
}

QJsonObject MainWindow::SensorQDataToJson(const SensorQData& data) {
    QJsonObject jsonObj;
    jsonObj["pointCount"] = static_cast<int>(data.pointCount); // Convert size_t to int

    QJsonArray xArray, zArray;
    for (double value : data.x)
        xArray.append(value);
    for (double value : data.z)
        zArray.append(value);

    jsonObj["x"] = xArray;
    jsonObj["z"] = zArray;

    return jsonObj;
}

// Function to convert JSON object to SensorQData structure
SensorQData MainWindow::JsonToSensorQData(const QJsonObject& jsonObj) {
    SensorQData data;

    // Extract point count as size_t
    if (jsonObj.contains("pointCount") && jsonObj["pointCount"].isDouble()) {
        data.pointCount = static_cast<size_t>(jsonObj["pointCount"].toInt());
    } else {
        data.pointCount = 0;  // Default if not available or invalid
    }

    // Extract x coordinates array
    if (jsonObj.contains("x") && jsonObj["x"].isArray()) {
        QJsonArray xArray = jsonObj["x"].toArray();
        data.x.reserve(xArray.size());  // Reserve space in the QVector
        for (const QJsonValue& value : xArray) {
            if (value.isDouble()) {
                data.x.append(value.toDouble());
            }
        }
    }

    // Extract z coordinates array
    if (jsonObj.contains("z") && jsonObj["z"].isArray()) {
        QJsonArray zArray = jsonObj["z"].toArray();
        data.z.reserve(zArray.size());  // Reserve space in the QVector
        for (const QJsonValue& value : zArray) {
            if (value.isDouble()) {
                data.z.append(value.toDouble());
            }
        }
    }

    return data;
}

void MainWindow::resetAllGraphsStatus() {
    graphStates_ = [=]{
        QMap<QCPGraph*, bool> map;
        for (int i = 0; i < customPlot_->graphCount(); ++i)
            map[customPlot_->graph(i)] = false;         // false indicates negative
        return map;
    }();
}

bool MainWindow::CheckCurData() {
    // Check if curData is empty
    if (curData_.pointCount == 0 || curData_.x.isEmpty() || curData_.z.isEmpty()) {
        emit logMessageReceived("Warning: No local current data.");
        QMessageBox::warning(nullptr, "Data Error", "No local current data.");
        return false;     // Exit if there's no data to save
    }
    
    return true;
}

QVector<QColor> MainWindow::generateColors(int numColors) {
    QVector<QColor> colors;
    for (int i = 0; i < numColors; ++i) {
        int hue = (i * 360 / numColors) % 360;
        colors.append(QColor::fromHsv(hue, 255, 255));
    }
    return colors;
}

QVector<SensorQData> MainWindow::cachePlotData(QCustomPlot* customPlot_) {
    QVector<SensorQData> dataCache;

    // Iterate over all graphs in customPlot_ and store their data into dataCache
    for (int i = 0; i < customPlot_->graphCount(); ++i) {
        QCPGraph *graph = customPlot_->graph(i);
        QCPDataContainer<QCPGraphData> *dataContainer = graph->data().data();

        // Create SensorQData to store x and y data
        SensorQData sensorData;
        
        // Iterate over the data container to get x and y values
        for (auto it = dataContainer->constBegin(); it != dataContainer->constEnd(); ++it) {
            sensorData.x.append(it->key);   // key is the x value
            sensorData.z.append(it->value); // value is the y value
        }
        
        // Store the number of points
        sensorData.pointCount = static_cast<size_t>(sensorData.x.size());

        // Append sensorData to dataCache
        dataCache.append(sensorData);
    }

    return dataCache;
}

