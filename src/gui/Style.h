// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QFont>
#include <QIcon>

class QWidget;

class Style
{
public:
    static void initStyle();
    static QStringList availableStyles();
    static void loadStyleSheetFile(const QString &fileName);
    static void loadStyle(const QString &styleName);
    static QIcon getIcon(const QString &iconName) { return QIcon(inst().m_imgDir.absoluteFilePath(iconName)); };
    static QIcon makeIcon(const QColor &color, int w = 32, int h = 32);
    static const tp::SectionColor &getTextAreaColor() { return inst().m_textAreaColor; }
    static const tp::SectionColor &getSelectedColor() { return inst().m_selectedColor; }
    static const tp::SectionColor &getSelectedTextMarkColor() { return inst().m_selectedTextMarkColor; }
    static const tp::SectionColor &getHeaderColor() { return inst().m_headerColor; }
    static const tp::SectionColor &getBookmarkColor() { return inst().m_bookmarkColor; }
    static const tp::SectionColor &getScrollBarColor() { return inst().m_scrollBarColor; }
    static tp::SInt getTextPadding() { return inst().m_textPadding; }
    static tp::SInt getColumnMargin() { return inst().m_columnMargin; }
    static tp::SInt getScrollBarThickness() { return inst().m_scrollBarThickness; }

    static const QFont &getFont();
    static const QPalette &getPalette();
    static tp::SInt getTextHeight(bool addPadding);
    static tp::SInt getCharWidth();
    static double getCharWidthF();
    static tp::SInt getTextWidth(const QString &text);
    static double getTextWidthF(const QString &text);
    static QString getElidedText(const QString &text, tp::SInt width, Qt::TextElideMode elideMode, int flags = 0);

    static void updateWidget(QWidget *widget);

private:
    Style() = default;
    static Style &inst();
    void clearStyle();
    void loadStyleFromJson(const rapidjson::Value &jsonObj);

    QMap<QString, QString> m_availableStyles;
    tp::SectionColor m_textAreaColor;
    tp::SectionColor m_selectedColor;
    tp::SectionColor m_selectedTextMarkColor;
    tp::SectionColor m_headerColor;
    tp::SectionColor m_bookmarkColor;
    tp::SectionColor m_scrollBarColor;
    QString m_style;
    QString m_qtStyle;
    QString m_qtStyleSheet;
    QDir m_imgDir;
    QPalette m_palette;
    QPalette m_paletteOri;
    tp::SInt m_textPadding = -1;
    tp::SInt m_columnMargin = -1;
    tp::SInt m_scrollBarThickness = -1;
};