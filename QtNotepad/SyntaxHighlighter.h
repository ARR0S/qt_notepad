#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>

struct HighlightingRule
{
    QRegularExpression pattern;
    QTextCharFormat format;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    SyntaxHighlighter(const QString&,
        QTextDocument* parent = nullptr,
        const QString& style_filename = ":/settings/styles.xml");
    bool isSupported();

protected:
    void highlightBlock(const QString& text) override;

private:
    QVector<HighlightingRule> rules;
    bool supported;
    QTextCharFormat multiLineCommentFormat;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};

