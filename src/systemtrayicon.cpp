#include "systemtrayicon.h"
#include "client/daemonclient.h"
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

SystemTrayIcon::SystemTrayIcon(DaemonClient *daemonClient, QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_trayMenu(nullptr),
      m_daemonClient(daemonClient), m_showAction(nullptr),
      m_quitAction(nullptr) {

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    qWarning() << "System tray is not available";
    return;
  }

  setupTrayIcon();
  setupMenu();

  // Connect to daemon client signals to update icon and tooltip
  connect(m_daemonClient, &DaemonClient::autoProtectionChanged, this,
          &SystemTrayIcon::onAutoProtectionChanged);
  connect(m_daemonClient, &DaemonClient::cpuLimitAppliedChanged, this,
          &SystemTrayIcon::onCpuLimitAppliedChanged);
  connect(m_daemonClient, &DaemonClient::connectedChanged, this,
          &SystemTrayIcon::updateIcon);
  connect(m_daemonClient, &DaemonClient::connectedChanged, this,
          &SystemTrayIcon::updateTooltip);
  connect(m_daemonClient, &DaemonClient::currentMaxFrequencyChanged, this,
          &SystemTrayIcon::updateTooltip);
  connect(m_daemonClient, &DaemonClient::gpuPowerChanged, this,
          &SystemTrayIcon::updateTooltip);

  // Initial update
  updateIcon();
  updateTooltip();
}

SystemTrayIcon::~SystemTrayIcon() {
  if (m_trayIcon) {
    m_trayIcon->hide();
    delete m_trayIcon;
  }
  if (m_trayMenu) {
    delete m_trayMenu;
  }
}

void SystemTrayIcon::setupTrayIcon() {
  m_trayIcon = new QSystemTrayIcon(this);
  // Set initial icon - will be updated by updateIcon()
  QPixmap initialIcon = createCpuIcon(false, true);
  m_trayIcon->setIcon(QIcon(initialIcon));

  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          &SystemTrayIcon::onActivated);
}

void SystemTrayIcon::setupMenu() {
  m_trayMenu = new QMenu();

  m_showAction =
      m_trayMenu->addAction(QIcon::fromTheme("window-new"), tr("Show Uncrash"));
  connect(m_showAction, &QAction::triggered, this,
          &SystemTrayIcon::showWindowRequested);

  m_trayMenu->addSeparator();

  m_quitAction =
      m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Quit"));
  m_quitAction->setShortcut(QKeySequence("Ctrl+Q"));
  connect(m_quitAction, &QAction::triggered, this,
          &SystemTrayIcon::quitRequested);

  m_trayIcon->setContextMenu(m_trayMenu);
}

void SystemTrayIcon::show() {
  if (m_trayIcon) {
    m_trayIcon->show();
  }
}

void SystemTrayIcon::hide() {
  if (m_trayIcon) {
    m_trayIcon->hide();
  }
}

bool SystemTrayIcon::isVisible() const {
  return m_trayIcon && m_trayIcon->isVisible();
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger ||
      reason == QSystemTrayIcon::DoubleClick) {
    emit showWindowRequested();
  }
}

void SystemTrayIcon::onAutoProtectionChanged() {
  updateIcon();
  updateTooltip();
}

void SystemTrayIcon::onCpuLimitAppliedChanged() {
  updateIcon();
  updateTooltip();
}

QPixmap SystemTrayIcon::createCpuIcon(bool active, bool disconnected) {
  const int size = 64; // Larger size for better quality
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  // Choose colors based on state
  QColor mainColor;
  QColor accentColor;

  if (disconnected) {
    mainColor = QColor(150, 150, 150); // Gray
    accentColor = QColor(100, 100, 100);
  } else if (active) {
    mainColor = QColor(220, 50, 50);   // Bright red
    accentColor = QColor(180, 30, 30); // Dark red
  } else {
    mainColor = QColor(80, 200, 120);  // Bright green
    accentColor = QColor(50, 160, 90); // Dark green
  }

  // Draw CPU chip body (rounded rectangle)
  const int margin = 8;
  const int chipSize = size - 2 * margin;
  QRectF chipRect(margin, margin, chipSize, chipSize);

  painter.setBrush(mainColor);
  painter.setPen(QPen(accentColor, 2));
  painter.drawRoundedRect(chipRect, 4, 4);

  // Draw CPU pins (small rectangles on sides)
  const int pinWidth = 3;
  const int pinLength = 6;
  const int pinCount = 4;
  const int pinSpacing = chipSize / (pinCount + 1);

  painter.setBrush(accentColor);
  painter.setPen(Qt::NoPen);

  // Top pins
  for (int i = 1; i <= pinCount; ++i) {
    QRectF pin(margin + i * pinSpacing - pinWidth / 2, margin - pinLength,
               pinWidth, pinLength);
    painter.drawRect(pin);
  }

  // Bottom pins
  for (int i = 1; i <= pinCount; ++i) {
    QRectF pin(margin + i * pinSpacing - pinWidth / 2, margin + chipSize,
               pinWidth, pinLength);
    painter.drawRect(pin);
  }

  // Left pins
  for (int i = 1; i <= pinCount; ++i) {
    QRectF pin(margin - pinLength, margin + i * pinSpacing - pinWidth / 2,
               pinLength, pinWidth);
    painter.drawRect(pin);
  }

  // Right pins
  for (int i = 1; i <= pinCount; ++i) {
    QRectF pin(margin + chipSize, margin + i * pinSpacing - pinWidth / 2,
               pinLength, pinWidth);
    painter.drawRect(pin);
  }

  // Draw CPU circuit lines (decorative)
  painter.setPen(QPen(accentColor, 1.5));
  const int lineMargin = margin + 10;
  const int lineSize = chipSize - 20;

  // Horizontal lines
  painter.drawLine(lineMargin, size / 2 - 6, lineMargin + lineSize,
                   size / 2 - 6);
  painter.drawLine(lineMargin, size / 2, lineMargin + lineSize, size / 2);
  painter.drawLine(lineMargin, size / 2 + 6, lineMargin + lineSize,
                   size / 2 + 6);

  // Vertical lines
  painter.drawLine(size / 2 - 6, lineMargin, size / 2 - 6,
                   lineMargin + lineSize);
  painter.drawLine(size / 2, lineMargin, size / 2, lineMargin + lineSize);
  painter.drawLine(size / 2 + 6, lineMargin, size / 2 + 6,
                   lineMargin + lineSize);

  // Draw center indicator (small circle or square)
  const int centerSize = 8;
  QRectF centerRect(size / 2 - centerSize / 2, size / 2 - centerSize / 2,
                    centerSize, centerSize);
  painter.setBrush(accentColor);
  painter.drawEllipse(centerRect);

  painter.end();
  return pixmap;
}

void SystemTrayIcon::updateIcon() {
  if (!m_trayIcon || !m_daemonClient) {
    qDebug() << "updateIcon: trayIcon or daemonClient is null";
    return;
  }

  bool isConnected = m_daemonClient->connected();
  bool isActive = m_daemonClient->cpuLimitApplied();

  qDebug() << "updateIcon: connected:" << isConnected
           << "cpuLimitApplied:" << isActive;

  // Create custom CPU icon
  QPixmap pixmap = createCpuIcon(isActive, !isConnected);
  m_trayIcon->setIcon(QIcon(pixmap));
}

void SystemTrayIcon::updateTooltip() {
  if (!m_trayIcon || !m_daemonClient) {
    return;
  }

  QString tooltip;

  if (!m_daemonClient->connected()) {
    tooltip = tr("Uncrash - Not Connected");
  } else if (m_daemonClient->cpuLimitApplied()) {
    // CPU limit is ON
    tooltip = tr("Uncrash - CPU Limit ON (GPU: %1W, CPU: %2 GHz)")
                  .arg(m_daemonClient->gpuPower(), 0, 'f', 1)
                  .arg(m_daemonClient->currentMaxFrequency(), 0, 'f', 2);
  } else {
    // CPU limit is OFF
    tooltip = tr("Uncrash - CPU Limit OFF (GPU: %1W)")
                  .arg(m_daemonClient->gpuPower(), 0, 'f', 1);
  }

  m_trayIcon->setToolTip(tooltip);
}
