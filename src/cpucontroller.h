#pragma once

#include <QObject>
#include <QTimer>

class CpuController : public QObject {
  Q_OBJECT
  Q_PROPERTY(double maxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY
                 maxFrequencyChanged)
  Q_PROPERTY(double currentMaxFrequency READ currentMaxFrequency NOTIFY
                 currentMaxFrequencyChanged)
  Q_PROPERTY(double currentFrequency READ currentFrequency NOTIFY
                 currentFrequencyChanged)
  Q_PROPERTY(bool regulationEnabled READ regulationEnabled WRITE
                 setRegulationEnabled NOTIFY regulationEnabledChanged)
  Q_PROPERTY(
      bool cpuLimitApplied READ cpuLimitApplied NOTIFY cpuLimitAppliedChanged)

public:
  explicit CpuController(QObject *parent = nullptr);

  double maxFrequency() const { return m_maxFrequency; }
  double currentMaxFrequency() const { return m_currentMaxFrequency; }
  double currentFrequency() const { return m_currentFrequency; }
  bool regulationEnabled() const { return m_regulationEnabled; }
  bool cpuLimitApplied() const { return m_cpuLimitApplied; }

  void setMaxFrequency(double frequency);
  void setRegulationEnabled(bool enabled);

  void applyFrequencyLimit();
  void removeFrequencyLimit();

signals:
  void maxFrequencyChanged();
  void currentMaxFrequencyChanged();
  void currentFrequencyChanged();
  void regulationEnabledChanged();
  void cpuLimitAppliedChanged();

private:
  bool setCpuMaxFrequency(double frequencyGHz);
  double readCurrentMaxFrequency();
  double readCurrentFrequency();
  void updateCurrentFrequency();

  double m_maxFrequency = 3.5; // Default: 3.5 GHz
  double m_currentMaxFrequency = 0.0;
  double m_currentFrequency = 0.0;
  bool m_regulationEnabled = true;
  bool m_cpuLimitApplied = false;
  QTimer *m_updateTimer;
};
