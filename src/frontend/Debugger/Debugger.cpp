#include <Debugger.hpp>
#include <QTextLayout>
#include <QLabel>
#include <fmt/core.h>

const std::string regNames[] = {
  "r0", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "s8", "ra",
};

DebuggerWindow::DebuggerWindow() : QWidget(nullptr) {
  if (objectName().isEmpty())
    setObjectName("Debugger");

  resize(700, 400);
  setWindowTitle("Debugger");
  setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

  disasmLayout = new QVBoxLayout;
  disasm = new QPlainTextEdit;
  disasm->setMinimumWidth(400);
  disasm->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  disasm->setReadOnly(true);
  disasmLayout->addWidget(disasm);
  regsLayout = new QVBoxLayout;

  for (int i = 0; i < 32; i+=2) {
    QLabel* r1 = new QLabel(QString::fromStdString(regNames[i]));
    QPlainTextEdit* r1v = new QPlainTextEdit;
    r1v->setMaximumHeight(20);
    r1v->setReadOnly(true);
    r1v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    r1v->insertPlainText(QString::fromStdString(fmt::format("{:016X}", 0)));
    QLabel* r2 = new QLabel(QString::fromStdString(regNames[i+1]));
    QPlainTextEdit* r2v = new QPlainTextEdit;
    r2v->setMaximumHeight(20);
    r2v->setReadOnly(true);
    r2v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    r2v->insertPlainText(QString::fromStdString(fmt::format("{:016X}", 0)));
    QHBoxLayout* regPair = new QHBoxLayout;
    regPair->addWidget(r1);
    regPair->addWidget(r1v);
    regPair->addStretch();
    regPair->addWidget(r2);
    regPair->addWidget(r2v);
    regsLayout->addLayout(regPair);
    regsLayout->addStretch();
  }

  mainLayout = new QHBoxLayout;
  mainLayout->addLayout(disasmLayout);
  mainLayout->addLayout(regsLayout);
  setLayout(mainLayout);
}