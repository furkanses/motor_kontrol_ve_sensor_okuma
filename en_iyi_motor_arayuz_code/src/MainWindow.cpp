#include "MainWindow.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QtEndian>

#include <QApplication>
#include <QtCharts/QChart>

// =======================
// MotorWidget Implementation
// =======================

MotorWidget::MotorWidget(QWidget *parent) : QWidget(parent) {
  setMinimumSize(300, 400);
  // Transparent background to let the pixel art bg show through,
  // or a semi-transparent dark block
  setStyleSheet("background-color: rgba(0, 0, 0, 150); border: 4px solid "
                "#00FFFF; border-radius: 0px;");
}

void MotorWidget::updateData(double onYanma, double arkaYanma) {
  m_onYanmaVal = onYanma;
  m_arkaYanmaVal = arkaYanma;
  update(); // Trigger repaint
}

void MotorWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  // DISABLE Antialiasing for pixel art look
  // p.setRenderHint(QPainter::Antialiasing);

  int w = width();
  int h = height();
  int cx = w / 2;

  // Maksimum alev için 20 bar baz alıyoruz
  double maxBar = 20.0;
  double combinedVal = (m_onYanmaVal + m_arkaYanmaVal);

  // Alev maksimum uzunluğu (widget'ın boyuna göre ayarlayabiliriz, şimdilik
  // sabit 150)
  double maxFlameLength = 150.0;
  double flameLength = (combinedVal / maxBar) * maxFlameLength;
  flameLength = qBound(0.0, flameLength, maxFlameLength);

  // Motor widgetini alta al: Nozzle'ın bitiş noktası, widgetin en altından (max
  // alev + boşluk) kadar yukarda olsun.
  int nozzleBottom = h - 20 - (int)maxFlameLength;

  // --- Draw Motor/Rocket Body ---
  // Pixel art style: Use simple Rects with black outlines
  int motorWidth = 60;
  int motorHeight = 120;
  // Motor gövdesi nozzle'ın 30px üstünden başlar
  QRect motorRect(cx - motorWidth / 2, nozzleBottom - 30 - motorHeight,
                  motorWidth, motorHeight);

  // Metal body
  p.setBrush(QColor(80, 80, 90));
  p.setPen(QPen(Qt::black, 3)); // Thick borders
  p.drawRect(motorRect);        // Draw simple rect, no rounded

  // Rivets (Pixel details)
  p.setBrush(QColor(40, 40, 40));
  p.setPen(Qt::NoPen);
  p.drawRect(motorRect.left() + 5, motorRect.top() + 5, 8, 8);
  p.drawRect(motorRect.right() - 13, motorRect.top() + 5, 8, 8);
  p.drawRect(motorRect.left() + 5, motorRect.bottom() - 13, 8, 8);
  p.drawRect(motorRect.right() - 13, motorRect.bottom() - 13, 8, 8);

  // Nozzle
  QPolygon nozzle;
  nozzle << QPoint(cx - 20, motorRect.bottom())
         << QPoint(cx + 20, motorRect.bottom())
         // Zig-zag / blocky steps for nozzle instead of smooth lines
         << QPoint(cx + 20, motorRect.bottom() + 10)
         << QPoint(cx + 35, motorRect.bottom() + 10)
         << QPoint(cx + 35, motorRect.bottom() + 30)
         << QPoint(cx - 35, motorRect.bottom() + 30)
         << QPoint(cx - 35, motorRect.bottom() + 10)
         << QPoint(cx - 20, motorRect.bottom() + 10);

  p.setBrush(QColor(40, 40, 40));
  p.setPen(QPen(Qt::black, 3));
  p.drawPolygon(nozzle);

  // --- Draw Flame ---
  if (flameLength > 10) {
    // blocky flame
    int nozzleBottom = motorRect.bottom() + 30;

    // Core
    p.setBrush(QColor(255, 255, 0)); // Pure Yellow
    p.setPen(Qt::NoPen);
    p.drawRect(cx - 10, nozzleBottom, 20, flameLength * 0.4);

    // Middle
    p.setBrush(QColor(255, 140, 0)); // Orange
    p.drawRect(cx - 15, nozzleBottom + flameLength * 0.4, 30,
               flameLength * 0.3);

    // Tip
    p.setBrush(QColor(255, 0, 0)); // Red
    p.drawRect(cx - 5, nozzleBottom + flameLength * 0.7, 10, flameLength * 0.3);

    // Random "pixels" sparks
    if (combinedVal > 0) {
      p.setBrush(QColor(255, 255, 255));
      p.drawRect(cx - 20, nozzleBottom + flameLength / 2, 4, 4);
      p.drawRect(cx + 20, nozzleBottom + flameLength / 3, 4, 4);
    }
  }
}

// =======================
// MainWindow Implementation
// =======================

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  m_port1 = new QSerialPort(this);
  m_port2 = new QSerialPort(this);
  m_port3 = new QSerialPort(this);
  m_port4 = new QSerialPort(this);

  m_startTime = QDateTime::currentMSecsSinceEpoch();

  m_sequenceTimer = new QTimer(this);
  connect(m_sequenceTimer, &QTimer::timeout, this, &MainWindow::updateSequence);

  // m_heartbeatTimer removed

  m_modbusTimer = new QTimer(this);
  connect(m_modbusTimer, &QTimer::timeout, this, &MainWindow::pollModbus);

  // Create Record Timer if periodic sampling is preferred. We will use a fast
  // timer to sample UI labels or variables to CSV
  m_recordTimer = new QTimer(this);
  connect(m_recordTimer, &QTimer::timeout, this, &MainWindow::recordDataLine);

  // UI Update Timer — 50ms = 20 Hz to avoid UI lag
  m_uiUpdateTimer = new QTimer(this);
  connect(m_uiUpdateTimer, &QTimer::timeout, this, &MainWindow::updateUI);
  m_uiUpdateTimer->start(50);

  connect(m_port1, &QSerialPort::readyRead, this, &MainWindow::readPort1);
  connect(m_port1, &QSerialPort::errorOccurred, this,
          &MainWindow::handleError1);

  connect(m_port2, &QSerialPort::readyRead, this, &MainWindow::readPort2);
  connect(m_port2, &QSerialPort::errorOccurred, this,
          &MainWindow::handleError2);

  connect(m_port3, &QSerialPort::readyRead, this, &MainWindow::readPort3);
  connect(m_port3, &QSerialPort::errorOccurred, this,
          &MainWindow::handleError3);

  connect(m_port4, &QSerialPort::readyRead, this, &MainWindow::readPort4);
  connect(m_port4, &QSerialPort::errorOccurred, this,
          &MainWindow::handleError4);

  // --- Theme Setup ---
  // Font
  QFont pixelFont("Courier New", 16);
  pixelFont.setStyleStrategy(QFont::NoAntialias);
  pixelFont.setBold(true);
  QApplication::setFont(pixelFont);

  initUI();
  setupCharts();
  applyTheme();

  // Populate ports
  const auto infos = QSerialPortInfo::availablePorts();
  QStringList portNames;
  for (const QSerialPortInfo &info : infos) {
    portNames << info.portName();
  }
  QStringList baudRates = {"9600", "19200", "38400", "57600", "115200"};

  m_cbPort1->addItems(portNames);
  m_cbBaud1->addItems(baudRates);
  m_cbBaud1->setCurrentText("9600");
  m_cbPort2->addItems(portNames);
  m_cbBaud2->addItems(baudRates);
  m_cbBaud2->setCurrentText("115200");
  m_cbPort3->addItems(portNames);
  m_cbBaud3->addItems(baudRates);
  m_cbBaud3->setCurrentText("115200");
  m_cbPort4->addItems(portNames);
  m_cbBaud4->addItems(baudRates);
  m_cbBaud4->setCurrentText("115200");
}

MainWindow::~MainWindow() {
  if (m_port1->isOpen())
    m_port1->close();
  if (m_port2->isOpen())
    m_port2->close();
  if (m_port3->isOpen())
    m_port3->close();
  if (m_port4->isOpen())
    m_port4->close();

  auto closeLog = [](QTextStream *&stream, QFile *&file) {
    delete stream;
    stream = nullptr;
    if (file) {
      if (file->isOpen())
        file->close();
      delete file;
      file = nullptr;
    }
  };
  closeLog(m_streamModbus, m_logModbus);
  closeLog(m_streamArabaLdc, m_logArabaLdc);
  closeLog(m_streamTupLdc, m_logTupLdc);
}

void MainWindow::initUI() {
  QWidget *central = new QWidget;
  central->setObjectName("CentralWidget"); // Named for styling
  setCentralWidget(central);
  QHBoxLayout *mainLayout = new QHBoxLayout(central);

  // Apply Stylesheet
  // Defer to applyTheme()

  // --- Left Panel: Controls & Data ---
  QVBoxLayout *leftLayout = new QVBoxLayout();

  m_cmbThemeSelect = new QComboBox();
  m_cmbThemeSelect->addItems(
      {"Karanlık Tema", "Aydınlık Tema", "Christmas Tema"});
  leftLayout->addWidget(m_cmbThemeSelect);

  // 1. Connection
  QGroupBox *connGroup = new QGroupBox("Bağlantı");
  QGridLayout *connLayout = new QGridLayout(connGroup);

  auto setupPortRow = [&](int row, const QString &title, QComboBox *&cbP,
                          QComboBox *&cbB, QPushButton *&btnC,
                          QPushButton *&btnD, QLabel *&lblS) {
    cbP = new QComboBox();
    cbB = new QComboBox();
    btnC = new QPushButton("Bağlan");
    btnD = new QPushButton("Kes");
    btnD->setEnabled(false);
    lblS = new QLabel("Yok");
    lblS->setStyleSheet(
        "color: gray; font-size: 12px; border: none; background: transparent;");

    connLayout->addWidget(new QLabel(title), row, 0);
    connLayout->addWidget(cbP, row, 1);
    connLayout->addWidget(cbB, row, 2);
    connLayout->addWidget(btnC, row, 3);
    connLayout->addWidget(btnD, row, 4);
    connLayout->addWidget(lblS, row, 5);
  };

  setupPortRow(0, "P1(Modbus)", m_cbPort1, m_cbBaud1, m_btnConnect1,
               m_btnDisconnect1, m_status1);
  setupPortRow(1, "P2(Araba)", m_cbPort2, m_cbBaud2, m_btnConnect2,
               m_btnDisconnect2, m_status2);
  setupPortRow(2, "P3(Tüp)", m_cbPort3, m_cbBaud3, m_btnConnect3,
               m_btnDisconnect3, m_status3);
  setupPortRow(3, "P4(Kontrol)", m_cbPort4, m_cbBaud4, m_btnConnect4,
               m_btnDisconnect4, m_status4);

  m_btnRefreshPorts = new QPushButton("Portları Yenile");
  connLayout->addWidget(m_btnRefreshPorts, 4, 0, 1, 6);

  // 2. Control Panel
  QGroupBox *ctrlGroup = new QGroupBox("Kontrol Paneli");
  QGridLayout *ctrlLayout = new QGridLayout(ctrlGroup);

  m_chkTestMode = new QCheckBox("Test Modu");
  m_chkManualMode = new QCheckBox("Manuel Mod");

  // Igniter & Valve Buttons
  m_btnIgniterOn = new QPushButton("Ateşleyici AÇ");
  m_btnIgniterOff = new QPushButton("Ateşleyici KAPAT");
  m_btnValveOn = new QPushButton("Valf AÇ");
  m_btnValveOff = new QPushButton("Valf KAPAT");

  // Deferred styling to applyTheme

  m_btnEmergency = new QPushButton("ACİL DURDUR");

  m_btnJingleBells = new QPushButton("Jingle Bells");
  m_btnJingleBells->setVisible(false);

  // Sequence Elements — only igniter-to-valve interval in ms
  m_spinRoleTime = new QDoubleSpinBox();
  m_spinRoleTime->setRange(0.0, 65535.0);
  m_spinRoleTime->setDecimals(0);
  m_spinRoleTime->setSingleStep(1);
  m_spinRoleTime->setSuffix(" ms");
  m_spinRoleTime->setValue(3200);

  m_btnStartSequence = new QPushButton("OTOMATİK BAŞLAT");
  // Deferred styling to applyTheme

  // Layout Placement
  ctrlLayout->addWidget(m_chkTestMode, 0, 0);
  ctrlLayout->addWidget(m_chkManualMode, 0, 1);

  // Row 1: Igniter buttons
  ctrlLayout->addWidget(m_btnIgniterOn, 1, 0);
  ctrlLayout->addWidget(m_btnIgniterOff, 1, 1);

  // Row 2: Valve buttons
  ctrlLayout->addWidget(m_btnValveOn, 2, 0);
  ctrlLayout->addWidget(m_btnValveOff, 2, 1);

  // Row 3: Sequence spinbox — igniter-to-valve interval only
  ctrlLayout->addWidget(new QLabel("Igniter-Valf Arası (ms):"), 3, 0);
  ctrlLayout->addWidget(m_spinRoleTime, 3, 1);

  // Row 4: Start Sequence
  ctrlLayout->addWidget(m_btnStartSequence, 4, 0, 1, 2);

  // Row 5: Emergency
  ctrlLayout->addWidget(m_btnEmergency, 5, 0, 1, 2);

  m_btnRecord = new QPushButton("Kaydı Başlat");
  ctrlLayout->addWidget(m_btnRecord, 6, 0, 1, 2);

  // 3. Sensor Data
  QGroupBox *dataGroup = new QGroupBox("Sensör Verileri");
  QGridLayout *dataLayout = new QGridLayout(dataGroup);

  m_lblOnYanma = new QLabel("0.00");
  m_lblArkaYanma = new QLabel("0.00");
  m_lblN2O = new QLabel("0.00");
  m_lblTupLdc = new QLabel("0.00");
  m_lblArabaLdc = new QLabel("0.00");
  m_lblTime = new QLabel("0");

  // Deferred styling to applyTheme

  dataLayout->addWidget(new QLabel("Ön Yanma:"), 0, 0);
  dataLayout->addWidget(m_lblOnYanma, 0, 1);
  dataLayout->addWidget(new QLabel("Arka Yanma:"), 1, 0);
  dataLayout->addWidget(m_lblArkaYanma, 1, 1);
  dataLayout->addWidget(new QLabel("N2O:"), 2, 0);
  dataLayout->addWidget(m_lblN2O, 2, 1);
  dataLayout->addWidget(new QLabel("Tüp Loadcell:"), 3, 0);
  dataLayout->addWidget(m_lblTupLdc, 3, 1);
  dataLayout->addWidget(new QLabel("Araba Loadcell:"), 4, 0);
  dataLayout->addWidget(m_lblArabaLdc, 4, 1);
  dataLayout->addWidget(new QLabel("Zaman (s):"), 5, 0);
  dataLayout->addWidget(m_lblTime, 5, 1);

  leftLayout->addWidget(connGroup);
  leftLayout->addWidget(ctrlGroup);
  leftLayout->addWidget(dataGroup);
  m_lblCountdown = new QLabel("");
  leftLayout->addWidget(m_lblCountdown);

  // Sol alt özel butonlar
  leftLayout->addWidget(m_btnJingleBells);

  leftLayout->addStretch();

  // --- Center: Visualization ---
  QVBoxLayout *centerLayout = new QVBoxLayout();

  QVBoxLayout *controlMsgLayout = new QVBoxLayout();
  m_lblControlMsg = new QPlainTextEdit();
  m_lblControlMsg->setReadOnly(true);
  m_lblControlMsg->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_lblControlMsg->setLineWrapMode(QPlainTextEdit::WidgetWidth);
  m_lblControlMsg->appendPlainText("Bekleniyor...");
  m_lblControlMsg->setMinimumHeight(
      200); // Yükseklik azaltıldı ki butonlara binmesin

  m_btnCancel = new QPushButton("İPTAL");
  m_btnStart = new QPushButton("BAŞLAT");
  m_btnReady = new QPushButton("HAZIR");
  m_btnFire = new QPushButton("ATEŞLE");

  m_btnCancel->setEnabled(false);
  m_btnStart->setEnabled(false);
  m_btnReady->setEnabled(false);
  m_btnFire->setEnabled(false);

  controlMsgLayout->addWidget(m_lblControlMsg, 1);
  controlMsgLayout->addSpacing(
      20); // Butonları mesaj kutusundan uzaklaştırmak için boşluk

  QGroupBox *loopGroup = new QGroupBox("ATEŞLEME LOOPU");
  QVBoxLayout *loopLayout = new QVBoxLayout(loopGroup);
  loopLayout->addWidget(m_btnCancel);
  loopLayout->addWidget(m_btnStart);
  loopLayout->addWidget(m_btnReady);
  loopLayout->addWidget(m_btnFire);

  controlMsgLayout->addWidget(loopGroup);

  m_motorWidget = new MotorWidget();

  centerLayout->addLayout(controlMsgLayout, 1);
  centerLayout->addWidget(m_motorWidget,
                          2); // Motor widget'ına daha fazla alan ver

  // --- Right: Charts ---
  QVBoxLayout *rightLayout = new QVBoxLayout();

  // Create view placeholders (setupCharts will fill them)
  m_pressureView = new QChartView();
  m_loadcellView = new QChartView();

  rightLayout->addWidget(m_pressureView);
  rightLayout->addWidget(m_loadcellView);

  // Add everything to main layout
  // Proportions: Left 25%, Center 25%, Right 50%
  mainLayout->addLayout(leftLayout, 5);
  mainLayout->addLayout(centerLayout, 5);
  mainLayout->addLayout(rightLayout, 10);

  // Connect signals
  connect(m_btnConnect1, &QPushButton::clicked, this, &MainWindow::openPort1);
  connect(m_btnDisconnect1, &QPushButton::clicked, this,
          &MainWindow::closePort1);
  connect(m_btnConnect2, &QPushButton::clicked, this, &MainWindow::openPort2);
  connect(m_btnDisconnect2, &QPushButton::clicked, this,
          &MainWindow::closePort2);
  connect(m_btnConnect3, &QPushButton::clicked, this, &MainWindow::openPort3);
  connect(m_btnDisconnect3, &QPushButton::clicked, this,
          &MainWindow::closePort3);
  connect(m_btnConnect4, &QPushButton::clicked, this, &MainWindow::openPort4);
  connect(m_btnDisconnect4, &QPushButton::clicked, this,
          &MainWindow::closePort4);
  connect(m_btnRefreshPorts, &QPushButton::clicked, this,
          &MainWindow::refreshPorts);
  connect(m_btnRecord, &QPushButton::clicked, this,
          &MainWindow::toggleRecording);
  connect(m_cmbThemeSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::changeTheme);

  // Igniter & Valve button signals
  connect(m_btnIgniterOn, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0xAA, 0xAA);
    logEventToFiles("Atesleyici_AC");
  });
  connect(m_btnIgniterOff, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0xAA, 0x00);
    logEventToFiles("Atesleyici_KAPAT");
  });
  connect(m_btnValveOn, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0xBB, 0xBB);
    logEventToFiles("Valf_AC");
  });
  connect(m_btnValveOff, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0xBB, 0x00);
    logEventToFiles("Valf_KAPAT");
  });

  connect(m_btnEmergency, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0x00, 0x00); // All off
    logEventToFiles("ACIL_DURDUR");
  });

  connect(m_btnJingleBells, &QPushButton::clicked, this, [this]() {
    sendControlCommand(0xFF, 0xFF);
    logEventToFiles("JINGLE_BELLS");
  });

  connect(m_btnCancel, &QPushButton::clicked, this, [this]() {
    if (m_port4 && m_port4->isOpen()) {
      char data = 0x00;
      m_port4->write(&data, 1);
    }
    logEventToFiles("CANCEL_0x00");
    // Optionally disable buttons until next state
    m_btnStart->setEnabled(m_chkManualMode->isChecked());
    m_btnReady->setEnabled(false);
    m_btnFire->setEnabled(false);
    m_firePressCount = 0;
  });

  connect(m_btnStart, &QPushButton::clicked, this, [this]() {
    if (m_chkManualMode->isChecked()) {
      sendControlCommand(0xAA, 0xBB); // Checksum FD AA BB + calculated
      logEventToFiles("START_MANUAL_CHECKSUM");
    } else {
      if (m_port4 && m_port4->isOpen()) {
        char data = 0x0F;
        m_port4->write(&data, 1);
      }
      logEventToFiles("START_0x0F");
      m_btnStart->setEnabled(false);
    }
  });

  connect(m_btnReady, &QPushButton::clicked, this, [this]() {
    if (m_port4 && m_port4->isOpen()) {
      char data = 0x0F;
      m_port4->write(&data, 1);
    }
    logEventToFiles("READY_0x0F");
    if (!m_chkManualMode->isChecked()) {
      m_btnReady->setEnabled(false);
    }
  });

  connect(m_btnFire, &QPushButton::clicked, this, [this]() {
    if (m_port4 && m_port4->isOpen()) {
      char data = 0x0F;
      m_port4->write(&data, 1);
    }
    logEventToFiles("FIRE_0x0F");

    if (m_chkManualMode->isChecked()) {
      m_firePressCount++;
      if (m_firePressCount >= 2) {
        m_btnFire->setEnabled(false);
        m_btnStart->setEnabled(true);
      }
    } else {
      m_btnFire->setEnabled(false);
    }
  });

  connect(m_btnStartSequence, &QPushButton::clicked, this,
          &MainWindow::startSequence);

  // Mode Logic
  auto updateModes = [this]() {
    bool testChecked = m_chkTestMode->isChecked();
    bool manChecked = m_chkManualMode->isChecked();

    m_btnIgniterOn->setEnabled(testChecked);
    m_btnIgniterOff->setEnabled(testChecked);
    m_btnValveOn->setEnabled(testChecked);
    m_btnValveOff->setEnabled(testChecked);
    m_btnStartSequence->setEnabled(!testChecked);
    m_btnEmergency->setEnabled(true);

    if (manChecked) {
      m_btnStart->setEnabled(true);
      m_btnCancel->setEnabled(true);
    } else {
      m_btnStart->setEnabled(false);
      m_btnCancel->setEnabled(false);
      m_btnReady->setEnabled(false);
      m_btnFire->setEnabled(false);
    }
    m_firePressCount = 0;
  };

  connect(m_chkTestMode, &QCheckBox::toggled, this,
          [this, updateModes](bool checked) {
            if (checked && m_chkManualMode->isChecked()) {
              m_chkManualMode->blockSignals(true);
              m_chkManualMode->setChecked(false);
              m_chkManualMode->blockSignals(false);
            }
            updateModes();
          });
  connect(m_chkManualMode, &QCheckBox::toggled, this,
          [this, updateModes](bool checked) {
            if (checked && m_chkTestMode->isChecked()) {
              m_chkTestMode->blockSignals(true);
              m_chkTestMode->setChecked(false);
              m_chkTestMode->blockSignals(false);
            }
            updateModes();
          });

  // Disable buttons by default
  m_chkTestMode->setChecked(false);
  m_chkManualMode->setChecked(false);
  updateModes();
}

void MainWindow::setupCharts() {

  // Pressure Chart
  QChart *pressureChart = new QChart();
  pressureChart->setTitle("Pressure Sensors");
  pressureChart->setTitleBrush(QBrush(QColor("#00FF00")));
  pressureChart->setBackgroundBrush(Qt::NoBrush); // Transparent

  m_seriesOnYanma = new QLineSeries();
  m_seriesOnYanma->setName("On Yanma");
  QPen p1(QColor("#00FFFF")); // Cyan
  p1.setWidth(2);
  m_seriesOnYanma->setPen(p1);

  m_seriesArkaYanma = new QLineSeries();
  m_seriesArkaYanma->setName("Arka Yanma");
  QPen p2(QColor("#0088FF")); // Slightly different blue
  p2.setWidth(2);
  m_seriesArkaYanma->setPen(p2);

  m_seriesN2O = new QLineSeries();
  m_seriesN2O->setName("N2O (Tüp Basınç)");
  QPen p3(QColor("#FF00FF")); // Magenta
  p3.setWidth(2);
  m_seriesN2O->setPen(p3);

  pressureChart->addSeries(m_seriesOnYanma);
  pressureChart->addSeries(m_seriesArkaYanma);
  pressureChart->addSeries(m_seriesN2O);

  m_axisX_Pressure = new QValueAxis();
  m_axisX_Pressure->setTitleText("Time (s)");
  m_axisX_Pressure->setLabelsBrush(QBrush(QColor("#00FF00")));
  m_axisX_Pressure->setTitleBrush(QBrush(QColor("#00FF00")));
  m_axisX_Pressure->setGridLinePen(QPen(QColor(0, 100, 0))); // Dark green grid

  m_axisY_Pressure = new QValueAxis();
  m_axisY_Pressure->setTitleText("Basınç (bar)");
  m_axisY_Pressure->setLabelsBrush(QBrush(QColor("#00FF00")));
  m_axisY_Pressure->setTitleBrush(QBrush(QColor("#00FF00")));
  m_axisY_Pressure->setGridLinePen(QPen(QColor(0, 100, 0)));

  pressureChart->addAxis(m_axisX_Pressure, Qt::AlignBottom);
  pressureChart->addAxis(m_axisY_Pressure, Qt::AlignLeft);

  m_seriesOnYanma->attachAxis(m_axisX_Pressure);
  m_seriesOnYanma->attachAxis(m_axisY_Pressure);
  m_seriesArkaYanma->attachAxis(m_axisX_Pressure);
  m_seriesArkaYanma->attachAxis(m_axisY_Pressure);
  m_seriesN2O->attachAxis(m_axisX_Pressure);
  m_seriesN2O->attachAxis(m_axisY_Pressure);

  // Create Legend
  pressureChart->legend()->setLabelColor(QColor("#00FF00"));

  m_pressureView->setChart(pressureChart);
  m_pressureView->setRenderHint(QPainter::Antialiasing);
  m_pressureView->setBackgroundBrush(Qt::NoBrush); // Transparent View

  // Loadcell Chart
  QChart *loadcellChart = new QChart();
  loadcellChart->setTitle("Load Cells");
  loadcellChart->setTitleBrush(QBrush(QColor("#00FF00")));
  loadcellChart->setBackgroundBrush(Qt::NoBrush);

  m_seriesTup = new QLineSeries();
  m_seriesTup->setName("Tup");
  m_seriesTup->setPen(p1);

  m_seriesAraba = new QLineSeries();
  m_seriesAraba->setName("Araba");
  m_seriesAraba->setPen(p2);

  loadcellChart->addSeries(m_seriesTup);
  loadcellChart->addSeries(m_seriesAraba);

  m_axisX_Loadcell = new QValueAxis();
  m_axisX_Loadcell->setTitleText("Time (s)");
  m_axisX_Loadcell->setLabelsBrush(QBrush(QColor("#00FF00")));
  m_axisX_Loadcell->setTitleBrush(QBrush(QColor("#00FF00")));
  m_axisX_Loadcell->setGridLinePen(QPen(QColor(0, 100, 0)));

  m_axisY_Loadcell = new QValueAxis();
  m_axisY_Loadcell->setTitleText("Force");
  m_axisY_Loadcell->setLabelsBrush(QBrush(QColor("#00FF00")));
  m_axisY_Loadcell->setTitleBrush(QBrush(QColor("#00FF00")));
  m_axisY_Loadcell->setGridLinePen(QPen(QColor(0, 100, 0)));

  loadcellChart->addAxis(m_axisX_Loadcell, Qt::AlignBottom);
  loadcellChart->addAxis(m_axisY_Loadcell, Qt::AlignLeft);

  m_seriesTup->attachAxis(m_axisX_Loadcell);
  m_seriesTup->attachAxis(m_axisY_Loadcell);
  m_seriesAraba->attachAxis(m_axisX_Loadcell);
  m_seriesAraba->attachAxis(m_axisY_Loadcell);

  loadcellChart->legend()->setLabelColor(QColor("#00FF00"));

  m_loadcellView->setChart(loadcellChart);
  m_loadcellView->setRenderHint(QPainter::Antialiasing);
  m_loadcellView->setBackgroundBrush(Qt::NoBrush);
}

void MainWindow::openPort1() {
  m_port1->setPortName(m_cbPort1->currentText());
  m_port1->setBaudRate(m_cbBaud1->currentText().toInt());
  if (m_port1->open(QIODevice::ReadWrite)) {
    m_btnConnect1->setEnabled(false);
    m_btnDisconnect1->setEnabled(true);
    m_cbPort1->setEnabled(false);
    m_cbBaud1->setEnabled(false);
    m_status1->setText("OK");
    m_status1->setStyleSheet("color: green; font-size:12px; border:none;");
    m_modbusTimer->start(40); // 40ms = 25 Hz polling
  } else {
    QMessageBox::critical(this, "Error P1", "Yanlış veya meşgul port seçildi!");
  }
}

void MainWindow::closePort1() {
  m_port1->close();
  m_modbusTimer->stop();
  m_btnConnect1->setEnabled(true);
  m_btnDisconnect1->setEnabled(false);
  m_cbPort1->setEnabled(true);
  m_cbBaud1->setEnabled(true);
  m_status1->setText("Yok");
  m_status1->setStyleSheet("color: gray; font-size:12px; border:none;");
}

void MainWindow::openPort2() {
  m_port2->setPortName(m_cbPort2->currentText());
  m_port2->setBaudRate(m_cbBaud2->currentText().toInt());
  if (m_port2->open(QIODevice::ReadOnly)) {
    m_btnConnect2->setEnabled(false);
    m_btnDisconnect2->setEnabled(true);
    m_cbPort2->setEnabled(false);
    m_cbBaud2->setEnabled(false);
    m_status2->setText("OK");
    m_status2->setStyleSheet("color: green; font-size:12px; border:none;");
  } else {
    QMessageBox::critical(this, "Error P2", "Yanlış veya meşgul port seçildi!");
  }
}

void MainWindow::closePort2() {
  m_port2->close();
  m_btnConnect2->setEnabled(true);
  m_btnDisconnect2->setEnabled(false);
  m_cbPort2->setEnabled(true);
  m_cbBaud2->setEnabled(true);
  m_status2->setText("Yok");
  m_status2->setStyleSheet("color: gray; font-size:12px; border:none;");
}

void MainWindow::openPort3() {
  m_port3->setPortName(m_cbPort3->currentText());
  m_port3->setBaudRate(m_cbBaud3->currentText().toInt());
  if (m_port3->open(QIODevice::ReadOnly)) {
    m_btnConnect3->setEnabled(false);
    m_btnDisconnect3->setEnabled(true);
    m_cbPort3->setEnabled(false);
    m_cbBaud3->setEnabled(false);
    m_status3->setText("OK");
    m_status3->setStyleSheet("color: green; font-size:12px; border:none;");
  } else {
    QMessageBox::critical(this, "Error P3", "Yanlış veya meşgul port seçildi!");
  }
}

void MainWindow::closePort3() {
  m_port3->close();
  m_btnConnect3->setEnabled(true);
  m_btnDisconnect3->setEnabled(false);
  m_cbPort3->setEnabled(true);
  m_cbBaud3->setEnabled(true);
  m_status3->setText("Yok");
  m_status3->setStyleSheet("color: gray; font-size:12px; border:none;");
}

void MainWindow::openPort4() {
  m_port4->setPortName(m_cbPort4->currentText());
  m_port4->setBaudRate(m_cbBaud4->currentText().toInt());
  m_port4->setDataBits(QSerialPort::Data8);
  m_port4->setParity(QSerialPort::NoParity);
  m_port4->setStopBits(QSerialPort::OneStop);
  m_port4->setFlowControl(QSerialPort::NoFlowControl);

  if (m_port4->open(QIODevice::ReadWrite)) {
    m_btnConnect4->setEnabled(false);
    m_btnDisconnect4->setEnabled(true);
    m_cbPort4->setEnabled(false);
    m_cbBaud4->setEnabled(false);
    m_status4->setText("OK");
    m_status4->setStyleSheet("color: green; font-size:12px; border:none;");
  } else {
    QMessageBox::critical(this, "Error P4", "Yanlış veya meşgul port seçildi!");
  }
}

void MainWindow::closePort4() {
  m_port4->close();
  m_btnConnect4->setEnabled(true);
  m_btnDisconnect4->setEnabled(false);
  m_cbPort4->setEnabled(true);
  m_cbBaud4->setEnabled(true);
  m_status4->setText("Yok");
  m_status4->setStyleSheet("color: gray; font-size:12px; border:none;");
}

void MainWindow::refreshPorts() {
  const auto infos = QSerialPortInfo::availablePorts();
  QStringList portNames;
  for (const QSerialPortInfo &info : infos) {
    portNames << info.portName();
  }

  auto updateCb = [&](QComboBox *cb) {
    if (!cb->isEnabled())
      return; // If port is open, combo is disabled
    QString current = cb->currentText();
    cb->clear();
    cb->addItems(portNames);
    if (portNames.contains(current)) {
      cb->setCurrentText(current);
    }
  };

  updateCb(m_cbPort1);
  updateCb(m_cbPort2);
  updateCb(m_cbPort3);
  updateCb(m_cbPort4);
}

void MainWindow::handleError1(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError)
    closePort1();
}
void MainWindow::handleError2(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError)
    closePort2();
}
void MainWindow::handleError3(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError)
    closePort3();
}
void MainWindow::handleError4(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError)
    closePort4();
}

void MainWindow::pollModbus() {
  if (m_port1->isOpen()) {
    QByteArray cmd;
    cmd.append((char)0x01);
    cmd.append((char)0x04);
    cmd.append((char)0x00);
    cmd.append((char)0x00);
    cmd.append((char)0x00);
    cmd.append((char)0x03);
    cmd.append((char)0xB0);
    cmd.append((char)0x0B);
    m_port1->write(cmd);
  }
}

void MainWindow::readPort1() {
  m_buffer1.append(m_port1->readAll());

  if (m_buffer1.size() > 1024)
    m_buffer1.clear();

  // Search for 01 04 06
  while (m_buffer1.size() >= 11) {
    int headerIdx = -1;
    for (int i = 0; i <= m_buffer1.size() - 11; ++i) {
      if ((unsigned char)m_buffer1[i] == 0x01 &&
          (unsigned char)m_buffer1[i + 1] == 0x04 &&
          (unsigned char)m_buffer1[i + 2] == 0x06) {
        headerIdx = i;
        break;
      }
    }

    if (headerIdx == -1) {
      m_buffer1.remove(0, qMax(0, m_buffer1.size() - 2));
      break;
    }

    m_buffer1.remove(0, headerIdx); // Align to header

    if (m_buffer1.size() < 11)
      break; // Need more bytes

    // Extract D1D2, D3D4, D5D6 (Big Endian)
    uint16_t rawOn =
        ((unsigned char)m_buffer1[3] << 8) | (unsigned char)m_buffer1[4];
    uint16_t rawArka =
        ((unsigned char)m_buffer1[5] << 8) | (unsigned char)m_buffer1[6];
    uint16_t rawN2O =
        ((unsigned char)m_buffer1[7] << 8) | (unsigned char)m_buffer1[8];

    // Scale to bar: 4000 -> 0 bar, 20000 -> 400 bar
    m_valOnYanma = rawToBar(rawOn);
    m_valArkaYanma = rawToBar(rawArka);
    m_valN2O = rawToBar(rawN2O);

    // UI updates deferred to updateUI() timer (20 Hz)

    // Event-driven logging: write to modbus log immediately
    if (m_isRecording && m_streamModbus) {
      qint64 tMs = QDateTime::currentMSecsSinceEpoch() - m_recordStartTime;
      *m_streamModbus << tMs << "," << QString::number(m_valOnYanma, 'f', 2)
                      << "," << QString::number(m_valArkaYanma, 'f', 2) << ","
                      << QString::number(m_valN2O, 'f', 2) << "\n";
    }

    m_buffer1.remove(0, 11);
  }
}

void MainWindow::readPort2() {
  while (m_port2->canReadLine()) {
    QString line = QString::fromLatin1(m_port2->readLine()).trimmed();
    if (!line.isEmpty()) {
      QRegularExpression re("\\x02\\s*D([+-]?\\d+)");
      QRegularExpressionMatch match = re.match(line);
      if (match.hasMatch()) {
        bool ok;
        double val = match.captured(1).toDouble(&ok);
        if (ok) {
          m_valArabaLdc = val / 1000.0;
          // UI updates deferred to updateUI() timer (20 Hz)

          // Event-driven logging
          if (m_isRecording && m_streamArabaLdc) {
            qint64 tMs =
                QDateTime::currentMSecsSinceEpoch() - m_recordStartTime;
            *m_streamArabaLdc << tMs << ","
                              << QString::number(m_valArabaLdc, 'f', 4) << "\n";
          }
        }
      }
    }
  }
}

void MainWindow::readPort3() {
  while (m_port3->canReadLine()) {
    QString line = QString::fromLatin1(m_port3->readLine()).trimmed();
    if (!line.isEmpty()) {
      QRegularExpression re("=MG([+-]?\\d+)g");
      QRegularExpressionMatch match = re.match(line);
      if (match.hasMatch()) {
        bool ok;
        double val = match.captured(1).toDouble(&ok);
        if (ok) {
          m_valTupLdc = val / 1000.0;
          // UI updates deferred to updateUI() timer (20 Hz)

          // Event-driven logging
          if (m_isRecording && m_streamTupLdc) {
            qint64 tMs =
                QDateTime::currentMSecsSinceEpoch() - m_recordStartTime;
            *m_streamTupLdc << tMs << ","
                            << QString::number(m_valTupLdc, 'f', 4) << "\n";
          }
        }
      }
    }
  }
}

void MainWindow::readPort4() {
  bool newMessage = false;
  while (m_port4->canReadLine()) {
    QString line = QString::fromLatin1(m_port4->readLine()).trimmed();
    if (!line.isEmpty()) {
      logEventToFiles("GELEN_MESAJ:" + line);
      m_lblControlMsg->appendPlainText(line);
      // Keep only last 50 lines
      if (m_lblControlMsg->document()->blockCount() > 50) {
        QTextCursor cursor = m_lblControlMsg->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar(); // remove newline
      }
      m_lblControlMsg->moveCursor(QTextCursor::End);

      // Parse commands
      if (line == "1") {
        if (!m_chkManualMode->isChecked())
          m_btnStart->setEnabled(true);
        m_btnCancel->setEnabled(true);
      } else if (line == "2") {
        m_btnReady->setEnabled(true);
        if (m_chkManualMode->isChecked())
          m_btnStart->setEnabled(false);
        m_btnCancel->setEnabled(true);
      } else if (line == "3") {
        m_btnFire->setEnabled(true);
        if (m_chkManualMode->isChecked())
          m_btnReady->setEnabled(false);
        m_btnCancel->setEnabled(true);
        if (m_chkManualMode->isChecked()) {
          m_firePressCount = 0; // reset
        }
      }
    }
  }
}

void MainWindow::logEventToFiles(const QString &eventName) {
  // Always log to persistent file
  QString desktopPath =
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  QString loglarPath = desktopPath + "/loglar";
  QDir dir(loglarPath);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  QFile globalLog(loglarPath + "/sabit_log.csv");
  if (globalLog.open(QIODevice::WriteOnly | QIODevice::Append |
                     QIODevice::Text)) {
    QTextStream stream(&globalLog);
    qint64 absoluteMs = QDateTime::currentMSecsSinceEpoch();
    stream << absoluteMs << "," << eventName << "\n";
    globalLog.close();
  }

  // Also log to recording files if active
  if (m_isRecording) {
    qint64 tMs = QDateTime::currentMSecsSinceEpoch() - m_recordStartTime;
    QString logLine = QString("%1,EVENT:%2\n").arg(tMs).arg(eventName);
    if (m_streamModbus)
      *m_streamModbus << logLine;
    if (m_streamArabaLdc)
      *m_streamArabaLdc << logLine;
    if (m_streamTupLdc)
      *m_streamTupLdc << logLine;
  }
}

void MainWindow::toggleRecording() {
  if (!m_isRecording) {
    QString desktopPath =
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString loglarPath = desktopPath + "/loglar";
    QDir dir(loglarPath);
    if (!dir.exists()) {
      dir.mkpath(".");
    }
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    // Open 3 separate log files
    auto openLog = [&](const QString &prefix, const QString &header,
                       QFile *&file, QTextStream *&stream) -> bool {
      QString path = QString("%1/%2_%3.csv").arg(loglarPath, prefix, timestamp);
      file = new QFile(path);
      if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        stream = new QTextStream(file);
        *stream << header << "\n";
        return true;
      }
      delete file;
      file = nullptr;
      return false;
    };

    bool ok1 =
        openLog("modbus", "Zaman(ms),OnYanma(bar),ArkaYanma(bar),N2O(bar)",
                m_logModbus, m_streamModbus);
    bool ok2 = openLog("araba_ldc", "Zaman(ms),ArabaLoadcell", m_logArabaLdc,
                       m_streamArabaLdc);
    bool ok3 = openLog("tup_ldc", "Zaman(ms),TupLoadcell", m_logTupLdc,
                       m_streamTupLdc);

    if (ok1 && ok2 && ok3) {
      m_isRecording = true;
      m_recordStartTime = QDateTime::currentMSecsSinceEpoch();
      m_btnRecord->setText("Kaydı Durdur");
      m_btnRecord->setStyleSheet(
          "QPushButton { background-color: #AA0000; color: white; font-weight: "
          "bold; border: 2px solid #FF0000; padding: 10px; }");
      m_recordTimer->start(50); // 50ms for time label update
    } else {
      QMessageBox::critical(this, "Error", "Log dosyaları oluşturulamadı.");
      // Cleanup any that opened
      auto cleanup = [](QTextStream *&s, QFile *&f) {
        delete s;
        s = nullptr;
        if (f) {
          if (f->isOpen())
            f->close();
          delete f;
          f = nullptr;
        }
      };
      cleanup(m_streamModbus, m_logModbus);
      cleanup(m_streamArabaLdc, m_logArabaLdc);
      cleanup(m_streamTupLdc, m_logTupLdc);
    }
  } else {
    m_isRecording = false;
    m_recordTimer->stop();

    auto closeLog = [](QTextStream *&stream, QFile *&file) {
      if (stream) {
        stream->flush();
      }
      delete stream;
      stream = nullptr;
      if (file) {
        if (file->isOpen())
          file->close();
        delete file;
        file = nullptr;
      }
    };
    closeLog(m_streamModbus, m_logModbus);
    closeLog(m_streamArabaLdc, m_logArabaLdc);
    closeLog(m_streamTupLdc, m_logTupLdc);

    m_btnRecord->setText("Kaydı Başlat");
    m_btnRecord->setStyleSheet(
        "QPushButton { background-color: #0000AA; color: white; font-weight: "
        "bold; border: 2px solid #0000FF; padding: 10px; }");
  }
}

void MainWindow::recordDataLine() {
  if (m_isRecording) {
    qint64 tMs = QDateTime::currentMSecsSinceEpoch() - m_recordStartTime;
    m_lblTime->setText(QString::number(tMs / 1000.0, 'f', 2));
  }
}

void MainWindow::updateUI() {
  // Update labels (20 Hz throttled)
  m_lblOnYanma->setText(QString::number(m_valOnYanma, 'f', 2) + " bar");
  m_lblArkaYanma->setText(QString::number(m_valArkaYanma, 'f', 2) + " bar");
  m_lblN2O->setText(QString::number(m_valN2O, 'f', 2) + " bar");
  m_lblTupLdc->setText(QString::number(m_valTupLdc, 'f', 4));
  m_lblArabaLdc->setText(QString::number(m_valArabaLdc, 'f', 4));

  // Update motor widget
  m_motorWidget->updateData(m_valOnYanma, m_valArkaYanma);

  // Update charts continuously, regardless of recording
  static qint64 lastUpdateMs = 0;
  if (lastUpdateMs == 0)
    lastUpdateMs = QDateTime::currentMSecsSinceEpoch();
  qint64 currentMs = QDateTime::currentMSecsSinceEpoch();
  double dt = (currentMs - lastUpdateMs) / 1000.0;
  lastUpdateMs = currentMs;

  auto shiftAndAppend = [dt](QLineSeries *series, double newVal) {
    QList<QPointF> points = series->points();
    for (int i = 0; i < points.size(); ++i) {
      points[i].rx() -= dt;
    }
    points.append(QPointF(0, newVal));
    while (!points.isEmpty() && points.first().x() < -10.0) {
      points.removeFirst();
    }
    series->replace(points);
  };

  shiftAndAppend(m_seriesOnYanma, m_valOnYanma);
  shiftAndAppend(m_seriesArkaYanma, m_valArkaYanma);
  shiftAndAppend(m_seriesN2O, m_valN2O);
  shiftAndAppend(m_seriesTup, m_valTupLdc);
  shiftAndAppend(m_seriesAraba, m_valArabaLdc);

  m_axisX_Pressure->setRange(-10, 0);
  m_axisX_Loadcell->setRange(-10, 0);

  m_axisX_Pressure->setTickCount(5);
  m_axisX_Loadcell->setTickCount(5);
  m_axisX_Pressure->setLabelFormat("%.1f");
  m_axisX_Loadcell->setLabelFormat("%.1f");
  m_axisY_Pressure->setRange(
      0,
      std::max(50.0, std::max({m_valOnYanma, m_valArkaYanma, m_valN2O}) * 1.2));
  m_axisY_Loadcell->setRange(
      0, std::max(100.0, std::max(m_valTupLdc, m_valArabaLdc) * 1.2));
}

double MainWindow::rawToBar(uint16_t raw) {
  // Linear scale: 4000 -> 0 bar, 20000 -> 400 bar
  double bar = (static_cast<double>(raw) - 4000.0) * 400.0 / 16000.0;
  if (bar < 0.0)
    bar = 0.0;
  return bar;
}

void MainWindow::sendControlCommand(uint8_t byte1, uint8_t byte2) {
  if (m_port4 && m_port4->isOpen()) {
    uint8_t packet[4];
    packet[0] = 0xFD;
    packet[1] = byte1;
    packet[2] = byte2;
    packet[3] = (uint8_t)(0xFD + byte1 + byte2);

    m_port4->write(reinterpret_cast<const char *>(packet), 4);
    m_port4->flush();
  }
}

void MainWindow::startSequence() {
  // Igniter-Valf arası süre (ms cinsinden, 1ms çözünürlük)
  uint16_t intervalMs = (uint16_t)m_spinRoleTime->value();

  // 2 byte'a böl: b1 = MSB, b2 = LSB
  uint8_t b1 = (uint8_t)(intervalMs >> 8);   // Üst byte
  uint8_t b2 = (uint8_t)(intervalMs & 0xFF); // Alt byte

  sendControlCommand(b1, b2);
  logEventToFiles("OTOMATIK_BASLAT");
}

void MainWindow::updateSequence() {
  // Obsolete: Sequence is handled by Arduino now.
}

void MainWindow::changeTheme(int index) {
  m_currentTheme = index;
  applyTheme();
}

void MainWindow::applyTheme() {
  bool isDark = (m_currentTheme == 0 || m_currentTheme == 2);

  QString bgPath;
  if (m_currentTheme == 0)
    bgPath = "dark_bg.png";
  else if (m_currentTheme == 1)
    bgPath = "light_bg.png";
  else
    bgPath = "christmas_bg.png";

  if (m_btnJingleBells) {
    m_btnJingleBells->setVisible(m_currentTheme == 2);
  }

  // 1. Central Widget and Globals
  QString centralStyle =
      QString(
          "#CentralWidget { border-image: url(%1) 0 0 0 0 stretch stretch; }")
          .arg(bgPath);

  QString globalStyle =
      centralStyle +
      (isDark
           ? "QLabel { color: #00FF00; font-family: 'Courier New'; font-size: "
             "18px; font-weight: bold; background: rgba(0,0,0,150); padding: "
             "4px; border: 2px solid #005500; }"
             "QGroupBox { color: #00FF00; font-family: 'Courier New'; "
             "font-weight: bold; border: 2px solid #00FF00; margin-top: 20px; "
             "background: rgba(0,0,0,180); font-size: 18px; }"
             "QGroupBox::title { subcontrol-origin: margin; "
             "subcontrol-position: top center; padding: 0 5px; background: "
             "rgba(0,0,0,200); }"
             "QPushButton { background-color: #003300; color: #00FF00; border: "
             "2px solid #00FF00; padding: 5px; font-family: 'Courier New'; "
             "font-weight: bold; font-size: 18px; }"
             "QPushButton:hover { background-color: #005500; }"
             "QPushButton:pressed { background-color: #00FF00; color: black; }"
             "QComboBox { background-color: #003300; color: #00FF00; border: "
             "2px solid #00FF00; font-family: 'Courier New'; font-size: 18px; }"
             "QComboBox QAbstractItemView { background-color: #003300; color: "
             "#00FF00; selection-background-color: #005500; font-family: "
             "'Courier New'; font-size: 18px; }"
             "QDoubleSpinBox { background-color: #003300; color: #00FF00; "
             "border: 2px solid #00FF00; font-family: 'Courier New'; "
             "font-size: 18px; }"
             "QCheckBox { color: #00FF00; font-family: 'Courier New'; "
             "font-weight: bold; font-size: 16px; background: transparent; "
             "border: none; }"
             "QChartView { background: rgba(0, 0, 0, 180); border: 1px solid "
             "#005500; border-radius: 4px; }"
           : "QLabel { color: #000000; font-family: 'Courier New'; font-size: "
             "18px; font-weight: bold; background: rgba(255,255,255,200); "
             "padding: 4px; border: 2px solid #555555; }"
             "QGroupBox { color: #000000; font-family: 'Courier New'; "
             "font-weight: bold; border: 2px solid #333333; margin-top: 20px; "
             "background: rgba(255,255,255,200); font-size: 18px; }"
             "QGroupBox::title { subcontrol-origin: margin; "
             "subcontrol-position: top center; padding: 0 5px; background: "
             "rgba(255,255,255,200); }"
             "QPushButton { background-color: #E0E0E0; color: #000000; border: "
             "2px solid #555555; padding: 5px; font-family: 'Courier New'; "
             "font-weight: bold; font-size: 18px; }"
             "QPushButton:hover { background-color: #CCCCCC; }"
             "QPushButton:pressed { background-color: #AAAAAA; color: black; }"
             "QComboBox { background-color: #FFFFFF; color: #000000; border: "
             "1px solid #555555; font-family: 'Courier New'; font-size: 18px; }"
             "QComboBox QAbstractItemView { background-color: #FFFFFF; color: "
             "#000000; selection-background-color: #CCCCCC; font-family: "
             "'Courier New'; font-size: 18px; }"
             "QDoubleSpinBox { background-color: #FFFFFF; color: #000000; "
             "border: 1px solid #555555; font-family: 'Courier New'; "
             "font-size: 18px; }"
             "QCheckBox { color: #000000; font-family: 'Courier New'; "
             "font-weight: bold; font-size: 16px; background: transparent; "
             "border: none; }"
             "QChartView { background: rgba(255, 255, 255, 200); border: 1px "
             "solid #555555; border-radius: 4px; }");
  this->setStyleSheet(globalStyle);

  // 2. Specific Buttons & Labels
  if (m_btnJingleBells)
    m_btnJingleBells->setStyleSheet(
        "QPushButton { background-color: #CC0000; color: #FFFFFF; font-family: "
        "'Courier New'; font-weight: bold; font-size: 18px; border: 2px solid "
        "#00AA00; padding: 10px; } QPushButton:hover { background-color: "
        "#FF0000; } QPushButton:pressed { background-color: #00AA00; color: "
        "white; }");

  QString igniterOnStyle =
      isDark
          ? "QPushButton { background-color: #AA5500; color: #FFCC00; "
            "font-weight: bold; border: 2px solid #FFAA00; padding: 8px; } "
            "QPushButton:hover { background-color: #CC6600; } "
            "QPushButton:pressed { background-color: #FFAA00; color: black; }"
          : "QPushButton { background-color: #FF8C00; color: #FFFFFF; "
            "font-weight: bold; border: 2px solid #D2691E; padding: 8px; } "
            "QPushButton:hover { background-color: #FFA500; } "
            "QPushButton:pressed { background-color: #FF4500; }";
  if (m_btnIgniterOn)
    m_btnIgniterOn->setStyleSheet(igniterOnStyle);

  QString igniterOffStyle =
      isDark
          ? "QPushButton { background-color: #332200; color: #FFAA00; "
            "font-weight: bold; border: 2px solid #664400; padding: 8px; } "
            "QPushButton:hover { background-color: #553300; } "
            "QPushButton:pressed { background-color: #664400; color: white; }"
          : "QPushButton { background-color: #FFF8DC; color: #8B4500; "
            "font-weight: bold; border: 2px solid #CD853F; padding: 8px; } "
            "QPushButton:hover { background-color: #FFE4B5; } "
            "QPushButton:pressed { background-color: #DEB887; }";
  if (m_btnIgniterOff)
    m_btnIgniterOff->setStyleSheet(igniterOffStyle);

  QString valveOnStyle =
      isDark
          ? "QPushButton { background-color: #0055AA; color: #00CCFF; "
            "font-weight: bold; border: 2px solid #0088FF; padding: 8px; } "
            "QPushButton:hover { background-color: #0066CC; } "
            "QPushButton:pressed { background-color: #0088FF; color: black; }"
          : "QPushButton { background-color: #1E90FF; color: #FFFFFF; "
            "font-weight: bold; border: 2px solid #0000CD; padding: 8px; } "
            "QPushButton:hover { background-color: #4169E1; } "
            "QPushButton:pressed { background-color: #000080; }";
  if (m_btnValveOn)
    m_btnValveOn->setStyleSheet(valveOnStyle);

  QString valveOffStyle =
      isDark
          ? "QPushButton { background-color: #002244; color: #0088CC; "
            "font-weight: bold; border: 2px solid #003366; padding: 8px; } "
            "QPushButton:hover { background-color: #003355; } "
            "QPushButton:pressed { background-color: #003366; color: white; }"
          : "QPushButton { background-color: #E6E6FA; color: #00008B; "
            "font-weight: bold; border: 2px solid #4682B4; padding: 8px; } "
            "QPushButton:hover { background-color: #B0C4DE; } "
            "QPushButton:pressed { background-color: #778899; }";
  if (m_btnValveOff)
    m_btnValveOff->setStyleSheet(valveOffStyle);

  if (m_btnEmergency)
    m_btnEmergency->setStyleSheet(
        isDark ? "QPushButton { background-color: #AA0000; color: white; "
                 "font-family: 'Courier New'; font-weight: bold; font-size: "
                 "18px; border: 2px solid #FF0000; padding: 10px; } "
                 "QPushButton:hover { background-color: #FF0000; } "
                 "QPushButton:pressed { background-color: #550000; }"
               : "QPushButton { background-color: #FF0000; color: white; "
                 "font-family: 'Courier New'; font-weight: bold; font-size: "
                 "18px; border: 2px solid #8B0000; padding: 10px; } "
                 "QPushButton:hover { background-color: #DC143C; } "
                 "QPushButton:pressed { background-color: #8B0000; }");

  if (m_btnStartSequence)
    m_btnStartSequence->setStyleSheet(
        isDark
            ? "QPushButton { background-color: #005500; color: #00FF00; "
              "font-family: 'Courier New'; font-weight: bold; font-size: 18px; "
              "border: 2px solid #00FF00; padding: 5px; } QPushButton:hover { "
              "background-color: #008800; } QPushButton:pressed { "
              "background-color: #00FF00; color: black; }"
            : "QPushButton { background-color: #32CD32; color: #FFFFFF; "
              "font-family: 'Courier New'; font-weight: bold; font-size: 18px; "
              "border: 2px solid #228B22; padding: 5px; } QPushButton:hover { "
              "background-color: #00FF00; } QPushButton:pressed { "
              "background-color: #006400; }");

  QString labelStyle = isDark ? "font-size: 18px; font-weight: bold; color: "
                                "#4CAF50; border:none; background:transparent;"
                              : "font-size: 18px; font-weight: bold; color: "
                                "#006400; border:none; background:transparent;";
  if (m_lblOnYanma)
    m_lblOnYanma->setStyleSheet(labelStyle);
  if (m_lblArkaYanma)
    m_lblArkaYanma->setStyleSheet(labelStyle);
  if (m_lblN2O)
    m_lblN2O->setStyleSheet(labelStyle);
  if (m_lblTupLdc)
    m_lblTupLdc->setStyleSheet(labelStyle);
  if (m_lblArabaLdc)
    m_lblArabaLdc->setStyleSheet(labelStyle);
  if (m_lblTime)
    m_lblTime->setStyleSheet(labelStyle +
                             (isDark ? "color: #00FFFF;" : "color: #00008B;"));

  if (m_lblCountdown)
    m_lblCountdown->setStyleSheet(
        isDark ? "font-size: 20px; font-weight: bold; color: #FF00FF; "
                 "qproperty-alignment: AlignCenter; border:none; "
                 "background:transparent;"
               : "font-size: 20px; font-weight: bold; color: #8B008B; "
                 "qproperty-alignment: AlignCenter; border:none; "
                 "background:transparent;");

  if (m_lblControlMsg)
    m_lblControlMsg->setStyleSheet(
        isDark
            ? "QPlainTextEdit { background: rgba(0, 0, 0, 180); border: 1px "
              "solid #005500; border-radius: 4px; color: #00FF00; font-family: "
              "'Courier New'; font-size: 18px; padding: 4px; }"
            : "QPlainTextEdit { background: rgba(255, 255, 255, 200); border: "
              "1px solid #555555; border-radius: 4px; color: #000000; "
              "font-family: 'Courier New'; font-size: 18px; padding: 4px; }");

  QString cancelStyle =
      isDark ? "QPushButton { background-color: #550000; color: #FF0000; "
               "border: 2px solid #FF0000; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#880000; } QPushButton:pressed { background-color: #FF0000; "
               "color: white; } QPushButton:disabled { background-color: "
               "#220000; color: #550000; border: 2px solid #550000; }"
             : "QPushButton { background-color: #FF4500; color: #FFFFFF; "
               "border: 2px solid #B22222; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#DC143C; } QPushButton:pressed { background-color: #8B0000; } "
               "QPushButton:disabled { background-color: #FFA07A; color: "
               "#CD5C5C; border: 2px solid #CD5C5C; }";

  QString startStyle =
      isDark ? "QPushButton { background-color: #005500; color: #00FF00; "
               "border: 2px solid #00FF00; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#008800; } QPushButton:pressed { background-color: #00FF00; "
               "color: black; } QPushButton:disabled { background-color: "
               "#002200; color: #005500; border: 2px solid #005500; }"
             : "QPushButton { background-color: #32CD32; color: #FFFFFF; "
               "border: 2px solid #228B22; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#2E8B57; } QPushButton:pressed { background-color: #006400; } "
               "QPushButton:disabled { background-color: #98FB98; color: "
               "#2E8B57; border: 2px solid #2E8B57; }";

  QString readyStyle =
      isDark ? "QPushButton { background-color: #AA5500; color: #FFCC00; "
               "border: 2px solid #FFAA00; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#CC6600; } QPushButton:pressed { background-color: #FFAA00; "
               "color: black; } QPushButton:disabled { background-color: "
               "#442200; color: #884400; border: 2px solid #884400; }"
             : "QPushButton { background-color: #FF8C00; color: #FFFFFF; "
               "border: 2px solid #D2691E; font-weight: bold; font-size: 18px; "
               "padding: 10px; } QPushButton:hover { background-color: "
               "#FFA500; } QPushButton:pressed { background-color: #FF4500; } "
               "QPushButton:disabled { background-color: #FFDAB9; color: "
               "#CD853F; border: 2px solid #CD853F; }";

  QString fireStyle =
      isDark ? "QPushButton { background-color: #550000; color: #FF0000; "
               "border: 2px dashed #FF0000; font-weight: bold; font-size: "
               "20px; padding: 15px; } QPushButton:hover { background-color: "
               "#880000; } QPushButton:pressed { background-color: #FF0000; "
               "color: white; } QPushButton:disabled { background-color: "
               "#220000; color: #550000; border: 2px dashed #550000; }"
             : "QPushButton { background-color: #FF0000; color: #FFFFFF; "
               "border: 2px dashed #B22222; font-weight: bold; font-size: "
               "20px; padding: 15px; } QPushButton:hover { background-color: "
               "#DC143C; } QPushButton:pressed { background-color: #8B0000; } "
               "QPushButton:disabled { background-color: #FFC0CB; color: "
               "#CD5C5C; border: 2px dashed #CD5C5C; }";

  if (m_btnCancel)
    m_btnCancel->setStyleSheet(cancelStyle);
  if (m_btnStart)
    m_btnStart->setStyleSheet(startStyle);
  if (m_btnReady)
    m_btnReady->setStyleSheet(readyStyle);
  if (m_btnFire)
    m_btnFire->setStyleSheet(fireStyle);

  // Record button (need to check if it's currently recording to keep red state)
  if (m_btnRecord) {
    if (m_isRecording) {
      m_btnRecord->setStyleSheet(
          isDark ? "QPushButton { background-color: #AA0000; color: white; "
                   "font-family: 'Courier New'; font-weight: bold; font-size: "
                   "18px; border: 2px solid #FF0000; padding: 10px; }"
                 : "QPushButton { background-color: #FF0000; color: white; "
                   "font-family: 'Courier New'; font-weight: bold; font-size: "
                   "18px; border: 2px solid #8B0000; padding: 10px; }");
    } else {
      m_btnRecord->setStyleSheet(
          isDark ? "QPushButton { background-color: #0000AA; color: white; "
                   "font-family: 'Courier New'; font-weight: bold; font-size: "
                   "18px; border: 2px solid #0000FF; padding: 10px; } "
                   "QPushButton:hover { background-color: #0000FF; }"
                 : "QPushButton { background-color: #4169E1; color: white; "
                   "font-family: 'Courier New'; font-weight: bold; font-size: "
                   "18px; border: 2px solid #0000CD; padding: 10px; } "
                   "QPushButton:hover { background-color: #1E90FF; }");
    }
  }

  // 3. MotorWidget
  if (m_motorWidget)
    m_motorWidget->setStyleSheet(
        isDark ? "background-color: rgba(0, 0, 0, 150); border: 4px solid "
                 "#00FFFF; border-radius: 0px;"
               : "background-color: rgba(255, 255, 255, 200); border: 4px "
                 "solid #0000CD; border-radius: 0px;");

  // 4. Charts Axis Colors
  QColor titleColor = isDark ? QColor("#00FF00") : QColor("#000000");
  QColor axisLabelColor = isDark ? QColor("#00FF00") : QColor("#000000");
  QColor gridLineColor = isDark ? QColor(0, 100, 0) : QColor(150, 150, 150);

  if (m_pressureView && m_pressureView->chart()) {
    m_pressureView->chart()->setTitleBrush(QBrush(titleColor));
    m_pressureView->chart()->legend()->setLabelColor(titleColor);
    if (m_axisX_Pressure) {
      m_axisX_Pressure->setLabelsBrush(QBrush(axisLabelColor));
      m_axisX_Pressure->setTitleBrush(QBrush(titleColor));
      m_axisX_Pressure->setGridLinePen(QPen(gridLineColor));
    }
    if (m_axisY_Pressure) {
      m_axisY_Pressure->setLabelsBrush(QBrush(axisLabelColor));
      m_axisY_Pressure->setTitleBrush(QBrush(titleColor));
      m_axisY_Pressure->setGridLinePen(QPen(gridLineColor));
    }
  }

  if (m_loadcellView && m_loadcellView->chart()) {
    m_loadcellView->chart()->setTitleBrush(QBrush(titleColor));
    m_loadcellView->chart()->legend()->setLabelColor(titleColor);
    if (m_axisX_Loadcell) {
      m_axisX_Loadcell->setLabelsBrush(QBrush(axisLabelColor));
      m_axisX_Loadcell->setTitleBrush(QBrush(titleColor));
      m_axisX_Loadcell->setGridLinePen(QPen(gridLineColor));
    }
    if (m_axisY_Loadcell) {
      m_axisY_Loadcell->setLabelsBrush(QBrush(axisLabelColor));
      m_axisY_Loadcell->setTitleBrush(QBrush(titleColor));
      m_axisY_Loadcell->setGridLinePen(QPen(gridLineColor));
    }
  }
}
