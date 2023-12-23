#include "SyntaxHighlighter.h"
SyntaxHighlighter::SyntaxHighlighter(const QString& str, QTextDocument* parent, const QString& style) : QSyntaxHighlighter(parent)
{
    multiLineCommentFormat.setForeground(QColor(0, 255, 255));
    supported = true;

    QDomDocument domDocument;
    QFile file(style);
    QTextCharFormat textCharFormat;
    HighlightingRule rule;
    int errorColumn;
    int errorRow;
    QString errorStr;

    if (file.open(QIODevice::ReadOnly))
    {
        if (domDocument.setContent(&file, &errorStr, &errorRow, &errorColumn))
        {
            QDomElement root = domDocument.documentElement();
            auto nodes = root.elementsByTagName("syntax");
            if (!nodes.isEmpty())
            {
                for (auto i = 0; i < nodes.count(); ++i)
                {
                    QStringList list = nodes.item(i).toElement().attribute("list").split(QRegularExpression("\\s+"));
                    if ((list.contains(str)) && (str != Q_NULLPTR))
                    {
                        auto ruleNodes = nodes.item(i).toElement().elementsByTagName("rule");
                        if (!ruleNodes.isEmpty())
                        {
                            for (int j = 0; j < ruleNodes.count(); ++j)
                            {
                                auto format = ruleNodes.item(j).toElement().elementsByTagName("format");
                                auto pattern = ruleNodes.item(j).toElement().elementsByTagName("pattern");
                                rule.pattern = QRegularExpression(pattern.item(0).toElement().attribute("value"));
                                rule.format.setFontWeight(format.at(0).toElement().attribute("font_weight").toInt());
                                rule.format.setForeground(QColor(format.item(0).toElement().attribute("foreground")));
                                rules.append(rule);
                            }
                        }
                        auto first = root.elementsByTagName("startComment").item(0).toElement();
                        auto last = root.elementsByTagName("endComment").item(0).toElement();
                        QString firstNode = first.elementsByTagName("pattern").at(0).toElement().attribute("value");
                        QString lastNode = last.elementsByTagName("pattern").at(0).toElement().attribute("value");

                        commentStartExpression = QRegularExpression(firstNode);
                        commentEndExpression = QRegularExpression(lastNode);
                    }
                    else supported = false;
                }
            }
        }
        else qDebug() << errorStr
            << " Error string " << errorRow
            << " Error column " << errorColumn;
    }
    else qDebug() << "Can't open XML file!";
}

bool SyntaxHighlighter::isSupported()
{
    return supported;
}

void SyntaxHighlighter::highlightBlock(const QString& txt)
{
    for (const HighlightingRule& rule : qAsConst(rules))
    {
        QRegularExpressionMatchIterator iter = rule.pattern.globalMatch(txt);
        while (iter.hasNext())
        {
            QRegularExpressionMatch match = iter.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);
    int first = 0;
    if (previousBlockState() != 1) first = txt.indexOf(commentStartExpression);

    while (first >= 0)
    {
        QRegularExpressionMatch match = commentEndExpression.match(txt, first);
        int last = match.capturedStart();
        int length = 0;
        if (last == -1)
        {
            setCurrentBlockState(1);
            length = txt.length() - first;
        }
        else length = last - first + match.capturedLength();
        setFormat(first, length, multiLineCommentFormat);
        first = txt.indexOf(commentStartExpression, length + first);
    }
}