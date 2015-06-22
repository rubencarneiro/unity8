/*
 * Copyright (C) 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMEZONEMODEL_H
#define TIMEZONEMODEL_H

#include <QAbstractListModel>

class TimeZoneModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedZoneId READ selectedZoneId WRITE setSelectedZoneId NOTIFY selectedZoneIdChanged)
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        Abbreviation,
        Country,
        City,
        Comment
    };

    explicit TimeZoneModel(QObject *parent = nullptr);
    ~TimeZoneModel() = default;

    QString selectedZoneId() const;
    void setSelectedZoneId(const QString &selectedZoneId);

protected:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void selectedZoneIdChanged(const QString &selectedZoneId);

private:
    void init();
    QByteArrayList m_zoneIds;
    QString m_selectedZoneId;
};

#endif
