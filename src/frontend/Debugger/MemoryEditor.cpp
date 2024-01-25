#include <MemoryEditor.hpp>

MemoryEditor::MemoryEditor() : QWidget(nullptr) {
  if (objectName().isEmpty())
    setObjectName("MemoryEditor");

  resize(500, 400);
  setWindowTitle("Memory editor");
}