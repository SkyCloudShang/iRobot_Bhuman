/**
 * @file Controller/Views/SensorView.h
 *
 * Declaration of class SensorView
 *
 * @author of original sensorview <a href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</a>
 * @author Jeff
 * @author Colin Graf
 */

#pragma once

#include <SimRobot.h>
#include <QHeaderView>
#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QSettings>
#include "Tools/Math/Eigen.h"

struct FsrSensorData;
struct InertialSensorData;
struct KeyStates;
struct SystemSensorData;
class RobotConsole;
class SensorWidget;

/**
 * @class SensorView
 * A class to represent a view with information about the sensor values.
 */
class SensorView : public SimRobot::Object
{
private:
  const QString fullName; /**< The path to this view in the scene graph */
  const QIcon icon; /**< The icon used for listing this view in the scene graph */
  RobotConsole& console; /**< A reference to the console object. */
  const FsrSensorData& fsrSensorData;
  const InertialSensorData& inertialSensorData;
  const KeyStates& keyStates;
  const SystemSensorData& systemSensorData;
  const unsigned& timeStamp;

  friend class SensorWidget;

public:
  /**
   * @param fullName The path to this view in the scene graph
   * @param robotConsole The robot console which owns \c sensorData.
   */
  SensorView(const QString& fullName, RobotConsole& robotConsole, const FsrSensorData& fsrSensorData,
             const InertialSensorData& inertialSensorData, const KeyStates& keyStates,
             const SystemSensorData& systemSensorData, const unsigned& timeStamp);

private:
  /**
   * The method returns a new instance of a widget for this direct view.
   * The caller has to delete this instance. (Qt handles this)
   * @return The widget.
   */
  virtual SimRobot::Widget* createWidget();
  virtual const QString& getFullName() const { return fullName; }
  virtual const QIcon* getIcon() const { return &icon; }
};

class SensorWidget : public QWidget
{
  Q_OBJECT

private:
  enum class ValueType
  {
    acceleration, // m/s^2
    angle,        // °
    angularSpeed, // °/s^2
    current,      // A
    pressure,     // g
    ratio,        // %
    temperature   // °C
  };

  SensorView& sensorView;
  QHeaderView* headerView;

  QPainter painter;
  int lineSpacing;
  int textOffset;

  QFont font;
  QBrush altBrush;
  QPen fontPen;
  QPen noPen;
  bool fillBackground;

  QRect paintRect;
  QRect paintRectField0;
  QRect paintRectField1;
  unsigned lastUpdateTimeStamp = 0;

public:
  SensorWidget(SensorView& sensorView, QHeaderView* headerView, QWidget* parent);
  virtual ~SensorWidget();

  void update();
  void paintEvent(QPaintEvent* event);

  public slots:
  void forceUpdate();

private:
  void paintFsrSensorData();
  void paintInertialSensorData();
  void paintKeyStates();
  void paintSystemSensorData();
  QString printValue(ValueType valueType, float value) const;
  QString printCoordinate(Vector2f val) const;
  QString printButton(bool pressed) const;
  void print(const QString& name, const QString& value);
  void newSection();
  QSize sizeHint() const { return QSize(250, 400); }
};

