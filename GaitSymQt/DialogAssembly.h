/*
 *  DialogAssembly.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef DIALOGASSEMBLY_H
#define DIALOGASSEMBLY_H

#include <QDialog>

class Simulation;
class Joint;
class QGridLayout;

namespace Ui {
class DialogAssembly;
}

class DialogAssembly : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAssembly(QWidget *parent = nullptr);
    ~DialogAssembly() Q_DECL_OVERRIDE;

    void initialise();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

signals:
    void jointCreated(const QString &joint);
    void markerCreated(const QString &marker);
    void jointDeleted(const QString &joint);
    void markerDeleted(const QString &joint);


public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void comboBoxBodyListCurrentIndexChanged(const QString &text);
    void reset();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogAssembly *ui;

    Simulation *m_simulation = nullptr;
    std::vector<Joint *> m_jointList;
    QGridLayout *m_gridLayout = nullptr;

    std::string m_assemblyAngularMotorSuffix = {"_assembly_angular_motor"};
    std::string m_assemblyLinearMotorSuffix = {"_assembly_linear_motor"};
    std::string m_assemblyMarkerSuffix = {"_assembly_marker"};
};

#endif // DIALOGASSEMBLY_H
