#pragma once
#include <QAbstractTableModel>

class CodeModel final : public QAbstractTableModel {
  Q_OBJECT
public:
  ~CodeModel() override {}
  explicit CodeModel(QObject *parent = nullptr) {}
  int rowCount(const QModelIndex &parent = QModelIndex()) const override { return 1; }
  int columnCount(const QModelIndex &parent = QModelIndex()) const override { return 2; }
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override { return {}; }
};