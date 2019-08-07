/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MeshesOptimizer.h"

MeshesOptimizer::MeshesOptimizer(bool processHeadparts, int optimizationLevel, bool resaveMeshes)
    : bMeshesHeadparts(processHeadparts)
    , bMeshesResave(resaveMeshes)
    , iMeshesOptimizationLevel(optimizationLevel)
{
    //Reading custom headparts file to add them to the list.
    //Done in the constructor since the file won't change at runtime.

    QFile customHeadpartsFile("resources/customHeadparts.txt");
    if (customHeadpartsFile.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&customHeadpartsFile);
        while (!ts.atEnd())
        {
            QString readLine = ts.readLine();
            if (readLine.left(1) != "#" && !readLine.isEmpty())
                headparts << readLine;
        }
    }
    else
        PLOG_ERROR << tr("No custom headparts file found. Vanilla headparts won't be detected.");
}

ScanResult MeshesOptimizer::scan(const QString &filePath) const
{
    NifFile nif;
    if (nif.Load(filePath.toStdString()) != 0)
    {
        PLOG_ERROR << tr("Cannot load mesh: ") + filePath;
        return doNotProcess;
    }
    ScanResult result = good;

    NiVersion version;
    version.SetFile(Games::meshesFileVersion());
    version.SetStream(Games::meshesStream());
    version.SetUser(Games::meshesUser());

    if (version.IsSSE())
    {
        for (const auto &shape : nif.GetShapes())
        {
            const bool needsOpt = shape->HasType<bhkMultiSphereShape>() || shape->HasType<NiTriStrips>()
                                  || shape->HasType<NiTriStripsData>() || shape->HasType<NiSkinPartition>();
            if (needsOpt)
                if (result < criticalIssue)
                    result = criticalIssue;

            if (shape->HasType<NiParticles>() || shape->HasType<NiParticleSystem>() || shape->HasType<NiParticlesData>())
                return doNotProcess;
        }
    }
    else if (version.IsSK())
        result = criticalIssue;
    else
        result = doNotProcess;

    return result;
}

void MeshesOptimizer::listHeadparts(const QString &directory)
{
    QDirIterator it(directory, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        it.next();
        if (it.fileName().contains(QRegularExpression("\\.es[plm]$")))
            headparts += PluginsOperations::listHeadparts(it.filePath());
    }

    headparts.removeDuplicates();
}

void MeshesOptimizer::optimize(const QString &filePath)
// Optimize the selected mesh
{
    NifFile nif;
    if (nif.Load(filePath.toStdString()) != 0)
    {
        PLOG_ERROR << tr("Cannot load mesh: ") + filePath;
        return;
    }
    PLOG_VERBOSE << tr("Loading mesh: ") << filePath;

    OptOptions options;
    options.targetVersion.SetFile(Games::meshesFileVersion());
    options.targetVersion.SetStream(Games::meshesStream());
    options.targetVersion.SetUser(Games::meshesUser());

    const ScanResult scanResult = scan(filePath);
    const QString relativeFilePath = filePath.mid(filePath.indexOf("/meshes/", Qt::CaseInsensitive) + 1);

    //Headparts have to get a special optimization
    if (iMeshesOptimizationLevel >= 1 && bMeshesHeadparts && headparts.contains(relativeFilePath, Qt::CaseInsensitive))
    {
        options.bsTriShape = true;
        options.headParts = true;
        PLOG_INFO << tr("Running NifOpt...") + tr("Processing: ") + filePath
                         + tr(" as an headpart due to necessary optimization");
        nif.OptimizeFor(options);
    }
    else
    {
        switch (scanResult)
        {
        case doNotProcess: return;
        case good:
        case lightIssue:
            if (iMeshesOptimizationLevel >= 3)
            {
                options.bsTriShape = true;
                PLOG_INFO << tr("Running NifOpt...") + tr("Processing: ") + filePath + tr(" due to full optimization");
                nif.OptimizeFor(options);
            }
            else if (iMeshesOptimizationLevel >= 2)
            {
                PLOG_INFO << tr("Running NifOpt...") + tr("Processing: ") + filePath
                                 + tr(" due to medium optimization");
                nif.OptimizeFor(options);
            }
            break;
        case criticalIssue:
            if (iMeshesOptimizationLevel >= 1)
            {
                options.bsTriShape = true;
                PLOG_INFO << tr("Running NifOpt...") + tr("Processing: ") + filePath
                                 + tr(" due to necessary optimization");
                nif.OptimizeFor(options);
            }
            break;
        }
    }
    if (bMeshesResave || (iMeshesOptimizationLevel >= 1 && scanResult >= lightIssue))
    {
        PLOG_VERBOSE << "Resaving mesh: " + filePath;
        nif.Save(filePath.toStdString());
    }
}

void MeshesOptimizer::dryOptimize(const QString &filePath) const
{
    const ScanResult scanResult = scan(filePath);
    const QString relativeFilePath = filePath.mid(filePath.indexOf("/meshes/", Qt::CaseInsensitive) + 1);

    //Headparts have to get a special optimization
    if (iMeshesOptimizationLevel >= 1 && bMeshesHeadparts && headparts.contains(relativeFilePath, Qt::CaseInsensitive))
        PLOG_INFO << tr("Running NifOpt...") + tr("Processing: ") + filePath
                         + tr(" as an headpart due to necessary optimization");
    else
    {
        switch (scanResult)
        {
        case doNotProcess: return;
        case good:
        case lightIssue:
            if (iMeshesOptimizationLevel >= 3)
                PLOG_INFO << filePath + tr(" would be optimized due to full optimization");

            else if (iMeshesOptimizationLevel >= 2)
            {
                PLOG_INFO << filePath + tr(" would be optimized due to medium optimization");
            }
            break;
        case criticalIssue: PLOG_INFO << filePath + tr(" would be optimized due to necessary optimization"); break;
        }
    }
}