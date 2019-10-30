/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "AnimationsOptimizer.hpp"
#include "BsaOptimizer.hpp"
#include "MeshesOptimizer.hpp"
#include "OptionsCAO.hpp"
#include "TextureFile.hpp"
#include "TexturesOptimizer.hpp"

/*!
 * \brief Coordinates all the subclasses in order to optimize BSAs, textures, meshes and animations
 */
class MainOptimizer final : public QObject
{
    Q_DECLARE_TR_FUNCTIONS(MainOptimizer)

public:
    explicit MainOptimizer(const OptionsCAO &optOptions);

    void process(const QString &file);
    void packBsa(const QString &folder);

private:
    void addLandscapeTextures();
    void addHeadparts();

    void processBsa(const QString &file) const;
    void processNif(const QString &file);
    void processTexture(const QString &file, const TextureFile::TextureType &type);
    void processHkx(const QString &file);

    OptionsCAO _optOptions;

    BsaOptimizer _bsaOpt;
    MeshesOptimizer _meshesOpt;
    AnimationsOptimizer _animOpt;
    TexturesOptimizer _texturesOpt;
};
