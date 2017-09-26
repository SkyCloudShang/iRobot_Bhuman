/**
 * @file Controller/Views/PlotView.h
 *
 * Declaration of class PlotView
 *
 * @author <a href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</a>
 * @author Colin Graf
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QIcon>
#include <string>
#include <SimRobot.h>

class RobotConsole;
class QPointF;
class QPainter;
class PlotWidget;

/**
 * @class PlotView
 * A class to represent a map with drawings sent by the robot code.
 */
class PlotView : public SimRobot::Object
{
public:
  /**
   * @param fullName The path to this view in the scene graph
   * @param console The console object.
   * @param name The name of the view.
   * @param plotSize The number of entries in a plot.
   * @param minValue The minimum value in a plot.
   * @param maxValue The maximum value in a plot.
   * @param yUnit The name of the y-axis.
   * @param xUnit The name of the x-axis.
   * @param xScale A scale factor for the x-axis.
   */
  PlotView(const QString& fullName, RobotConsole& console, const std::string& name,
           unsigned int plotSize, float minValue, float maxValue,
           const std::string& yUnit = "", const std::string& xUnit = "", float xScale = 1);

  ~PlotView();

  /**
   * Changes the parameters of the plot.
   * @param plotSize The number of entries in a plot.
   * @param minValue The minimum value in a plot.
   * @param maxValue The maximum value in a plot.
   * @param yUnit The name of the y-axis.
   * @param xUnit The name of the x-axis.
   * @param xScale A scale factor for the x-axis.
   */
  void setParameters(unsigned int plotSize, float minValue, float maxValue,
                     const std::string& yUnit = "", const std::string& xUnit = "", float xScale = 1);

private:
  const QString fullName; /**< The path to this view in the scene graph */
  const QIcon icon; /**< The icon used for listing this view in the scene graph */
  RobotConsole& console; /**< A reference to the console object. */
  PlotWidget* plotWidget = nullptr;
  const std::string name; /**< The name of the view. */
  unsigned int plotSize; /**< The number of entries in a plot. */
  float minValue; /**< The minimum value in a plot. */
  float maxValue; /**< The maximum value in a plot. */
  float valueLength;
  std::string yUnit; /**< The name of the y-axis. */
  std::string xUnit; /**< The name of the x-axis. */
  float xScale; /**< A scale factor for the x-axis. */
  QPointF* points; /**< A buffer for drawing points. */

  /**
   * The method returns a new instance of a widget for this direct view.
   * The caller has to delete this instance. (Qt handles this)
   * @return The widget.
   */
  virtual SimRobot::Widget* createWidget();

  virtual const QString& getFullName() const { return fullName; }
  virtual const QIcon* getIcon() const { return &icon; }

  friend class PlotWidget;
};

class PlotWidget : public QWidget, public SimRobot::Widget
{
  Q_OBJECT

public:
  PlotWidget(PlotView& plotView, PlotWidget*& plotWidget);

  virtual ~PlotWidget();

public slots:
  void determineMinMaxValue();
  void toggleDrawUnits();
  void toggleDrawLegend();
  void toggleAntialiasing();
  void exportAsGnuplot();

private:
  PlotView& plotView;
  PlotWidget*& plotWidget;
  unsigned int lastTimeStamp = 0; /**< Timestamp of the last plot drawing. */
  QPainter painter; /**< The painter used for painting the plot. */
  QPen blackPen;
  QPen grayPen;

  bool drawUnits = true;
  bool drawLegend = true;
  bool antialiasing = false;

  bool needsRepaint() const;

  virtual QSize sizeHint() const { return QSize(320, 240); }

  virtual void paintEvent(QPaintEvent* event);

  virtual QWidget* getWidget() { return this; }
  virtual void update();
  virtual QMenu* createUserMenu() const;
  virtual void paint(QPainter& painter);
};
