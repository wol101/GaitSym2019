#ifndef DIALOGMUSCLES_H
#define DIALOGMUSCLES_H

#include "Preferences.h"

#include <QDialog>

#include <memory>

class Simulation;
class Muscle;
class Strap;
class QGridLayout;
class QLabel;
class QComboBox;

namespace Ui {
class DialogMuscles;
}

class DialogMuscles : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMuscles(QWidget *parent = nullptr);
    virtual ~DialogMuscles() Q_DECL_OVERRIDE;

    void lateInitialise();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    void setInputMuscle(Muscle *inputMuscle);

    std::unique_ptr<Muscle> outputMuscle();

    std::unique_ptr<Strap> outputStrap();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void properties();
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChanged(const QString &text);
    void checkBoxChanged(int index);
    void tabChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogMuscles *ui = nullptr;

    Simulation *m_simulation = nullptr;
    Muscle *m_inputMuscle = nullptr;
    std::unique_ptr<Muscle> m_outputMuscle;
    std::unique_ptr<Strap> m_outputStrap;

    QList<QLabel *> m_viaPointLabelList;
    QList<QComboBox *> m_viaPointComboBoxList;
    QGridLayout *m_gridLayoutViaPoints = nullptr;

    QList<QLabel *> m_torqueMarkerLabelList;
    QList<QComboBox *> m_torqueMarkerComboBoxList;
    QGridLayout *m_gridLayoutTorqueMarkers = nullptr;

    QMap<QString, SettingsItem> m_properties;

    void updateActivation();

};

#endif // DIALOGMUSCLES_H
