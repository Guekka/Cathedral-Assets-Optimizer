/* Copyright (C) 2020 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LevelSelector.hpp"
#include "GUI/QuickAutoPortWindow.hpp"
#include "MainWindow.hpp"

namespace CAO {
LevelSelector::LevelSelector(MainWindow &mw)
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
    , ui_(new Ui::LevelSelector)
{
    ui_->setupUi(this);

    GuiMode curMode = toGuiMode(ui_->levelSlider->value());

    ui_->label->setText(getHelpText(curMode));

    connect(ui_->levelSlider, &QSlider::valueChanged, this, [this](int val) {
        GuiMode mode = toGuiMode(val);
        ui_->label->setText(getHelpText(mode));
    });

    connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, [&mw, this] {
        GuiMode mode = toGuiMode(ui_->levelSlider->value());
        setupWindow(mw, mode);
    });
}

void LevelSelector::setupWindow(MainWindow &mw, GuiMode level)
{
    switch (level)
    {
        case GuiMode::QuickAutoPort:
        {
            mw.addModule<QuickAutoPortWindow>("Quick Auto Port");
            break;
        }
        case GuiMode::Medium:
        {
            throw std::runtime_error("This level has not yet been implemented.");
        }
        case GuiMode::Advanced:
        {
            throw std::runtime_error("This level has not yet been implemented.");
        }
        case GuiMode::Invalid: throw std::runtime_error("This level does not exist.");
    }
}

QString LevelSelector::getHelpText(GuiMode level)
{
    switch (level)
    {
        case GuiMode::QuickAutoPort:
        {
            return tr("Quick Auto Port\n"
                      "Quick Auto Port uses default settings for porting a mod between LE and SSE.\n"
                      "It will work for most of the cases and is the recommended way to port a mod.\n"
                      "It is safe to apply it on a mod, and it is recommended to apply it to your whole "
                      "mod list if you experience crashes.");
        }
        case GuiMode::Medium:
        {
            return tr("Medium mode\n"
                      "NOT IMPLEMENTED");
        }
        case GuiMode::Advanced:
        {
            return tr("Advanced mode\n"
                      "NOT IMPLEMENTED\n"
                      "The full CAO experience. With profiles and patterns, you can fully customize "
                      "how CAO will optimize your files.");
        }
        case GuiMode::Invalid:
        {
            throw std::runtime_error("This level does not exist.");
        }
    }
    return tr("Unknown mode");
}
} // namespace CAO
