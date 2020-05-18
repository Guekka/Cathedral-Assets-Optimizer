/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "Settings/PatternMap.hpp"

#include "pch.hpp"
class MainWindow;

namespace CAO {

class Profile
{
public:
    static inline const QString generalSettingsFileName = "GeneralSettings.json";
    static inline const QString patternSettingsFileName = "PatternSettings.json";
    static inline const QString isBaseProfileFilename   = "isBase";

    Profile(QDir profileDir);

    QFile getFile(const QString &filename) const;

    /* Getters */
    bool isBaseProfile() const;
    QDir profileDirectory() const;
    QString logPath() const;
    QString generalSettingsPath() const;
    QString patternSettingsPath() const;

    /* Settings */
    PatternSettings &getSettings(const QString &filePath);
    const PatternSettings &getSettings(const QString &filePath) const;
    GeneralSettings &getGeneralSettings();
    const GeneralSettings &getGeneralSettings() const;
    PatternMap &getPatterns();
    const PatternMap &getPatterns() const;

    void saveToJSON();

    void saveToUi(MainWindow &window) const;
    void readFromUi(const MainWindow &window);

private:
    GeneralSettings generalSettings_;
    PatternMap patternSettings_;

    QDir profileDir_;
    QString logPath_;
};

class Profiles
{
public:
    static inline const QString commonSettingsFileName = "common.ini";
    static inline const QString defaultProfile         = "SSE";

    static Profiles &getInstance();

    void setDir(const QDir &dir);

    /* Profiles operations */
    void create(const QString &name, const QString &baseProfile);
    void create(const QString &name);
    void create(const Profile &profile, const QString &name);
    Profile &setCurrent(const QString &profile);

    //Also sets the current profile. Reads the current profile from INI
    Profile &getCurrent();
    Profile &get(const QString &profile);

    QString currentProfileName();

    QStringList list();
    bool exists(const QString &profile);
    void update(bool fullRefresh = false);

    QSettings &commonSettings() { return _commonSettings; }

private:
    std::unordered_map<QString, Profile> profiles_;
    QDir rootProfileDir_;
    QSettings _commonSettings;

    /* Constructor */
    Profiles();
};

static Profile &currentProfile()
{
    return Profiles::getInstance().getCurrent();
}

} // namespace CAO
