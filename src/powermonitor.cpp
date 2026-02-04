#include "powermonitor.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

PowerMonitor::PowerMonitor(QObject *parent) : QObject(parent) {
  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, &QTimer::timeout, this, &PowerMonitor::updateGpuPower);

  // Update every 1 second
  m_updateTimer->start(1000);

  // Initial update
  updateGpuPower();
}

void PowerMonitor::setGpuPowerThreshold(double threshold) {
  if (qFuzzyCompare(m_gpuPowerThreshold, threshold))
    return;

  m_gpuPowerThreshold = threshold;
  emit gpuPowerThresholdChanged();

  // Re-check threshold
  bool wasExceeded = m_thresholdExceeded;
  m_thresholdExceeded = m_gpuPower > m_gpuPowerThreshold;
  if (wasExceeded != m_thresholdExceeded) {
    emit thresholdExceededChanged();
  }
}

void PowerMonitor::updateGpuPower() {
  double newPower = readGpuPowerFromSysfs();

  if (!qFuzzyCompare(m_gpuPower, newPower)) {
    m_gpuPower = newPower;
    emit gpuPowerChanged();
  }

  // Check threshold
  bool wasExceeded = m_thresholdExceeded;
  m_thresholdExceeded = m_gpuPower > m_gpuPowerThreshold;
  if (wasExceeded != m_thresholdExceeded) {
    emit thresholdExceededChanged();
  }
}

double PowerMonitor::readGpuPowerFromSysfs() {
  // Try NVIDIA first using nvidia-smi
  QProcess nvidiaSmi;
  nvidiaSmi.start("nvidia-smi", QStringList()
                                    << "--query-gpu=power.draw"
                                    << "--format=csv,noheader,nounits");
  if (nvidiaSmi.waitForStarted(1000)) {
    if (nvidiaSmi.waitForFinished(2000)) {
      if (nvidiaSmi.exitCode() == 0) {
        QString output =
            QString::fromUtf8(nvidiaSmi.readAllStandardOutput()).trimmed();
        bool ok;
        double power = output.toDouble(&ok);
        if (ok && power > 0) {
          return power;
        }
      }
    }
  }

  // Try AMD GPU sysfs
  QDir hwmonDir("/sys/class/hwmon");
  if (hwmonDir.exists()) {
    QStringList hwmonDevices =
        hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &device : hwmonDevices) {
      QString namePath = QString("/sys/class/hwmon/%1/name").arg(device);
      QFile nameFile(namePath);
      if (nameFile.open(QIODevice::ReadOnly)) {
        QString name = QString::fromUtf8(nameFile.readAll()).trimmed();
        nameFile.close();

        // Check if this is an AMD GPU (amdgpu)
        if (name == "amdgpu") {
          // Try to read power1_average (in microwatts)
          QString powerPath =
              QString("/sys/class/hwmon/%1/power1_average").arg(device);
          QFile powerFile(powerPath);
          if (powerFile.open(QIODevice::ReadOnly)) {
            QString powerStr = QString::fromUtf8(powerFile.readAll()).trimmed();
            bool ok;
            qint64 powerMicroWatts = powerStr.toLongLong(&ok);
            if (ok) {
              // Convert from microwatts to watts
              double powerWatts = powerMicroWatts / 1000000.0;
              return powerWatts;
            }
          }
        }
      }
    }
  }

  qWarning() << "Could not find GPU power reading (tried NVIDIA and AMD)";
  return 0.0;
}
