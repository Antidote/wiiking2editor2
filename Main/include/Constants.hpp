// This file is part of WiiKing2 Editor.
//
// WiiKing2 Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Wiiking2 Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WiiKing2 Editor.  If not, see <http://www.gnu.org/licenses/>

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <QSettings>
#include <QDir>
// Only ONE of these may be defined
//#define WK2_PREVIEW
#define WK2_INTERNAL

#if defined(WK2_PREVIEW) && defined(WK2_INTERNAL)
#error "You can only define either WK2_PREVIEW or WK2_INTERNAL"
#endif

namespace Constants
{
#define tr QObject::tr
const int     WIIKING2_MAJOR                    = 0;
const int     WIIKING2_MINOR                    = 1;
const int     WIIKING2_PATCH                    = 1;
const int     WIIKING2_VERSION                  = (WIIKING2_MAJOR << 16) | (WIIKING2_MINOR << 8) | WIIKING2_PATCH;
const QString WIIKING2_VERSION_STR              = QString("%1.%2.%3")
                                                  .arg(WIIKING2_MAJOR)
                                                  .arg(WIIKING2_MINOR)
                                                  .arg(WIIKING2_PATCH);
#ifdef WK2_PREVIEW
const QString WIIKING2_APP_VERSION               = QString(WIIKING2_VERSION_STR + " PREVIEW 2");
#elif defined(WK2_INTERNAL)
const QString WIIKING2_APP_VERSION               = QString(WIIKING2_VERSION_STR + " INTERNAL");
#elif defined(DEBUG)
const QString WIIKING2_APP_VERSION               = QString(WIIKING2_VERSION_STR + " DEBUG");
#else
const QString WIIKING2_APP_VERSION               = QString(WIIKING2_VERSION_STR);
#endif

const QString WIIKING2_LOCK_FILE                 = QString(QDir::temp().absolutePath() + "/wk.lck");
const QString WIIKING2_TITLE                     = tr("WiiKing2 Editor");
// Work around for Qt's retarded method of setWindowFilePath
// It doesn't work if you set a custom window title
// But the only way to set the window title to what you want is to
// use setWindowTitle
const QString WIIKING2_TITLE_FILE                = tr("%1%2 - Wiiking2 Editor");
const QString WIIKING2_TITLE_DIRTY               = tr("*");
const QString WIIKING2_APP_NAME                  = tr("wiiking2editor");
const QString WIIKING2_UPDATE_CHECKING           = tr("Checking for updates please wait...");
const QString WIIKING2_UPDATE_CHECKING_MSG       = tr("%1 is checking for updates, please wait.")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_UPDATE_CONTACT_ERROR      = tr("Unable to contact server");
const QString WIIKING2_UPDATE_CONTACT_ERROR_MSG  = tr("%1 was unable to contact the update server.<br />"
                                                      "check your network settings and try again.<br />"
                                                      "If the issue persists, the server may not be available <br/>"
                                                      "in that case try again later or contact a developer for assistance")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_UPDATE_PARSE_ERROR        = tr("Error while parsing...");
const QString WIIKING2_UPDATE_PARSE_ERROR_MSG    = tr("Unable to parse update file <br />"
                                                      "Please contact a developer for assistance with this message:<br />%1");
const QString WIIKING2_LATEST_VERSION            = tr("%1 is up to date...")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_LATEST_VERSION_MSG        = tr("%1 is at the latest version.<br />"
                                                      "If you feel this is an error please contact a developer with your platform.")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_NOT_LATEST_VERSION        = tr("%1 is out of date...")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_NOT_LATEST_VERSION_MSG    = tr("An update is available for %1<br />"
                                                      "You may download the update at: <a href=\"%2\">%2</a><br />"
                                                      "MD5 %3<br />"
                                                      "The changelog is available at: <a href=\"%4\">%4</a>")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_INSTANCE_EXISTS           = tr("Single Instance error...");
const QString WIIKING2_INSTANCE_EXISTS_MSG       = tr("An instance %1 is already running<br />"
                                                      "If this is an error you can delete the lock file at<br>"
                                                      "%2<br/>"
                                                      "%1 is designed to ignore locks older than 60 seconds, so you may also wait if you wish")
                                                   .arg(WIIKING2_TITLE)
                                                   .arg(WIIKING2_LOCK_FILE);
const QString WIIKING2_BUILD_DATE                = tr(__DATE__ " " __TIME__);
const QString WIIKING2_NO_PLUGINS_ERROR          = tr("Error Loading Plugins...");
const QString WIIKING2_NO_PLUGINS_ERROR_MSG      = tr("No plugins were loaded.\n"
                                                      "Please check the plugins directory in the application's root");
const QString WIIKING2_PLUGIN_RELOAD_WARNING     = tr("Reload plugin?");
const QString WIIKING2_PLUGIN_RELOAD_WARNING_MSG = tr("You will lose <b>all</b> unsaved data associated with this plugin<br />"
                                                      "Are you sure you wish to reload?");
const QString WIIKING2_ABOUT_TITLE               = tr("About %1...")
                                                   .arg(WIIKING2_TITLE);
const QString WIIKING2_ERROR_LOADING             = tr("Error loading file...");
const QString WIIKING2_ERROR_LOADING_MSG         = tr("Error loading file '%1' please check that it exists");
const QString WIIKING2_DIRTY_NAG                 = tr("File has been changed...");
const QString WIIKING2_DIRTY_NAG_MSG             = tr("The file '%1' has been modified, do you wish to save?");
const QString WIIKING2_ALREADY_OPENED            = tr("Already Opened...");
const QString WIIKING2_ALREADY_OPENED_MSG        = tr("The file '%1' is already opened and will not be reopened");
const QString WIIKING2_OPEN_FAILED               = tr("Failed to load file...");
const QString WIIKING2_OPEN_FAILED_MSG           = tr("Failed to load '%1', please check that it exists, and that it is valid for the plugin '%2'");
const QString WIIKING2_UPDATE_PLATFORM           = tr("Unsupported Platform...");
const QString WIIKING2_UPDATE_PLATFORM_MSG       = tr("The updater currently does not support your platform.<br />"
                                                      "If you are using an unofficial build, we do not provide support.<br />"
                                                      "We appologize for the inconvenience."
                                                      "<br />"
                                                      "If you think this is in error, please contact a developer with your platform.");


namespace Settings
{
#ifdef WK2_INTERNAL
const QString WIIKING2_UPDATE_URL_DEFAULT     = QString("http://update.local/wiiking2_editorv2/latest.update");
#else
const QString WIIKING2_UPDATE_URL_DEFAULT     = QString("http://update.wiiking2.com/wiiking2_editorv2/latest.update");
#endif
const QString WIIKING2_UPDATE_URL             = QString("updateUrl");
const QString WIIKING2_RECENT_FILES           = QString("recentFiles");
const QString WIIKING2_RECENT_DIRECTORY       = QString("recentDirectory");
const QString WIIKING2_DEFAULT_STYLE          = QString("defaultStyle");
const QString WIIKING2_CURRENT_STYLE          = QString("currentStyle");
const QString WIIKING2_CHECK_ON_START         = QString("checkForUpdatesOnStart");
}

#undef tr
}
#endif // CONSTANTS_HPP
