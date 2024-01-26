#include <MemoryEditor.hpp>
#include <QFontDatabase>

MemoryEditor::MemoryEditor() : QTextEdit(nullptr) {
  if (objectName().isEmpty())
    setObjectName("MemoryEditor");

  resize(500, 400);
  setWindowTitle("Memory editor");

  setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}