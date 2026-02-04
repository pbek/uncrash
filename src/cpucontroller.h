#pragma once

#include <QObject>

class CpuController : public QObject {
  Q_OBJECT
  Q_PROPERTY(double maxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY
                 maxFrequencyChanged)
  Q_PROPERTY(double currentMaxFrequency READ currentMaxFrequency NOTIFY
                 currentMaxFrequencyChanged)
  Q_PROPERTY(bool regulationEnabled READ regulationEnabled WRITE
                 setRegulationEnabled NOTIFY regulationEnabledChanged)

public:
  explicit CpuController(QObject *parent = nullptr);

  double maxFrequency() const { return m_maxFrequency; }
  double currentMaxFrequency() const { return m_currentMaxFrequency; }
  bool regulationEnabled() const { return m_regulationEnabled; }

  void setMaxFrequency(double frequency);
  void setRegulationEnabled(bool enabled);

  void applyFrequencyLimit();
  void removeFrequencyLimit();

signals:
  void maxFrequencyChanged();
  void currentMaxFrequencyChanged();
  void regulationEnabledChanged();

private:
  bool setCpuMaxFrequency(double frequencyGHz);
  double readCurrentMaxFrequency();

  double m_maxFrequency = 3.5; // Default: 3.5 GHz
  double m_currentMaxFrequency = 0.0;
  bool m_regulationEnabled = true;
};
