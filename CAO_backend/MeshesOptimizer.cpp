/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MeshesOptimizer.h"

MeshesOptimizer::MeshesOptimizer(bool processHeadparts, int optimizationLevel) :
    bMeshesHeadparts (processHeadparts),
    iMeshesOptimizationLevel (optimizationLevel)
{
    //Reading custom headparts file to add them to the list.
    //Done in the constructor since the file won't change at runtime.

    QFile customHeadpartsFile("resources/customHeadparts.txt");
    if(customHeadpartsFile.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&customHeadpartsFile);
        while (!ts.atEnd())
        {
            QString readLine = ts.readLine();
            if(readLine.left(1) != "#" && !readLine.isEmpty())
                headparts << readLine;
        }
    }
    else
    {
        QLogger::QLog_Warning("MeshesOptimizer", tr("No custom headparts file found. If you haven't created one, please ignore this message."));
        QLogger::QLog_Warning("Errors", tr("No custom headparts file found. If you haven't created one, please ignore this message."));
    }
}

ScanResult MeshesOptimizer::scan(const QString &filePath)
{
    NifFile nif (filePath.toStdString());
    ScanResult result;

    result = good;


    for(const auto& shape : nif.GetShapes())
    {
        if (shape->HasType<NiTriStrips>())
        {
            qDebug() << filePath << "NiTriStrip is not supported by SSE";
            if(result < criticalIssue) result = criticalIssue;
        }

        if(shape->HasType<bhkMultiSphereShape>())
        {
            qDebug() << "bhkMultiSphereShape is not supported by SSE";
            if(result < criticalIssue) result = criticalIssue;
        }
        if(shape->HasType<NiParticles>() || shape->HasType<NiParticleSystem>()
                || shape->HasType<NiParticlesData>())
        {
            qDebug() << "Is particle";
            return doNotProcess;
        }
        if(shape->HasType<BSXFlags>() && shape->GetName() != "BSX")
        {
            qDebug() << "BSXFlags node must have 'BSX' name";
            if(result < lightIssue) result = lightIssue;
        }
    }
    return  result;
}

void MeshesOptimizer::listHeadparts(const QString& directory)
{
    QProcess listHeadparts;
    listHeadparts.start("resources/ListHeadParts.exe", QStringList() << directory);

    if(!listHeadparts.waitForFinished(180000))
    {
        QLogger::QLog_Error("MeshesOptimizer", tr("ListHeadparts has not finished within 3 minutes. Skipping headparts optimization for this mod."));
        QLogger::QLog_Error("Errors", tr("ListHeadparts has not finished within 3 minutes. Skipping headparts optimization for this mod."));
    }

    while(listHeadparts.canReadLine())
    {
        QString readLine = QString::fromLocal8Bit(listHeadparts.readLine()).simplified();
        headparts << QDir::cleanPath(readLine);
    }

    headparts.removeDuplicates();
}


void MeshesOptimizer::optimize(const QString &filePath) // Optimize the selected mesh
{
    NifFile nif(filePath.toStdString());
    OptOptions options;
    options.targetVersion.SetFile(Games::getMeshesFileVersion());
    options.targetVersion.SetStream(Games::getMeshesStream());
    options.targetVersion.SetUser(Games::getMeshesUser());

    ScanResult scanResult = scan(filePath);
    QString relativeFilePath = filePath.mid(filePath.indexOf("/meshes/", Qt::CaseInsensitive) + 1);

    //Headparts have to get a special optimization
    if(iMeshesOptimizationLevel >= 1 && bMeshesHeadparts && headparts.contains(relativeFilePath, Qt::CaseInsensitive))
    {
        options.bsTriShape = true;
        options.headParts = true;
        QLogger::QLog_Note("MeshesOptimizer", tr("Running NifOpt...")  + tr("Processing: ") + filePath + tr(" as an headpart due to necessary optimization"));
        nif.OptimizeFor(options);
    }
    else
    {
        switch (scanResult)
        {
        case doNotProcess: return;
        case good:
        case lightIssue:
            if(iMeshesOptimizationLevel >= 3)
            {
                options.bsTriShape = true;
                QLogger::QLog_Note("MeshesOptimizer", tr("Running NifOpt...")  + tr("Processing: ") + filePath + tr(" due to full optimization"));
                nif.OptimizeFor(options);
            }
            else if(iMeshesOptimizationLevel >= 2)
            {
                QLogger::QLog_Note("MeshesOptimizer", tr("Running NifOpt...")  + tr("Processing: ") + filePath + tr(" due to medium optimization"));
                nif.OptimizeFor(options);
            }
            break;
        case criticalIssue:
            options.bsTriShape = true;
            QLogger::QLog_Note("MeshesOptimizer", tr("Running NifOpt...")  + tr("Processing: ") + filePath + tr(" due to necessary optimization"));
            nif.OptimizeFor(options);
            break;
        }
    }
    nif.Save(filePath.toStdString());
}


void MeshesOptimizer::dryOptimize(const QString &filePath)
{
    ScanResult scanResult = scan(filePath);
    QString relativeFilePath = filePath.right(filePath.indexOf("meshes", Qt::CaseInsensitive));

    //TODO update meshes dryOptimize

    //Headparts have to get a special optimization
    if(iMeshesOptimizationLevel >= 1 && bMeshesHeadparts && headparts.contains(relativeFilePath, Qt::CaseInsensitive))
        QLogger::QLog_Note("MeshesOptimizer", filePath + tr(" would be optimized as an headpart due to necessary optimization"));
    else
    {
        switch (scanResult)
        {
        case doNotProcess: break;
        case good:
            if(iMeshesOptimizationLevel >= 3)
                QLogger::QLog_Note("MeshesOptimizer", filePath + tr(" would be optimized due to full optimization"));
            break;

        case lightIssue:
            if(iMeshesOptimizationLevel >= 3)
                QLogger::QLog_Note("MeshesOptimizer", filePath + tr(" would be optimized due to full optimization"));
            else if(iMeshesOptimizationLevel >= 2)
                QLogger::QLog_Note("MeshesOptimizer", filePath + tr(" would be optimized due to medium optimization"));
            break;
        case criticalIssue:
            QLogger::QLog_Note("MeshesOptimizer", filePath + tr(" would be optimized due to necessary optimization"));
        }
    }
}





