#ifndef DIALOGMUSCLES_H
#define DIALOGMUSCLES_H

#include <QDialog>

class Simulation;
class Muscle;
class QGridLayout;
class QSpacerItem;
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

    Muscle *muscle() const;
    void setMuscle(Muscle *muscle);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChanged(const QString &text);
    void checkBoxChanged(int index);
    void tabChanged(int index);

private:
    Ui::DialogMuscles *ui = nullptr;

    Simulation *m_simulation = nullptr;
    Muscle *m_muscle = nullptr;
    Muscle *m_origMuscle = nullptr;

    QList<QLabel *> m_viaPointLabelList;
    QList<QComboBox *> m_viaPointComboBoxList;
    QGridLayout *m_gridLayout = nullptr;
    QSpacerItem *m_gridSpacer = nullptr;

    void updateActivation();

};

#endif // DIALOGMUSCLES_H
