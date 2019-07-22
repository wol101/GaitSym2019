#ifndef DIALOGGEOMS_H
#define DIALOGGEOMS_H

#include <QDialog>

class Geom;
class Simulation;

namespace Ui {
class DialogGeoms;
}

class DialogGeoms : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGeoms(QWidget *parent = nullptr);
    virtual ~DialogGeoms() Q_DECL_OVERRIDE;

    void lateInitialise();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    Geom *geom() const;
    void setGeom(Geom *geom);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChanged(const QString &text);
    void checkBoxChanged(int index);

private:
    Ui::DialogGeoms *ui = nullptr;

    Simulation *m_simulation = nullptr;
    Geom *m_geom = nullptr;
    Geom *m_origGeom = nullptr;

    void updateActivation();

};

#endif // DIALOGGEOMS_H
