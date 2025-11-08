#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::string csv = "C:/Users/jacob/OneDrive/Documents/untitled2/homes_gen_50states_realcities_with_price.csv"; //im using absolute pathing since i don't think a TA will be executing this file.
    index.loadCSV(csv, true);
    QStringList ListofStates = addStatesToList(index);
    //reference for the "combo box" (dropdown) https://doc.qt.io/qt-6/qcombobox.html
    ui->StateInput->addItems(ListofStates);
    //automatically populate it with arkansaw aswell AT FIRST.

    QStringList firstCityList2 = {};

    std::vector<std::string> FirstCityList = index.getCities("AK");
    for (std::string v: FirstCityList) {
        firstCityList2.append(QString::fromStdString(v));
    }
    ui->CityInput->addItems(firstCityList2);
    ui->RBTreeMode->setChecked(true);

    //reference for push button: https://doc.qt.io/qt-6/qpushbutton.html
    //reference for connect interactivity: https://doc.qt.io/qt-6/signalsandslots.html
    //reference for connect, QLineEdit. https://doc.qt.io/qt-6/qlineedit.html
    //reference for QString documentation: https://doc.qt.io/qt-6/qstring.html
    connect(ui->MaxHeapMode, &QRadioButton::clicked, this, &MainWindow::SwitchModeToMaxHeap);
    connect(ui->RBTreeMode, &QRadioButton::clicked, this, &MainWindow::SwitchModeToRedBlack);
    connect(ui->StartSearchButton, &QPushButton::clicked, this, &MainWindow::startSearch);
    connect(ui->StateInput, &QComboBox::currentTextChanged, this, &MainWindow::StateInputUpdate);
    connect(ui->DistanceFromCityCenterInput, &QLineEdit::textChanged, this, &MainWindow::UpdateRangeInput);
}

void MainWindow::SwitchModeToMaxHeap() {
    Mode = 1;
}

void MainWindow::SwitchModeToRedBlack() {
    Mode = 0;
}


void MainWindow::startSearch() {
    QString stateInputValue = ui->StateInput->currentText();
    qDebug() << "Selected state:" << stateInputValue;
    QString cityInputValue = ui->CityInput->currentText();
    qDebug() << "Select city: " << cityInputValue;
    qDebug() << "Range: " << Range;
    if (Mode == 1) {
        qDebug() << "Max Heap Mode Activated";
    } else {
        qDebug() << "Red Black Tree Mode Activated";
    }
    AddItemsToListWidget(cityInputValue.toStdString(), stateInputValue.toStdString());
}

void MainWindow::AddItemsToListWidget(string city, string State) {

    //Reference: Time taken to execute a certain function (compute runtime/efficency)
    //https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c

    //remove quotes from the two:

    //do function logic

    //First, use the algorithm to find the list of points.
    ui->HouseListDisplay->clear();
    CenterLL checkCenter;
    index.getCenter(State, city, checkCenter);
    vector<PointLL> pts = index.queryKmFast(State, city, Range);
    if (!pts.empty()) {
    } else {
        qDebug() << "pts is empty!";
        return;
    }
    std::vector<PointLL> sortedPoints;
    //now, insert the points into the RB tree, then remove then, then return an already sorted vector based on cheapest price!
    auto t1 = std::chrono::high_resolution_clock::now();
    if (Mode == 0) {//RB Tree Mode.
        for (int i = 0; i < pts.size();i++) {
            redblack.insert(pts[i]);
        }
        sortedPoints = redblack[pts.size()];
        redblack.clear();
    } else { //obviously, mode = 1 (Max_Heap mode)
        for (int i = 0; i < pts.size();i++) {
            maxheap.push(pts[i]);
        }
        for (int i = 0; i < pts.size();i++) {
            sortedPoints.push_back(maxheap.pop());
        }
    }
    for (int i = 0; i < sortedPoints.size();i++) {
    std::ostringstream output;
    double latoutput = sortedPoints[i].lat;
    double lonoutput = sortedPoints[i].lon;
    double priceoutput = sortedPoints[i].price;
    //https://cplusplus.com/reference/iomanip/setprecision/
    output << std::fixed << std::setprecision(14);
    output << "Lat: " << latoutput << "  " << "Lon: " << lonoutput << "  ";
    output << std::fixed << std::setprecision(2) << "Price: " << priceoutput;
    string result = output.str();
    QString result2 = QString::fromStdString(result);
    ui->HouseListDisplay->addItem(result2);
    }
    qDebug() << sortedPoints.size();
    //end of function logic

    auto t2 = std::chrono::high_resolution_clock::now();
    //Reference for converting from runtime to string https://stackoverflow.com/questions/42866524/convert-chronoduration-to-string-or-c-string
    ostringstream out;
    auto runtime = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1);
    out << runtime.count() << "us";
    QString runtimeQ = QString::fromStdString(out.str());
    //Reference: https://doc.qt.io/qt-6/qlabel.html for changing the labels
    ui->RuntimeLabel->setText(runtimeQ);
    //END OF MASSIVE FUNCTION.
    return;
}

void MainWindow::StateInputUpdate(QString other) {
    std::string abletoProcess = other.toStdString();
    vector<string> cities = index.getCities(abletoProcess);
    QStringList returnList = {};
    for (std::string v: cities) {
        returnList.append(QString::fromStdString(v));
    }
    ui->CityInput->clear();
    ui->CityInput->addItems(returnList);
    return;
}

void MainWindow::UpdateRangeInput(QString other) {
    bool checkSuccess = false;
    double temp = other.toDouble(&checkSuccess);
    if (checkSuccess == true) {
        Range = temp;
    } else {
        qDebug() << "range change unsuccessful";
    }
    return;
}

QStringList MainWindow::addStatesToList(SpatialIndex& other) {
    vector<string> states = other.getStates();
    QStringList returnList = {};
    for (std::string v: states) {
        returnList.append(QString::fromStdString(v));
    }
    return returnList;
}

MainWindow::~MainWindow()
{
    delete ui;
}
