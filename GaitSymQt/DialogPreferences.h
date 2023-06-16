/*
 *  DialogPreferences.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 12/02/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef DIALOGPREFERENCES_H
#define DIALOGPREFERENCES_H

#include <QDialog>
#include <QColor>

#include "Preferences.h"

namespace Ui {
class DialogPreferences;
}

class DialogPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreferences(QWidget *parent = nullptr);
    virtual ~DialogPreferences() Q_DECL_OVERRIDE;

    void initialise();
    void update();

public slots:
    void colourButtonClicked();
    void fontButtonClicked();
    void importButtonClicked();
    void exportButtonClicked();
    void defaultsButtonClicked();
    void acceptButtonClicked();
    void rejectButtonClicked();
    void menuRequestPath(QPoint pos);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    struct SettingsWidget
    {
        QWidget *widget;
        SettingsItem item;
    };

    Ui::DialogPreferences *ui;

    QList<SettingsWidget> m_SettingsWidgetList;

    void initialiseTab(const QString &tabName, const QVector<SettingsItem> &settingItems);
    QColor getIdealTextColour(const QColor &rBackgroundColour);
    QColor getAlphaColourHint(const QColor &colour);

};

#endif // DIALOGPREFERENCES_H
