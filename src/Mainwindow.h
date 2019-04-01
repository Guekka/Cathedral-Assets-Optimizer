#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MainOptimizer.h"
#include "pch.h"
#include "QLogger.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_DECLARE_TR_FUNCTIONS(MainWindow)

public:
    MainWindow();
    ~MainWindow();

private:
    QFileDialog *fileDialog{};
    Ui::MainWindow *ui;
    MainOptimizer *optimizer;

    bool bDarkMode = true;
    bool bLockVariables = false;

    void saveUIToFile();
    void loadUIFromFile();
    void saveSettings();
    void loadSettings();
    void updateLog();

    int progressBarValue;

    QThread* workerThread;

    QSettings *settings;
};

#endif // MAINWINDOW_H
