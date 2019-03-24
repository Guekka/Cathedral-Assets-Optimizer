#include "Optimiser.hpp"

const QString errorColor("<font color=Red>");
const QString stepsColor("<font color=Green>");
const QString noteColor("<font color=Grey>");
const QString currentModColor("<font color=Orange>");
const QString endColor("</font>\n");

Optimiser::Optimiser(QTextEdit* textedit, QProgressBar* bar) : log(textedit), progressBar(bar), debugLogFile("debugLog.html"), debugLogStream(&debugLogFile){}


bool Optimiser::setup() //Some necessary operations before running
{
    saveSettings();
    printSettings();

    modDirs.clear();
    progressBar->reset();
    log->clear();

    //Disabling BSA process if Skyrim folder is choosed

    if(options.userPath == findSkyrimDirectory() + "/data" && (options.bBsaExtract || options.bBsaCreate))
    {
        log->append(errorColor + "You are currently in the Skyrim Dir. BSA won't be processed");
        options.bBsaExtract = false;
        options.bBsaCreate = false;
        options.bBsaPackLooseFiles = false;
        options.bBsaDeleteBackup = false;
    }

    //Checking if all the requirements are in the resources folder

    QStringList requirements;
    QFile havokFile(findSkyrimDirectory() + "/Tools/HavokBehaviorPostProcess/HavokBehaviorPostProcess.exe");

    if(havokFile.exists() && !QFile("resources/HavokBehaviorPostProcess.exe").exists())
        havokFile.copy("resources/HavokBehaviorPostProcess.exe");

    else if(!havokFile.exists() && !QFile("resources/HavokBehaviorPostProcess.exe").exists())
        log->append(errorColor + tr("Havok Tool not found. Are you sure the Creation Kit is installed ? You can also put HavokBehaviorPostProcess.exe in the resources folder") + endColor);

    requirements << "bsarch.exe" << "NifScan.exe" << "nifopt.exe" << "texconv.exe" << "texdiag.exe" << "ListHeadParts.exe";

    if(options.bAnimationsOptimization)
        requirements << "HavokBehaviorPostProcess.exe";

    for (int i = 0; i < requirements.size(); ++i)
    {
        QFile file("resources/" + requirements.at(i));
        if(!file.exists())
        {
            log->append(errorColor + requirements.at(i) + tr(" not found. Cancelling.") + endColor);
            return false;
        }
    }

    //Reading custom headparts

    QFile customHeadpartsFile("resources/customHeadparts.txt");
    QString readLine;
    if(customHeadpartsFile.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&customHeadpartsFile);
        while (!ts.atEnd())
        {
            readLine = QDir::cleanPath(ts.readLine());
            if(readLine.left(1) != "#")
                customHeadparts << readLine;
        }
    }
    else
        log->append(noteColor + "No custom headparts file found. If you haven't created one, please ignore this message." + endColor);

    //Adding all dirs to process to modDirs

    if(options.mode == 1) //Several mods mode
    {
        QDir dir(options.userPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        modDirs = dir.entryList();
    }

    if(options.mode == 0) //One mod mode
        modDirs << ""; //if modDirs is empty, the loop won't be run

    modpathDir.setPath(options.userPath + "/" + modDirs.at(0)); //In case it is ran from debug UI
    debugLogStream << stepsColor << "[SETUP FUNC]" << endColor << "Modpath: " << modpathDir.path() << "\n" << stepsColor << "[/SETUP FUNC]" << endColor;

    progressBar->setMaximum(modDirs.size()*(options.bBsaExtract + 1 + options.bBsaCreate));

    QCoreApplication::processEvents();
    return true;
}


int Optimiser::mainProcess() // Process the userPath according to all user options
{
    if(!setup())
        return 1;

    if(options.bDryRun)
    {
        dryRun();
        return 2;
    }

    int progressBarMax = progressBar->maximum();
    int progressBarValue = progressBar->value();

    //Preparing logging

    log->append(stepsColor + tr("Beginning...") + endColor);
    printSettings();

    QFile fileLog("log.html");
    QTextStream fileLogStream(&fileLog);

    fileLog.open(QFile::WriteOnly);
    debugLogFile.open(QFile::WriteOnly);

    fileLogStream << "<h1>" << QDateTime::currentDateTime().toString() << "</h1>";
    debugLogStream << "<h1>" << QDateTime::currentDateTime().toString() << "</h1>";

    //This loop applies the selected optimizations to each mod specified in ModDirs

    for(int i=0; i < modDirs.size(); ++i)
    {
        modpathDir.setPath(options.userPath + "/" + modDirs.at(i));
        QDirIterator it(modpathDir, QDirIterator::Subdirectories);

        debugLogStream << stepsColor << "[MAIN PROCESS FUNC]" << endColor << "ModDirs size: " << QString::number(modDirs.size()) << "\nCurrent index: " << QString::number(i);
        log->append(currentModColor + tr("Current mod: ") + modpathDir.path() + endColor);

        meshesList();

        if(options.bBsaExtract)
        {
            log->append(stepsColor + tr("Extracting BSA...") + endColor);
            QCoreApplication::processEvents();

            QDirIterator bsaIt(modpathDir);

            while(bsaIt.hasNext())
            {
                if(bsaIt.next().right(4) == ".bsa")
                {
                    log->append(tr("BSA found ! Extracting...(this may take a long time, do not force close the program): ") + bsaIt.fileName());
                    QCoreApplication::processEvents();
                    try
                    {
                        bsaExtract(bsaIt.filePath());
                    }
                    catch(QString e)
                    {
                        log->append(errorColor + e + endColor);
                    }
                }
            }
            progressBar->setValue(progressBar->value() + 1);
        }

        log->append(stepsColor + "Optimizing textures and meshes..." + endColor);

        while(it.hasNext())
        {
            progressBarValue = progressBar->value();
            progressBar->setMaximum(0); //This will show a busy indicator on the progress bar
            progressBar->setMinimum(0);

            it.next();
            //debugLog->append(noteColor + "Current file: " + it.filePath() + endColor);     Too much cluttering, will have to find a better way

            QCoreApplication::processEvents();

            if((options.bMeshesNecessaryOptimization || options.bMeshesMediumOptimization || options.bMeshesFullOptimization) && it.fileName().right(4).toLower() == ".nif")
            {
                meshesOptimize(&it);
                //textureCaseFixMesh(&it);    Currently not working, WIP
            }
            if((options.bTexturesFullOptimization) && it.fileName().right(4).toLower() == ".dds")
                texturesBc7Conversion(&it);

            if((options.bTexturesNecessaryOptimization) && it.fileName().right(4).toLower() == ".tga")
                texturesTgaToDds(&it);

            if(options.bAnimationsOptimization && it.fileName().right(4).toLower() == ".hkx")
                animationsOptimize(&it);
        }

        progressBar->setValue(progressBarValue + 1);
        progressBar->setMaximum(progressBarMax);

        if(options.bBsaCreate)
        {
            try {
                bsaCreate();
            } catch (QString e) {
                log->append(errorColor + e + endColor);
                fileLogStream << log->toHtml();
            }

            progressBar->setValue(progressBar->value() + 1);
        }

        QCoreApplication::processEvents();

        fileLogStream << log->toHtml();

    }

    //Deleting empty dirs

    system(QString("cd /d \"" + options.userPath + R"(" && for /f "delims=" %d in ('dir /s /b /ad ^| sort /r') do rd "%d" >nul 2>&1)").toStdString().c_str());

    progressBar->setValue(progressBar->maximum());
    log->append(stepsColor + tr("Completed. Please read the above text to check if any errors occurred (displayed in red).") + endColor);

    debugLogStream << stepsColor << "[/MAIN PROCESS FUNC]" << endColor;

    fileLogStream << log->toHtml();

    return 0;
}


void Optimiser::dryRun() // Perform a dry run : list files without actually modifying them
{
    setup();

    log->append(tr("Beginning the dry run..."));

    for(int i=0; i < modDirs.size(); ++i)
    {
        modpathDir.setPath(QDir::cleanPath(options.userPath + "/" + modDirs.at(i)));
        QDirIterator it(modpathDir, QDirIterator::Subdirectories);

        meshesList();

        log->append(currentModColor + tr("Current mod: ") + modpathDir.path() + endColor);

        if(options.bBsaExtract)
        {
            log->append(stepsColor + tr("Extracting BSA...") + endColor);

            QDirIterator bsaIt(modpathDir);

            while(bsaIt.hasNext())
            {
                if(bsaIt.next().right(4) == ".bsa")
                {
                    log->append(tr("BSA found ! Extracting...(this may take a long time, do not force close the program): ") + bsaIt.fileName());
                    try
                    {
                        bsaExtract(bsaIt.filePath());
                    }
                    catch(QString e)
                    {
                        log->append(errorColor + e + endColor);
                    }
                }
            }
            progressBar->setValue(progressBar->value() + 1);
        }

        while(it.hasNext())
        {
            it.next();

            QCoreApplication::processEvents();

            if(it.fileName().contains(".nif", Qt::CaseInsensitive))
            {
                if(options.bMeshesNecessaryOptimization && headparts.contains(it.filePath(), Qt::CaseInsensitive))
                    log->append(it.filePath() + tr(" would be optimized by Headparts meshes option"));

                else if(options.bMeshesMediumOptimization && otherMeshes.contains(it.filePath()))
                    log->append(it.filePath() + tr(" would be optimized lightly by the Other Meshes option"));

                else if(options.bMeshesNecessaryOptimization && crashingMeshes.contains(it.filePath(), Qt::CaseInsensitive))
                    log->append(it.filePath() + tr(" would be optimized in full by the Hard Crashing Meshes option.\n"));

                else if(options.bMeshesFullOptimization)
                    log->append(it.filePath() + tr(" would be optimized lightly by the Other Meshes option"));
            }
            if((options.bTexturesFullOptimization) && it.fileName().contains(".dds", Qt::CaseInsensitive))
            {
                QProcess texDiag;
                QStringList texdiagArg;

                texdiagArg << "info" << it.filePath();

                texDiag.start("resources/texdiag.exe", texdiagArg);
                texDiag.waitForFinished(-1);

                if(texDiag.readAllStandardOutput().contains("compressed = no"))
                {
                    log->append(it.filePath() + tr(" would be optimized using BC7 compression.\n"));
                }
            }

            if((options.bTexturesNecessaryOptimization) && it.fileName().contains(".tga", Qt::CaseInsensitive))
                log->append(it.filePath() + tr(" would be converted to DDS"));
        }

        progressBar->setValue(progressBar->value() + 1);

        QCoreApplication::processEvents();
    }

    progressBar->setValue(progressBar->maximum());
    log->append(stepsColor + tr("Completed.") + endColor);
}


void Optimiser::bsaExtract(const QString& bsaPath) //Extracts all BSA in modPath
{
    QProcess bsarch;
    QStringList bsarchArgs;

    QStringList files(modpathDir.entryList());

    QString bsaFolder = modpathDir.filePath(bsaPath + ".extracted");

    modpathDir.mkdir(bsaFolder);

    if(options.bBsaDeleteBackup)
        bsarchArgs << "unpack" << bsaPath << bsaFolder ;
    else
    {
        QFile bsaBackupFile(bsaPath + ".bak");
        QFile bsaFile(bsaPath);

        if(!bsaBackupFile.exists())
            QFile::rename(bsaPath, bsaBackupFile.fileName());
        else
        {
            if(bsaFile.size() == bsaBackupFile.size())
                QFile::remove(bsaBackupFile.fileName());
            else
                QFile::rename(bsaBackupFile.fileName(), bsaBackupFile.fileName() + ".bak");
        }

        QFile::rename(bsaPath, bsaBackupFile.fileName());
        bsarchArgs << "unpack" << bsaBackupFile.fileName() << bsaFolder ;
    }

    bsarch.start("resources/bsarch.exe", bsarchArgs);
    bsarch.waitForFinished(-1);

    debugLogStream << stepsColor + "[EXTRACT BSA FUNC]" + endColor + "BSArch Args :" + bsarchArgs.join(" ") + "\nBSA folder :" + bsaFolder + "\nBSA Name :" + bsaPath + "\n" + stepsColor + "[/EXTRACT BSA FUNC]" + endColor;

    if(!bsarch.readAllStandardOutput().contains("Done"))
        throw tr("An error occured during the extraction. Please extract it manually. The BSA was not deleted.");
    else
    {
        if(!options.bBsaCreate)
        {
            try
            {
                moveFiles(bsaFolder, modpathDir.path(), false);
            }
            catch (QString e)
            {
                log->append(errorColor + e + endColor);
                throw tr("An error occured during the extraction. Please extract it manually. The BSA was not deleted.");
            }
        }
        if(options.bBsaDeleteBackup)
            QFile::remove(bsaPath);
    }
}


void Optimiser::bsaCreate() //Once all the optimizations are done, create a new BSA
{
    QStringList dirs(modpathDir.entryList(QDir::Dirs));
    QStringList bsaList(modpathDir.entryList(QDir::Dirs));

    QStringList bsarchArgs;
    QString bsaName;
    QString espName = getPlugin();

    QDir bsaDir;
    QStringList bsaDirs;
    QProcess bsarch;

    bool hasSound=false;

    for (int i = 0; i < dirs.size(); ++i)
    {
        if(dirs.at(i).right(13) == "bsa.extracted" && dirs.at(i).contains("- Textures", Qt::CaseInsensitive))
            bsaList << modpathDir.filePath(dirs.at(i));
    }


    if(options.bBsaPackLooseFiles && bsaList.isEmpty())
    {
        QFile::copy("resources/BlankSSEPlugin.esp", modpathDir.path() + "/" + espName);
        bsaList << QDir::cleanPath(modpathDir.path() + "/" + espName.chopped(4) + ".bsa.extracted");
    }

    if(options.bBsaPackLooseFiles)
    {
        try
        {
            splitAssets();
        }
        catch (QString e)
        {
            log->append(errorColor + e + endColor);
            throw tr("BSA creation cancelled");
        }
    }

    //Doing twice the same list since moveAssets() can create dirs

    bsaList.clear();
    dirs = modpathDir.entryList(QDir::Dirs);

    for (int i = 0; i < dirs.size(); ++i)
    {
        if(dirs.at(i).right(13) == "bsa.extracted")
            bsaList << modpathDir.filePath(dirs.at(i));
    }

    for (int i = 0; i < bsaList.size(); ++i)
    {
        bsarchArgs.clear();
        bsaDir.setPath(bsaList.at(i));
        bsaDirs = bsaDir.entryList(QDir::Dirs);

        //Detecting if BSA will contain sounds, since compressing BSA breaks sounds. Same for strings, Wrye Bash complains

        hasSound = false;
        for (int j = 0; j < bsaDirs.size(); ++j)
        {
            if(bsaDirs.at(j).toLower() == "sound" || bsaDirs.at(j).toLower() == "music" || bsaDirs.at(j).toLower() == "strings")
                hasSound=true;
        }

        //Checking if it a textures BSA

        if(bsaDir.count() == 3 && bsaDirs.contains("TEXTURES") && !bsaList.at(i).contains("textures", Qt::CaseInsensitive))
            bsaName = bsaList.at(i).chopped(14) + " - Textures.bsa";
        else
            bsaName = bsaList.at(i).chopped(14) + ".bsa";

        bsarchArgs << "pack" << bsaList.at(i) << bsaName << "-sse" << "-share";

        if (!hasSound) //Compressing BSA breaks sounds
            bsarchArgs << "-z";

        if(!QFile(bsaName).exists())
        {
            modpathDir.rename(bsaName + "/meshes/actors/character/animations", "meshes/actors/character/animations");
            modpathDir.rename(bsaName + "/meshes/actors/character/behaviors", "meshes/actors/character/behaviors");
            bsarch.start("resources/bsarch.exe", bsarchArgs);
            bsarch.waitForFinished(-1);
        }
        else
        {
            log->append(errorColor + tr("Cannot pack existing loose files: a BSA already exists.") + endColor);
            try
            {
                moveFiles(bsaList.at(i), modpathDir.path(), false);
            }
            catch (QString e)
            {
                log->append(errorColor + e + endColor);
            }
        }

        debugLogStream << stepsColor << "[CREATE BSA FUNC]" << endColor << "BSArch Args :" << bsarchArgs.join(" ") << "\nBSA folder :" << bsaList.at(i) << "\nBsaName : " << bsaName << "\nBSAsize: " << QString::number(QFile(bsaName).size()) << "\n" << stepsColor << "[/CREATE BSA FUNC]" << endColor;

        if(bsarch.readAllStandardOutput().contains("Done"))
        {
            if(QFile(bsaName).size() < 2147483648)
            {
                log->append(tr("BSA successfully compressed: ") + bsaName + "\n");
                bsaDir.setPath(bsaList.at(i));
                bsaDir.removeRecursively();
            }
            else if(QFile(bsaName).size() < 2400000000)
                log->append(noteColor + "Warning: the BSA is nearly over its maximum size. It still should work." + endColor);

            else
            {
                log->append(errorColor + tr("The BSA was not compressed: it is over 2.2gb: ") + bsaName + endColor);
                QFile::remove(bsaName);
                if(QFile(modpathDir.path() + "/" + espName).size() == QFile("resources/BlankSSEPlugin.esp").size())
                    QFile::remove(modpathDir.filePath(espName));
                try
                {
                    moveFiles(bsaList.at(i), modpathDir.path(), false);
                }
                catch (QString e)
                {
                    log->append(errorColor + e + endColor);
                }
            }
        }
    }
    QCoreApplication::processEvents();
}



void Optimiser::texturesBc7Conversion(QDirIterator *it) //Compress uncompressed textures to BC7
{
    QProcess texDiag;
    QStringList texdiagArg;

    QProcess texconv;
    QStringList texconvArg;

    texdiagArg << "info" << it->filePath();

    texDiag.start("resources/texdiag.exe", texdiagArg);
    texDiag.waitForFinished(-1);

    QString texDiagOutput = texDiag.readAllStandardOutput();

    if(texDiagOutput.contains("compressed = no"))
    {
        QString width = texDiagOutput.mid(texDiagOutput.indexOf("width = ")+8, 4);
        QString height = texDiagOutput.mid(texDiagOutput.indexOf("height = ")+9, 4);
        int textureSize = width.split("\\n")[0].toInt() * height.split("\\n")[0].toInt();

        if(textureSize > 16)
        {
            texconvArg.clear();
            texconvArg << "-nologo" << "-y" << "-m" << "0" << "-pow2" << "-if" << "FANT" << "-f" << "BC7_UNORM" << it->filePath();

            log->append(tr("\nUncompressed texture found:\n") + it->fileName() + tr("\nCompressing..."));
            texconv.start("resources/texconv.exe",  texconvArg);
            texconv.waitForFinished(-1);
        }
    }
}


void Optimiser::texturesTgaToDds(QDirIterator* it) //Convert TGA textures to DDS
{
    log->append(stepsColor + tr("Converting TGA files...") + endColor);

    QProcess texconv;
    QStringList texconvArg;

    log->append(tr("\nTGA file found: \n") + it->fileName() + tr("\nCompressing..."));


    texconvArg.clear();
    texconvArg << "-nologo" << "-m" << "0" << "-pow2" << "-if" << "FANT" << "-f" << "R8G8B8A8_UNORM" << it->filePath();
    texconv.start("resources/texconv.exe",  texconvArg);
    texconv.waitForFinished(-1);

    QFile tga(it->filePath());
    tga.remove();

}


void Optimiser::meshesList() //Run NifScan on modPath. Detected meshes will be stored to a list, accorded to their types.
{
    log->append(stepsColor + tr("Listing meshes...") + endColor);
    QCoreApplication::processEvents();

    crashingMeshes.clear();
    otherMeshes.clear();
    headparts.clear();

    QString readLine;
    QString currentFile;

    QFile nifScan_file("resources/NifScan.exe");

    QProcess nifScan;
    QProcess listHeadparts;

    QStringList listHeadpartsArgs;
    QStringList nifscanArgs;

    //Running Nifscan and ListHeadparts to fill lists

    nifScan.setReadChannel(QProcess::StandardOutput);
    nifscanArgs << modpathDir.path() << "-fixdds";

    nifScan.start("resources/NifScan.exe", nifscanArgs);

    if(!nifScan.waitForFinished(180000))
        log->append(errorColor + "Nifscan has not finished withing 3 minutes. Skipping mesh optimization for this mod." + endColor);

    while(nifScan.canReadLine())
    {
        readLine=QString::fromLocal8Bit(nifScan.readLine());

        if(readLine.contains("meshes\\", Qt::CaseInsensitive))
        {
            currentFile = readLine.simplified();
            if(currentFile.contains("facegendata"))
                headparts << modpathDir.filePath(currentFile);
            else
                otherMeshes << modpathDir.filePath(currentFile);
        }

        else if(readLine.contains("unsupported", Qt::CaseInsensitive) || readLine.contains("not supported", Qt::CaseInsensitive))
        {
            crashingMeshes << modpathDir.filePath(currentFile);;
            otherMeshes.removeAll(modpathDir.filePath(currentFile));
        }
    }


    listHeadpartsArgs << modpathDir.path();
    listHeadparts.start("resources/ListHeadParts.exe", listHeadpartsArgs);
    listHeadparts.waitForFinished(-1);

    while(listHeadparts.canReadLine())
    {
        readLine=QString::fromLocal8Bit(listHeadparts.readLine());
        headparts << modpathDir.filePath(readLine.simplified());
    }

    //Adding custom headparts to detected headparts

    for(int i = 0; i < customHeadparts.size(); ++i)
    {
        headparts << QDir::cleanPath(modpathDir.filePath(customHeadparts.at(i)));
    }

    //Removing hard crashing meshes from other meshes list

    QStringListIterator it(crashingMeshes);

    while (it.hasNext())
    {
        otherMeshes.removeAll(it.next());
    }

    QStringListIterator it2(headparts);
    QString temp;

    while(it2.hasNext())
    {
        temp = it2.next();
        otherMeshes.removeAll(temp);
        crashingMeshes.removeAll(temp);
    }

    //Cleaning the lists

    headparts.removeDuplicates();
    otherMeshes.removeDuplicates();
    crashingMeshes.removeDuplicates();

    headparts.removeAll("");
    otherMeshes.removeAll("");
    crashingMeshes.removeAll("");
}


void Optimiser::meshesOptimize(QDirIterator *it) // Optimize the selected mesh
{
    QProcess nifOpt;
    QStringList nifOptArgs;

    if(options.bMeshesNecessaryOptimization && headparts.contains(it->filePath(), Qt::CaseInsensitive) && options.bMeshesHeadparts)
    {
        crashingMeshes.removeAll(it->filePath());
        nifOptArgs << it->filePath() << "-head" << "1" << "-bsTriShape" << "1";
        log->append(stepsColor +  tr("Running NifOpt...") + endColor + tr("Processing: ") + it->filePath() + tr(" as an headpart due to crashing meshes option"));
    }

    else if(options.bMeshesNecessaryOptimization && crashingMeshes.contains(it->filePath(), Qt::CaseInsensitive))
    {
        nifOptArgs << it->filePath() << "-head" << "0" << "-bsTriShape" << "1";
        log->append(stepsColor +  tr("Running NifOpt...") + endColor + tr("Processing: ") + it->filePath() + tr(" due to crashing meshes option"));
    }

    else if(options.bMeshesFullOptimization && otherMeshes.contains(it->filePath(), Qt::CaseInsensitive))
    {
        nifOptArgs << it->filePath() << "-head" << "0" << "-bsTriShape" << "1";
        log->append(stepsColor +  tr("Running NifOpt...") + endColor + tr("Processing: ") + it->filePath() + tr(" due to all meshes option"));
    }

    else if(options.bMeshesMediumOptimization && otherMeshes.contains(it->filePath(), Qt::CaseInsensitive))
    {
        nifOptArgs << it->filePath() << "-head" << "0" << "-bsTriShape" << "0";
        log->append(stepsColor +  tr("Running NifOpt...") + endColor + tr("Processing: ") + it->filePath() + tr(" due to other meshes option"));
    }

    else if(options.bMeshesFullOptimization)
    {
        nifOptArgs << it->filePath() << "-head" << "0" << "-bsTriShape" << "1";
        log->append(stepsColor +  tr("Running NifOpt...") + endColor + tr("Processing: ") + it->filePath() + tr(" due to all meshes option"));
    }

    nifOpt.start("resources/nifopt.exe", nifOptArgs);
    nifOpt.waitForFinished(-1);
}


void Optimiser::meshesTexturesCaseFix(QDirIterator *it) //Unused. Work in progress. Same func as NIF Texcase Fixer
{
    QFile file(it->filePath());
    QString binaryData;
    QString texturePath;
    QStringList storedTextures;
    QVector <QStringRef> matches;

    QDirIterator textures(modpathDir, QDirIterator::Subdirectories);

    while (it->hasNext())
    {
        if(it->next().right(3).toLower() == "dds")
            storedTextures << modpathDir.relativeFilePath(textures.filePath());
    }

    file.open(QFile::ReadWrite);
    binaryData = QTextCodec::codecForMib(106)->toUnicode(file.read(999999));

    qDebug() << it->filePath();

    if(binaryData.contains(".dds"))
    {
        matches = binaryData.splitRef(QRegularExpression(R"(?:[a-zA-Z]:(?:.*?))?textures(?:.*?)dds)"));
        for (const auto& match : matches)
        {
            for (const auto& tex : storedTextures)
            {
                if(match == tex)
                    break;

                else if(match.toString().toLower() == tex.toLower())
                    binaryData.replace(match.toString().toUtf8(), tex.toUtf8());

                else if(match.endsWith(tex, Qt::CaseInsensitive))
                {
                    binaryData.replace(match.toString().toUtf8(), tex.toUtf8());
                }
            }
        }
    }

    file.close();
}


void Optimiser::animationsOptimize(QDirIterator *it) //Run Bethesda Havok Tool to port the selected animation
{
    log->append(stepsColor + tr("Processing animations...") + endColor);

    QProcess havokProcess;
    QStringList havokArgs;

    log->append(tr("Current file: ") + it->filePath() + tr("\nProcessing...\n"));

    havokArgs.clear();
    havokArgs << "--platformamd64" << it->filePath() << it->filePath();
    havokProcess.start("resources/HavokBehaviorPostProcess.exe", havokArgs);

    havokProcess.waitForFinished(-1);

    if(havokProcess.readAllStandardOutput().isEmpty())
        log->append(stepsColor + tr("Animation successfully ported.") + endColor);
    else
        log->append(noteColor + tr("An error occurred during the animation porting. Maybe it is already compatible with SSE ?") + endColor);
}


QString Optimiser::findSkyrimDirectory() //Find Skyrim directory using the registry key
{
    QSettings SkyrimReg(R"(HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Bethesda Softworks\Skyrim Special Edition)", QSettings::NativeFormat);
    QDir SkyrimDir = QDir::cleanPath(SkyrimReg.value("Installed Path").toString());
    return SkyrimDir.path();
}


QString Optimiser::getPlugin() //Find esp/esl/esm name using an iterator and regex. Also creates a plugin if there isn't one.
{
    QDirIterator it(modpathDir);
    QString espName;

    while (it.hasNext())
    {
        it.next();

        if(it.fileName().contains(QRegularExpression("\\.es[plm]")))
        {
            espName=it.fileName();
            log->append(tr("Esp found: ") + espName + "\n");
            return espName;
        }
    }
    espName = modpathDir.dirName();
    return espName;
}


void Optimiser::splitAssets() //Split assets between several folders
{
    QStringList assets;
    QString relativeFilename;

    QStringList bsaList;
    QStringList texturesBsaList;
    QStringList dirs(modpathDir.entryList(QDir::Dirs));

    QDirIterator it(modpathDir, QDirIterator::Subdirectories);

    QFile oldAsset;
    QFile newAsset;

    assets << "nif" << "seq" << "pex" << "psc" << "lod" << "fuz" << "waw" << "xwm" << "swf" << "hkx" << "wav" << "tri" << "btr" << "bto" << "btt" << "lip";

    //listing all BSA and moving files to modpath root directory

    for (int i = 0; i < dirs.size(); ++i)
    {
        if(dirs.at(i).right(13) == "bsa.extracted" && dirs.at(i).contains("- Textures", Qt::CaseInsensitive))
        {
            texturesBsaList << modpathDir.filePath(dirs.at(i));
            moveFiles(modpathDir.filePath(dirs.at(i)), modpathDir.path(), false);
        }
        else if(dirs.at(i).right(13) == "bsa.extracted")
        {
            bsaList << modpathDir.filePath(dirs.at(i));
            moveFiles(modpathDir.filePath(dirs.at(i)), modpathDir.path(), false);
        }
    }

    if(texturesBsaList.isEmpty())
        texturesBsaList << bsaList.at(0).chopped(13) + "- Textures.bsa.extracted";

    int i = 0;
    int j = 0;

    while(it.hasNext())
    {
        it.next();

        if(assets.contains(it.fileName().right(3), Qt::CaseInsensitive) || it.fileName().right(3).toLower() == "dds" || it.fileName().right(3).toLower() == "png")
        {
            if(assets.contains(it.fileName().right(3), Qt::CaseInsensitive))
            {
                ++i;
                if(i >= bsaList.size() || i < 0)
                    i = 0;
                newAsset.setFileName(modpathDir.filePath(bsaList.at(i) + "/" + relativeFilename));
            }

            else if(it.fileName().right(3).toLower() == "dds" || it.fileName().right(3).toLower() == "png")
            {
                ++j;
                if(j >= texturesBsaList.size() || j < 0)
                    j = 0;
                newAsset.setFileName(modpathDir.filePath(texturesBsaList.at(j) + "/" + relativeFilename));
            }

            oldAsset.setFileName(it.filePath());
            relativeFilename = modpathDir.relativeFilePath(it.filePath());


            //removing the duplicate assets and checking for path size

            if(newAsset.fileName().size() >= 260) //Max path size for Windows
                throw  tr("The filepath is more than 260 characters long. Please reduce it.");

            else if(oldAsset.size() == newAsset.size())
                QFile::remove(newAsset.fileName());

            modpathDir.mkpath(newAsset.fileName().left(newAsset.fileName().lastIndexOf("/")));
            modpathDir.rename(oldAsset.fileName(), newAsset.fileName());
        }
    }
}


void Optimiser::moveFiles(QString source, QString destination, bool overwriteExisting)
{
    QString relativeFilename;

    QDir sourceDir(source);
    QDirIterator it(source, QDirIterator::Subdirectories);

    QFile oldFile;
    QFile newFile;

    source = QDir::cleanPath(source) + "/";
    destination = QDir::cleanPath(destination) + "/";

    debugLogStream << stepsColor << "[MOVE FILES FUNC]" << endColor << "dest folder: " << destination << "\nsource folder: " << source;

    sourceDir.mkdir(destination);

    while (it.hasNext())
    {
        it.next();
        if(it.path() != destination)
        {
            relativeFilename = sourceDir.relativeFilePath(it.filePath());
            oldFile.setFileName(it.filePath());
            newFile.setFileName(destination + relativeFilename);

            //removing the duplicate files from new folder (if overwriteExisting) or from old folder (if !overwriteExisting)

            if(newFile.fileName().size() >= 260)
                throw  tr("An error occurred while moving files. Try reducing path size (260 characters is the maximum)");

            else if(oldFile.size() == newFile.size() && overwriteExisting)
                QFile::remove(newFile.fileName());
            else if(oldFile.size() == newFile.size() && !overwriteExisting)
                QFile::remove(oldFile.fileName());

            sourceDir.mkpath(newFile.fileName().left(newFile.fileName().lastIndexOf("/")));
            sourceDir.rename(oldFile.fileName(), newFile.fileName());
        }
        QCoreApplication::processEvents();
    }
    debugLogStream << stepsColor << "[/MOVE FILES FUNC]" << endColor;
}


void Optimiser::saveSettings() //Saves settings to an ini file
{
    QSettings settings("SSE Assets Optimiser.ini", QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "SSE Assets Optimiser.ini");


    settings.setValue("options.mode", options.mode);
    settings.setValue("DryRun", options.bDryRun);
    settings.setValue("SelectedPath", options.userPath);

    settings.setValue("bBsaExtract", options.bBsaExtract);
    settings.setValue("bBsaCreate", options.bBsaCreate);
    settings.setValue("bBsaPackLooseFiles", options.bBsaPackLooseFiles);
    settings.setValue("bBsaDeleteBackup", options.bBsaDeleteBackup);

    settings.setValue("bMeshesNecessaryOptimization", options.bMeshesNecessaryOptimization);
    settings.setValue("bMeshesMediumOptimization", options.bMeshesMediumOptimization);
    settings.setValue("bMeshesFullOptimization", options.bMeshesFullOptimization);

    settings.setValue("bTexturesNecessaryOptimization", options.bTexturesNecessaryOptimization);
    settings.setValue("bTexturesFullOptimization", options.bTexturesFullOptimization);

    settings.setValue("bAnimationsOptimization", options.bAnimationsOptimization);
}


void Optimiser::loadSettings() //Loads settings from the ini file
{
    QSettings settings("SSE Assets Optimiser.ini", QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "SSE Assets Optimiser.ini");

    options.mode = settings.value("options.mode").toInt();
    options.bDryRun = settings.value("DryRun").toBool();
    options.userPath = settings.value("SelectedPath").toString();

    options.bBsaExtract = settings.value("bBsaExtract").toBool();
    options.bBsaCreate = settings.value("bBsaCreate").toBool();
    options.bBsaPackLooseFiles = settings.value("bBsaPackLooseFiles").toBool();
    options.bBsaDeleteBackup = settings.value("bBsaDeleteBackup").toBool();

    options.bMeshesNecessaryOptimization = settings.value("bMeshesNecessaryOptimization").toBool();
    options.bMeshesMediumOptimization = settings.value("bMeshesMediumOptimization").toBool();
    options.bMeshesFullOptimization = settings.value("bMeshesFullOptimization").toBool();

    options.bTexturesNecessaryOptimization = settings.value("bTexturesNecessaryOptimization").toBool();
    options.bTexturesFullOptimization = settings.value("bTexturesFullOptimization").toBool();

    options.bAnimationsOptimization = settings.value("bAnimationsOptimization").toBool();
}


void Optimiser::resetToDefaultSettings() //Reset to default (recommended) settings
{
    options.mode = 0;
    options.userPath = "";

    options.bBsaExtract = true;
    options.bBsaCreate = true;
    options.bBsaPackLooseFiles = false;
    options.bBsaDeleteBackup = false;

    options.bMeshesNecessaryOptimization = true;
    options.bMeshesMediumOptimization = false;
    options.bMeshesFullOptimization = false;

    options.bTexturesNecessaryOptimization = true;
    options.bTexturesFullOptimization = true;

    options.bAnimationsOptimization = true;

    options.bDryRun = false;
}


void Optimiser::printSettings() //Will print settings into debug log
{
    QSettings settings("SSE Assets Optimiser.ini", QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "SSE Assets Optimiser.ini");

    debugLog->append("mode: " + QString::number(options.mode));
    debugLog->append("DryRun: " + QString::number(options.bDryRun));
    debugLog->append("userPath: "+ options.userPath);

    debugLog->append("bBsaExtract: "+ QString::number(options.bBsaExtract));
    debugLog->append("bBsaCreate: " + QString::number(options.bBsaCreate));
    debugLog->append("bBsaPackLooseFiles: " + QString::number(options.bBsaPackLooseFiles));
    debugLog->append("bBsaDeleteBackup: " + QString::number(options.bBsaDeleteBackup));

    debugLog->append("bMeshesNecessaryOptimization: "+ QString::number(options.bMeshesNecessaryOptimization));
    debugLog->append("bMeshesMediumOptimization: "+ QString::number(options.bMeshesMediumOptimization));
    debugLog->append("bMeshesFullOptimization: " + QString::number(options.bMeshesFullOptimization));

    debugLog->append("bTexturesNecessaryOptimization: "+ QString::number( options.bTexturesNecessaryOptimization));
    debugLog->append("bTexturesFullOptimization: " + QString::number( options.bTexturesFullOptimization));

    debugLog->append("bAnimationsOptimization: " + QString::number(options.bAnimationsOptimization));
}


void Optimiser::setDebugLog(QTextEdit *log) //Setter for debugLog
{
    debugLog = log;
}
