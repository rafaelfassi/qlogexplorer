#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QAbstractScrollArea>

class AbstractLogModel;

class LogView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    LogView(QWidget *parent = nullptr);
    ~LogView();

    void setLogModel(AbstractLogModel *logModel);

public slots:
    void updateDisplaySize();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    //void scrollContentsBy(int dx, int dy);


private:
    AbstractLogModel *m_logModel = nullptr;
    QFont m_font;
    QFontMetrics m_fm;
    int m_rowHeight;
    int m_itemsPerPage;
};

#endif // LOGVIEW_H
