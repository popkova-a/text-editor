#include <highlighter.h>

void SyntaxHighlighter::xmlParser (const QDomNode &node)
{
    QDomNode domNode = node.firstChild();
    while (!domNode.isNull()) {
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if (!domElement.isNull()) {

                if(domElement.tagName() == "keyword")
                    m_lstKeywords << domElement.text();
                if(domElement.tagName() == "command")
                    m_lstCommands <<domElement.text();
            }
        }

        xmlParser(domNode);
        domNode = domNode.nextSibling();
    }
}

QString SyntaxHighlighter::getKeyword(int nPos, const QString& str) const
{
    QString strTemp = "";
    foreach (QString strKeyword, m_lstKeywords) {
        if (str.mid(nPos, strKeyword.length()) == strKeyword) {
            strTemp = strKeyword;
            break;
        }
    }

    return strTemp;
}

QString SyntaxHighlighter::getCommand (int nPos, const QString& str) const
{
    QString strTemp = "";
    foreach (QString strCommand, m_lstCommands) {
        if (str.mid(nPos, strCommand.length()) == strCommand) {
            strTemp = strCommand;
            break;
        }
    }

    return strTemp;
}

void SyntaxHighlighter::highlightBlock(const QString &str)
{
    int nState = previousBlockState();
    int nStart = 0;

    for (int i=0; i < str.length (); ++i)
    {
        if (nState == InsideCStyleComment) {
            if (str.mid(i, 2) == "*/") {
                nState = NormalState;
                setFormat(nStart, i - nStart + 2, Qt::darkGray);
                i++;
            }
        }
       else
            if (nState == InsideCString) {
                if (str.mid(i, 1) == "\"" || str.mid(i, 1) == "\'") {
                    if (str.mid(i - 1, 2) != "\\\"" && str.mid(i - 1, 2) != "\\\'") {
                        nState = NormalState;
                        setFormat(nStart, i - nStart + 1, Qt::cyan);
                    }
                }
            }
            else
            {
                if (str.mid(i, 2) == "//") {
                    setFormat (i, str.length() - i, Qt::darkGray);
                    break;
                }
                else
                    if (str.at(i).isNumber())
                       {
                         setFormat(i, 1, Qt::darkGreen);
                       }
                    else
                        if (str.mid(i, 2) == "/*")
                        {
                            nStart = i;
                            nState = InsideCStyleComment;
                        }
                        else
                            if (str.mid(i, 1) == "\"" || str.mid(i, 1) == "\'")
                            {
                                nStart  = i;
                                nState = InsideCString;
                            }
                            else
                            {
                                QString strKeyword = getKeyword(i, str);
                                if (!strKeyword.isEmpty()) {
                                    setFormat(i, strKeyword.length(),Qt::magenta);
                                    i += strKeyword.length() - 1;
                                }
                                else
                                {
                                    QString strCommand = getCommand(i, str);
                                    if (!strCommand.isEmpty()) {
                                        setFormat(i, strCommand.length(), Qt::darkRed);
                                        i += strCommand.length() - 1;
                                    }
                                }
                            }
            }
    }

    if (nState == InsideCStyleComment)
        setFormat(nStart, str.length() - nStart, Qt::darkGray);

    if (nState == InsideCString)
        setFormat (nStart, str.length() - nStart, Qt::cyan);

    setCurrentBlockState(nState);
}

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    QDomDocument domDoc;
    QFile file(":/highlight.xml");

    if(file.open(QIODevice::ReadOnly)) {
        QString err;
        if(domDoc.setContent(&file, &err)) {
            QDomElement domElement = domDoc.documentElement();
            xmlParser(domElement);
        }
        else
            qDebug() << err;
    }
}
