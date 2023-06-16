#include "DialogCreateMirrorElements.h"
#include "ui_DialogCreateMirrorElements.h"

#include "Preferences.h"
#include "BasicXMLSyntaxHighlighter.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "PGDMath.h"

#include "pystring.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QRegularExpression>

#include <string>
#include <vector>
#include <set>

using namespace std::literals::string_literals;

DialogCreateMirrorElements::DialogCreateMirrorElements(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCreateMirrorElements)
{
    ui->setupUi(this);
    setWindowTitle(tr("Create Mirror Elements"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    loadPreferences();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->lineEditFrom, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->lineEditTo, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->plainTextEdit, SIGNAL(modificationChanged(bool)), this, SLOT(modificationChanged(bool)));

    setEditorFonts();
    enableControls();
}

DialogCreateMirrorElements::~DialogCreateMirrorElements()
{
    delete ui;
}

void DialogCreateMirrorElements::accept()
{
    qDebug() << "DialogCreateMirrorElements::accept()";

    std::string *errorMessage = validate();
    if (errorMessage)
    {
        QMessageBox::warning(this, "Error validating XML file", QString("Error message:\n%1\nSuggest either fix error or Cancel").arg(QString::fromStdString(*errorMessage)));
        return;
    }

    savePreferences();
    QDialog::accept();
}

void DialogCreateMirrorElements::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogCreateMirrorElements::reject()";

    if (isModified())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Click OK to quit, and Cancel to continue working on the document");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel) return;
    }

    savePreferences();
    QDialog::reject();
}

void DialogCreateMirrorElements::loadPreferences()
{
    m_editorFont = Preferences::valueQFont("DialogCreateMirrorElementsEditorFont");
    ui->checkBoxCaseSensitive->setChecked(Preferences::valueBool("DialogCreateMirrorElementsCaseSensitiveSearch"));
    ui->checkBoxRegularExpression->setChecked(Preferences::valueBool("DialogCreateMirrorElementsRegularExpressionSearch"));
    ui->checkBoxDeleteTo->setChecked(Preferences::valueBool("DialogCreateMirrorElementsDeleteTo"));
    ui->lineEditFrom->setText(Preferences::valueQString("DialogCreateMirrorElementsFrom"));
    ui->lineEditTo->setText(Preferences::valueQString("DialogCreateMirrorElementsTo"));
    if (Preferences::valueInt("DialogCreateMirrorElementsAxis") == 0) ui->radioButtonX->setChecked(true);
    if (Preferences::valueInt("DialogCreateMirrorElementsAxis") == 1) ui->radioButtonY->setChecked(true);
    if (Preferences::valueInt("DialogCreateMirrorElementsAxis") == 2) ui->radioButtonZ->setChecked(true);
    restoreGeometry(Preferences::valueQByteArray("DialogCreateMirrorElementsGeometry"));
}

void DialogCreateMirrorElements::savePreferences()
{
    Preferences::insert("DialogCreateMirrorElementsCaseSensitiveSearch", ui->checkBoxCaseSensitive->isChecked());
    Preferences::insert("DialogCreateMirrorElementsRegularExpressionSearch", ui->checkBoxRegularExpression->isChecked());
    Preferences::insert("DialogCreateMirrorElementsDeleteTo", ui->checkBoxDeleteTo->isChecked());
    Preferences::insert("DialogCreateMirrorElementsEditorFont", m_editorFont);
    Preferences::insert("DialogCreateMirrorElementsFrom", ui->lineEditFrom->text());
    Preferences::insert("DialogCreateMirrorElementsTo", ui->lineEditTo->text());
    if (ui->radioButtonX->isChecked()) Preferences::insert("DialogCreateMirrorElementsAxis", 0);
    if (ui->radioButtonY->isChecked()) Preferences::insert("DialogCreateMirrorElementsAxis", 1);
    if (ui->radioButtonZ->isChecked()) Preferences::insert("DialogCreateMirrorElementsAxis", 2);
    Preferences::insert("DialogCreateMirrorElementsGeometry", saveGeometry());
}

QString DialogCreateMirrorElements::editorText() const
{
    return ui->plainTextEdit->toPlainText();
}

void DialogCreateMirrorElements::setEditorText(const QString &editorText)
{
    ui->plainTextEdit->setPlainText(editorText);
}

void DialogCreateMirrorElements::useXMLSyntaxHighlighter()
{
    m_basicXMLSyntaxHighlighter = new BasicXMLSyntaxHighlighter(ui->plainTextEdit->document());
}

void DialogCreateMirrorElements::setEditorFonts()
{
    QList<QLineEdit *> listQLineEdit = this->findChildren<QLineEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QLineEdit *>::iterator it = listQLineEdit.begin(); it != listQLineEdit.end(); it++) (*it)->setFont(m_editorFont);

    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}

void DialogCreateMirrorElements::enableControls()
{
    ui->pushButtonApply->setEnabled(ui->lineEditTo->text().size() > 0 && ui->lineEditFrom->text().size() > 0);
    ui->pushButtonOK->setEnabled(isModified());
}

std::string *DialogCreateMirrorElements::validate()
{
    Simulation simulation;
    QByteArray editFileData = ui->plainTextEdit->toPlainText().toUtf8();
    std::string *errorMessage = simulation.LoadModel(editFileData.constData(), size_t(editFileData.size()));
    if (errorMessage)
    {
        m_lastError = *errorMessage;
        return &m_lastError;
    }
    return nullptr;
}

void DialogCreateMirrorElements::textChanged(const QString & /*text*/)
{
    enableControls();
}

bool DialogCreateMirrorElements::isModified() const
{
    return ui->plainTextEdit->document()->isModified();
}

void DialogCreateMirrorElements::setModified(bool modified)
{
    ui->plainTextEdit->document()->setModified(modified);
    enableControls();
}

void DialogCreateMirrorElements::apply()
{
    bool localModified = isModified();
    std::string *lastError;
    std::string xml = ui->plainTextEdit->toPlainText().toStdString();
    std::string rootNodeTag = "GAITSYM2019"s;
    lastError = m_parseXML.LoadModel(xml.c_str(), xml.size(), rootNodeTag);
    if (lastError)
    {
        QMessageBox::warning(this, "XML parse error", QString("'%1'").arg(QString::fromStdString(*lastError)));
        return;
    }
    std::string fromString = ui->lineEditFrom->text().toStdString();
    std::string toString = ui->lineEditTo->text().toStdString();

    size_t axis = 1;
    if (ui->radioButtonX->isChecked()) axis = 0;
    if (ui->radioButtonY->isChecked()) axis = 1;
    if (ui->radioButtonZ->isChecked()) axis = 2;

    applyMirrorCreate(fromString, toString, axis);

    std::string newXML = m_parseXML.SaveModel("GAITSYM2019"s, "Created from DialogCreateMirrorElements::apply"s);
    ui->plainTextEdit->setPlainText(QString::fromStdString(newXML));
    if (localModified || (xml != newXML)) setModified(true);
}

void DialogCreateMirrorElements::applyMirrorCreate(const std::string &fromString, const std::string &toString, size_t axis)
{
    std::vector<std::unique_ptr<ParseXML::XMLElement>> newElements;
    std::vector<std::unique_ptr<ParseXML::XMLElement>> *elementList = m_parseXML.elementList();
    // do we have to delete matching tags first?
    if (ui->checkBoxDeleteTo->isChecked())
    {
        for (auto it = elementList->begin(); it != elementList->end(); )
        {
            auto findID = it->get()->attributes.find("ID"s);
            if (findID != it->get()->attributes.end() && attributeFind(findID->second, toString))
            {
                it = elementList->erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    // handle BODYs first
    for (auto &&tagElementIt : *elementList)
    {
        if (tagElementIt->tag == "BODY"s)
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end() && attributeFind(findID->second, fromString))
            {
                std::unique_ptr<ParseXML::XMLElement> newElement = std::make_unique<ParseXML::XMLElement>();
                newElement->tag = tagElementIt->tag;
                newElement->attributes = tagElementIt->attributes;
                auto newID = newElement->attributes.find("ID"s);
                if (newID != newElement->attributes.end())
                    newID->second = attributeReplace(newID->second, fromString, toString);
                auto constructionPosition = newElement->attributes.find("ConstructionPosition"s);
                if (constructionPosition != newElement->attributes.end())
                    constructionPosition->second = mirrorString(constructionPosition->second, axis);
                auto position = newElement->attributes.find("Position"s);
                if (position != newElement->attributes.end())
                    position->second = mirrorString(position->second, axis);
                auto quaternion = newElement->attributes.find("Quaternion"s);
                if (quaternion != newElement->attributes.end())
                {
                    quaternion->second = mirrorString(quaternion->second, 0);
                    quaternion->second = mirrorString(quaternion->second, axis + 1);
                }
                auto graphicsFile1 = newElement->attributes.find("GraphicFile1"s);
                if (graphicsFile1 != newElement->attributes.end())
                    graphicsFile1->second = attributeReplace(graphicsFile1->second, fromString, toString);
                auto graphicsFile2 = newElement->attributes.find("GraphicFile2"s);
                if (graphicsFile2 != newElement->attributes.end())
                    graphicsFile2->second = attributeReplace(graphicsFile2->second, fromString, toString);
                auto graphicsFile3 = newElement->attributes.find("GraphicFile3"s);
                if (graphicsFile3 != newElement->attributes.end())
                    graphicsFile3->second = attributeReplace(graphicsFile3->second, fromString, toString);
                auto moi = newElement->attributes.find("MOI"s);
                if (moi != newElement->attributes.end())
                    moi->second = mirrorMOIString(moi->second, axis);
                newElements.push_back(std::move(newElement));
            }
        }
    }
    // then MARKERs
    for (auto &&tagElementIt : *elementList)
    {
        if (tagElementIt->tag == "MARKER"s)
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end() && attributeFind(findID->second, fromString))
            {
                std::unique_ptr<ParseXML::XMLElement> newElement = std::make_unique<ParseXML::XMLElement>();
                newElement->tag = tagElementIt->tag;
                newElement->attributes = tagElementIt->attributes;
                auto newID = newElement->attributes.find("ID"s);
                if (newID != newElement->attributes.end())
                    newID->second = attributeReplace(newID->second, fromString, toString);
                auto bodyID = newElement->attributes.find("BodyID"s);
                if (bodyID != newElement->attributes.end())
                    bodyID->second = attributeReplace(bodyID->second, fromString, toString);
                auto position = newElement->attributes.find("Position"s);
                if (position != newElement->attributes.end())
                {
                    position->second = mirrorString(position->second, axis + 1);
                    position->second = attributeReplace(position->second, fromString, toString);
                }
                auto quaternion = newElement->attributes.find("Quaternion"s);
                if (quaternion != newElement->attributes.end())
                {
                    quaternion->second = mirrorString(quaternion->second, 1);
                    quaternion->second = mirrorString(quaternion->second, axis + 2);
                    quaternion->second = attributeReplace(quaternion->second, fromString, toString);
                }
                newElements.push_back(std::move(newElement));
            }
        }
    }
    // then everything else
    for (auto &&tagElementIt : *elementList)
    {
        if (
                tagElementIt->tag == "JOINT"s ||
                tagElementIt->tag == "GEOM"s ||
                tagElementIt->tag == "STRAP"s ||
                tagElementIt->tag == "MUSCLE"s ||
                tagElementIt->tag == "FLUIDSAC"s ||
                tagElementIt->tag == "DRIVER"s ||
                tagElementIt->tag == "CONTROLLER"s ||
                tagElementIt->tag == "DATATARGET"s )
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end() && attributeFind(findID->second, fromString))
            {
                std::unique_ptr<ParseXML::XMLElement> newElement = std::make_unique<ParseXML::XMLElement>();
                newElement->tag = tagElementIt->tag;
                newElement->attributes = tagElementIt->attributes;
                for (auto &&attribute : newElement->attributes)
                {
                    if (pystring::endswith(attribute.first, "ID"s))
                    {
                        attribute.second = attributeReplace(attribute.second, fromString, toString);
                    }
                    if (pystring::find(attribute.first, "IDList"s) != -1)
                    {
                        std::vector<std::string> idList;
                        pystring::split(attribute.second, idList);
                        for (size_t i =0; i < idList.size(); i++)
                            idList[i] = attributeReplace(idList[i], fromString, toString);
                        attribute.second = pystring::join(" "s, idList);
                    }
                }
                newElements.push_back(std::move(newElement));
            }
        }
    }

    std::set<std::string> markersToReverse;
    for (auto &&tagElementIt : newElements)
    {
        if (tagElementIt->tag == "JOINT"s) // hinge joint ranges need reversing and negating
        {
            auto findType = tagElementIt->attributes.find("Type"s);
            if (findType != tagElementIt->attributes.end() && findType->second == "Hinge"s)
            {
                auto highStop = tagElementIt->attributes.find("HighStop"s);
                auto lowStop = tagElementIt->attributes.find("LowStop"s);
                if (highStop != tagElementIt->attributes.end() && lowStop != tagElementIt->attributes.end())
                {
                    std::string tempStr = mirrorString(highStop->second, 0);
                    highStop->second = mirrorString(lowStop->second, 0);
                    lowStop->second = tempStr;
                }
            }
            continue;
        }
        if (tagElementIt->tag == "DRIVER"s) // ThreeHingeJoint drivers joint ranges need reversing and negating
        {
            auto findType = tagElementIt->attributes.find("Type"s);
            if (findType != tagElementIt->attributes.end() && findType->second == "ThreeHingeJoint"s)
            {
                auto proximalJointRange = tagElementIt->attributes.find("ProximalJointRange"s);
                if (proximalJointRange != tagElementIt->attributes.end())
                {
                    proximalJointRange->second = mirrorRotationRange(proximalJointRange->second);
                }
                auto intermediateJointRange = tagElementIt->attributes.find("IntermediateJointRange"s);
                if (intermediateJointRange != tagElementIt->attributes.end())
                {
                    intermediateJointRange->second = mirrorRotationRange(intermediateJointRange->second);
                }
                auto distalJointRange = tagElementIt->attributes.find("DistalJointRange"s);
                if (distalJointRange != tagElementIt->attributes.end())
                {
                    distalJointRange->second = mirrorRotationRange(distalJointRange->second);
                }
            }
            continue;
        }
        if (tagElementIt->tag == "STRAP"s) // muscle cylinders wrap the wrong way so need reversing - this requires a second pass later
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end())
            {
                auto findIt = tagElementIt->attributes.find("CylinderMarkerID"s);
                if (findIt != tagElementIt->attributes.end()) markersToReverse.insert(findIt->second);
                findIt = tagElementIt->attributes.find("Cylinder1MarkerID"s);
                if (findIt != tagElementIt->attributes.end()) markersToReverse.insert(findIt->second);
                findIt = tagElementIt->attributes.find("Cylinder2MarkerID"s);
                if (findIt != tagElementIt->attributes.end()) markersToReverse.insert(findIt->second);
            }
            continue;
        }
    }
    // now loop through and reverse the markers that have been tagged for reversal
    for (auto &&tagElementIt : newElements)
    {
        if (tagElementIt->tag == "MARKER"s)
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end() && markersToReverse.count(findID->second))
            {
                auto quaternion = tagElementIt->attributes.find("Quaternion"s);
                if (quaternion != tagElementIt->attributes.end())
                {
                    // this bit gets back the original quaternion
                    quaternion->second = mirrorString(quaternion->second, 1);
                    quaternion->second = mirrorString(quaternion->second, axis + 2);
                    // and this bit rotates a unit X vector
                    std::vector<std::string> tokens;
                    pystring::split(quaternion->second, tokens);
                    std::vector<std::string> tokensPrime(tokens.begin() + 1, tokens.end()); // because tokens[0] is the body name
                    pgd::Quaternion q = GSUtil::GetQuaternion(tokensPrime, 0);
                    pgd::Vector3 unitX(1, 0, 0);
                    pgd::Vector3 unitXPrime = pgd::QVRotate(q, unitX);
                    // and it seems that reversing the non axes components does the trick
                    switch(axis)
                    {
                    case 0: // x axis
                        unitXPrime.y = -unitXPrime.y;
                        unitXPrime.z = -unitXPrime.z;
                        break;
                    case 1: // y axis
                        unitXPrime.x = -unitXPrime.x;
                        unitXPrime.z = -unitXPrime.z;
                        break;
                    case 2: // z axis
                        unitXPrime.x = -unitXPrime.x;
                        unitXPrime.y = -unitXPrime.y;
                        break;
                    }
                    // now convert this back to a quaternion
                    pgd::Quaternion qPrime = pgd::FindRotation(unitX, unitXPrime);
                    quaternion->second = tokens[0] + " "s + GSUtil::ToString(qPrime);
                }
            }
        }
    }

    for (auto &&newElementIt : newElements)
    {
        elementList->push_back(std::move(newElementIt));
    }
}

bool DialogCreateMirrorElements::attributeFind(const std::string &refStr, const std::string &findStr)
{
    bool caseSensitive = false;
    bool useRegex = false;
    if (ui->checkBoxCaseSensitive->isChecked()) caseSensitive = true;
    if (ui->checkBoxRegularExpression->isChecked()) useRegex = true;
    QString qRefStr = QString::fromStdString(refStr);
    QString qFindStr = QString::fromStdString(findStr);
    if (!useRegex) return qRefStr.contains(qFindStr, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

    QRegularExpression regExp(qFindStr, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regExp.match(qRefStr);
    if (match.capturedStart(0) == -1) return false;
    return true;
}

std::string DialogCreateMirrorElements::attributeReplace(const std::string input, const std::string &before, const std::string &after)
{
    bool caseSensitive = false;
    bool useRegex = false;
    if (ui->checkBoxCaseSensitive->isChecked()) caseSensitive = true;
    if (ui->checkBoxRegularExpression->isChecked()) useRegex = true;
    QString qInput = QString::fromStdString(input);
    QString qBefore = QString::fromStdString(before);
    QString qAfter = QString::fromStdString(after);
    if (!useRegex)
    {
        QString qReplaceStr = qInput.replace(qBefore, qAfter, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        return qReplaceStr.toStdString();
    }

    QRegularExpression regExp(qBefore, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    QString qReplaceStr = qInput.replace(regExp, qAfter);
    return qReplaceStr.toStdString();
}

std::string DialogCreateMirrorElements::mirrorString(const std::string &inputString, size_t tokenNumber)
{
    std::vector<std::string> tokens;
    pystring::split(inputString, tokens);
    if (tokens.size() <= tokenNumber) return inputString;
    if (pystring::startswith(tokens[tokenNumber], "-"s))
    {
        tokens[tokenNumber] = tokens[tokenNumber].substr(1);
        return pystring::join(" "s, tokens);
    }
    if (pystring::startswith(tokens[tokenNumber], "+"s))
    {
        tokens[tokenNumber] = "-"s + tokens[tokenNumber].substr(1);
        return pystring::join(" "s, tokens);
    }
    tokens[tokenNumber] = "-"s + tokens[tokenNumber];
    return pystring::join(" "s, tokens);
}

std::string DialogCreateMirrorElements::mirrorMOIString(const std::string &inputString, size_t axis)
{
    // MOI string has elements in this order: xx yy zz xy xz yz
    std::vector<std::string> tokens;
    pystring::split(inputString, tokens);
    if (tokens.size() != 6) return inputString;
    switch (axis)
    {
    case 0: // X
        tokens[3] = mirrorString(tokens[3], 0);
        tokens[4] = mirrorString(tokens[4], 0);
        break;
    case 1: // Y
        tokens[3] = mirrorString(tokens[3], 0);
        tokens[5] = mirrorString(tokens[5], 0);
        break;
    case 2: // Z
        tokens[4] = mirrorString(tokens[4], 0);
        tokens[5] = mirrorString(tokens[5], 0);
        break;
    }
    return pystring::join(" "s, tokens);
}

std::string DialogCreateMirrorElements::mirrorRotationRange(const std::string &inputString)
{
    // reverse the order and negate the angle range
    std::vector<std::string> tokens;
    pystring::split(inputString, tokens);
    if (tokens.size() != 2) return inputString;
    std::vector<std::string> newTokens;
    newTokens.push_back(mirrorString(tokens[1], 0));
    newTokens.push_back(mirrorString(tokens[0], 0));
    return pystring::join(" "s, newTokens);
}

void DialogCreateMirrorElements::modificationChanged(bool /*changed*/)
{
    enableControls();
}



