#include "temperaturemonitor.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

TemperatureMonitor::TemperatureMonitor(QObject *parent)
    : QObject(parent), m_updateTimer(new QTimer(this)) {
  // Detect GPU vendor first
  detectGpuVendor();

  // Find CPU temperature hwmon (support AMD and Intel)
  m_cpuTempPath = findHwmonByName("k10temp"); // AMD Ryzen/EPYC
  if (m_cpuTempPath.isEmpty()) {
    m_cpuTempPath = findHwmonByName("coretemp"); // Intel
  }
  if (m_cpuTempPath.isEmpty()) {
    m_cpuTempPath = findHwmonByName("zenpower"); // Alternative AMD driver
  }

  // Find motherboard sensor hwmon (support various chipsets)
  QStringList motherboardDrivers = {
      "asus_wmi_sensors", // ASUS motherboards
      "nct6775",          // Nuvoton NCT6775/6776/6779
      "it87",             // ITE IT87xx
      "w83627ehf",        // Winbond W83627EHF
      "f71882fg",         // Fintek F71882FG
      "nct6683"           // Nuvoton NCT6683
  };

  for (const QString &driver : motherboardDrivers) {
    m_motherboardPath = findHwmonByName(driver);
    if (!m_motherboardPath.isEmpty()) {
      qDebug() << "Found motherboard sensors:" << driver << "at"
               << m_motherboardPath;
      break;
    }
  }

  // For AMD GPUs, find the GPU hwmon
  if (m_gpuVendor == "AMD") {
    // AMD GPUs typically use amdgpu driver
    QStringList amdPaths = findAllHwmonByPattern("amdgpu");
    if (!amdPaths.isEmpty()) {
      m_gpuHwmonPath = amdPaths.first();
      qDebug() << "Found AMD GPU hwmon at" << m_gpuHwmonPath;
    }
  }

  if (m_cpuTempPath.isEmpty()) {
    qWarning() << "CPU temperature hwmon not found";
  }
  if (m_motherboardPath.isEmpty()) {
    qWarning() << "Motherboard sensors hwmon not found";
  }

  connect(m_updateTimer, &QTimer::timeout, this,
          &TemperatureMonitor::updateSensors);
}

void TemperatureMonitor::startMonitoring(int intervalMs) {
  updateSensors(); // Immediate first update
  m_updateTimer->start(intervalMs);
}

void TemperatureMonitor::stopMonitoring() { m_updateTimer->stop(); }

void TemperatureMonitor::updateSensors() {
  bool changed = false;

  // Update GPU sensors
  SensorData newGpuData = readGpuSensors();
  if (newGpuData.valid && (newGpuData.temperature != m_gpuData.temperature ||
                           newGpuData.fanSpeed != m_gpuData.fanSpeed)) {
    m_gpuData = newGpuData;
    emit gpuTemperatureChanged(m_gpuData.temperature);
    changed = true;
  }

  // Update CPU sensors
  SensorData newCpuData = readCpuSensors();
  if (newCpuData.valid && newCpuData.temperature != m_cpuData.temperature) {
    m_cpuData = newCpuData;
    emit cpuTemperatureChanged(m_cpuData.temperature);
    changed = true;
  }

  // Update motherboard sensors
  SensorData newMoboData = readMotherboardSensors();
  if (newMoboData.valid &&
      newMoboData.temperature != m_motherboardData.temperature) {
    m_motherboardData = newMoboData;
    emit motherboardTemperatureChanged(m_motherboardData.temperature);
    changed = true;
  }

  // Update fan sensors
  readFanSensors();

  if (changed) {
    emit temperaturesUpdated();
  }
}

SensorData TemperatureMonitor::readGpuSensors() {
  if (m_gpuVendor == "NVIDIA") {
    return readNvidiaGpu();
  } else if (m_gpuVendor == "AMD") {
    return readAmdGpu();
  }

  SensorData data;
  return data;
}

SensorData TemperatureMonitor::readNvidiaGpu() {
  SensorData data;

  QProcess process;
  process.start("nvidia-smi", QStringList()
                                  << "--query-gpu=temperature.gpu,fan.speed"
                                  << "--format=csv,noheader,nounits");

  if (!process.waitForFinished(1000)) {
    return data;
  }

  QString output = process.readAllStandardOutput().trimmed();
  QStringList parts = output.split(',');

  if (parts.size() >= 2) {
    bool ok;
    double temp = parts[0].trimmed().toDouble(&ok);
    if (ok) {
      data.temperature = temp;
      data.valid = true;
    }

    int fanPercent = parts[1].trimmed().toInt(&ok);
    if (ok) {
      m_gpuFanPercent = fanPercent;
      // Store percentage for NVIDIA (RPM not available via nvidia-smi)
      data.fanSpeed = fanPercent;
    }
  }

  return data;
}

SensorData TemperatureMonitor::readAmdGpu() {
  SensorData data;

  if (m_gpuHwmonPath.isEmpty()) {
    return data;
  }

  // AMD GPU temperature is typically in temp1_input
  double temp = readHwmonTemp(m_gpuHwmonPath, "temp1_input");
  if (temp > 0) {
    data.temperature = temp;
    data.valid = true;
  }

  // AMD GPU fan speed (RPM)
  int fanSpeed = readHwmonFan(m_gpuHwmonPath, "fan1_input");
  if (fanSpeed > 0) {
    data.fanSpeed = fanSpeed;
  }

  // Try to read fan percentage if available (pwm1)
  QString pwmPath = m_gpuHwmonPath + "/pwm1";
  QFile pwmFile(pwmPath);
  if (pwmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString value = QTextStream(&pwmFile).readAll().trimmed();
    bool ok;
    int pwmValue = value.toInt(&ok);
    if (ok) {
      // PWM is typically 0-255, convert to percentage
      m_gpuFanPercent = (pwmValue * 100) / 255;
    }
  }

  return data;
}

SensorData TemperatureMonitor::readCpuSensors() {
  SensorData data;

  if (m_cpuTempPath.isEmpty()) {
    return data;
  }

  // Read main CPU temperature (temp1_input for most drivers)
  double temp = readHwmonTemp(m_cpuTempPath, "temp1_input");
  if (temp > 0) {
    data.temperature = temp;
    data.valid = true;
  }

  // Read CPU fan from motherboard sensors
  if (!m_motherboardPath.isEmpty()) {
    // Try to find CPU fan by label
    for (int i = 1; i <= 10; ++i) {
      QString label =
          readHwmonLabel(m_motherboardPath, QString("fan%1_label").arg(i));
      if (label.contains("CPU", Qt::CaseInsensitive)) {
        int fanSpeed =
            readHwmonFan(m_motherboardPath, QString("fan%1_input").arg(i));
        if (fanSpeed > 0) {
          data.fanSpeed = fanSpeed;
          break;
        }
      }
    }

    // If no labeled fan found, try fan1 as fallback
    if (data.fanSpeed == 0) {
      int fanSpeed = readHwmonFan(m_motherboardPath, "fan1_input");
      if (fanSpeed > 0) {
        data.fanSpeed = fanSpeed;
      }
    }
  }

  return data;
}

SensorData TemperatureMonitor::readMotherboardSensors() {
  SensorData data;

  if (m_motherboardPath.isEmpty()) {
    return data;
  }

  // Try to find motherboard temperature by scanning all temp sensors
  // Look for labels like "Motherboard", "System", "MB", etc.
  for (int i = 1; i <= 10; ++i) {
    QString label =
        readHwmonLabel(m_motherboardPath, QString("temp%1_label").arg(i));
    if (label.contains("Motherboard", Qt::CaseInsensitive) ||
        label.contains("System", Qt::CaseInsensitive) ||
        label.contains(" MB", Qt::CaseInsensitive)) {
      double temp =
          readHwmonTemp(m_motherboardPath, QString("temp%1_input").arg(i));
      if (temp > 0) {
        data.temperature = temp;
        data.valid = true;
        break;
      }
    }

    // Also try to find CPU socket temperature
    if (label.contains("Socket", Qt::CaseInsensitive) ||
        label.contains("CPUTIN", Qt::CaseInsensitive)) {
      m_cpuSocketTemperature =
          readHwmonTemp(m_motherboardPath, QString("temp%1_input").arg(i));
    }
  }

  // Fallback: if no labeled motherboard temp, try temp2 or temp3
  if (!data.valid) {
    double temp = readHwmonTemp(m_motherboardPath, "temp2_input");
    if (temp == 0) {
      temp = readHwmonTemp(m_motherboardPath, "temp3_input");
    }
    if (temp > 0) {
      data.temperature = temp;
      data.valid = true;
    }
  }

  return data;
}

void TemperatureMonitor::readFanSensors() {
  if (m_motherboardPath.isEmpty()) {
    return;
  }

  m_caseFanSpeeds.clear();

  // Scan all fan sensors and read their labels
  for (int i = 1; i <= 15; ++i) {
    QString label =
        readHwmonLabel(m_motherboardPath, QString("fan%1_label").arg(i));
    int speed = readHwmonFan(m_motherboardPath, QString("fan%1_input").arg(i));

    // Skip CPU fan (already handled in CPU sensors) and stopped fans
    if (speed > 0 && !label.contains("CPU", Qt::CaseInsensitive)) {
      if (label.isEmpty()) {
        label = QString("Fan %1").arg(i);
      }
      m_caseFanSpeeds[label] = speed;
    }
  }

  emit fanSpeedsChanged();
}

double TemperatureMonitor::readHwmonTemp(const QString &hwmonPath,
                                         const QString &tempFile) {
  QString path = hwmonPath + "/" + tempFile;
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0.0;
  }

  QTextStream in(&file);
  QString value = in.readAll().trimmed();

  bool ok;
  int milliCelsius = value.toInt(&ok);
  if (ok) {
    return milliCelsius / 1000.0; // Convert from millidegrees to degrees
  }

  return 0.0;
}

int TemperatureMonitor::readHwmonFan(const QString &hwmonPath,
                                     const QString &fanFile) {
  QString path = hwmonPath + "/" + fanFile;
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }

  QTextStream in(&file);
  QString value = in.readAll().trimmed();

  bool ok;
  int rpm = value.toInt(&ok);
  if (ok) {
    return rpm;
  }

  return 0;
}

QString TemperatureMonitor::findHwmonByName(const QString &name) {
  QDir hwmonDir("/sys/class/hwmon");
  QStringList hwmons = hwmonDir.entryList(QStringList() << "hwmon*",
                                          QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QString &hwmon : hwmons) {
    QString namePath = hwmonDir.absoluteFilePath(hwmon) + "/name";
    QFile nameFile(namePath);

    if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString hwmonName = QTextStream(&nameFile).readAll().trimmed();
      if (hwmonName == name) {
        return hwmonDir.absoluteFilePath(hwmon);
      }
    }
  }

  return QString();
}

QStringList TemperatureMonitor::findAllHwmonByPattern(const QString &pattern) {
  QStringList result;
  QDir hwmonDir("/sys/class/hwmon");
  QStringList hwmons = hwmonDir.entryList(QStringList() << "hwmon*",
                                          QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QString &hwmon : hwmons) {
    QString namePath = hwmonDir.absoluteFilePath(hwmon) + "/name";
    QFile nameFile(namePath);

    if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString hwmonName = QTextStream(&nameFile).readAll().trimmed();
      if (hwmonName.contains(pattern, Qt::CaseInsensitive)) {
        result.append(hwmonDir.absoluteFilePath(hwmon));
      }
    }
  }

  return result;
}

QString TemperatureMonitor::readHwmonLabel(const QString &hwmonPath,
                                           const QString &labelFile) {
  QString path = hwmonPath + "/" + labelFile;
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QString();
  }

  return QTextStream(&file).readAll().trimmed();
}

void TemperatureMonitor::detectGpuVendor() {
  // Try NVIDIA first
  QProcess nvidiaCheck;
  nvidiaCheck.start("nvidia-smi", QStringList() << "--query-gpu=name"
                                                << "--format=csv,noheader");

  if (nvidiaCheck.waitForFinished(1000)) {
    QString output = nvidiaCheck.readAllStandardOutput().trimmed();
    if (!output.isEmpty() && nvidiaCheck.exitCode() == 0) {
      m_gpuVendor = "NVIDIA";
      m_gpuName = output;
      qDebug() << "Detected NVIDIA GPU:" << m_gpuName;
      return;
    }
  }

  // Try AMD
  QStringList amdPaths = findAllHwmonByPattern("amdgpu");
  if (!amdPaths.isEmpty()) {
    m_gpuVendor = "AMD";

    // Try to get GPU name from sysfs
    QDir drmDir("/sys/class/drm");
    QStringList cards = drmDir.entryList(QStringList() << "card*",
                                         QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &card : cards) {
      QString devicePath = drmDir.absoluteFilePath(card) + "/device";

      // Read device name
      QFile nameFile(devicePath + "/product_name");
      if (!nameFile.exists()) {
        nameFile.setFileName(devicePath + "/model");
      }

      if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_gpuName = QTextStream(&nameFile).readAll().trimmed();
        if (!m_gpuName.isEmpty()) {
          qDebug() << "Detected AMD GPU:" << m_gpuName;
          return;
        }
      }
    }

    m_gpuName = "AMD GPU";
    qDebug() << "Detected AMD GPU (name unavailable)";
    return;
  }

  // No GPU detected
  m_gpuVendor = "Unknown";
  m_gpuName = "No GPU detected";
  qWarning() << "No NVIDIA or AMD GPU detected";
}
