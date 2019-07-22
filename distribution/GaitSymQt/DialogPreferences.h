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

class QStatusBar;

struct SettingsWidget
{
    QWidget *widget;
    SettingsItem item;
};

class DialogPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreferences(QWidget *parent = nullptr);
    virtual ~DialogPreferences() Q_DECL_OVERRIDE;

    void initialise();
    void update();

    QColor getIdealTextColour(const QColor &rBackgroundColour);
    QColor getAlphaColourHint(const QColor &colour);

public slots:
    void colourButtonClicked();
    void importButtonClicked();
    void exportButtonClicked();
    void defaultsButtonClicked();
    void acceptButtonClicked();
    void rejectButtonClicked();
    void menuRequestPath(QPoint pos);

private:
    QList<SettingsWidget> m_SettingsWidgetList;
    QStatusBar *m_statusBar;
};

#endif // DIALOGPREFERENCES_H
