#ifndef DIALOGDRIVERS_H
#define DIALOGDRIVERS_H

#include <QDialog>

#include <memory>

class Driver;
class Simulation;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QComboBox;
class LineEditDouble;

namespace Ui {
class DialogDrivers;
}

class DialogDrivers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDrivers(QWidget *parent = nullptr);
    virtual ~DialogDrivers() Q_DECL_OVERRIDE;

    void lateInitialise();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    void setInputDriver(Driver *inputDriver);

    std::unique_ptr<Driver> outputDriver();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChangedTargets(int value);
    void spinBoxChangedSteps(int value);
    void spinBoxChangedStepsPerCycle(int value);
    void spinBoxChangedBoxcarStackSize(int value);
    void checkBoxChanged(int index);
    void tabChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogDrivers *ui = nullptr;

    Simulation *m_simulation = nullptr;
    QStringList m_drivableIDs;
    Driver *m_inputDriver = nullptr;
    std::unique_ptr<Driver> m_outputDriver;

    QList<QLabel *> m_targetLabelList;
    QList<QComboBox *> m_targetComboBoxList;
    QGridLayout *m_targetGridLayout = nullptr;
    QSpacerItem *m_targetGridSpacer = nullptr;

    QList<QLabel *> m_boxcarLabelList;
    QList<LineEditDouble *> m_boxcarLineEditDoubleList;
    QGridLayout *m_boxcarGridLayout = nullptr;
    QSpacerItem *m_boxcarGridSpacer = nullptr;

    void updateActivation();
};

#endif // DIALOGDRIVERS_H
