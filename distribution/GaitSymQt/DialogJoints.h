#ifndef DIALOGJOINTS_H
#define DIALOGJOINTS_H

#include <QDialog>

class Simulation;
class Joint;

namespace Ui {
class DialogJoints;
}

class DialogJoints : public QDialog
{
    Q_OBJECT

public:
    explicit DialogJoints(QWidget *parent = nullptr);
    virtual ~DialogJoints() Q_DECL_OVERRIDE;

    void lateInitialise();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    Joint *joint() const;
    void setJoint(Joint *joint);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChanged(const QString &text);
    void checkBoxChanged(int index);
    void tabChanged(int index);

private:
    Ui::DialogJoints *ui = nullptr;

    Simulation *m_simulation = nullptr;
    Joint *m_joint = nullptr;
    Joint *m_origJoint = nullptr;

    void updateActivation();
};

#endif // DIALOGJOINTS_H
