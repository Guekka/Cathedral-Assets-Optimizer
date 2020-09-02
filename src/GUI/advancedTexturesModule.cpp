/* Copyright (C) 2020 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AdvancedTexturesModule.hpp"
#include "ListDialog.hpp"
#include "Settings/GeneralSettings.hpp"
#include "Settings/PatternSettings.hpp"
#include "Utils.hpp"
#include "ui_AdvancedTexturesModule.h"

namespace CAO {
AdvancedTexturesModule::AdvancedTexturesModule(QWidget *parent)
    : IWindowModule(parent)
    , ui_(std::make_unique<Ui::AdvancedTexturesModule>())
    , textureFormatDialog_(std::make_unique<ListDialog>(ListDialog::sortByText))
{
    ui_->setupUi(this);
}

void AdvancedTexturesModule::disconnectAll()
{
    IWindowModule::disconnectAll();
}

void AdvancedTexturesModule::init([[maybe_unused]] PatternSettings &pSets,
                                  [[maybe_unused]] GeneralSettings &gSets)
{
    connectGroupBox(ui_->mainBox, ui_->mainNecessary, ui_->mainCompress, ui_->mainMipMaps);

    connectGroupBox(ui_->resizingBox,
                    ui_->resizingMode,
                    ui_->resizingMinimumCheckBox,
                    ui_->resizingMinimumWidth,
                    ui_->resizingMinimumHeight,
                    ui_->resizingWidth,
                    ui_->resizingHeight);

    setData(*ui_->resizingMode, "By ratio", TextureResizingMode::ByRatio);
    setData(*ui_->resizingMode, "By fixed size", TextureResizingMode::BySize);

    setData(*ui_->outputFormat, "BC7 (BC7_UNORM)", DXGI_FORMAT_BC7_UNORM);
    setData(*ui_->outputFormat, "BC5 (BC5_UNORM)", DXGI_FORMAT_BC5_UNORM);
    setData(*ui_->outputFormat, "BC3 (BC3_UNORM)", DXGI_FORMAT_BC3_UNORM);
    setData(*ui_->outputFormat, "BC1 (BC1_UNORM)", DXGI_FORMAT_BC1_UNORM);
    setData(*ui_->outputFormat, "Uncompressed (R8G8B8A8_UNORM)", DXGI_FORMAT_R8G8B8A8_UNORM);

    //Init unwanted formats
    textureFormatDialog_->setUserAddItemVisible(false);

    const auto unwantedFormats = pSets.slTextureUnwantedFormats();

    for (const auto &value : Detail::DxgiFormats)
    {
        auto item = new QListWidgetItem(value.name);
        item->setData(Qt::UserRole, value.format);

        if (contains(unwantedFormats, value.format))
            item->setCheckState(Qt::Checked);

        textureFormatDialog_->addItem(item);
    }
}

void AdvancedTexturesModule::connectAll(PatternSettings &pSets, [[maybe_unused]] GeneralSettings &gSets)
{
    //Base
    connectWrapper(*ui_->mainNecessary, pSets.bTexturesNecessary);
    connectWrapper(*ui_->mainCompress, pSets.bTexturesCompress);
    connectWrapper(*ui_->mainMipMaps, pSets.bTexturesMipmaps);
    connectWrapper(*ui_->mainAlphaLandscape, pSets.bTexturesLandscapeAlpha);

    //Resizing

    connect(ui_->resizingMode,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this, &pSets](int index) {
                if (!ui_->resizingBox->isChecked())
                    return;

                const auto data = ui_->resizingMode->itemData(index).value<TextureResizingMode>();
                pSets.eTexturesResizingMode = data;
            });

    connectWrapper(*ui_->resizingWidth, pSets.iTexturesResizingWidth);
    connectWrapper(*ui_->resizingHeight, pSets.iTexturesResizingHeight);

    connectWrapper(*ui_->resizingMinimumCheckBox, pSets.bTexturesResizeMinimum);
    connectWrapper(*ui_->resizingMinimumWidth, pSets.iTexturesMinimumWidth);
    connectWrapper(*ui_->resizingMinimumHeight, pSets.iTexturesMinimumHeight);

    connectWrapper(*ui_->forceConversion, pSets.bTexturesForceConvert);

    connect(ui_->outputFormat,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            &pSets.eTexturesFormat,
            [&pSets, this](int idx) {
                const auto data             = ui_->outputFormat->itemData(idx);
                pSets.eTexturesFormat       = data.value<DXGI_FORMAT>();
            });

    connect(ui_->texturesUnwantedFormatsEdit,
            &QPushButton::pressed,
            textureFormatDialog_.get(),
            &ListDialog::open);

    connect(textureFormatDialog_.get(), &QDialog::finished, this, [&pSets, this] {
        pSets.slTextureUnwantedFormats = textureFormatDialog_->getChoices() | rx::transform([](auto *item) {
                                             return item->data(Qt::UserRole).template value<DXGI_FORMAT>();
                                         })
                                         | rx::to_vector();
    });
}

QString AdvancedTexturesModule::name()
{
    return tr("Textures (Patterns)");
}

bool AdvancedTexturesModule::isSupportedGame(Games game)
{
    switch (game)
    {
        case Games::Morrowind:
        case Games::Oblivion:
        case Games::SkyrimLE:
        case Games::SkyrimSE:
        case Games::Fallout3:
        case Games::FalloutNewVegas:
        case Games::Fallout4: return true;
    }
    return false;
}

} // namespace CAO
