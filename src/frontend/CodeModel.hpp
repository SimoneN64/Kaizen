#pragma once
#include <QAbstractTableModel>

class CodeModel final : public QAbstractTableModel {
  Q_OBJECT
public:
  ~CodeModel() override = default;
  explicit CodeModel(QObject *parent = nullptr) {}
  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override { return 1; }
  [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override { return 2; }
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override { return {}; }
};