/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include "MainOptimizer.h"
#include "pch.h"

class Manager final : public QObject
{
    Q_OBJECT
public:
    /*!
   * \brief Constructor that will perform a number of functions
   */
    explicit Manager(OptionsCAO &opt);
    /*!
   * \brief The main process
   */
    void runOptimization();
    /*!
   * \brief Print the progress to stdout
   * \param text The text to display
   * \param total The total number of files to process
   */
    void printProgress(const int &total, const QString &text);

private:
    /*!
   * \brief List all the directories to process
   */
    void listDirectories();
    /*!
   * \brief List all the files in the modsToProcess list and store them. Also add their weights to filesWeight
   */
    void listFiles();
    /*!
   * \brief Read ignoredMods.txt and store it to a list
   */
    void readIgnoredMods();
    /*!
   * \brief The number of all files. Used to determine progress
   */
    int _numberFiles = 0;
    /*!
   * \brief The number of completed files. Used to determine progress
   */
    int _numberCompletedFiles = 0;
    /*!
   * \brief The optimization options, that will be given to the MainOptimizer
   */
    OptionsCAO &_options;
    /*!
    * \brief The list of directories to process
    */
    QStringList _modsToProcess;
    /*!
   * \brief Mods on this list won't be processed
   */
    QStringList _ignoredMods;
    /*!
   * \brief Used to read the INI
   */
    QSettings *_settings;
    /*!
   * \brief The files to process
   */
    QStringList _files;
    /*!
   * \brief The animations to process. Separated, since they don't support multithreading
   */
    QStringList _animations;
    /*!
   * \brief The BSAs to extract
   */
    QStringList BSAs;
signals:
    void progressBarTextChanged(QString, int, int);
};