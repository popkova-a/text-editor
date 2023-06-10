#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QtXml>
#include <QColor>

class SyntaxHighlighter: public QSyntaxHighlighter {

    Q_OBJECT

private:

    QStringList m_lstKeywords;

    QStringList m_lstCommands;

protected:

    void xmlParser(const QDomNode &node);

    enum { NormalState = -1, InsideCStyleComment, InsideCString };

    virtual void highlightBlock(const QString&) override;

    QString getKeyword(int i, const QString& str) const;

    QString getCommand(int i, const QString& str) const;

public:

    SyntaxHighlighter(QTextDocument *parent = nullptr);
};

#endif // HIGHLIGHTER_H
