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

#ifndef SKYWARDSWORDGAMEFILE_HPP
#define SKYWARDSWORDGAMEFILE_HPP

#include <GameDocument.hpp>
#include <QObject>

class CopyWidget;
class SkywardSwordEditorForm;
namespace zelda
{
namespace io
{
class BinaryReader;
}
}

class SkywardSwordGameDocument : public GameDocument
{
    Q_OBJECT
public:
    SkywardSwordGameDocument(const PluginInterface* loader, const QString& file = QString());
    ~SkywardSwordGameDocument();
    QString game() const;
    bool save(const QString &filename = QString());

    bool loadFile();

    bool reload();
    bool supportsWiiSave() const;
    bool exportWiiSave();
private slots:
    void onModified();
    void onCopy(SkywardSwordEditorForm* source);
    void onTabMoved(int left, int right);
private:
    bool loadData(zelda::io::BinaryReader reader);
    char*       m_skipData;
    char        m_region;
    CopyWidget* m_copyWidget;
};

#endif // SKYWARDSWORDGAMEFILE_HPP
