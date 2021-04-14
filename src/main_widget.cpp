#include "main_widget.h"
#include "config.h"
#include <QtWidgets>
#include <thread>


class SliderCombo {
public:
  QLayout *layout;
  QSlider *slider;
  QLineEdit *edit;
};


static QCheckBox *_AddCheckBox(
  QVBoxLayout *pLayout,
  const char *szLabel,
  Hjson::Value& config,
  const char *szConfig,
  std::function<void()> pOnChecked = nullptr,
  std::function<void()> pOnUnchecked = nullptr
) {
  pLayout->addSpacing(10);
  auto tmpCheckBox = new QCheckBox(szLabel);

  QObject::connect(
    tmpCheckBox,
    &QCheckBox::stateChanged,
    [&config, szConfig, pOnChecked, pOnUnchecked](int state) {
      switch (state)
      {
      case Qt::Checked:
        if (szConfig) {
          config[szConfig] = true;
        }
        if (pOnChecked) {
          pOnChecked();
        }
        break;

      case Qt::Unchecked:
        if (szConfig) {
          config[szConfig] = false;
        }
        if (pOnUnchecked) {
          pOnUnchecked();
        }
        break;

      default:
        break;
      }
    }
  );

  if (szConfig) {
    if (!!config[szConfig]) {
      tmpCheckBox->setChecked(true);
    } else if (pOnUnchecked) {
      pOnUnchecked();
    }
  }

  pLayout->addWidget(tmpCheckBox);

  return tmpCheckBox;
}


static SliderCombo _AddSlider(
  QVBoxLayout *pLayout,
  const char *szLabel,
  int minVal,
  int maxVal,
  int tickInterval,
  Hjson::Value& config,
  const char *szConfig
) {
  auto tmpVL = new QVBoxLayout();
  tmpVL->addSpacing(20);
  tmpVL->addWidget(new QLabel(szLabel));
  tmpVL->addSpacing(5);

  auto tmpHL = new QHBoxLayout();
  auto tmpSlider = new QSlider(Qt::Orientation::Horizontal);
  tmpSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
  tmpSlider->setMinimum(minVal);
  tmpSlider->setMaximum(maxVal);
  tmpSlider->setTickInterval(tickInterval);
  tmpHL->addWidget(tmpSlider);
  tmpHL->addSpacing(10);
  auto tmpEdit = new QLineEdit();
  tmpEdit->setAlignment(Qt::AlignRight);
  tmpEdit->setFixedWidth(50);
  tmpEdit->setValidator(new QIntValidator(tmpEdit));
  tmpHL->addWidget(tmpEdit);

  QObject::connect(
    tmpSlider,
    &QSlider::valueChanged,
    [tmpEdit, &config, szConfig](int value) {
      if (tmpEdit->text() != QString::number(value)) {
        tmpEdit->setText(QString::number(value));
      }
      config[szConfig] = value;
    }
  );

  QObject::connect(
    tmpEdit,
    &QLineEdit::textChanged,
    [tmpSlider](const QString& str) {
      int val = str.toInt();
      if (val >= tmpSlider->minimum() && val <= tmpSlider->maximum() &&
        tmpSlider->value() != val)
      {
        tmpSlider->setValue(val);
      }
    }
  );

  // Use to_int64() to always get a number even if the value is a string or
  // something else.
  tmpSlider->setValue(config[szConfig].to_int64());

  tmpVL->addItem(tmpHL);
  pLayout->addItem(tmpVL);

  return {tmpVL, tmpSlider, tmpEdit};
}


static QPushButton *_AddButton(
  QVBoxLayout *pLayout,
  const char *szLabel,
  std::function<void()> pOnClick
) {
  pLayout->addSpacing(40);
  auto tmpBtn = new QPushButton();
  tmpBtn->setText(szLabel);
  tmpBtn->setFixedWidth(70);
  tmpBtn->setFixedHeight(30);
  pLayout->addWidget(tmpBtn, 0, Qt::AlignRight);
  QObject::connect(tmpBtn, &QPushButton::clicked, pOnClick);

  return tmpBtn;
}


static void _InitDialog(QDialog *pDialog) {
  pDialog->resize(200, 150);
  auto dialogLayout = new QVBoxLayout();
  dialogLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  dialogLayout->addSpacing(35);
  dialogLayout->addWidget(new QLabel("Working..."));
  pDialog->setLayout(dialogLayout);
  pDialog->setModal(true);
}


MainWidget::MainWidget(Hjson::Value& config)
  : m_dialog(new QDialog(this, Qt::CoverWindow))
{
  _InitDialog(m_dialog);

  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setAlignment(Qt::AlignTop);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  auto alphaCombo = _AddSlider(mainLayout, "Alpha", 0, 3000, 500, config,
    Cfg::alpha);

  auto checkbox = _AddCheckBox(
    mainLayout,
    "Enable alpha",
    config,
    Cfg::enableAlpha,
    [alphaCombo]() {
      alphaCombo.slider->setEnabled(true);
      alphaCombo.edit->setEnabled(true);
    },
    [alphaCombo]() {
      alphaCombo.slider->setEnabled(false);
      alphaCombo.edit->setEnabled(false);
    }
  );

  // Move down the alpha slider to below the checkbox.
  mainLayout->removeItem(alphaCombo.layout);
  mainLayout->addItem(alphaCombo.layout);

  _AddSlider(mainLayout, "Beta", 0, 256, 32, config, Cfg::beta);

  _AddSlider(mainLayout, "Gamma", 0, 10, 1, config, Cfg::gamma);

  mainLayout->addSpacing(20);

  // Use to_string() to always get a string even if the value is a number or
  // something else.
  mainLayout->addWidget(new QLabel(
    config[Cfg::exampleString].to_string().c_str()));

  _AddButton(
    mainLayout,
    "Run",
    [this]() {
      this->m_dialog->open();
      // Shows how to do heavy work in a background thread in order to not
      // block the UI.
      std::thread([this]() {
        // Simulate heavy work by just sleeping for three seconds.
        std::this_thread::sleep_for(std::chrono::seconds(3));
        // Calls the member function work_finished() from the UI thread instead
        // of from the current thread.
        QMetaObject::invokeMethod(this, "work_finished");
      }).detach();
    }
  );

  setLayout(mainLayout);
  setWindowTitle(tr("HjsonExample"));
  setMinimumSize(200, 389);
}


void MainWidget::work_finished() {
  m_dialog->close();
}
