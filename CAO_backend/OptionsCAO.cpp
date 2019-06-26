/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "OptionsCAO.h"

OptionsCAO::OptionsCAO() {}

OptionsCAO::OptionsCAO(const OptionsCAO& other)
{
    bBsaExtract = other.bBsaExtract;
    bBsaCreate = other.bBsaCreate;
    bBsaDeleteBackup = other.bBsaDeleteBackup;
    bBsaProcessContent = other.bBsaProcessContent;

    bAnimationsOptimization = other.bAnimationsOptimization;

    bDryRun = other.bDryRun;

    bMeshesHeadparts = other.bMeshesHeadparts;
    iMeshesOptimizationLevel = other.iMeshesOptimizationLevel;
    iTexturesOptimizationLevel = other.iTexturesOptimizationLevel;

    iLogLevel = other.iLogLevel;
    mode = other.mode;
    userPath = other.userPath;
}

void OptionsCAO::saveToIni(QSettings *settings)
{
    //General
    settings->setValue("bDryRun", bDryRun);
    settings->setValue("iLogLevel", iLogLevel);
    settings->setValue("mode", mode);
    settings->setValue("userPath", userPath);

    //BSA
    settings->beginGroup("BSA");
    settings->setValue("bBsaExtract", bBsaExtract);
    settings->setValue("bBsaCreate", bBsaCreate);
    settings->setValue("bBsaDeleteBackup", bBsaDeleteBackup);
    settings->setValue("bBsaProcessContent", bBsaProcessContent);
    settings->endGroup();

    //Textures
    settings->setValue("Textures/iTexturesOptimizationLevel", iTexturesOptimizationLevel);

    //Meshes
    settings->setValue("Meshes/iMeshesOptimizationLevel", iMeshesOptimizationLevel);

    //Meshes advanced
    settings->setValue("Meshes/bMeshesHeadparts", bMeshesHeadparts);

    //Animations
    settings->setValue("Animations/bAnimationsOptimization", bAnimationsOptimization);
}

void OptionsCAO::readFromIni(QSettings *settings)
{
    //General
    bDryRun = settings->value("bDryRun").toBool();
    iLogLevel = settings->value("iLogLevel").toInt();
    mode = settings->value("mode").value<OptimizationMode>();
    userPath = settings->value("userPath").toString();

    //BSA
    settings->beginGroup("BSA");
    bBsaExtract = settings->value("bBsaExtract").toBool();
    bBsaCreate = settings->value("bBsaCreate").toBool();
    bBsaDeleteBackup = settings->value("bBsaDeleteBackup").toBool();
    bBsaProcessContent = settings->value("bBsaProcessContent").toBool();
    settings->endGroup();

    //Textures
    iTexturesOptimizationLevel = settings->value("Textures/iTexturesOptimizationLevel").toInt();

    //Meshes
    iMeshesOptimizationLevel = settings->value("Meshes/iMeshesOptimizationLevel").toInt();

    //Meshes advanced
    bMeshesHeadparts = settings->value("Meshes/bMeshesHeadparts").toBool();

    //Animations
    bAnimationsOptimization = settings->value("Animations/bAnimationsOptimization").toBool();
}

void OptionsCAO::saveToUi(Ui::Custom *ui)
{
    //BSA
    ui->bsaExtractCheckBox->setChecked(bBsaExtract);
    ui->bsaCreateCheckbox->setChecked(bBsaCreate);
    ui->bsaDeleteBackupsCheckbox->setChecked(bBsaDeleteBackup);
    ui->bsaProcessContentCheckBox->setChecked(bBsaProcessContent);

    //Textures

    switch (iTexturesOptimizationLevel)
    {
    case 0: ui->texturesGroupBox->setChecked(false); break;
    case 1: ui->texturesGroupBox->setChecked(true);     ui->texturesNecessaryOptimizationRadioButton->setChecked(true);  break;
    case 2: ui->texturesGroupBox->setChecked(true);     ui->texturesFullOptimizationRadioButton->setChecked(true); break;
    }

    //Meshes

    switch(iMeshesOptimizationLevel)
    {
    case 0: ui->meshesGroupBox->setChecked(false); break;
    case 1: ui->meshesGroupBox->setChecked(true);     ui->meshesNecessaryOptimizationRadioButton->setChecked(true);  break;
    case 2: ui->meshesGroupBox->setChecked(true);     ui->meshesMediumOptimizationRadioButton->setChecked(true); break;
    case 3: ui->meshesGroupBox->setChecked(true);     ui->meshesFullOptimizationRadioButton->setChecked(true); break;
    }

    //Animations
    ui->animationsGroupBox->setChecked(bAnimationsOptimization);

    //General and GUI
    ui->dryRunCheckBox->setChecked(bDryRun);
    ui->modeChooserComboBox->setCurrentIndex(mode);

    //Log level
    ui->actionLogVerbosityInfo->setChecked(false);
    ui->actionLogVerbosityNote->setChecked(false);
    ui->actionLogVerbosityTrace->setChecked(false);

    switch (iLogLevel)
    {
    case 6: ui->actionLogVerbosityTrace->setChecked(true); break;
    case 4: ui->actionLogVerbosityNote->setChecked(true); break;
    case 3: ui->actionLogVerbosityInfo->setChecked(true); break;
    }
}

void OptionsCAO::readFromUi(Ui::Custom *ui)
{
    //BSA
    bBsaExtract = ui->bsaExtractCheckBox->isChecked();
    bBsaCreate = ui->bsaCreateCheckbox->isChecked();
    bBsaDeleteBackup =ui->bsaDeleteBackupsCheckbox->isChecked();
    bBsaProcessContent = ui->bsaProcessContentCheckBox->isChecked();

    //Textures
    if(ui->texturesNecessaryOptimizationRadioButton->isChecked())
        iTexturesOptimizationLevel = 1;
    else if(ui->texturesFullOptimizationRadioButton->isChecked())
        iTexturesOptimizationLevel = 2;
    if(!ui->texturesGroupBox->isChecked())
        iTexturesOptimizationLevel = 0;

    //Meshes base
    if(ui->meshesNecessaryOptimizationRadioButton->isChecked())
        iMeshesOptimizationLevel = 1;
    else if(ui->meshesMediumOptimizationRadioButton->isChecked())
        iMeshesOptimizationLevel = 2;
    else if(ui->meshesFullOptimizationRadioButton->isChecked())
        iMeshesOptimizationLevel = 3;
    if(!ui->meshesGroupBox->isChecked())
        iMeshesOptimizationLevel = 0;

    //Meshes advanced
    bMeshesHeadparts = ui->meshesHeadpartsCheckBox->isChecked();

    //Animations
    bAnimationsOptimization = ui->animationsNecessaryOptimizationCheckBox->isChecked();

    //General
    bDryRun = ui->dryRunCheckBox->isChecked();
    userPath = QDir::cleanPath(ui->userPathTextEdit->text());
    mode = ui->modeChooserComboBox->currentData().value<OptimizationMode>();

    if(ui->actionLogVerbosityInfo->isChecked())
        iLogLevel = 3;
    else if(ui->actionLogVerbosityNote->isChecked())
        iLogLevel = 4;
    else if(ui->actionLogVerbosityTrace->isChecked())
        iLogLevel = 6;
}

void OptionsCAO::parseArguments(const QStringList &args)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addPositionalArgument("folder", "The folder to process, surrounded with quotes.");
    parser.addPositionalArgument("mode", "Either om (one mod) or sm (several mods)");
    parser.addPositionalArgument("game", "Currently, only 'SSE', 'TES5', 'FO4' and 'Custom' are supported");

    parser.addOptions({
                          {"dr", "Enables dry run"},
                          {"l", "Log level: from 0 (maximum) to 6", "value", "0"},
                          {"m", "Mesh processing level: 0 (default) to disable optimization, 1 for necessary optimization, "
                           "2 for medium optimization, 3 for full optimization.", "value", "0"},

                          {"t", "Texture processing level: 0 (default) to disable optimization, "
                           "1 for necessary optimization, 2 for full optimization.", "value", "0"},

                          {"a", "Enables animations processing"},
                          {"mh", "Enables headparts detection and processing"},
                          {"be", "Enables BSA extraction."},
                          {"bc", "Enables BSA creation."},
                          {"bd", "Enables deletion of BSA backups."},
                          {"bo", "NOT WORKING. Enables BSA optimization. The files inside the "
                           "BSA will be extracted to memory and processed according to the provided settings "},
                      });

    parser.process(args);

    QString path = QDir::cleanPath(parser.positionalArguments().at(0));
    userPath = path;

    QString readMode = parser.positionalArguments().at(1);
    if(readMode == "om")
        mode = singleMod;
    else if(readMode == "sm")
        mode = severalMods;

    QString readGame = parser.positionalArguments().at(2);
    if(readGame == "SSE")
        CAO_SET_CURRENT_GAME(Games::Custom)
                else if(readGame == "TES5")
                CAO_SET_CURRENT_GAME(Games::TES5)
                else if(readGame == "FO4")
                CAO_SET_CURRENT_GAME(Games::FO4)
                else if(readGame == "Custom")
                CAO_SET_CURRENT_GAME(Games::Custom)
                else
                throw std::runtime_error("Cannot set game. Game:'" + readGame.toStdString() + "' does not exist");

    bDryRun = parser.isSet("dr");
    iLogLevel = parser.value("l").toInt();

    iMeshesOptimizationLevel = parser.value("m").toInt();
    bMeshesHeadparts = parser.isSet("mh");

    iTexturesOptimizationLevel = parser.value("t").toInt();

    bAnimationsOptimization = parser.isSet("a");

    bBsaExtract = parser.isSet("be");
    bBsaCreate = parser.isSet("bc");
    bBsaDeleteBackup = parser.isSet("bd");
    bBsaProcessContent = parser.isSet("bo");
}

bool OptionsCAO::isValid()
{
    if(!QDir(userPath).exists() || userPath.size() < 5)
    {
        PLOG_FATAL << "This path does not exist or is shorter than 5 characters. Path:'" + userPath + "'";
        return false;
    }

    if (mode != OptionsCAO::singleMod && mode != OptionsCAO::severalMods)
    {
        PLOG_FATAL << "This mode does not exist.";
        return false;
    }

    if(iLogLevel <= 0 || iLogLevel > 6)
    {
        PLOG_FATAL << "This log level does not exist. Log level: " + std::to_string(iLogLevel);
        return false;
    }

    if(iMeshesOptimizationLevel < 0 || iMeshesOptimizationLevel > 3)
    {
        PLOG_FATAL << "This meshes optimization level does not exist. Level: " + std::to_string(iMeshesOptimizationLevel);
        return false;
    }

    if(iTexturesOptimizationLevel < 0 || iTexturesOptimizationLevel > 2)
    {
        PLOG_FATAL << "This textures optimization level does not exist. Level: " + std::to_string(iTexturesOptimizationLevel);
        return false;
    }
    return true;
}
