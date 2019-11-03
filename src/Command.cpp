/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Command.hpp"

namespace CAO {
CommandResult Command::processIfApplicable(File &file, const OptionsCAO &options)
{
    CommandResult result;
    result.processedFile = false;
    result.errorCode = 0;

    if (isApplicable(file, options))
        return process(file, options);
    return result;
}
} // namespace CAO
