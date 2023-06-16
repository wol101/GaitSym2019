/*
 *  DialogProperties.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 16/03/2020.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef DIALOGPROPERTIES_H
#define DIALOGPROPERTIES_H

#include <QDialog>

#include "Preferences.h"

namespace Ui {
class DialogProperties;
}

class DialogProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProperties(QWidget *parent = nullptr);
    ~DialogProperties() Q_DECL_OVERRIDE;

    void initialise();
    void update();

    void setInputSettingsItems(QMap<QString, SettingsItem> &inputSettingsItems);

    QMap<QString, SettingsItem> getOutputSettingsItems() const;


public slots:
    void colourButtonClicked();
    void acceptButtonClicked();
    void rejectButtonClicked();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    struct SettingsWidget
    {
        QWidget *widget;
        SettingsItem item;
    };

    void initialiseTab(const QString &tabName, const QVector<SettingsItem> &settingItems);
    QColor getIdealTextColour(const QColor &rBackgroundColour);
    QColor getAlphaColourHint(const QColor &colour);

    Ui::DialogProperties *ui;

    QMap<QString, SettingsItem> m_inputSettingsItems;
    QMap<QString, SettingsItem> m_outputSettingsItems;
    QList<SettingsWidget> m_SettingsWidgetList;

};

#endif // DIALOGPROPERTIES_H
