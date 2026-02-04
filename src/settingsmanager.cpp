#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent), m_settings("uncrash", "uncrash") {}

// Window geometry
int SettingsManager::windowWidth() const {
  return m_settings.value("window/width", 800).toInt();
}

void SettingsManager::setWindowWidth(int width) {
  if (windowWidth() == width)
    return;

  m_settings.setValue("window/width", width);
  emit windowWidthChanged();
}

int SettingsManager::windowHeight() const {
  return m_settings.value("window/height", 600).toInt();
}

void SettingsManager::setWindowHeight(int height) {
  if (windowHeight() == height)
    return;

  m_settings.setValue("window/height", height);
  emit windowHeightChanged();
}

// GPU settings
double SettingsManager::gpuPowerThreshold() const {
  return m_settings.value("gpu/powerThreshold", 100.0).toDouble();
}

void SettingsManager::setGpuPowerThreshold(double threshold) {
  if (qFuzzyCompare(gpuPowerThreshold(), threshold))
    return;

  m_settings.setValue("gpu/powerThreshold", threshold);
  emit gpuPowerThresholdChanged();
}

// CPU settings
double SettingsManager::cpuMaxFrequency() const {
  return m_settings.value("cpu/maxFrequency", 3.5).toDouble();
}

void SettingsManager::setCpuMaxFrequency(double frequency) {
  if (qFuzzyCompare(cpuMaxFrequency(), frequency))
    return;

  m_settings.setValue("cpu/maxFrequency", frequency);
  emit cpuMaxFrequencyChanged();
}

// Protection settings
bool SettingsManager::autoProtection() const {
  return m_settings.value("protection/auto", true).toBool();
}

void SettingsManager::setAutoProtection(bool enabled) {
  if (autoProtection() == enabled)
    return;

  m_settings.setValue("protection/auto", enabled);
  emit autoProtectionChanged();
}
