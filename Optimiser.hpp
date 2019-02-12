#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QDirIterator>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSettings>
#include <QProgressBar>

#include <utility>

class Optimiser
{

public:

    Optimiser(QString mod, QPlainTextEdit* textEdit, QProgressBar* bar);

    int mainProcess();

    bool extractBsa();
    bool textOpt();
    bool nifOpt();
    bool createBsa();
    bool createTexturesBsa();
    bool renameBsa();
    bool animOpt();

    bool setUserPath(const QString& path);
    bool checkResourcesFolder(const QString& exe);

    QString getUserPath() const;
    QString findEspName();
    QString findSkyrimDir();

    void setMode(int index);

    void setBsaOptBool(bool state);
    void setExtractBsaBool(bool state);
    void setTextOptBool(bool state);
    void setNifOptBool(bool state);
    void setCreateBsaBool(bool state);
    void setRenameBsaBool(bool state);
    void setAnimOptBool(bool state);

    bool getBsaOptBool();
    bool getExtractBsaBool();
    bool getTextOptBool();
    bool getNifOptBool();
    bool getCreateBsaBool();
    bool getrenameBsaBool();
    bool getAnimOptBool();

    void saveSettings();
    void loadSettings();

private:
    QString modPath;
    QString userPath;

    QPlainTextEdit* log;
    QProgressBar* progressBar;

    bool bsaOptBool{};
    bool extractBsaBool{};
    bool textOptBool{};
    bool nifOptBool{};
    bool createBsaBool{};
    bool renameBsaBool{};
    bool animOptBool{};


    int mode{};

};


#endif // FUNCTIONS_H
