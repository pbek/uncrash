#include "cpucontroller.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

CpuController::CpuController(QObject *parent) : QObject(parent) {
  m_currentMaxFrequency = readCurrentMaxFrequency();
}

void CpuController::setMaxFrequency(double frequency) {
  if (qFuzzyCompare(m_maxFrequency, frequency))
    return;

  m_maxFrequency = frequency;
  emit maxFrequencyChanged();

  // Apply immediately if regulation is enabled
  if (m_regulationEnabled) {
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
  } else {
    qWarning() << "Failed to remove CPU frequency limit";
  }
}

bool CpuController::setCpuMaxFrequency(double frequencyGHz) {
  // Convert GHz to KHz for sysfs
  qint64 frequencyKHz = static_cast<qint64>(frequencyGHz * 1000000);

  // Create a shell command that writes to all CPU frequency files
  QString command =
      QString("for cpu in "
              "/sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do "
              "[ -f \"$cpu\" ] && echo %1 > \"$cpu\"; "
              "done")
          .arg(frequencyKHz);

  // Use pkexec with sh -c to execute the command
  QProcess process;
  process.start("pkexec", QStringList() << "sh" << "-c" << command);
  process.waitForFinished(10000);

  if (process.exitCode() == 0) {
    return true;
  } else {
    QString errorOutput = QString::fromUtf8(process.readAllStandardError());
    qWarning() << "Failed to set CPU frequency:" << errorOutput;

    // If pkexec fails, try with sudo as fallback
    QProcess sudoProcess;
    sudoProcess.start("sudo", QStringList() << "sh" << "-c" << command);
    sudoProcess.waitForFinished(10000);

    if (sudoProcess.exitCode() == 0) {
      return true;
    } else {
      qWarning() << "Failed to set CPU frequency with sudo:"
                 << QString::fromUtf8(sudoProcess.readAllStandardError());
      return false;
    }
  }
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
