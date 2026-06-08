#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

// Custom Widget for 2D Motor Visualization
class MotorWidget : public QWidget {
  Q_OBJECT
public:
  explicit MotorWidget(QWidget *parent = nullptr);
  void updateData(double onYanma, double arkaYanma); // Update flame parameters

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  double m_onYanmaVal = 0;
  double m_arkaYanmaVal = 0;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void openPort1();
  void closePort1();
  void openPort2();
  void closePort2();
  void openPort3();
  void closePort3();
  void openPort4();
  void closePort4();

  void refreshPorts();

  void readPort1();
  void readPort2();
  void readPort3();
  void readPort4();

  void pollModbus();
  void toggleRecording();
  void recordDataLine();
  void updateUI();
  void logEventToFiles(const QString &eventName);

  void changeTheme(int index);

  void handleError1(QSerialPort::SerialPortError error);
  void handleError2(QSerialPort::SerialPortError error);
  void handleError3(QSerialPort::SerialPortError error);
  void handleError4(QSerialPort::SerialPortError error);
  void startSequence();
  void updateSequence();

private:
  void initUI();
  void setupCharts();
  void applyTheme();
  void sendControlCommand(uint8_t byte1, uint8_t byte2);
  double rawToBar(uint16_t raw);

  // Serial Ports
  QSerialPort *m_port1 = nullptr;
  QSerialPort *m_port2 = nullptr;
  QSerialPort *m_port3 = nullptr;
  QSerialPort *m_port4 = nullptr;

  QByteArray m_buffer1;
  QByteArray m_buffer2;
  QByteArray m_buffer3;
  QByteArray m_buffer4;

  // Logging — 3 separate files
  QFile *m_logModbus = nullptr;
  QFile *m_logArabaLdc = nullptr;
  QFile *m_logTupLdc = nullptr;
  QTextStream *m_streamModbus = nullptr;
  QTextStream *m_streamArabaLdc = nullptr;
  QTextStream *m_streamTupLdc = nullptr;
  bool m_isRecording = false;
  qint64 m_recordStartTime = 0;
  QPushButton *m_btnRecord = nullptr;
  QTimer *m_modbusTimer = nullptr;
  QTimer *m_recordTimer = nullptr;
  QTimer *m_uiUpdateTimer = nullptr;

  // Data Values
  double m_valOnYanma = 0.0;
  double m_valArkaYanma = 0.0;
  double m_valN2O = 0.0;
  double m_valTupLdc = 0.0;
  double m_valArabaLdc = 0.0;

  // UI Elements
  QComboBox *m_cbPort1 = nullptr, *m_cbBaud1 = nullptr;
  QComboBox *m_cbPort2 = nullptr, *m_cbBaud2 = nullptr;
  QComboBox *m_cbPort3 = nullptr, *m_cbBaud3 = nullptr;
  QComboBox *m_cbPort4 = nullptr, *m_cbBaud4 = nullptr;

  QPushButton *m_btnConnect1 = nullptr, *m_btnDisconnect1 = nullptr;
  QPushButton *m_btnConnect2 = nullptr, *m_btnDisconnect2 = nullptr;
  QPushButton *m_btnConnect3 = nullptr, *m_btnDisconnect3 = nullptr;
  QPushButton *m_btnConnect4 = nullptr, *m_btnDisconnect4 = nullptr;
  QPushButton *m_btnRefreshPorts = nullptr;

  QLabel *m_status1 = nullptr;
  QLabel *m_status2 = nullptr;
  QLabel *m_status3 = nullptr;
  QLabel *m_status4 = nullptr;

  // Data Displays (Text)
  QLabel *m_lblOnYanma = nullptr;
  QLabel *m_lblArkaYanma = nullptr;
  QLabel *m_lblN2O = nullptr;
  QLabel *m_lblTupLdc = nullptr;
  QLabel *m_lblArabaLdc = nullptr;
  QLabel *m_lblTime = nullptr;
  QLabel *m_lblCountdown = nullptr;
  QPlainTextEdit *m_lblControlMsg = nullptr;

  // Controls
  QComboBox *m_cmbThemeSelect = nullptr;
  QPushButton *m_btnCancel = nullptr;
  QPushButton *m_btnStart = nullptr;
  QPushButton *m_btnReady = nullptr;
  QPushButton *m_btnFire = nullptr;
  int m_firePressCount = 0;
  QPushButton *m_btnIgniterOn = nullptr;
  QPushButton *m_btnIgniterOff = nullptr;
  QPushButton *m_btnValveOn = nullptr;
  QPushButton *m_btnValveOff = nullptr;

  // Sequence Controls
  QDoubleSpinBox *m_spinRoleTime = nullptr;

  QPushButton *m_btnStartSequence = nullptr;

  QPushButton *m_btnEmergency = nullptr;
  QPushButton *m_btnJingleBells = nullptr;

  QCheckBox *m_chkTestMode = nullptr;
  QCheckBox *m_chkManualMode = nullptr;

  // Sequence Logic
  QTimer *m_sequenceTimer = nullptr;
  QTimer *m_heartbeatTimer = nullptr;
  QElapsedTimer m_sequenceElapsed;
  bool m_roleTriggered = false;
  bool m_stage1Triggered = false;

  // Visualizations
  MotorWidget *m_motorWidget = nullptr;

  // Charts
  QChartView *m_pressureView = nullptr;
  QLineSeries *m_seriesOnYanma = nullptr;
  QLineSeries *m_seriesArkaYanma = nullptr;
  QLineSeries *m_seriesN2O = nullptr;
  QValueAxis *m_axisX_Pressure = nullptr;
  QValueAxis *m_axisY_Pressure = nullptr;

  QChartView *m_loadcellView = nullptr;
  QLineSeries *m_seriesTup = nullptr;
  QLineSeries *m_seriesAraba = nullptr;
  QValueAxis *m_axisX_Loadcell = nullptr;
  QValueAxis *m_axisY_Loadcell = nullptr;

  qint64 m_pointCounter = 0;
  qint64 m_startTime = 0;
  int m_currentTheme = 0; // 0: Dark, 1: Light, 2: Christmas

  static const int MAX_CHART_POINTS = 5000;
  static const int MAX_BUFFER_SIZE = 4096;
};

#endif // MAINWINDOW_H
