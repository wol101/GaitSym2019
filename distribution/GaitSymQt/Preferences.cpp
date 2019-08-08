/*
 *  Preferences.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 12/02/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include "Preferences.h"

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QDomDocument>
#include <QSettings>

#include <cfloat>

const QString Preferences::applicationName("GaitSym2019");
const QString Preferences::organizationName("AnimalSimulationLaboratory");
QSettings *Preferences::m_qtSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
        Preferences::organizationName, Preferences::applicationName);
QMap<QString, SettingsItem> Preferences::m_settings;

Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

void Preferences::Write()
{
    Preferences::clear();
    for (QMap<QString, SettingsItem>::const_iterator i = m_settings.constBegin();
            i != m_settings.constEnd(); i++)
    {
        qDebug("%s: %s", qUtf8Printable(i.key()), qUtf8Printable(i.value().value.toString()));
        Preferences::setQtValue(i.key(), i.value().value);
    }
    Preferences::sync();
}

void Preferences::Read()
{
    qDebug() << "Preferences::Read() fileName = " << fileName();
    LoadDefaults();
    // check whether the settings are the right ones
    if (Preferences::qtValue("SettingsCode", QString()) != m_settings["SettingsCode"].value)
    {
        Write();
    }
    else
    {
        QStringList keys = Preferences::allKeys();
        for (int i = 0; i < keys.size(); i++) insert(keys[i], Preferences::qtValue(keys[i], QVariant()));
    }
}

void Preferences::Export(const QString &filename)
{
    QDomDocument doc("GaitSym2019Preferences");
    doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    QDomElement root = doc.createElement("PREFERENCES");
    doc.appendChild(root);

    // this bit of code gets the SettingsItems sorted by order
    QStringList keys = m_settings.keys();
    keys.sort(Qt::CaseInsensitive);
    QMultiMap<int, SettingsItem> sortedItems;
    for (int i = 0; i < keys.size(); i++)
    {
        SettingsItem item = m_settings[keys[i]];
        sortedItems.insert(item.order, item);
    }

    for (QMultiMap<int, SettingsItem>::const_iterator i = sortedItems.constBegin();
            i != sortedItems.constEnd(); i++)
    {
//        qDebug("%d: %s", i.key(), qUtf8Printable(i.value().value.toString()));
        QDomElement setting = doc.createElement("SETTING");
        root.appendChild(setting);
        setting.setAttribute("key", i.value().key);
        setting.setAttribute("type", i.value().value.typeName());
        setting.setAttribute("display", QString::number(i.value().display));
        setting.setAttribute("label", i.value().label);
        setting.setAttribute("order", QString::number(i.value().order));

        QMetaType::Type type = static_cast<QMetaType::Type>(i.value().value.type());
        switch (type)
        {
        case QMetaType::QByteArray:
            setting.setAttribute("defaultValue",
                                 QString::fromUtf8(i.value().defaultValue.toByteArray().toBase64(QByteArray::Base64UrlEncoding)));
            setting.setAttribute("value", QString::fromUtf8(i.value().value.toByteArray().toBase64(
                                     QByteArray::Base64UrlEncoding)));
            break;
        case QMetaType::QColor:
            setting.setAttribute("defaultValue",
                                 qvariant_cast<QColor>(i.value().defaultValue).name(QColor::HexArgb));
            setting.setAttribute("value", qvariant_cast<QColor>(i.value().value).name(QColor::HexArgb));
            break;
        case QMetaType::Float:
        case QMetaType::Double:
        case QMetaType::Int:
            setting.setAttribute("defaultValue", i.value().defaultValue.toString());
            setting.setAttribute("value", i.value().value.toString());
            setting.setAttribute("minimumValue", i.value().minimumValue.toString());
            setting.setAttribute("maximumValue", i.value().maximumValue.toString());
            break;
        default:
            setting.setAttribute("defaultValue", i.value().defaultValue.toString());
            setting.setAttribute("value", i.value().value.toString());
        }
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Unable to open settings export file: %s", qPrintable(filename));
        return;
    }

    // and now the actual xml doc
    QString xmlString = doc.toString();
    QByteArray xmlData = xmlString.toUtf8();
    qint64 bytesWritten = file.write(xmlData);
    if (bytesWritten != xmlData.size()) qWarning("Unable to write to settings export file: %s",
                qPrintable(filename));
    file.close();
}

void Preferences::Import(const QString &filename)
{
    m_settings.clear();

    QDomDocument doc("GaitSym2019Preferences");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Unable to open settings export file: %s", qPrintable(filename));
        return;
    }
    if (!doc.setContent(&file))
    {
        qWarning("Unable to read settings export file: %s", qPrintable(filename));
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    ParseQDomElement(docElem);
}

void Preferences::ParseQDomElement(const QDomElement &docElem)
{
//    qDebug() << qPrintable(docElem.tagName()) << "\n";
    if (docElem.tagName() != "PREFERENCES")
    {
        qWarning("Unable to find tag PREFERENCES: %s", qPrintable(docElem.tagName()));
        return;
    }

    SettingsItem item;
    QString key;
    QDomNode n = docElem.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull())
        {
            if (e.tagName() == "SETTING")
            {
                item.key = e.attribute("key");
                QMetaType::Type type = static_cast<QMetaType::Type>(QVariant::nameToType(
                                           e.attribute("type").toUtf8()));
                item.display = toBool(e.attribute("display"));
                item.label = e.attribute("label");
                item.order = e.attribute("order").toInt();
                switch (type)
                {
                case QMetaType::QByteArray:
                    item.value = QByteArray::fromBase64(QString(e.attribute("value")).toUtf8(),
                                                        QByteArray::Base64UrlEncoding);
                    item.defaultValue = QByteArray::fromBase64(QString(e.attribute("defaultValue")).toUtf8(),
                                        QByteArray::Base64UrlEncoding);
                    break;
                case QMetaType::QColor:
                    item.value = QColor(e.attribute("value"));
                    item.defaultValue = QColor(e.attribute("defaultValue"));
                    break;
                case QMetaType::Float:
                case QMetaType::Double:
                case QMetaType::Int:
                    item.value = e.attribute("value");
                    item.defaultValue = e.attribute("defaultValue");
                    item.minimumValue = e.attribute("minimumValue");
                    item.maximumValue = e.attribute("maximumValue");
                    item.value.convert(type);
                    item.defaultValue.convert(type);
                    item.minimumValue.convert(type);
                    item.maximumValue.convert(type);
                    break;
                default:
                    item.value = e.attribute("value");
                    item.defaultValue = e.attribute("defaultValue");
                    item.value.convert(type);
                    item.defaultValue.convert(type);
                }
                m_settings[item.key] = item;
            }
        }
        n = n.nextSibling();
    }
}

void Preferences::LoadDefaults()
{
    Import(":/preferences/default_values.xml");

    for (QMap<QString, SettingsItem>::const_iterator i = m_settings.constBegin();
            i != m_settings.constEnd(); i++) insert(i.key(), i.value().defaultValue);
}

bool Preferences::toBool(const QString &string)
{
    if (string.trimmed().compare("true", Qt::CaseInsensitive) == 0 || string.toInt() != 0) return true;
    return false;
}

const SettingsItem Preferences::settingsItem(const QString &key)
{
    return m_settings.value(key);
}

const QVariant Preferences::valueQVariant(const QString &key)
{
    QVariant v;
    if (m_settings.contains(key)) v = m_settings.value(key).value;
    else insert(key, v);
    return v;
}

const QString Preferences::valueQString(const QString &key)
{
    QString v;
    if (m_settings.contains(key)) v = m_settings.value(key).value.toString();
    else insert(key, v);
    return v;
}

const QColor Preferences::valueQColor(const QString &key)
{
    QColor v;
    if (m_settings.contains(key)) v = qvariant_cast<QColor>(m_settings.value(key).value);
    else insert(key, v);
    return v;
}

const QByteArray Preferences::valueQByteArray(const QString &key)
{
    QByteArray v;
    if (m_settings.contains(key)) v = qvariant_cast<QByteArray>(m_settings.value(key).value);
    else insert(key, v);
    return v;
}

double Preferences::valueDouble(const QString &key)
{
    double v = 0.0;
    if (m_settings.contains(key)) v = m_settings.value(key).value.toDouble();
    else insert(key, v);
    return v;
}

float Preferences::valueFloat(const QString &key)
{
    float v = 0.0f;
    if (m_settings.contains(key)) v = m_settings.value(key).value.toFloat();
    else insert(key, v);
    return v;
}

int Preferences::valueInt(const QString &key)
{
    int v = 0;
    if (m_settings.contains(key)) v = m_settings.value(key).value.toInt();
    else insert(key, v);
    return v;
}

bool Preferences::valueBool(const QString &key)
{
    bool v = false;
    if (m_settings.contains(key)) v = m_settings.value(key).value.toBool();
    else insert(key, v);
    return v;
}


void Preferences::insert(const SettingsItem &item)
{
    m_settings.insert(item.key, item);
}

bool Preferences::contains(const QString &key)
{
    return m_settings.contains(key);
}

QStringList Preferences::keys()
{
    return m_settings.keys();
}

void Preferences::insert(const QString &key, const QVariant &value)
{
    SettingsItem item;
    if (m_settings.contains(key))
    {
        item = m_settings.value(key);
        item.value = value;
        m_settings.insert(key, item);
    }
    else
    {
        qDebug("Preferences::value %s \"%s\" missing", value.typeName(), qUtf8Printable(key));
        item.key = key;
        item.display = false;
        item.label = key;
        item.order = -1;
        item.value = value;
        item.type = static_cast<QMetaType::Type>(value.type());
        item.defaultValue = QVariant(value.type());
        switch (item.type)
        {
        case QMetaType::Double:
            item.minimumValue = -DBL_MAX;
            item.maximumValue = DBL_MAX;
            break;
        case QMetaType::Float:
            item.minimumValue = -FLT_MAX;
            item.maximumValue = FLT_MAX;
            break;
        case QMetaType::Int:
            item.minimumValue = INT_MIN;
            item.maximumValue = INT_MAX;
            break;
        default:
            item.minimumValue = QString();
            item.maximumValue = QString();
        }
        m_settings.insert(key, item);
    }
}

void Preferences::setQtValue(const QString &key, const QVariant &value)
{
    Q_ASSERT(m_qtSettings);
    QString newKey1 = QString("%1_key_%2").arg(applicationName).arg(key);
    QString newKey2 = QString("%1_type_%2").arg(applicationName).arg(key);
    m_qtSettings->setValue(newKey1, value);
    m_qtSettings->setValue(newKey2,
                           value.typeName()); // we have to do this because some settings formats lose the explicit type
}

QVariant Preferences::qtValue(const QString &key, const QVariant &defaultValue)
{
    Q_ASSERT(m_qtSettings);
    QVariant variant = defaultValue;
    QString newKey1 = QString("%1_key_%2").arg(applicationName).arg(key);
    QString newKey2 = QString("%1_type_%2").arg(applicationName).arg(key);
    if (m_qtSettings->contains(newKey1))
    {
        variant = m_qtSettings->value(newKey1);
        if (m_qtSettings->contains(newKey2))
        {
            QString typeName = m_qtSettings->value(newKey2).toString();
            variant.convert(QVariant::nameToType(
                                typeName.toUtf8())); // convert to the type stored in the settings
        }
        else
        {
            m_qtSettings->setValue(newKey2, defaultValue.typeName());
        }
    }
    else     // if the setting does not exist, create it and set it to the default value
    {
        setQtValue(key, defaultValue);
    }
    return variant;
}

void Preferences::clear()
{
    Q_ASSERT(m_qtSettings);
    m_qtSettings->clear();
}

void Preferences::sync()
{
    Q_ASSERT(m_qtSettings);
    m_qtSettings->sync();
}

QStringList Preferences::allKeys()
{
    Q_ASSERT(m_qtSettings);
    QStringList keys = m_qtSettings->allKeys();
    QStringList newKeys;
    newKeys.reserve(keys.size());
    QString prefix = QString("%1_key_").arg(applicationName);
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys.at(i).startsWith(prefix) == false) continue;
        QString newKey = keys.at(i).mid(prefix.size());
        newKeys.append(newKey);
    }
    return newKeys;
}

QString Preferences::fileName()
{
    Q_ASSERT(m_qtSettings);
    return m_qtSettings->fileName();
}