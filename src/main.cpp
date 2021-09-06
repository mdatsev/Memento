////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Ripose
//
// This file is part of Memento.
//
// Memento is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License.
//
// Memento is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Memento.  If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMessageBox>
#include <QSettings>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <QStandardPaths>
    #include <windows.h>
#endif

#if __APPLE__
    #include <locale.h>
#endif

#include "audio/audioplayer.h"
#include "gui/mainwindow.h"
#include "util/constants.h"
#include "util/globalmediator.h"
#include "util/iconfactory.h"
#include "util/utils.h"

/**
 * Updates the QSettings before the MainWindow is created.
 * This is used during updates to make sure that configurations for old versions
 * don't cause problems.
 */
void updateSettings()
{
    QSettings settings;
    uint version = settings.value(SETTINGS_VERSION, 0).toUInt();
    if (version == SETTINGS_VERSION_CURRENT)
    {
        return;
    }
    else if (version > SETTINGS_VERSION_CURRENT)
    {
        QMessageBox message;
        message.critical(
            0, "Settings From a Newer Version Found",
            "The Memento settings found belong to a newer version.\n"
            "No guarantees can be made that nothing will break or get lost."
        );
    }

    /* Migrate the settings */
    switch(version)
    {
    case 0:
    {
        settings.beginGroup(SETTINGS_INTERFACE);
        settings.remove(SETTINGS_INTERFACE_SUBTITLE_LIST_STYLE);
        settings.endGroup();
    }
    case 1:
    {
        /* These paths are hardcoded because DirectoryUtils may change in the
         * future. */
    #if __linux__
        QDir configDir(QString(getenv("HOME")) + "/.config/memento");
        configDir.rename("./dict/dictionaries.sqlite", "./dictionaries.sqlite");

        QDir dictDir(configDir.absolutePath() + "/dict");
        dictDir.removeRecursively();
    #elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        QDir programDir(DirectoryUtils::getProgramDirectory());
        programDir.rename(
            ".\\config\\dict\\dictionaries.sqlite",
            ".\\config\\dictionaries.sqlite"
        );

        QDir dictDir(DirectoryUtils::getProgramDirectory() + "config\\dict");
        dictDir.removeRecursively();

        QString configPath = QDir::toNativeSeparators(
                QStandardPaths::writableLocation(
                    QStandardPaths::AppConfigLocation
                )
            );
        configPath.chop(sizeof("memento") - 1);
        QDir configDir(configPath);
        configDir.removeRecursively();

        programDir.rename(".\\config", configDir.absolutePath());
    #endif
    }
    }

    /* Remove saved window configurations just to be safe */
    settings.beginGroup(SETTINGS_GROUP_WINDOW);
    settings.remove("");
    settings.endGroup();

    /* Set the version */
    settings.setValue(SETTINGS_VERSION, SETTINGS_VERSION_CURRENT);
}

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    /* Image Formats Windows */
    QCoreApplication::addLibraryPath(DirectoryUtils::getProgramDirectory());

    /* Enable output in console */
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
    /* HiDPI support */
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    /* Originization Info */
    QCoreApplication::setOrganizationName("memento");
    QCoreApplication::setOrganizationDomain("ripose.projects");
    QCoreApplication::setApplicationName("memento");

    /* Construct the application */
    QApplication memento(argc, argv);

    /* Set the window icon */
    QGuiApplication::setWindowIcon(QIcon(":memento.svg"));

    updateSettings();

    /* Create the configuration directory if it doesn't exist */
    if (!QDir(DirectoryUtils::getConfigDir()).exists() &&
        !QDir().mkdir(DirectoryUtils::getConfigDir()))
    {
        QMessageBox message;
        message.critical(
            0, "Error Creating Config Directory",
            "Could not make configuration directory at " +
            DirectoryUtils::getConfigDir()
        );
        return EXIT_FAILURE;
    }

    /* General Setup */
    setlocale(LC_NUMERIC, "C"); // mpv requires this

    GlobalMediator::createGlobalMedaitor();
    GlobalMediator::getGlobalMediator()->setAudioPlayer(new AudioPlayer);

    MainWindow *main_window = new MainWindow;
    main_window->show();
    int ret = memento.exec();

    /* Deallocate shared resources */
    delete main_window;
    delete GlobalMediator::getGlobalMediator()->getAudioPlayer();
    delete GlobalMediator::getGlobalMediator();
    delete IconFactory::create();

    return ret;
}
