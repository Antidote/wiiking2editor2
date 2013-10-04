#include "PreferencesDialog.hpp"
#include "ui_PreferencesDialog.h"
#include "Constants.hpp"
#include "WiiKeyManager.hpp"

#include <QSettings>
#include <QFile>
#include <QDateTime>
#include <QStyleFactory>
#include <QApplication>
#include <QMessageBox>
#include <QUrl>
#include <QFileDialog>

class HexValidator : public QValidator
{
public:
    HexValidator(QObject* parent = 0) :
        QValidator(parent)
    {}
protected:
    QValidator::State validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos)
        input = input.toUpper();
        // match against needed regexp
        QRegExp rx("(?:[0-9a-fA-F]{2})*[0-9a-fA-F]{0,2}");
        if (rx.exactMatch(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }
};

PreferencesDialog::PreferencesDialog(WiiKeyManager* keyManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    m_currentChanged(false),
    m_defaultChanged(false),
    m_singleInstance(false),
    m_keyManager(keyManager)
{
    ui->setupUi(this);
    // Key Settings
    ui->ngIDLineEdit->setValidator(new HexValidator(this));
    ui->ngKeyIDLineEdit->setValidator(new HexValidator(this));
    ui->ngSigPt1LineEdit->setValidator(new HexValidator(this));
    ui->ngSigPt2LineEdit->setValidator(new HexValidator(this));
    ui->ngPrivLineEdit->setValidator(new HexValidator(this));
    ui->macAddrLineEdit->setValidator(new HexValidator(this));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::showEvent(QShowEvent* se)
{
    QDialog::showEvent(se);
    QSettings settings;

    m_currentStyle = settings.value(Constants::Settings::SAKURASUITE_CURRENT_STYLE).toString();
    m_defaultStyle = settings.value(Constants::Settings::SAKURASUITE_DEFAULT_STYLE).toString();

    updateKeys();

    m_singleInstance = settings.value("singleInstance").toBool();

    ui->checkOnStart->setChecked(settings.value(Constants::Settings::SAKURASUITE_CHECK_ON_START, false).toBool());

    int index = 0;
    this->setUpdatesEnabled(false);
    foreach(QString style, QStyleFactory::keys())
    {
        ui->currentStyleCombo->addItem(style);
        ui->defaultStyleCombo->addItem(style);
        if (!QString::compare(style, m_currentStyle, Qt::CaseInsensitive))
            ui->currentStyleCombo->setCurrentIndex(index);
        if (!QString::compare(style, m_defaultStyle, Qt::CaseInsensitive))
            ui->defaultStyleCombo->setCurrentIndex(index);
        index++;
    }

    ui->updateUrlLineEdit->setText(settings.value(Constants::Settings::SAKURASUITE_UPDATE_URL, Constants::Settings::SAKURASUITE_UPDATE_URL_DEFAULT).toString());
    //ui->updateUrlLineEdit->setModified(false);
    ui->singleInstanceCheckBox->setChecked(QSettings().value("singleInstance", false).toBool());
    this->setUpdatesEnabled(true);
}

void PreferencesDialog::accept()
{
    QSettings settings;

    // Check the update url first, so we can bail out if it's invalid
    if (ui->updateUrlLineEdit->isModified() && !ui->updateUrlLineEdit->text().isEmpty())
    {
        if (ui->updateUrlLineEdit->property("valid").toBool())
            settings.setValue(Constants::Settings::SAKURASUITE_UPDATE_URL, ui->updateUrlLineEdit->text());
        else
        {
            QMessageBox mbox(this);
            mbox.setWindowTitle("Invalid update url...");
            mbox.setText("The url you specified is not valid.<br />"
                         "Please check it and make the necessary corrections.<br />"
                         "All urls must start with http:// or https://");
            mbox.setStandardButtons(QMessageBox::Ok);
            mbox.exec();
            return;
        }
    }


    if (m_currentChanged)
        settings.setValue(Constants::Settings::SAKURASUITE_CURRENT_STYLE, ui->currentStyleCombo->currentText());
    if (m_defaultChanged)
        settings.setValue(Constants::Settings::SAKURASUITE_DEFAULT_STYLE, ui->defaultStyleCombo->currentText());

    qApp->setStyle(ui->currentStyleCombo->currentText());

    if (ui->ngIDLineEdit->hasAcceptableInput() && ui->ngIDLineEdit->text().length() == 8)
    {
        quint32 id;
        bool ok;
        id = ui->ngIDLineEdit->text().toInt(&ok, 16);
        if (ok)
            m_keyManager->setNGId(id);
    }
    if (ui->ngKeyIDLineEdit->hasAcceptableInput() && ui->ngKeyIDLineEdit->text().length() == 8)
    {
        quint32 id;
        bool ok;
        id = ui->ngKeyIDLineEdit->text().toInt(&ok, 16);
        if (ok)
            m_keyManager->setNGKeyId(id);
    }
    if (ui->ngSigPt1LineEdit->hasAcceptableInput() && ui->ngSigPt2LineEdit->hasAcceptableInput() &&
        ui->ngSigPt1LineEdit->text().length() == 60 && ui->ngSigPt1LineEdit->text().length() == 60)
    {
        QByteArray ngSig = QByteArray::fromHex(ui->ngSigPt1LineEdit->text().toStdString().c_str());
        ngSig.append(QByteArray::fromHex(ui->ngSigPt2LineEdit->text().toStdString().c_str()));
        m_keyManager->setNGSig(ngSig);
    }

    if (ui->ngPrivLineEdit->hasAcceptableInput() && ui->ngPrivLineEdit->text().length() == 60)
    {
        QByteArray ngPriv = QByteArray::fromHex(ui->ngPrivLineEdit->text().toStdString().c_str());
        m_keyManager->setNGPriv(ngPriv);
    }
    if (ui->macAddrLineEdit->hasAcceptableInput() && ui->macAddrLineEdit->text().length() == 12)
    {
        QByteArray macAddr = QByteArray::fromHex(ui->macAddrLineEdit->text().toStdString().c_str());
        m_keyManager->setMacAddr(macAddr);
    }

    settings.setValue(Constants::Settings::SAKURASUITE_CHECK_ON_START, ui->checkOnStart->isChecked());
    settings.setValue("singleInstance", m_singleInstance);

    if (m_singleInstance && !QFile::exists(Constants::SAKURASUITE_LOCK_FILE))
    {
        QFile file(Constants::SAKURASUITE_LOCK_FILE);
        if (file.open(QFile::WriteOnly))
        {
            file.seek(0);
            file.write(QString(QDateTime::currentDateTime().toString() + "\n").toLatin1());
            file.resize(file.pos());
        }
    }

    QDialog::accept();
}

void PreferencesDialog::reject()
{
    qApp->setStyle(m_currentStyle);

    QDialog::reject();
}

void PreferencesDialog::onCurrentIndexChanged(QString style)
{
    if (!this->updatesEnabled())
        return;

    if (sender() == ui->currentStyleCombo)
    {
        m_currentChanged = true;
        qApp->setStyle(style);
    }
    else if (sender() == ui->defaultStyleCombo)
    {
        m_defaultChanged = true;
    }
}

void PreferencesDialog::onTextChanged(QString text)
{
    QRegExp http("^(http|https)://", Qt::CaseInsensitive);
    if (sender() == ui->updateUrlLineEdit)
    {
        if (text.isEmpty() || !text.contains(http))
        {
            ui->updateUrlLineEdit->setProperty("valid", false);
            ui->statusLabel->setText("Invalid url");
        }
        else
        {
            ui->updateUrlLineEdit->setProperty("valid", true);
            ui->statusLabel->clear();
        }
        style()->unpolish(ui->updateUrlLineEdit);
        style()->polish(ui->updateUrlLineEdit);

        // If the text matches what is currently stored
        // Set the line edit is unmodified
        if (text == QSettings().value(Constants::Settings::SAKURASUITE_UPDATE_URL, Constants::Settings::SAKURASUITE_UPDATE_URL).toString())
            ui->updateUrlLineEdit->setModified(false);
    }
}

void PreferencesDialog::onSingleInstanceToggled(bool checked)
{
    m_singleInstance = checked;
}

void PreferencesDialog::onLoadKeys()
{
    QFileInfo finfo(qApp->applicationDirPath() + "/keys.bin");
    if (!finfo.exists())
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Load Keys", qApp->applicationDirPath(), "BootMii keys.bin (*.bin)");

        if (!fileName.isEmpty())
        {

            if (m_keyManager->open(fileName, true))
                updateKeys();
        }
    }
}

void PreferencesDialog::onLoadMac()
{
    QFileInfo finfo(qApp->applicationDirPath() + "/mac.txt");
    if (!finfo.exists())
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Load Keys", qApp->applicationDirPath(), "Mac address (*.txt *.bin)");

        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (file.open(QFile::ReadOnly))
            {
                QByteArray array = file.readLine(12+5);
                QByteArray mac;
                if (array.contains(':'))
                {
                    char* str = (char*)QString(array).toStdString().c_str();
                    int* tmp = new int[6];
                    sscanf(str, "%x:%x:%x:%x:%x:%x", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);

                    mac.push_back((char)tmp[0]);
                    mac.push_back((char)tmp[1]);
                    mac.push_back((char)tmp[2]);
                    mac.push_back((char)tmp[3]);
                    mac.push_back((char)tmp[4]);
                    mac.push_back((char)tmp[5]);
                    delete[] tmp;

                    file.close();
                    m_keyManager->setMacAddr(mac);
                    this->hide();
                    this->show();
                    return;
                }
                else
                {
                    QByteArray macArray = QByteArray::fromHex(array);
                    if (macArray.size() == 6)
                    {
                        file.close();
                        m_keyManager->setMacAddr(macArray);
                        updateKeys();
                    }
                    else
                    {
                        file.close();
                        return;
                    }
                    return;
                }
            }
        }
    }
}

void PreferencesDialog::updateKeys()
{
    ui->ngIDLineEdit->setText(QString(QByteArray::fromHex(QString::number(m_keyManager->ngId(), 16).toStdString().c_str()).toHex()));
    ui->ngKeyIDLineEdit->setText(QString(QByteArray::fromHex(QString::number(m_keyManager->ngKeyId(), 16).toStdString().c_str()).toHex()));
    ui->ngSigPt1LineEdit->setText(m_keyManager->ngSig().remove(30, 30).toHex());
    ui->ngSigPt2LineEdit->setText(m_keyManager->ngSig().mid(30).toHex());
    ui->ngPrivLineEdit->setText(m_keyManager->ngPriv().toHex());
    ui->macAddrLineEdit->setText(m_keyManager->macAddr().toHex());
}
