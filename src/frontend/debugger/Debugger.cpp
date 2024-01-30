#include <Debugger.hpp>
#include <QTextLayout>
#include <QLabel>
#include <fmt/core.h>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>

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

  setFixedSize(960, 600);
  setWindowTitle("Debugger");
  installEventFilter(this);

  disasmLayout = new QVBoxLayout;
  disasm = new QTextEdit;

  disasm->setMinimumWidth(600);
  disasm->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  disasm->setReadOnly(true);
  disasmLayout->addWidget(disasm);
  regsLayout = new QVBoxLayout;

  for (int i = 0; i < 32; i+=2) {
    auto r1 = new QLabel(QString::fromStdString(regNames[i]));
    auto r1v = new QPlainTextEdit;
    r1v->setMinimumHeight(20);
    r1v->setMaximumHeight(20);
    r1v->setMinimumWidth(120);
    r1v->setMaximumWidth(120);
    r1v->setReadOnly(true);
    r1v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto r2 = new QLabel(QString::fromStdString(regNames[i+1]));
    auto r2v = new QPlainTextEdit;
    r2v->setMinimumHeight(20);
    r2v->setMaximumHeight(20);
    r2v->setMinimumWidth(120);
    r2v->setMaximumWidth(120);
    r2v->setReadOnly(true);
    r2v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto regPair = new QHBoxLayout;
    regPair->addStretch();
    regPair->addWidget(r1);
    regPair->addWidget(r1v);
    regPair->addWidget(r2);
    regPair->addWidget(r2v);
    regsLayout->addLayout(regPair);
  }

  auto buttonsLayout = new QHBoxLayout;
  auto continueBtn = new QPushButton;
  auto stepInBtn = new QPushButton;
  auto stepOverBtn = new QPushButton;
  auto stepOutBtn = new QPushButton;
  auto followPC = new QCheckBox;
  addressBox = new QTextEdit;
  addressBox->setReadOnly(false);
  addressBox->grabKeyboard();
  auto goToAddressLabel = new QLabel("Go to address:");
  auto goToAddressBtn = new QPushButton("Go");

  connect(goToAddressBtn, &QPushButton::pressed, this, [&]() {
    bool ok = false;
    addressBox->toPlainText().toUInt(&ok, 16);
    if(!ok) {
      QMessageBox::critical(
      this, tr("Invalid address"),
      tr("Make sure you are using hexadecimal digits"));
    }
  });

  addressBox->setMaximumHeight(25);
  addressBox->setMaximumWidth(80);
  addressBox->setMinimumHeight(25);
  addressBox->setMinimumWidth(80);
  addressBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  addressBox->setPlaceholderText("00000000");
  continueBtn->setText("Continue");
  stepInBtn->setText("Step In");
  stepOverBtn->setText("Step Over");
  stepOutBtn->setText("Step Out");
  followPC->setChecked(true);
  followPC->setText("Follow PC");

  buttonsLayout->addWidget(continueBtn);
  buttonsLayout->addWidget(stepInBtn);
  buttonsLayout->addWidget(stepOutBtn);
  buttonsLayout->addWidget(stepOverBtn);
  buttonsLayout->addWidget(followPC);
  buttonsLayout->addWidget(goToAddressLabel);
  buttonsLayout->addWidget(addressBox);
  buttonsLayout->addWidget(goToAddressBtn);

  auto viewsLayout = new QHBoxLayout;
  viewsLayout->addLayout(disasmLayout);
  viewsLayout->addLayout(regsLayout);

  mainLayout = new QVBoxLayout;
  mainLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(viewsLayout);
  setLayout(mainLayout);
}

void DebuggerWindow::toggleBkp(u32 addr) {
  auto pos = std::find(bkps.begin(), bkps.end(), addr);
  if (pos == bkps.end()) {
    bkps.push_back(addr);
  }
  else {
    bkps.erase(pos);
  }
}

bool DebuggerWindow::eventFilter(QObject *o, QEvent *e) {
  if (o == this && e->type() == QEvent::MouseButtonPress) {
    if(disasm->underMouse()) {
      auto mouseEvent = static_cast<QMouseEvent*>(e);
      auto linesCount = disasm->size().height() / disasm->font().pixelSize();
      auto relY = int(mouseEvent->position().y() - disasm->pos().toPointF().y()); // downcast
      (void)linesCount;
      (void)relY;
      // implement actually toggling the breakpoint
    }
    return true;
  }
  return QWidget::eventFilter(o, e);
}