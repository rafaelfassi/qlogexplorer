#include "AbstractModel.h"
#include <QDebug>

AbstractModel::AbstractModel(const std::string& file_name, QObject *parent) :
    QAbstractTableModel(parent), m_ifs(file_name)
{
    updateData();
}

QHash<int, QByteArray> AbstractModel::roleNames() const
{
    return m_rolesList;
}

void AbstractModel::addRoleName(int id, const QString &name)
{
    m_rolesList[id] = name.toLatin1();
}

QString AbstractModel::getRoleName(const int id) const
{
    auto it = m_rolesList.find(id);
    if (it != m_rolesList.end())
        return it.value();
    return QString();
}

int AbstractModel::getRole(const QString &roleName) const
{
    return m_rolesList.key(roleName.toLatin1());
}

QVariant AbstractModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= this->rowCount())
        return QVariant();

//    if(role == Qt::DisplayRole)
//    {
//        role = index.column() + Qt::UserRole + 1;
//    }

    //if(role > Qt::UserRole)
    {
        //QString tagName = getRoleName(role);
        //if(!tagName.isEmpty())
        {
            const auto& val = parseRow(index.row());
            if(val.empty())
                return QVariant(QVariant::String);
            return QString::fromStdString(val);
        }
    }

    return QVariant();
}

int AbstractModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;

    return m_rolesList.size();
}

int AbstractModel::rowCount(const QModelIndex &parent) const
{
    return m_lines.size();
}

void AbstractModel::updateData()
{
    auto last_pos = m_ifs.tellg();
    for (std::string line; std::getline(m_ifs, line);)
    {
        //QString st = QString::fromStdString(line);
        m_lines.push_back(last_pos);
        last_pos = m_ifs.tellg();
    }
}

std::string AbstractModel::parseRow(size_t row) const
{
    m_ifs.clear();
    std::string line;
    const auto pos = m_lines.at(row);
    m_ifs.seekg(pos, std::ios::beg);
    std::getline(m_ifs, line);
    return line;
}
