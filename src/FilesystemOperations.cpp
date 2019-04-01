#include "FilesystemOperations.h"

void FilesystemOperations::splitAssets(const QString& folderPath) //Split assets between several folders
{
    QLogger::QLog_Trace("FilesystemOperations", "Entering splitAssets function");

    QDir directory(folderPath);

    QStringList bsaList;
    QStringList texturesBsaList;
    QStringList dirs(directory.entryList(QDir::Dirs));

    QStringList assets {"nif", "seq",  "pex", "psc", "lod", "fuz", "waw", "xwm", "swf", "hkx", "wav", "tri", "btr", "bto", "btt", "lip"};

    QLogger::QLog_Trace("FilesystemOperations", "Listing all BSA folders and moving files to modpath root directory");

    for (int i = 0; i < dirs.size(); ++i)
    {
        if(dirs.at(i).right(13) == "bsa.extracted" && dirs.at(i).contains("- Textures", Qt::CaseInsensitive))
        {
            texturesBsaList << directory.filePath(dirs.at(i));
            moveFiles(directory.filePath(dirs.at(i)), directory.path(), false);
        }
        else if(dirs.at(i).right(13) == "bsa.extracted")
        {
            bsaList << directory.filePath(dirs.at(i));
            moveFiles(directory.filePath(dirs.at(i)), directory.path(), false);
        }
    }

    QString espName = PluginsOperations::findPlugin(folderPath).chopped(4);

/*TODO Restore the if(bBsaSplitAsset) statement */

    QLogger::QLog_Trace("FilesystemOperations", "Creating enough folders to contain all the files");

        QPair<qint64, qint64> size = assetsSize(directory.path());

        int i = 0;
        QString bsaName;
        while(texturesBsaList.size() < qCeil(size.first/2900000000.0))
        {
            if(i == 0)
                bsaName = espName + " - Textures.bsa.extracted";
            else
                bsaName = espName + QString::number(i) + " - Textures.bsa.extracted";

            texturesBsaList << bsaName;
            texturesBsaList.removeDuplicates();
            ++i;
        }

        i = 0;
        while(bsaList.size() < qCeil(size.second/2076980377.0))
        {
            if(i == 0)
                bsaName = espName + ".bsa.extracted";
            else
                bsaName = espName + QString::number(i) + ".bsa.extracted";

            bsaList << bsaName ;
            bsaList.removeDuplicates();
            ++i;
        }

/* END OF IF STATEMENT TO RESTORE */

    //else //If assets splitting is disabled, use only one bsa folder and one textures bsa folder

        if(texturesBsaList.isEmpty())
            texturesBsaList << espName + " - Textures.bsa.extracted";
        if(bsaList.isEmpty())
            bsaList << espName + ".bsa.extracted";
    /*END OF ELSE STATEMENT TO RESTORE*/

    system(qPrintable("cd /d \"" + folderPath + R"(" && for /f "delims=" %d in ('dir /s /b /ad ^| sort /r') do rd "%d" >nul 2>nul)"));

    QLogger::QLog_Trace("FilesystemOperations", "Total: " + QString::number(bsaList.size()) + " bsa folders:\n" + bsaList.join("\n") + "\n"
                        + QString::number(texturesBsaList.size()) + " textures bsa folders:\n" + texturesBsaList.join("\n"));
    QLogger::QLog_Trace("FilesystemOperations", "Splitting files between bsa folders");

    int k = 0;
    int j = 0;

    QDirIterator it(directory, QDirIterator::Subdirectories);

    while(it.hasNext())
    {
        it.next();

        if(assets.contains(it.fileName().right(3), Qt::CaseInsensitive) || it.fileName().right(3).toLower() == "dds" || it.fileName().right(3).toLower() == "png")
        {
            QFile oldAsset;
            QFile newAsset;
            QString relativeFilename = directory.relativeFilePath(it.filePath());

            if(assets.contains(it.fileName().right(3), Qt::CaseInsensitive))
            {
                ++k;
                if(k >= bsaList.size() || k < 0)
                    k = 0;
                newAsset.setFileName(directory.filePath(bsaList.at(k) + "/" + relativeFilename));
            }

            else if(it.fileName().right(3).toLower() == "dds" || it.fileName().right(3).toLower() == "png")
            {
                ++j;
                if(j >= texturesBsaList.size() || j < 0)
                    j = 0;
                newAsset.setFileName(directory.filePath(texturesBsaList.at(j) + "/" + relativeFilename));
            }

            oldAsset.setFileName(it.filePath());

            //removing the duplicate assets and checking for path size

            QString newAssetRelativeFilename = directory.relativeFilePath(newAsset.fileName());
            QString oldAssetRelativeFilename = directory.relativeFilePath(oldAsset.fileName());

            if(newAssetRelativeFilename.size() >= 255) //Max path size for Windows
                throw tr("The filepath is more than 260 characters long. Please reduce it.");

            if(oldAsset.size() == newAsset.size())
                QFile::remove(newAsset.fileName());

            directory.mkpath(newAssetRelativeFilename.left(newAssetRelativeFilename.lastIndexOf("/")));
            directory.rename(oldAssetRelativeFilename, newAssetRelativeFilename);
        }
    }

    system(qPrintable("cd /d \"" + folderPath + R"(" && for /f "delims=" %d in ('dir /s /b /ad ^| sort /r') do rd "%d" >nul 2>nul)"));

    QLogger::QLog_Trace("FilesystemOperations", "Exiting splitAssets function");
}


void FilesystemOperations::moveFiles(const QString& source, const QString& destination, bool overwriteExisting)
{
    QDir sourceDir(source);
    QDir destinationDir(destination);
    QDirIterator it(source, QDirIterator::Subdirectories);

    QLogger::QLog_Trace("FilesystemOperations", "Entering moveFiles function");
    QLogger::QLog_Debug("FilesystemOperations", "dest folder: " + destination + "\nsource folder: " + source);

    sourceDir.mkdir(destination);

    while (it.hasNext())
    {
        it.next();
        if(it.path() != destination && it.fileName().right(1) != ".")
        {
            QString relativeFilename = QDir::cleanPath(sourceDir.relativeFilePath(it.filePath()));
            QFile oldFile(it.filePath());
            QFile newFile(destination + relativeFilename);

            QString newFileRelativeFilename = destinationDir.relativeFilePath(newFile.fileName());
            QString oldFileRelativeFilename = destinationDir.relativeFilePath(oldFile.fileName());

            if(newFileRelativeFilename.size() >= 255)
                throw tr("An error occurred while moving files. Try reducing path size (260 characters is the maximum)");

            //removing the duplicate files from new folder (if overwriteExisting) or from old folder (if !overwriteExisting)

            if(overwriteExisting)
                destinationDir.remove(newFileRelativeFilename);
            else if(!overwriteExisting)
                destinationDir.remove(oldFileRelativeFilename);

            destinationDir.mkpath(newFileRelativeFilename.left(newFileRelativeFilename.lastIndexOf("/")));
            destinationDir.rename(oldFileRelativeFilename, newFileRelativeFilename);
        }
    }
    QLogger::QLog_Trace("FilesystemOperations", "Exiting moveFiles function");
}


QPair <qint64, qint64> FilesystemOperations::assetsSize(const QString& path) // Return textures size and other assets size in a directory
{
    QPair <qint64, qint64> size;
    //First will be textures, second will be other assets

    QDirIterator it(path, QDirIterator::Subdirectories);
    QStringList assets {"nif", "seq", "pex", "psc", "lod", "fuz", "waw", "xwm", "swf", "hkx", "wav", "tri", "btr", "bto", "btt", "lip"};
    //TODO The hardcoded list "assets" is used at two different places => high risk of human error when modifying it

    while (it.hasNext())
    {
        QFile currentFile(it.next());

        if(currentFile.fileName().right(3).toLower() == "dds" || currentFile.fileName().right(3).toLower() == "png")
            size.first += currentFile.size();
        else if(assets.contains(currentFile.fileName().right(3), Qt::CaseInsensitive))
            size.second += currentFile.size();
    }
    return size;
}


QString FilesystemOperations::findSkyrimDirectory() //Find Skyrim directory using the registry key
{
    QSettings SkyrimReg(R"(HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Bethesda Softworks\Skyrim Special Edition)", QSettings::NativeFormat);
    QString SkyrimDir = QDir::cleanPath(SkyrimReg.value("Installed Path").toString());
    return SkyrimDir;
}



