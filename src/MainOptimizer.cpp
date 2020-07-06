/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainOptimizer.hpp"
#include "Commands/Plugins/PluginsOperations.hpp"
#include "Settings/Profiles.hpp"

namespace CAO {
MainOptimizer::MainOptimizer() {}

void MainOptimizer::process(File &file)
{
    auto type = file.type();
    try
    {
        if (!loadFile(file))
            return;

        for (auto command : _commandBook.getCommandList(type))
            if (!runCommand(command, file))
                return;

        if (!file.optimizedCurrentFile())
            return;

        if (!saveFile(file))
            return;
    }
    catch (const std::exception &e)
    {
        PLOG_ERROR << "An exception was caught while processing '" << file.getName() << "'\n"
                   << "Error message was: '" << e.what() << "'\n"
                   << "You can probably assume this file is broken";
        return;
    }

    PLOG_INFO << "Successfully optimized " << file.getName();
}

void MainOptimizer::extractBSA(File &file)
{
    if (currentProfile().getGeneralSettings().bDryRun())
        return; //TODO if "dry run" run dry run on the assets in the BSA

    if (!loadFile(file))
        return;

    PLOG_INFO << "Extracting BSA: " + file.getName();
    auto command = _commandBook.getCommand<BSAExtract>();
    if (!runCommand(command, file))
        return;

    //TODO if(settings.bBsaOptimizeAssets)
}

void MainOptimizer::packBsa(const QString &folder)
{
    PLOG_INFO << "Creating BSA...";
    BSAFolder bsa;
    bsa.setName(folder);

    if (!loadFile(bsa))
        return;

    auto command = _commandBook.getCommand<BSACreate>();
    if (!runCommand(command, bsa))
        return;

    if (!saveFile(bsa))
        return;

    if (currentProfile().getGeneralSettings().bBSACreateDummies())
        PluginsOperations::makeDummyPlugins(folder, currentProfile().getGeneralSettings());
}

bool MainOptimizer::runCommand(CommandPtr command, File &file)
{
    const auto &result = command->processIfApplicable(file);
    if (result.processedFile)
    {
        PLOG_VERBOSE << QString("Successfully applied module '%1' while processing '%2'")
                            .arg(command->name(), file.getName());
        return true;
    }
    else if (result.errorCode)
    {
        PLOG_ERROR << QString("An error happened in module '%1' while processing '%2': '%3'")
                          .arg(command->name(), file.getName(), result.errorMessage);
        return false;
    }
    else
    {
        PLOG_VERBOSE << QString("Module '%1' was not applied because it was not necessary").arg(command->name());
        return true;
    }
}

bool MainOptimizer::loadFile(File &file)
{
    if (file.loadFromDisk())
    {
        PLOG_ERROR << "Cannot load file from disk: " << file.getName();
        return false;
    }
    return true;
}

bool MainOptimizer::saveFile(File &file)
{
    if (file.saveToDisk())
    {
        PLOG_ERROR << "Cannot save file to disk: " << file.getName();
        return false;
    }
    return true;
}

} // namespace CAO
