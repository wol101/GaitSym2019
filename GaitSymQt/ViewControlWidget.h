/*
 *  ViewControlWidget.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef VIEWCONTROLWIDGET_H
#define VIEWCONTROLWIDGET_H

#include <QWidget>

class QPixmap;

class ViewControlWidget : public QWidget
{
    Q_OBJECT

public:
    ViewControlWidget(QWidget *parent = nullptr);
    virtual ~ViewControlWidget() Q_DECL_OVERRIDE;

    int FindClosestVertex(const double data[][3], int count, double x, double y, double z);

public slots:

signals:
    void EmitCameraVec(double x, double y, double z);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent (QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QPixmap *backgroundImage;
    QPixmap *blob;

    double lastX;
    double lastY;
    double lastZ;
};

#endif // VIEWCONTROLWIDGET_H
