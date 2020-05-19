/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BSACreate.hpp"
#include "Commands/BSA/Utils/BSACallback.hpp"

namespace CAO {
CommandResult BSACreate::process(File &file)
{
    auto bsaFolder = dynamic_cast<const BSAFolderResource *>(&file.getFile());
    if (!bsaFolder)
        return _resultFactory.getCannotCastFileResult();

    auto bsas = BSASplit::splitBSA(*bsaFolder, file.generalSettings());

    for (auto &bsa : bsas)
    {
        bsa.name(bsaFolder->path(), file.generalSettings());

        //Checking if a bsa already exists
        if (QFile(bsa.path).exists())
            return _resultFactory.getFailedResult(1, "Failed to create BSA: a BSA already exists.");

        libbsarch::bs_archive_auto archive(bsa.format);
        archive.set_share_data(true);
        archive.set_compressed(file.generalSettings().bBSACompressArchive());
        const libbsarch::convertible_string &rootPath = bsaFolder->path();
        archive.set_dds_callback(&BSACallback, rootPath);

        try
        {
            //Detecting if BSA will contain sounds, since compressing BSA breaks sounds. Same for strings, Wrye Bash complains
            //Then creating the archive
            for (const auto &fileInBSA : bsa.files)
            {
                if (!canBeCompressedFile(fileInBSA))
                    archive.set_compressed(false);

                archive.add_file_from_disk(libbsarch::disk_blob(rootPath, fileInBSA));
            }
            archive.save_to_disk(libbsarch::convertible_string(bsa.path));
        }
        catch (const std::exception &e)
        {
            return _resultFactory.getFailedResult(3, e.what());
        }

        bsa.files >>= pipes::for_each([](const QString &filePath) { QFile::remove(filePath); });
    }
    return _resultFactory.getSuccessfulResult();
}

bool BSACreate::isApplicable(File &file)
{
    auto bsaFolder = dynamic_cast<const BSAFolderResource *>(&file.getFile());
    if (!bsaFolder)
        return false;

    return true;
}

bool BSACreate::canBeCompressedFile(const QString &filename)
{
    const bool cantBeCompressed = (filename.endsWith(".wav", Qt::CaseInsensitive)
                                   || filename.endsWith(".xwm", Qt::CaseInsensitive)
                                   || filename.contains(QRegularExpression("^.+\\.[^.]*strings$")));
    return !cantBeCompressed;
}

} // namespace CAO