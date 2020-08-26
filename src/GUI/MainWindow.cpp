/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindow.hpp"
#include "GUI/Utils/SetTheme.hpp"
#include "Manager.hpp"
#include "PatternsManagerWindow.hpp"
#include "ProfilesManagerWindow.hpp"
#include "SelectGPUWindow.hpp"
#include "Utils.hpp"

namespace CAO {
MainWindow::MainWindow()
    : ui_(std::make_unique<Ui::MainWindow>())
{
    ui_->setupUi(this);
    setAcceptDrops(true);

    //Setting data for widgets
    setData(*ui_->modeChooserComboBox, "One mod", SingleMod);
    setData(*ui_->modeChooserComboBox, "Several mods", SeveralMods);

    //Connecting widgets that do not depend on current profile

    QObject::connect(ui_->manageProfiles, &QPushButton::pressed, this, [this] {
        ProfilesManagerWindow profilesManager(getProfiles());
        profilesManager.exec();
        updateProfiles();
    });

    QObject::connect(ui_->managePatterns, &QPushButton::pressed, this, [this] {
        PatternsManagerWindow patternsManager(currentProfile());
        patternsManager.exec();
        updatePatterns();
    });

    auto &commonSettings = getProfiles().commonSettings();

    ui_->actionEnableDarkTheme->setChecked(commonSettings.eTheme.value_or(GuiTheme::Dark) == GuiTheme::Dark);
    setTheme(commonSettings.eTheme.value_or(GuiTheme::Dark));

    QObject::connect(ui_->actionEnableDarkTheme,
                     &QAction::triggered,
                     &commonSettings.eTheme,
                     [this, &commonSettings](bool state) {
                         GuiTheme theme        = state ? GuiTheme::Dark : GuiTheme::Light;
                         commonSettings.eTheme = theme;
                         ui_->actionEnableDarkTheme->setChecked(commonSettings.eTheme() == GuiTheme::Dark);
                         setTheme(theme);
                     });

    selectText(*ui_->profiles, commonSettings.sProfile());
    QObject::connect(ui_->profiles, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        getProfiles().setCurrent(ui_->profiles->itemText(idx));
        reconnectAll();
    });

    QObject::connect(ui_->processButton, &QPushButton::pressed, this, &MainWindow::initProcess);
    connectWrapper(*ui_->actionShow_tutorials, commonSettings.bShowTutorials);

    //Menu buttons
    QObject::connect(ui_->actionOpen_log_file, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("file:///" + currentProfile().logPath(), QUrl::TolerantMode));
    });

    QObject::connect(ui_->actionDocumentation, &QAction::triggered, this, [&] {
        QDesktopServices::openUrl(QUrl("https://www.nexusmods.com/skyrimspecialedition/mods/23316"));
    });

    QObject::connect(ui_->actionDiscord, &QAction::triggered, this, [&] {
        QDesktopServices::openUrl(QUrl("https://discordapp.com/invite/B9abN8d"));
    });

    QObject::connect(ui_->actionDonate, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("https://ko-fi.com/guekka"));
    });

    QObject::connect(ui_->actionAbout, &QAction::triggered, this, [this] {
        constexpr char message[]
            = "\nMade by G'k\nThis program is distributed in the hope that it will be useful "
              "but WITHOUT ANY WARRANTLY. See the Mozilla Public License";

        const QString text = QString("%1 %2 %3")
                                 .arg(QCoreApplication::applicationName(),
                                      QCoreApplication::applicationVersion(),
                                      tr(message));

        QMessageBox::about(this, tr("About"), text);
    });

    QObject::connect(ui_->actionAbout_Qt, &QAction::triggered, this, [this] { QMessageBox::aboutQt(this); });

    initSettings();

    firstStart();
}

void MainWindow::addModule(IWindowModule *module)
{
    ui_->tabWidget->addTab(module, module->name());
    ui_->tabWidget->setCurrentIndex(0);
    connectModule(*module);
}

void MainWindow::clearModules()
{
    auto &tabs = ui_->tabWidget;
    for (int i = 0; i < tabs->count(); ++i)
    {
        tabs->widget(i)->deleteLater();
        tabs->removeTab(i);
    }
}

void MainWindow::setPatternsEnabled(bool state)
{
    const std::array patternsObject = {static_cast<QWidget *>(ui_->patterns),
                                       static_cast<QWidget *>(ui_->managePatterns),
                                       static_cast<QWidget *>(ui_->patternsLabel)};

    if (!selectText(*ui_->patterns, "*"))
    {
        QMessageBox::critical(this,
                              tr("Pattern not found"),
                              tr("Couldn't find the pattern '*'. Please reinstall the application"));
    }

    for (auto *widget : patternsObject)
    {
        widget->setEnabled(state);
        widget->setVisible(state);
    }
}

void MainWindow::setLevelSelectorHandler(const std::function<void()> &callback)
{
    QObject::connect(ui_->actionChange_level, &QAction::triggered, this, callback);
}

void MainWindow::initSettings()
{
    updateProfiles();
    updatePatterns();
}

template<typename... Args>
void MainWindow::connect(Args &&... args)
{
    connections_.emplace_back(QObject::connect(std::forward<Args>(args)...));
}

template<typename... Args>
void MainWindow::connectWrapper(Args &&... args)
{
    connections_.emplace_back(CAO::connectWrapper(std::forward<Args>(args)...));
}

std::vector<IWindowModule *> MainWindow::getModules()
{
    std::vector<IWindowModule *> modules;
    for (int i = 0; i < ui_->tabWidget->count(); i++)
        if (auto mod = dynamic_cast<IWindowModule *>(ui_->tabWidget->widget(i)); mod)
            modules.emplace_back(mod);

    return modules;
}

void MainWindow::connectModule(IWindowModule &mod)
{
    auto &pattern = currentProfile().getPatterns().getSettingsByName(ui_->patterns->currentText());
    mod.connectAll(pattern, currentProfile().getGeneralSettings());
}

void MainWindow::reconnectModules()
{
    for (auto *mod : getModules())
    {
        mod->disconnectAll();
        connectModule(*mod);
    }
}

void MainWindow::freezeModules(bool state)
{
    for (auto *mod : getModules())
        mod->setDisabled(state);
}

void MainWindow::connectAll()
{
    auto &generalSettings = currentProfile().getGeneralSettings();

    connect(ui_->actionSelect_GPU, &QAction::triggered, this, [] {
        SelectGPUWindow window;
        window.setSelectedIndex(getProfiles().commonSettings().iGPUIndex());
        window.exec();
        if (window.result() == QDialog::Accepted)
        {
            auto idx = window.getSelectedIndex();
            if (idx.has_value())
                getProfiles().commonSettings().iGPUIndex = idx.value();
        }
    });

    connect(ui_->userPathButton, &QPushButton::pressed, this, [&generalSettings, this] {
        const QString &currentPath = generalSettings.sInputPath.value_or(QDir::currentPath());

        const QString &dir = QFileDialog::getExistingDirectory(this,
                                                               tr("Open Directory"),
                                                               currentPath,
                                                               QFileDialog::ShowDirsOnly
                                                                   | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty())
            generalSettings.sInputPath = dir;
    });

    selectText(*ui_->patterns, generalSettings.sCurrentPattern.value_or("*"));
    connect(ui_->patterns,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            &generalSettings.sCurrentPattern,
            [this, &generalSettings](int idx) {
                generalSettings.sCurrentPattern = ui_->patterns->itemText(idx);
                reconnectAll();
            });

    const int currentModeIndex = ui_->modeChooserComboBox->findData(generalSettings.eMode());
    ui_->modeChooserComboBox->setCurrentIndex(currentModeIndex);

    connect(ui_->modeChooserComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            &generalSettings.eMode,
            [&generalSettings, this](int idx) {
                const auto data       = ui_->modeChooserComboBox->itemData(idx);
                generalSettings.eMode = data.value<OptimizationMode>();
            });

    connectWrapper(*ui_->dryRunCheckBox, generalSettings.bDryRun);
    connectWrapper(*ui_->inputDirTextEdit, generalSettings.sInputPath);
    connectWrapper(*ui_->actionRedirect_output, generalSettings.bEnableOutputPath);

    connect(ui_->actionSet_output_path, &QAction::triggered, this, [&generalSettings, this] {
        const QString &inPath      = generalSettings.sInputPath.value_or(QDir::currentPath());
        const QString &currentPath = generalSettings.sOutputPath.value_or(inPath);

        const QString &dir = QFileDialog::getExistingDirectory(this,
                                                               tr("Open Directory"),
                                                               currentPath,
                                                               QFileDialog::ShowDirsOnly
                                                                   | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty())
            generalSettings.sOutputPath = dir;
    });
}

void MainWindow::disconnectAll()
{
    for (auto connection : connections_)
        QObject::disconnect(connection);
}

void MainWindow::reconnectAll()
{
    disconnectAll();
    connectAll();
    reconnectModules();
}

void MainWindow::updateProfiles()
{
    auto *profiles              = ui_->profiles;
    const QString &previousText = profiles->currentText();
    ProfilesManagerWindow(getProfiles()).updateProfiles(*ui_->profiles);
    selectText(*profiles, previousText);
    reconnectAll();
}

void MainWindow::updatePatterns()
{
    auto *patterns              = ui_->patterns;
    const QString &previousText = patterns->currentText();
    PatternsManagerWindow(currentProfile()).updatePatterns(*ui_->patterns);
    selectText(*patterns, previousText);
    reconnectAll();
}

void MainWindow::loadUi()
{
    auto &commonSettings = getProfiles().commonSettings();

    setTheme(commonSettings.eTheme());

    ui_->profiles->setCurrentIndex(ui_->profiles->findText(getProfiles().currentProfileName()));
    ui_->actionShow_tutorials->setChecked(commonSettings.bShowTutorials());
}

void MainWindow::resetUi()
{
    ui_->tabWidget->clear();
}

void MainWindow::initProcess()
{
    getProfiles().saveCommonSettings();
    currentProfile().saveToJSON();
    ui_->processButton->setDisabled(true);

    try
    {
        caoProcess_     = std::make_unique<Manager>();
        progressWindow_ = std::make_unique<ProgressWindow>(currentProfile().logPath());

        connect(caoProcess_.get(),
                &Manager::progressBarTextChanged,
                progressWindow_.get(),
                &ProgressWindow::update);

        connect(caoProcess_.get(), &Manager::end, this, &MainWindow::endProcess);

        progressWindow_->show();

        freezeModules();

        QtConcurrent::run(caoProcess_.get(), &Manager::runOptimization);
    }
    catch (const std::exception &e)
    {
        QMessageBox box(QMessageBox::Critical,
                        tr("Error"),
                        tr("An exception has been encountered and the process was forced to stop: ")
                            + QString(e.what()));
        box.exec();
        endProcess();
    }
}

void MainWindow::endProcess()
{
    ui_->processButton->setDisabled(false);

    if (caoProcess_)
        caoProcess_->disconnect();

    if (progressWindow_)
    {
        progressWindow_->end();
        progressWindow_->disconnect();
    }

    getProfiles().saveCommonSettings();
    for (const auto &profile : getProfiles().list())
        getProfiles().get(profile).saveToJSON();

    freezeModules(false);
}

void MainWindow::showTutorialWindow(const QString &title, const QString &text)
{
    if (getProfiles().commonSettings().bShowTutorials())
        QMessageBox::information(this, title, text);
}

void MainWindow::firstStart()
{
    constexpr char welcome[]
        = "It appears you are running CAO for the first time. All options have tooltips explaining what "
          "they do. If you need help, you can also <a href=\"https://discordapp.com/invite/B9abN8d\">join us "
          "on Discord</a>. A dark theme is also available."
          "\n\nIf you like my work, <a href=\"https://ko-fi.com/guekka\">please consider supporting me</a>. "
          "Thanks for using CAO!";

    if (getProfiles().commonSettings().bFirstStart.value_or(true))
    {
        QMessageBox box(QMessageBox::Information,
                        tr("Welcome to %1 %2")
                            .arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()),
                        tr(welcome));
        box.setTextFormat(Qt::TextFormat::RichText);
        box.exec();

        getProfiles().commonSettings().bFirstStart = false;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    endProcess();
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    const QString &fileName = e->mimeData()->urls().at(0).toLocalFile();
    QDir dir;
    if (dir.exists(fileName))
        ui_->inputDirTextEdit->setText(QDir::cleanPath(fileName));
}

} // namespace CAO
