// This file is part of Sakura Suite.
//
// Sakura Suite is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Sakura Suite is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Sakura Suite.  If not, see <http://www.gnu.org/licenses/>

#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"
#include "Constants.hpp"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(Constants::SAKURASUITE_ABOUT_TITLE);
    ui->applicationNameLbl->setText(Constants::SAKURASUITE_APP_NAME);
    ui->applicationVersionLbl->setText(Constants::SAKURASUITE_APP_VERSION);
    ui->buildDateLbl->setText(Constants::SAKURASUITE_BUILD_DATE);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
