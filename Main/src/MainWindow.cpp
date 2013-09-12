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

#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "GameFile.hpp"
#include "PluginsManager.hpp"
#include "PluginInterface.hpp"
#include "AboutDialog.hpp"
#include <QNetworkReply>
#include <QLabel>
#include <QMenu>

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyleFactory>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_currentFile(NULL),
    m_pluginsManager(new PluginsManager(this)),
    m_aboutDialog(NULL)
  #ifdef WK2_PREVIEW
  ,m_previewLayout(NULL),
    m_previewLabel(NULL)
  #endif
{
    ui->setupUi(this);


    setWindowIcon(QIcon(":/icon/Bomb64x64.png"));
    m_defaultWindowGeometry = this->saveGeometry();
    m_defaultWindowState = this->saveState();

    // Setup the context menu for documentList
    ui->documentList->addAction(ui->actionOpen);
    ui->documentList->addAction(ui->actionNew);
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    ui->documentList->addAction(separator);
    ui->documentList->addAction(ui->actionSave);
    ui->documentList->addAction(ui->actionSaveAs);
    separator = new QAction(this);
    separator->setSeparator(true);
    ui->documentList->addAction(separator);
    ui->documentList->addAction(ui->actionClose);
    this->setWindowTitle(Constants::WIIKING2_TITLE);

    // Setup "Styles" menu
    setupStyleActions();

    // Add default filter
    m_fileFilters << "All Files *.* (*.*)";

    // lets load the plugins
    m_pluginsManager->loadPlugins();

    // Setup the MRU list
    m_recentFileSeparator = ui->menuRecentFiles->insertSeparator(ui->actionClearRecent);

    for (int i = 0; i < MAXRECENT; ++i)
    {
        QAction* act = new QAction(this);
        act->setText(tr("&%1 %2").arg(i + 1).arg("test"));
        act->setVisible(false);
        connect(act, SIGNAL(triggered()), this, SLOT(openRecentFile()));
        m_recentFileActions.append(act);
        ui->menuRecentFiles->insertAction(m_recentFileSeparator, m_recentFileActions[i]);
    }

    // On preview and builds we inject a label into the menu bar to inform the user
#if defined(WK2_PREVIEW) || defined(WK2_INTERNAL)
    QMenuBar* bar = this->menuBar();
    m_previewLayout = new QHBoxLayout(bar);
    m_previewLayout->addStretch();
    m_previewLabel  = new QLabel(bar);
    m_previewLabel->setObjectName("previewLabel");
#ifdef WK2_PREVIEW
    m_previewLabel->setText("PREVIEW BUILD");
#elif defined(WK2_INTERNAL)
    m_previewLabel->setText("INTENAL BUILD");
#endif
    m_previewLayout->setContentsMargins(150, 0, 6, 0);
    m_previewLayout->addWidget(m_previewLabel);
    bar->setLayout(m_previewLayout);
#endif
    // Now load the MRU
    updateRecentFileActions();

    // Restore window from saved states
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

    connect(&m_updateAccess, SIGNAL(finished(QNetworkReply*)), this, SLOT(onNetworkFinished(QNetworkReply*)));

    // Hide the toolbar if it has no actions
    ui->mainToolBar->setVisible((ui->mainToolBar->actions().count() > 0 ? true : false));
}

MainWindow::~MainWindow()
{
#if defined(WK2_PREVIEW) || defined(WK2_INTERNAL)
    if (m_previewLayout)
    {
        delete m_previewLayout;
        m_previewLayout = NULL;
    }

    if (m_previewLabel)
    {
        delete m_previewLabel;
        m_previewLabel = NULL;
    }
    else
        qDebug() << "alreadyDeleted";
#endif
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    onCloseAll();
    delete ui;
}

void MainWindow::addFileFilter(const QString& filter)
{
    if (m_fileFilters.contains(filter))
        return;

    m_fileFilters << filter;
}

void MainWindow::removeFileFilter(const QString& filter)
{
    if (!m_fileFilters.contains(filter))
        return;

    m_fileFilters.removeOne(filter);
}

void MainWindow::closeFilesFromLoader(PluginInterface* loader)
{
    QList<GameFile*> targets;
    foreach(GameFile* file, m_documents.values())
    {
        if (file->loadedBy() == loader)
        {
            targets.append(file);
            for (int i = 0; i < ui->documentList->count(); i++)
            {
                qDebug() << "Widget: " << ui->documentList->item(i)->data(FILEPATH).toString();
                qDebug() << "File: " << file->filePath();
                if (cleanPath(ui->documentList->item(i)->data(FILEPATH).toString()) == cleanPath(file->filePath()))
                {
                    delete ui->documentList->item(i);
                    break;
                }
            }
        }
    }

    foreach(GameFile* file, targets)
    {
        m_documents.remove(cleanPath(file->filePath()));
        delete file;
    }
    targets.clear();
}

QString MainWindow::cleanPath(const QString& currentFile)
{
    QString filePath = currentFile;
    filePath = filePath.replace("\\", "/");
#ifdef Q_OS_WIN
    filePath = filePath.toLower();
#endif

    return filePath;
}

void MainWindow::openFile(const QString& currentFile)
{
    // Change any '\' to '/' for maximum compatibility
    QString filePath = cleanPath(currentFile);

    if (m_documents.contains(filePath))
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(Constants::WIIKING2_ALREADY_OPENED_NAG);
        msgBox.setText(Constants::WIIKING2_ALREADY_OPENED_NAG_MSG.arg(strippedName(filePath)));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
    PluginInterface* loader = m_pluginsManager->preferredPlugin(filePath);
    if (!loader)
        return;

    GameFile* file = loader->loadFile(filePath);

    // TODO: Show message upon failure
    if (!file)
        return;
    connect(file, SIGNAL(modified()), this, SLOT(updateWindowTitle()));
    m_documents[filePath] = file;

    QListWidgetItem* item = new QListWidgetItem();
    item->setData(FILENAME, m_documents[filePath]->fileName());
    item->setData(FILEPATH, filePath);
    item->setText(item->data(FILENAME).toString());
    if (!loader->icon().isNull())
        item->setIcon(loader->icon());

    ui->documentList->addItem(item);
    ui->documentList->setCurrentItem(item);
    m_currentFile = file;
    updateMRU(filePath);
    QSettings settings;
    settings.setValue(Constants::Settings::WIIKING2_RECENT_DIRECTORY, cleanPath(QFileInfo(filePath).absolutePath()));
    updateWindowTitle();
}

void MainWindow::onDocumentChanged()
{
    if (!ui->documentList->currentItem())
    {
        m_currentFile = NULL;
        return;
    }

    // First we need to store the old Widget so we can remove
    // it from the main layout.
    GameFile* oldFile = m_currentFile;
    qDebug() << ui->documentList->currentItem()->data(FILEPATH).toString();
    m_currentFile = m_documents[cleanPath(ui->documentList->currentItem()->data(FILEPATH).toString())];

    if (!m_currentFile)
        return;

    if (oldFile)
    {
        if (oldFile->widget())
        {
            ui->mainLayout->removeWidget(oldFile->widget());
            oldFile->widget()->hide();
        }
    }

    if (m_currentFile->widget())
    {
        ui->mainLayout->addWidget(m_currentFile->widget());
        m_currentFile->widget()->show();
    }

    updateWindowTitle();
}


void MainWindow::onClose()
{
    if (m_documents.count() <= 0)
        return;

    qDebug() << "Closing file" << cleanPath(m_currentFile->filePath());
    m_documents.remove(cleanPath(m_currentFile->filePath()));
    delete m_currentFile;
    m_currentFile = NULL;
    delete ui->documentList->currentItem();

    if (ui->documentList->count() <= 0)
        this->setWindowTitle(Constants::WIIKING2_TITLE);
}

void MainWindow::onCloseAll()
{
    while (m_documents.count() > 0)
        onClose();
}

void MainWindow::onOpen()
{
    QString MRD = mostRecentDirectory();
    QString file = QFileDialog::getOpenFileName(this, "Open file...", MRD, m_fileFilters.join(";;").trimmed());

    if (!file.isEmpty())
    {
        openFile(cleanPath(file));
    }
}

void MainWindow::onSave()
{

#ifdef WK2_PREVIEW
    QMessageBox mbox(this);
    mbox.setWindowTitle("Saving disabled");
    mbox.setText("<center>Saving has been disabled in this preview build.<br />"
                 "The application is far too unstable for everyday use.<br />"
                 "<br />"
                 "But your interest and support are much appreciated. <br />"
                 "<br />"
                 "<strong>Thank you for checking out this preview build.<strong></center>");
    mbox.setStandardButtons(QMessageBox::Ok);
    mbox.exec();
    return;
#endif

    if (m_documents.count() <= 0)
        return;
    if (!m_currentFile || !m_currentFile->isDirty())
        return;

    if (m_currentFile->filePath().isEmpty())
        onSaveAs();

    if (m_currentFile->save())
        statusBar()->showMessage(tr("Save successful"), 2000);
    else
        statusBar()->showMessage(tr("Save failed"), 2000);

    updateWindowTitle();
}

void MainWindow::onSaveAs()
{
#ifdef WK2_PREVIEW
    QMessageBox mbox;
    mbox.setWindowTitle("Saving disabled");
    mbox.setText("<center>Saving has been disabled in this preview build.<br />"
                 "The application is far too unstable for everyday use.<br />"
                 "<br />"
                 "But your interest and support are much appreciated. <br />"
                 "<br />"
                 "<strong>Thank you for checking out this preview build.<strong></center>");
    mbox.setStandardButtons(QMessageBox::Ok);
    mbox.exec();
    return;
#endif
    if (m_documents.count() <= 0)
        return;
    if (!m_currentFile)
        return;
    QString MRD = mostRecentDirectory();
    QString file = QFileDialog::getSaveFileName(this, "Save file as...", MRD, m_fileFilters.join(";;").trimmed());
    cleanPath(file);
    if (cleanPath(m_currentFile->filePath()) != file)
    {
        m_documents.remove(cleanPath(m_currentFile->filePath()));
        m_currentFile->save(file);
        file = cleanPath(m_currentFile->filePath());
        m_documents[file] = m_currentFile;
        QListWidgetItem* item = ui->documentList->currentItem();
        item->setData(FILENAME, cleanPath(m_currentFile->fileName()));
        item->setData(FILEPATH, file);
        item->setText(item->data(FILENAME).toString());
    }
    else
    {
        m_currentFile->save();
    }

    updateWindowTitle();
}

void MainWindow::onExit()
{
    qApp->quit();
}

void MainWindow::onAbout()
{
    if (!m_aboutDialog)
        m_aboutDialog = new AboutDialog(this);

    m_aboutDialog->exec();
}

void MainWindow::onAboutQt()
{
    qApp->aboutQt();
}

void MainWindow::updateMRU(const QString& file)
{
    QSettings settings;
    QStringList files = settings.value(Constants::Settings::WIIKING2_RECENT_FILES).toStringList();
    files.removeAll(cleanPath(file));
    files.prepend(cleanPath(file));

    while (files.size() > MAXRECENT)
        files.removeLast();

    settings.setValue(Constants::Settings::WIIKING2_RECENT_FILES, files);
    updateRecentFileActions();

    foreach (QWidget* widget, QApplication::topLevelWidgets())
    {
        MainWindow* mainWin = qobject_cast<MainWindow*>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value(Constants::Settings::WIIKING2_RECENT_FILES).toStringList();

    int numRecentFiles = qMin(files.size(), (int)MAXRECENT);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i+1).arg(strippedName(files[i]));
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(files[i]);
        m_recentFileActions[i]->setVisible(true);
        m_recentFileActions[i]->setStatusTip(files[i]);
    }

    for (int j = numRecentFiles; j < MAXRECENT; ++j)
        m_recentFileActions[j]->setVisible(false);

    m_recentFileSeparator->setVisible(true);
}

void MainWindow::setupStyleActions()
{
    if (!QSettings().value(Constants::Settings::WIIKING2_DEFAULT_STYLE).isValid())
    {
        if(qApp->style())
            QSettings().setValue(Constants::Settings::WIIKING2_DEFAULT_STYLE, qApp->style()->objectName());
        else
            QSettings().setValue(Constants::Settings::WIIKING2_DEFAULT_STYLE, qApp->desktop()->style()->objectName());
    }
    QStringList styles = QStyleFactory::keys();
    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionDefaultStyle);

    QString currentStyle = QSettings().value(Constants::Settings::WIIKING2_CURRENT_STYLE).toString();
    qApp->setStyle(currentStyle);

    foreach (QString style, styles)
    {
        QAction* a = ui->menuStyles->addAction(style);
        a->setCheckable(true);
        if (!QString::compare(currentStyle, style, Qt::CaseInsensitive))
            a->setChecked(true);

        a->setObjectName(style.toLower() + "Action");
        a->setActionGroup(actionGroup);
        connect(a, SIGNAL(triggered()), this, SLOT(onStyleChanged()));
    }
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (action)
        openFile(action->data().toString());
}

void MainWindow::updateWindowTitle()
{
    if (m_currentFile->isDirty() && !ui->documentList->currentItem()->text().contains("*"))
        ui->documentList->currentItem()->setText(ui->documentList->currentItem()->text() + "*");
    else if (!m_currentFile->isDirty() && ui->documentList->currentItem()->text().contains("*"))
    {
        ui->documentList->currentItem()->setText(ui->documentList->currentItem()->data(FILENAME).toString());
    }

    setWindowTitle(Constants::WIIKING2_TITLE_FILE.arg(m_currentFile->fileName()).arg(m_currentFile->isDirty() ? Constants::WIIKING2_TITLE_DIRTY : ""));
}

void MainWindow::showEvent(QShowEvent* se)
{
    QMainWindow::showEvent(se);

    if (m_pluginsManager->plugins().count() <= 0)
    {
        QMessageBox mbox;
        mbox.setWindowTitle(Constants::WIIKING2_NO_PLUGINS_ERROR);
        mbox.setText(Constants::WIIKING2_NO_PLUGINS_ERROR_MSG);
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();
        exit(1);
    }
}

QUrl MainWindow::redirectUrl(const QUrl& possibleRedirect, const QUrl& oldRedirect) const
{
    if (!possibleRedirect.isEmpty() && possibleRedirect != oldRedirect)
        return possibleRedirect;

    return QUrl();

}

void MainWindow::onPlugins()
{
    m_pluginsManager->dialog();
}

void MainWindow::onClearRecent()
{
    QSettings settings;
    settings.setValue(Constants::Settings::WIIKING2_RECENT_FILES, QVariant());

    updateRecentFileActions();
}

void MainWindow::onRestoreDefault()
{
    this->restoreState(m_defaultWindowState);
    this->restoreGeometry(m_defaultWindowGeometry);
}

void MainWindow::onCheckUpdate()
{
    m_updateAccess.get(QNetworkRequest(QUrl("http://update.wiiking2.com/wiiking2_editorv2/latest.update")));
}

void MainWindow::onNetworkFinished(QNetworkReply* nr)
{
    QMessageBox msgBox(this);
    QUrl possibleRedirect = nr->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    QUrl redirect = redirectUrl(possibleRedirect, nr->url());
    qDebug() << redirect;
    if (redirect.isEmpty() && nr->isReadable())
    {
        QStringList data = QString(nr->readAll()).split("\n");
        if (data[0].contains("version"))
        {
            QString serverVersion = data[0].split("=")[1];
            if (!QString::compare(serverVersion, Constants::WIIKING2_APP_VERSION, Qt::CaseInsensitive))
            {
                msgBox.setWindowTitle(Constants::WIIKING2_LATEST_VERSION);
                msgBox.setText(Constants::WIIKING2_LATEST_VERSION_MSG);
                msgBox.exec();
            }
            else
            {
                QString url;
                QString platform;
#ifdef QT_ARCH_X86_64
#ifdef Q_OS_LINUX
                platform = "unix-x64";
#elif Q_OS_WIN32
                platform = "win32";
#endif
#else
#ifdef Q_OS_LINUX
                platform = "unix-x86";
#elif defined(Q_OS_WIN32)
                platform = "win32";
#endif
#endif
                if (platform.isEmpty())
                    return;

                if (data.filter(platform).count() <= 0)
                    return;

                url = data.filter(platform)[0];
                if (url.contains("="))
                    url = url.split("=")[1];

                QString changeLog;
                if (data.filter("changelog").count() > 0)
                {
                    QString rawString = data.filter("changelog")[0];
                    if (rawString.contains("="))
                        changeLog = rawString.split("=")[1];
                }

                msgBox.setWindowTitle(Constants::WIIKING2_NOT_LATEST_VERSION.arg(Constants::WIIKING2_TITLE));
                msgBox.setText(Constants::WIIKING2_NOT_LATEST_VERSION_MSG.arg(Constants::WIIKING2_TITLE).arg(url).arg(changeLog));
                msgBox.exec();
            }
        }
        else
        {
            msgBox.setWindowTitle(Constants::WIIKING2_UPDATE_ERROR);
            msgBox.setText(Constants::WIIKING2_UPDATE_ERROR_MSG.arg(Constants::WIIKING2_TITLE));
            msgBox.exec();
        }
        qDebug() << data;
    }
    else
    {
        m_updateAccess.get(QNetworkRequest(redirect));
    }
}

void MainWindow::onStyleChanged()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a)
    {
        QString style = a->text().toLower();
        a->setChecked(true);
        if (style.contains("default"))
        {
            QString tmp = QSettings().value(Constants::Settings::WIIKING2_DEFAULT_STYLE).toString();
            qApp->setStyle(tmp);
            foreach (QAction* action, a->actionGroup()->actions())
            {
                if (action->text().toLower() == tmp.toLower())
                {
                    action->setChecked(true);
                    break;
                }
            }
        }
        else
            qApp->setStyle(a->text());
        QSettings().setValue(Constants::Settings::WIIKING2_CURRENT_STYLE, (style.contains("default") ?
                                                                               QSettings().value(Constants::Settings::WIIKING2_DEFAULT_STYLE).toString()
                                                                             : style));
    }
}

QString MainWindow::strippedName(const QString& fullFileName) const
{
    return QFileInfo(fullFileName).fileName();
}

QString MainWindow::mostRecentDirectory()
{
    QSettings settings;
    return settings.value(Constants::Settings::WIIKING2_RECENT_DIRECTORY).toString();
}
