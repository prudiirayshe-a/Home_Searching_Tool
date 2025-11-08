#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "index.h"
#include <QApplication>
#include "max_heap.h"
#include <chrono>
#include "RedBlack.h"
#include "RedBlack.cpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QStringList addStatesToList(SpatialIndex& other);
    void AddItemsToListWidget(string city, string State);

private slots:
    void startSearch();
    void StateInputUpdate(QString other);
    void UpdateRangeInput(QString other);
    void SwitchModeToMaxHeap();
    void SwitchModeToRedBlack();

private:
    SpatialIndex index;
    Ui::MainWindow *ui;
    bool Mode = 0; //Basically, 0 is the RB Tree mode, 1 is the max_heap mode.
    double Range = 0;
    red_b<PointLL> redblack;
    max_heap<PointLL> maxheap;
};
#endif // MAINWINDOW_H
