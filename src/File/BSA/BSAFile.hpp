/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "File/File.hpp"

namespace CAO {
class BSAFile : public File
{
public:
    int loadFromDisk(const QString &filePath) override;
    int saveToDisk(const QString &filePath) override;

    //Required or BSAFile will be an abstract class
    //This makes me think more and more that BSAFile is not a File and should not derive from it
    int loadFromMemory(const void *pSource, size_t size, const QString &fileName) override;
    int saveToMemory(std::vector<std::byte> &out) override;

    bool setFile(Resource &&file, bool optimizedFile = false) override;

    CommandType type() const override { return CommandType::BSAFile; }
};
} // namespace CAO
