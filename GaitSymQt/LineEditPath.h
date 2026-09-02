#ifndef LINEEDITPATH_H
#define LINEEDITPATH_H

#include <QLineEdit>

class QFocusEvent;

class LineEditPath : public QLineEdit
{
    Q_OBJECT
public:
    LineEditPath(QWidget *parent = nullptr);

    enum PathType { FileForOpen, FileForSave, Folder };

    PathType pathType() const;
    void setPathType(const PathType &pathType);

    void setHighlighted(bool highlight);

public slots:
    void menuRequestPath(const QPoint &pos);
    void textChangedSlot(const QString &text);

signals:
    void focussed(bool hasFocus);

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

private:
    void generateLocalStyleSheet();

    PathType m_pathType;
    QString m_backgroundStyle;
    QString m_foregroundStyle;
};

#endif // LINEEDITPATH_H
