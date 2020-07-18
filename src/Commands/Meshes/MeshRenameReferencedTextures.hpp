/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "Commands/CommandBook.hpp"
#include "File/Meshes/MeshFile.hpp"

namespace CAO {
class MeshRenameReferencedTextures : public Command
{
public:
    QString name() const override { return "Rename referenced textures in Mesh"; }
    CommandType type() const override { return CommandType::Mesh; };
    Priority priority() const override { return Low; };

    CommandResult process(File& file) const override;
    bool isApplicable(File &file) const override;
};
REGISTER_COMMAND(MeshRenameReferencedTextures)
} // namespace CAO
