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
#include <QVariantMap>
#include <QVariant>
#include <QMap>
#include <QDomElement>

class QSettings;

struct SettingsItem
{
    SettingsItem()
    {
        order = -1;
        display = false;
        type = QMetaType::QVariant;
    }
    QString key;
    QString label;
    QVariant value;
    QVariant defaultValue;
    QVariant minimumValue;
    QVariant maximumValue;
    int order;
    bool display;
    QMetaType::Type type;
};

class Preferences
{
public:
    Preferences();
    virtual ~Preferences();

    static void Read();
    static void Write();
    static void Export(const QString &filename);
    static void Import(const QString &filename);
    static void LoadDefaults();

    static const SettingsItem settingsItem(const QString &key);
    static const QVariant valueQVariant(const QString &key);
    static const QString valueQString(const QString &key);
    static const QColor valueQColor(const QString &key);
    static const QByteArray valueQByteArray(const QString &key);
    static double valueDouble(const QString &key);
    static float valueFloat(const QString &key);
    static int valueInt(const QString &key);
    static bool valueBool(const QString &key);

    static void insert(const SettingsItem &item);
    static void insert(const QString &key, const QVariant &value);
    static void value(const QString &key, QVariant *value);
    static void value(const QString &key, QString *value);
    static void value(const QString &key, QColor *value);
    static void value(const QString &key, QByteArray *value);
    static void value(const QString &key, double *value);
    static void value(const QString &key, float *value);
    static void value(const QString &key, int *value);
    static void value(const QString &key, bool *value);

    static bool contains(const QString &key);
    static QStringList keys();
    static QString fileName();

private:

    static QMap<QString, SettingsItem> m_settings;
    static QSettings *m_qtSettings;

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
