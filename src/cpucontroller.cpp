#include "cpucontroller.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTimer>

CpuController::CpuController(QObject *parent) : QObject(parent) {
  m_currentMaxFrequency = readCurrentMaxFrequency();

  // Set up timer to periodically update current frequency
  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, &QTimer::timeout, this,
          &CpuController::updateCurrentFrequency);
  m_updateTimer->start(2000); // Update every 2 seconds
}

void CpuController::setMaxFrequency(double frequency) {
  if (qFuzzyCompare(m_maxFrequency, frequency))
    return;

  m_maxFrequency = frequency;
  emit maxFrequencyChanged();

  // If a limit is currently applied, update it to the new frequency
  if (m_cpuLimitApplied) {
    applyFrequencyLimit();
  }
}

void CpuController::setRegulationEnabled(bool enabled) {
  if (m_regulationEnabled == enabled)
    return;

  m_regulationEnabled = enabled;
  emit regulationEnabledChanged();

  if (enabled) {
    applyFrequencyLimit();
  } else {
    removeFrequencyLimit();
  }
}

void CpuController::applyFrequencyLimit() {
  if (!m_regulationEnabled)
    return;

  bool success = setCpuMaxFrequency(m_maxFrequency);
  if (success) {
    qDebug() << "Applied CPU frequency limit:" << m_maxFrequency << "GHz";
    m_currentMaxFrequency = readCurrentMaxFrequency();
    emit currentMaxFrequencyChanged();

    if (!m_cpuLimitApplied) {
      m_cpuLimitApplied = true;
      emit cpuLimitAppliedChanged();
    }
  } else {
    qWarning() << "Failed to apply CPU frequency limit";
  }
}

void CpuController::removeFrequencyLimit() {
  // Set to a high value to effectively remove the limit
  // This will allow the CPU to run at its maximum frequency
  bool success = setCpuMaxFrequency(99.0); // 99 GHz (effectively unlimited)
  if (success) {
    qDebug() << "Removed CPU frequency limit";
    m_currentMaxFrequency = readCurrentMaxFrequency();
    emit currentMaxFrequencyChanged();

    if (m_cpuLimitApplied) {
      m_cpuLimitApplied = false;
      emit cpuLimitAppliedChanged();
    }
  } else {
    qWarning() << "Failed to remove CPU frequency limit";
  }
}

bool CpuController::setCpuMaxFrequency(double frequencyGHz) {
  // Convert GHz to KHz for sysfs
  qint64 frequencyKHz = static_cast<qint64>(frequencyGHz * 1000000);

  // Try to write directly to sysfs files (works when running as root)
  QDir cpuDir("/sys/devices/system/cpu");
  QStringList cpuDirs = cpuDir.entryList(QStringList() << "cpu*", QDir::Dirs);

  bool anySuccess = false;
  int successCount = 0;
  int totalCount = 0;

  for (const QString &cpuDirName : cpuDirs) {
    QString freqPath =
        QString("/sys/devices/system/cpu/%1/cpufreq/scaling_max_freq")
            .arg(cpuDirName);
    QFile file(freqPath);

    if (!file.exists()) {
      continue; // Skip if cpufreq not available for this CPU
    }

    totalCount++;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << frequencyKHz;
      file.close();

      if (file.error() == QFile::NoError) {
        anySuccess = true;
        successCount++;
      } else {
        qWarning() << "Error writing to" << freqPath << ":"
                   << file.errorString();
      }
    } else {
      qWarning() << "Failed to open" << freqPath << ":" << file.errorString();
    }
  }

  if (totalCount == 0) {
    qWarning() << "No CPU frequency control files found";
    return false;
  }

  if (anySuccess) {
    qDebug() << "Successfully set frequency on" << successCount << "of"
             << totalCount << "CPUs";
    return true;
  }

  qWarning() << "Failed to set frequency on any CPU";
  return false;
}

double CpuController::readCurrentMaxFrequency() {
  // Read from the first CPU core
  QString maxFreqPath = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
  QFile file(maxFreqPath);

  if (file.open(QIODevice::ReadOnly)) {
    QString freqStr = QString::fromUtf8(file.readAll()).trimmed();
    bool ok;
    qint64 frequencyKHz = freqStr.toLongLong(&ok);
    if (ok) {
      // Convert from KHz to GHz
      return frequencyKHz / 1000000.0;
    }
  }

  return 0.0;
}

void CpuController::updateCurrentFrequency() {
  double newFrequency = readCurrentMaxFrequency();
  if (!qFuzzyCompare(m_currentMaxFrequency, newFrequency)) {
    m_currentMaxFrequency = newFrequency;
    emit currentMaxFrequencyChanged();
  }
}
