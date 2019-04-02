#include "PluginsOperations.h"

void PluginsOperations::makeDummyPlugins(const QString& folderPath)
{
    QLogger::QLog_Trace("PluginsOperations", "Entering makeDummyPluginsfunction: creating enough dummy plugins to load BSAs");

    QDirIterator it(folderPath);

    while (it.hasNext())
    {
        QString espName;

        it.next();

        if(it.fileName().right(4).toLower() == ".bsa" && it.fileName().contains("- Textures", Qt::CaseInsensitive))
        {
            espName = it.fileName().remove("- Textures.bsa") + ".esp";
            QFile::copy(QCoreApplication::applicationDirPath() + "/resources/BlankSSEPlugin.esp", folderPath + "/" + espName);
            QLogger::QLog_Trace("PluginsOperations", "Created textures bsa plugin:" + espName);
        }
        else if(it.fileName().right(4).toLower() == ".bsa")
        {
            espName = it.fileName().remove(".bsa") + ".esp";
            QFile::copy(QCoreApplication::applicationDirPath() + "/resources/BlankSSEPlugin.esp", folderPath + "/" + espName);
            QLogger::QLog_Trace("PluginsOperations", "Created standard bsa plugin:" + espName);
        }
    }
    QLogger::QLog_Trace("PluginsOperations", "Exiting makeDummyPlugins function");
}


QString PluginsOperations::findPlugin(const QString& folderPath) //Find esp/esl/esm name using an iterator and regex. Also creates a plugin if there isn't one.
{
    QDirIterator it(folderPath);
    QString espName;

    while (it.hasNext())
    {
        it.next();

        if(it.fileName().contains(QRegularExpression("\\.es[plm]")))
        {
            espName=it.fileName();
            QLogger::QLog_Note("PluginsOperations", tr("Esp found: ") + espName);
            return espName;
        }
    }
    espName = QDir(folderPath).dirName();
    return espName;
}

