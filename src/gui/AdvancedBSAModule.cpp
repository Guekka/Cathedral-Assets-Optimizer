/* Copyright (C) 2020 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AdvancedBSAModule.hpp"

#include "settings/per_file_settings.hpp"
#include "ui_AdvancedBSAModule.h"
#include "utils/utils.hpp"

#include <QButtonGroup>

namespace cao {
AdvancedBSAModule::AdvancedBSAModule(QWidget *parent)
    : IWindowModule(parent)
    , ui_(std::make_unique<Ui::AdvancedBSAModule>())
{
    ui_->setupUi(this);

    connectGroupBox(ui_->createTexturesBSAGroupBox, ui_->maxTexturesSize);
}

AdvancedBSAModule::~AdvancedBSAModule() = default;

void AdvancedBSAModule::set_ui_data(const cao::Settings &settings)
{
    /* TODO
    ui_->BSAExtract->setChecked(!gSets.bsa_create());
    ui_->BSACreate->setChecked(gSets.bsa_create());
    ui_->BSAProcessContent->setChecked(gSets.bsa_process_content());
    */
}

void AdvancedBSAModule::ui_to_settings(Settings &settings) const
{
    // TODO
}

auto AdvancedBSAModule::is_supported_game(btu::Game game) const noexcept -> bool
{
    switch (game)
    {
        case btu::Game::TES3:
        case btu::Game::TES4:
        case btu::Game::SLE:
        case btu::Game::SSE:

        case btu::Game::FNV:
        case btu::Game::FO4: return true;
        case btu::Game::Custom: return false;
    }
    return false;
}

auto AdvancedBSAModule::name() const noexcept -> QString
{
    return QObject::tr("BSA (General)");
}

} // namespace cao
