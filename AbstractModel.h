#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

#include <QAbstractTableModel>
#include <fstream>

class AbstractModel : public QAbstractTableModel
{
    Q_OBJECT

    typedef QHash<int, QByteArray> RolesList;
public:

    AbstractModel(const std::string& file_name, QObject *parent = 0);
    QHash<int, QByteArray> roleNames() const override;
    void addRoleName(int id, const QString &name);
    QString getRoleName(const int id) const;
    int getRole(const QString &roleName) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void updateData();
    std::string parseRow(size_t row) const;


signals:
    void countChanged();
    void updateFinished();
    void changesSubmitted();

protected:
    RolesList m_rolesList;
    mutable std::ifstream m_ifs;
    std::vector<std::size_t> m_lines;
};

#endif // ABSTRACTMODEL_H
