/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "UiBase.h"
#include "pch.h"
#include "ui_FO4.h"

namespace Ui {
    class FO4;
}

class FO4 : public UiBase
{
    Q_DECLARE_TR_FUNCTIONS(FO4)

public:
    FO4();
    ~FO4();

private:
    Ui::FO4 *ui;

    virtual void saveUIToFile();
    virtual void loadUIFromFile();

    void initProcess();
};
