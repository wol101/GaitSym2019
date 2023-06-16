/*
 *  Preferences.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 12/02/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QColor>
#include <QString>
#include <QByteArray>
#include <QVector2D>
#include <QVector3D>
#include <QVariantMap>
#include <QVariant>
#include <QMap>
#include <QDomElement>

class QSettings;

struct SettingsItem
{
    SettingsItem()
    {
        display = false;
        type = QMetaType::QVariant;
    }
    QString key;
    QString label;
    QVariant value;
    QVariant defaultValue;
    QVariant minimumValue;
    QVariant maximumValue;
    bool display;
    QMetaType::Type type;
};

class Preferences
{
public:
    static void Read();
    static void Write();
    static void Export(const QString &filename);
    static void Import(const QString &filename);
    static QByteArray ExportData();
    static int ImportData(const QByteArray &xmlData);
    static void LoadDefaults();

    static const SettingsItem settingsItem(const QString &key);
    static const QVariant valueQVariant(const QString &key);
    static const QString valueQString(const QString &key);
    static const QColor valueQColor(const QString &key);
    static const QFont valueQFont(const QString &key);
    static const QByteArray valueQByteArray(const QString &key);
    static const QStringList valueQStringList(const QString &key);
    static const QVector2D valueQVector2D(const QString &key);
    static const QVector3D valueQVector3D(const QString &key);
    static double valueDouble(const QString &key);
    static float valueFloat(const QString &key);
    static int valueInt(const QString &key);
    static bool valueBool(const QString &key);
    static const QVariant valueQVariant(const QString &key, const QVariant &defaultValue);
    static const QString valueQString(const QString &key, const QString &defaultValue);
    static const QColor valueQColor(const QString &key, const QColor &defaultValue);
    static const QFont valueQFont(const QString &key, const QFont &defaultValue);
    static const QByteArray valueQByteArray(const QString &key, const QByteArray &defaultValue);
    static const QStringList valueQStringList(const QString &key, const QStringList &defaultValue);
    static const QVector2D valueQVector2D(const QString &key, const QVector2D &defaultValue);
    static const QVector3D valueQVector3D(const QString &key, const QVector3D &defaultValue);
    static double valueDouble(const QString &key, double defaultValue);
    static float valueFloat(const QString &key, float defaultValue);
    static int valueInt(const QString &key, int defaultValue);
    static bool valueBool(const QString &key, bool defaultValue);

    static void insert(const SettingsItem &item);
    static void insert(const QString &key, const QVariant &value);
    static void insert(const QString &key, const QVariant &value, const QVariant &defaultValue);

    static bool contains(const QString &key);
    static QStringList keys();
    static QString fileName();

    static const QString &getApplicationName();
    static const QString &getOrganizationName();

private:

    static QString serialize(const QStringList &data);
    static void deserialize(const QString &qString, QStringList *data);

    static QMap<QString, SettingsItem> m_settings;
    static QSettings m_qtSettings;

    static const QString applicationName;
    static const QString organizationName;

    static void ParseQDomElement(const QDomElement &docElem);

    static void setQtValue(const QString &key, const QVariant &value);
    static QVariant qtValue(const QString &key, const QVariant &defaultValue);

    static void clear();
    static void sync();

    static QStringList allKeys();

    static bool toBool(const QString &string);

};

#endif // PREFERENCES_H
