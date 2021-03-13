#ifndef DIALOGGEOMS_H
#define DIALOGGEOMS_H

#include "Preferences.h"

#include <QDialog>

#include <memory>

class Geom;
class Simulation;
class QGridLayout;
class QLabel;
class QComboBox;

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

    void setInputGeom(Geom *inputGeom);

    std::unique_ptr<Geom> outputGeom();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void properties();
    void comboBoxChanged(int index);
    void lineEditChanged(const QString &text);
    void spinBoxChanged(const QString &text);
    void checkBoxChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogGeoms *ui = nullptr;

    Simulation *m_simulation = nullptr;
    Geom *m_inputGeom = nullptr;
    std::unique_ptr<Geom> m_outputGeom;

    QList<QLabel *> m_excludedGeomLabelList;
    QList<QComboBox *> m_excludedGeomComboBoxList;
    QGridLayout *m_gridLayoutExcludedGeoms = nullptr;


    QMap<QString, SettingsItem> m_properties;

    void updateActivation();

};

#endif // DIALOGGEOMS_H
