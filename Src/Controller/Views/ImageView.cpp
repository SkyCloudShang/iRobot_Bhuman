/**
 * @file Controller/Views/ImageView.cpp
 *
 * Implementation of class ImageView
 *
 * @author <a href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</a>
 * @author Colin Graf
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 * @author Felix Thielke
 */

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QPinchGesture>
#include <QTouchEvent>
#include <QWidget>
#include <QSettings>
#include <QSignalMapper>
#include <QMenu>
#include <QProcess>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>
#include <QElapsedTimer>
#include <QDir>
#include <sstream>
#include <iostream>

#include "ImageView.h"
#include "ColorCalibrationView/ColorCalibrationView.h"
#include "Controller/RobotConsole.h"
#include "Controller/RoboCupCtrl.h"
#include "Controller/Visualization/PaintMethods.h"
#include "Controller/ImageViewAdapter.h"
#include "Representations/Infrastructure/Image.h"
#include "Platform/Thread.h"
#include "Tools/ImageProcessing/ColorModelConversions.h"
#include "Tools/ImageProcessing/YHS2SimpleConversion.h"
#include "Tools/Math/Approx.h"

#include "Tools/Math/Eigen.h"

ImageView::ImageView(const QString& fullName, RobotConsole& console, const std::string& background, const std::string& name, bool segmented, bool upperCam, float gain) :
  upperCam(upperCam), fullName(fullName), icon(":/Icons/tag_green.png"), console(console),
  background(background), name(name), segmented(segmented),
  gain(gain)
{}

void ImageView::forwardLastImage()
{
  if(widget != nullptr)
    widget->forwardLastImage();
}

SimRobot::Widget* ImageView::createWidget()
{
  widget = new ImageWidget(*this);
  return widget;
}

ImageWidget::ImageWidget(ImageView& imageView) :
  imageView(imageView), dragStart(-1, -1), offset(0, 0)
{
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  grabGesture(Qt::PinchGesture);
  setAttribute(Qt::WA_AcceptTouchEvents);

  QSettings& settings = RoboCupCtrl::application->getLayoutSettings();
  settings.beginGroup(imageView.fullName);
  zoom = (float)settings.value("Zoom", 1.).toDouble();
  offset = settings.value("Offset", QPointF()).toPoint();
  settings.endGroup();
}

ImageWidget::~ImageWidget()
{
  QSettings& settings = RoboCupCtrl::application->getLayoutSettings();
  settings.beginGroup(imageView.fullName);
  settings.setValue("Zoom", (double)zoom);
  settings.setValue("Offset", offset);
  settings.endGroup();

  imageView.widget = nullptr;

  if(imageData)
    delete imageData;
  if(imageDataStorage)
    Memory::alignedFree(imageDataStorage);
}

void ImageWidget::paintEvent(QPaintEvent* event)
{
  painter.begin(this);
  paint(painter);
  painter.end();
}

void ImageWidget::paint(QPainter& painter)
{
  SYNC_WITH(imageView.console);

  const DebugImage* image = nullptr;
  RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
  RobotConsole::Images::const_iterator i = currentImages.find(imageView.background);

  if(i != currentImages.end())
  {
    image = i->second.image;
    imageWidth = image->getImageWidth();
    imageHeight = image->height;
  }
  else if(!currentImages.empty())
  {
    imageWidth = currentImages.begin()->second.image->getImageWidth();
    imageHeight = currentImages.begin()->second.image->height;
  }

  const QSize& size = painter.window().size();
  float xScale = float(size.width()) / float(imageWidth);
  float yScale = float(size.height()) / float(imageHeight);
  scale = xScale < yScale ? xScale : yScale;
  scale *= zoom;
  float imageXOffset = (float(size.width()) - float(imageWidth) * scale) * 0.5f + float(offset.x()) * scale;
  float imageYOffset = (float(size.height()) - float(imageHeight) * scale) * 0.5f + float(offset.y()) * scale;

  painter.setTransform(QTransform(scale, 0, 0, scale, imageXOffset, imageYOffset));

  if(image)
    paintImage(painter, *image);
  else
    lastImageTimeStamp = 0;

  paintDrawings(painter);
}

void ImageWidget::paintDrawings(QPainter& painter)
{
  const QTransform baseTrans = painter.transform();
  const std::list<std::string>& drawings = imageView.console.imageViews[imageView.name];
  for(const std::string& drawing : drawings)
  {
    auto& camDrawings = imageView.upperCam ? imageView.console.upperCamImageDrawings : imageView.console.lowerCamImageDrawings;
    auto debugDrawing = camDrawings.find(drawing);
    if(debugDrawing != camDrawings.end())
    {
      PaintMethods::paintDebugDrawing(painter, debugDrawing->second, baseTrans);
      if(debugDrawing->second.timeStamp > lastDrawingsTimeStamp)
        lastDrawingsTimeStamp = debugDrawing->second.timeStamp;
    }
    auto& motinoDrawings = imageView.console.motionImageDrawings;
    debugDrawing = motinoDrawings.find(drawing);
    if(debugDrawing != motinoDrawings.end())
    {
      PaintMethods::paintDebugDrawing(painter, debugDrawing->second, baseTrans);
      if(debugDrawing->second.timeStamp > lastDrawingsTimeStamp)
        lastDrawingsTimeStamp = debugDrawing->second.timeStamp;
    }
  }
  painter.setTransform(baseTrans);
}

void ImageWidget::copyImage(const DebugImage& srcImage)
{
  int width = srcImage.width;
  int height = srcImage.height;

  const QImage::Format desiredFormat =  QImage::Format::Format_RGB32;

  if(!imageData || !imageDataStorage ||
     !(imageData->width() == srcImage.getImageWidth() && imageData->height() == height) ||
     imageData->format() != desiredFormat)
  {
    if(imageData)
      delete imageData;
    if(imageDataStorage)
      Memory::alignedFree(imageDataStorage);
    imageDataStorage = Memory::alignedMalloc(srcImage.getImageWidth() * height * 4, 32);
    imageData = new QImage(reinterpret_cast<unsigned char*>(imageDataStorage), srcImage.getImageWidth(), height, desiredFormat);
  }

  void* dest = imageData->scanLine(0);
  if(srcImage.type == PixelTypes::BGRA)
  {
    memcpy(dest, srcImage.getView<PixelTypes::BGRAPixel>()[0], width * height * PixelTypes::pixelSize(srcImage.type));
  }
  else
  {
    srcImage.convertToBGRA(dest);
  }

  if(imageView.gain != 1.f)
  {
    unsigned* p = (unsigned*)imageData->scanLine(0);
    float gain = imageView.gain;
    for(unsigned* pEnd = p + width * height; p < pEnd; ++p)
    {
      int r = (int)(gain * (float)((*p >> 16) & 0xff));
      int g = (int)(gain * (float)((*p >> 8) & 0xff));
      int b = (int)(gain * (float)((*p) & 0xff));

      *p++ = (r < 0 ? 0 : r > 255 ? 255 : r) << 16 |
             (g < 0 ? 0 : g > 255 ? 255 : g) << 8 |
             (b < 0 ? 0 : b > 255 ? 255 : b) |
             0xff000000;
    }
  }
}

void ImageWidget::copyImageSegmented(const DebugImage& srcImage)
{
  if(!imageData || !imageDataStorage ||
     !(imageData->width() == srcImage.getImageWidth() && imageData->height() == srcImage.height) ||
     imageData->format() != QImage::Format::Format_RGB32)
  {
    if(imageData)
      delete imageData;
    if(imageDataStorage)
      Memory::alignedFree(imageDataStorage);
    imageDataStorage = Memory::alignedMalloc(srcImage.getImageWidth() * srcImage.height * 4, 32);
    imageData = new QImage(reinterpret_cast<unsigned char*>(imageDataStorage), srcImage.getImageWidth(), srcImage.height, QImage::Format::Format_RGB32);
  }

  segmentImage(srcImage);
}

void ImageWidget::segmentImage(const DebugImage& srcImage)
{
  if(srcImage.type != PixelTypes::YUYV)
    return;

  TImage<PixelTypes::ColoredPixel> colored(srcImage.width * 2, srcImage.height);
  TImage<PixelTypes::GrayscaledPixel> grayPixel(0, 0);
  TImage<PixelTypes::HuePixel> huePixel(0, 0);
  YHS2s::updateSSE<true, false, false, false, false>(srcImage.getView<PixelTypes::YUYVPixel>()[0], srcImage.width, srcImage.height, imageView.console.colorCalibration,
      grayPixel, grayPixel, grayPixel, colored, huePixel, grayPixel);

  DebugImage debugColored(colored);
  void* dest = imageData->scanLine(0);
  debugColored.convertToBGRA(dest);
}

void ImageWidget::paintImage(QPainter& painter, const DebugImage& srcImage)
{
  if(srcImage.timeStamp != lastImageTimeStamp || imageView.segmented)
  {
    if(imageView.segmented)
      copyImageSegmented(srcImage);
    else
      copyImage(srcImage);

    lastImageTimeStamp = srcImage.timeStamp;
    if(imageView.segmented)
      lastColorTableTimeStamp = imageView.console.colorCalibrationTimeStamp;
  }
  else if(!imageData || imageWidth != imageData->width() || imageHeight != imageData->height())
  {
    // make sure we have a buffer
    if(imageData)
      delete imageData;
    imageData = new QImage(imageWidth, imageHeight, QImage::Format_RGB32);
  }

  painter.drawImage(QRectF(0, 0, imageWidth, imageHeight), *imageData);
}

bool ImageWidget::needsRepaint() const
{
  SYNC_WITH(imageView.console);
  DebugImage* image = nullptr;
  RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
  RobotConsole::Images::const_iterator j = currentImages.find(imageView.background);
  if(j != currentImages.end())
    image = j->second.image;

  if(!image)
  {
    const std::list<std::string>& drawings(imageView.console.imageViews[imageView.name]);
    for(const std::string& drawing : drawings)
    {
      const DebugDrawing& debugDrawing(imageView.upperCam ? imageView.console.upperCamImageDrawings[drawing] : imageView.console.lowerCamImageDrawings[drawing]);
      if(debugDrawing.timeStamp > lastDrawingsTimeStamp)
        return true;
    }
    return lastImageTimeStamp != 0;
  }
  else
    return image->timeStamp != lastImageTimeStamp ||
           (imageView.segmented && imageView.console.colorCalibrationTimeStamp != lastColorTableTimeStamp);
}

void ImageWidget::window2viewport(QPointF& point)
{
  const QSize& size(this->size());
  float xScale = float(size.width()) / float(imageWidth);
  float yScale = float(size.height()) / float(imageHeight);
  float scale = xScale < yScale ? xScale : yScale;
  scale *= zoom;
  float xOffset = (float(size.width()) - float(imageWidth) * scale) * 0.5f + float(offset.x()) * scale;
  float yOffset = (float(size.height()) - float(imageHeight) * scale) * 0.5f + float(offset.y()) * scale;
  point = QPointF((point.x() - xOffset) / scale, (point.y() - yOffset) / scale);
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);
  SYNC_WITH(imageView.console);
  QPointF pos(event->pos());
  mousePos = pos;

  if(dragStart != QPointF(-1, -1))
  {
    offset = dragStartOffset + (pos - dragStart) / scale;
    QWidget::update();
    return;
  }

  window2viewport(pos);

  DebugImage* image = nullptr;
  RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
  RobotConsole::Images::const_iterator i = currentImages.find(imageView.background);
  if(i != currentImages.end())
    image = i->second.image;

  // Update tool tip
  const char* text = 0;
  const std::list<std::string>& drawings(imageView.console.imageViews[imageView.name]);
  Pose2f origin;
  for(const std::string& drawing : drawings)
  {
    const DebugDrawing& debugDrawing(imageView.upperCam ? imageView.console.upperCamImageDrawings[drawing] : imageView.console.lowerCamImageDrawings[drawing]);
    debugDrawing.updateOrigin(origin);
    {
      int x = static_cast<int>(pos.x());
      int y = static_cast<int>(pos.y());
      text = debugDrawing.getTip(x, y, origin);
      pos.setX(x);
      pos.setY(y);
    }
    if(text)
      break;
  }

  if(text)
    setToolTip(QString(text));
  else if(image && pos.rx() >= 0 && pos.ry() >= 0 && pos.rx() < image->getImageWidth() && pos.ry() < image->height)
  {
    char tooltipstr[128];
    sprintf(tooltipstr, "x=%d, y=%d\n", static_cast<int>(pos.rx()), static_cast<int>(pos.ry()));

    char* tooltip = tooltipstr + strlen(tooltipstr);

    switch(image->type)
    {
      case PixelTypes::RGB:
      {
        const PixelTypes::RGBPixel& px = image->getView<PixelTypes::RGBPixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())];
        sprintf(tooltip, "R=%d, G=%d, B=%d", px.r, px.g, px.b);
      }
      break;
      case PixelTypes::BGRA:
      {
        const PixelTypes::BGRAPixel& px = image->getView<PixelTypes::BGRAPixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())];
        sprintf(tooltip, "R=%d, G=%d, B=%d", px.r, px.g, px.b);
      }
      break;
      case PixelTypes::YUV:
      {
        const PixelTypes::YUVPixel& px = image->getView<PixelTypes::YUVPixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())];
        sprintf(tooltip, "Y=%d, U=%d, V=%d", px.y, px.u, px.v);
      }
      break;
      case PixelTypes::YUYV:
      {
        const PixelTypes::YUYVPixel& px = image->getView<PixelTypes::YUYVPixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx() / 2)];
        sprintf(tooltip, "Y=%d, U=%d, V=%d", px.y(static_cast<size_t>(pos.rx())), px.u, px.v);
      }
      break;
      case PixelTypes::Grayscale:
        sprintf(tooltip, "Luminosity=%d", image->getView<PixelTypes::GrayscaledPixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())]);
        break;
      case PixelTypes::Colored:
        switch(image->getView<PixelTypes::ColoredPixel>()[static_cast<size_t>(pos.ry())][static_cast<int>(pos.rx())])
        {
          case FieldColors::none:
            strcpy(tooltip, "none");
            break;
          case FieldColors::white:
            strcpy(tooltip, "white");
            break;
          case FieldColors::black:
            strcpy(tooltip, "black");
            break;
          case FieldColors::field:
            strcpy(tooltip, "field");
            break;
          default:
            ASSERT(false);
            return;
        }
        break;
      case PixelTypes::Hue:
        sprintf(tooltip, "Hue=%d", (int)image->getView<PixelTypes::HuePixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())]);
        break;
      case PixelTypes::Edge2:
      {
        const PixelTypes::Edge2Pixel& px = image->getView<PixelTypes::Edge2Pixel>()[static_cast<int>(pos.ry())][static_cast<int>(pos.rx())];
        sprintf(tooltip, "FilterX=%d, FilterY=%d", px.filterX, px.filterY);
      }
      break;
      default:
        break;
    }
    setToolTip(QString(tooltipstr));
  }
  else
    setToolTip(QString());
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{
  QWidget::mousePressEvent(event);

  if(event->button() == Qt::LeftButton || event->button() == Qt::MidButton)
  {
    dragStart = event->pos();
    dragStartOffset = offset;
  }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
  QWidget::mouseReleaseEvent(event);
  QPointF pos = QPointF(event->pos());
  if(dragStart != pos && dragStart != QPointF(-1, -1))
  {
    dragStart = QPointF(-1, -1);
    QWidget::update();
    return;
  }
  dragStart = QPointF(-1, -1);
  window2viewport(pos);
  Vector2i v = Vector2i(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
  if(event->modifiers() & Qt::ShiftModifier)
  {
    if(!headControlMode)
    {
      imageView.console.handleConsole("mr HeadMotionRequest ManualHeadMotionProvider");
      headControlMode = true;
    }
    std::stringstream command;
    command << "set parameters:ManualHeadMotionProvider xImg = " << v.x()
            << "; yImg = " << v.y()
            << "; camera = " << (imageView.upperCam ? "upper" : "lower") << ";";
    imageView.console.handleConsole(command.str());
  }
  {
    SYNC_WITH(imageView.console);
    if(!(event->modifiers() & Qt::ShiftModifier))
    {
      if((!event->modifiers() || (event->modifiers() & Qt::ControlModifier))
         && imageView.console.colorCalibrationView
         && imageView.console.colorCalibrationView->widget
         && imageView.console.colorCalibrationView->widget->expandColorMode)
      {
        DebugImage* image = nullptr;
        RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
        RobotConsole::Images::const_iterator i = currentImages.find(imageView.background);
        if(i != currentImages.end())
          image = i->second.image;
        if(image && pos.x() >= 0 && pos.y() >= 0 && pos.x() < image->width && pos.y() < image->height)
          imageView.console.colorCalibrationView->widget->expandCurrentColor(image->getView<PixelTypes::YUYVPixel>()[static_cast<int>(pos.y())][static_cast<int>(pos.x())], event->modifiers() & Qt::ControlModifier);
      }
      else if(event->modifiers() & Qt::ControlModifier)
      {
        ImageViewAdapter::fireClick(imageView.name, v, imageView.upperCam, false);
      }
      else
        ImageViewAdapter::fireClick(imageView.name, v, imageView.upperCam, true);
    }
  }
}

#define ZOOM_MAX_VALUE 500.f
#define ZOOM_MIN_VALUE 0.01f
void ImageWidget::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_PageUp:
    case Qt::Key_Plus:
      event->accept();
      zoom += 0.1f * zoom;
      if(zoom > ZOOM_MAX_VALUE)
        zoom = ZOOM_MAX_VALUE;
      QWidget::update();
      break;
    case Qt::Key_PageDown:
    case Qt::Key_Minus:
      event->accept();
      zoom -= 0.1f * zoom;
      if(zoom < ZOOM_MIN_VALUE)
        zoom = ZOOM_MIN_VALUE;
      QWidget::update();
      break;
    case Qt::Key_Up:
      offset += QPointF(0, 20 / zoom);
      QWidget::update();
      break;
    case Qt::Key_Down:
      offset += QPointF(0, -20 / zoom);
      QWidget::update();
      break;
    case Qt::Key_Left:
      offset += QPointF(20 / zoom, 0);
      QWidget::update();
      break;
    case Qt::Key_Right:
      offset += QPointF(-20 / zoom, 0);
      QWidget::update();
      break;
    default:
      QWidget::keyPressEvent(event);
      break;
  }
}

bool ImageWidget::event(QEvent* event)
{
  if(event->type() == QEvent::Gesture)
  {
    QPinchGesture* pinch = static_cast<QPinchGesture*>(static_cast<QGestureEvent*>(event)->gesture(Qt::PinchGesture));
    if(pinch && (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged))
    {
#ifdef FIX_MACOS_NO_CENTER_IN_PINCH_GESTURE_BUG
      QPoint center = mapFromGlobal(QCursor::pos());
#else
      QPointF center(pinch->centerPoint().x(),
                     pinch->centerPoint().y());
#endif
      QPointF before(center);
      window2viewport(before);
      scale /= zoom;
#ifdef FIX_MACOS_PINCH_SCALE_RELATIVE_BUG
      pinch->setLastScaleFactor(1.f);
#endif
      zoom *= pinch->scaleFactor() / pinch->lastScaleFactor();
      if(zoom > ZOOM_MAX_VALUE)
        zoom = ZOOM_MAX_VALUE;
      else if(zoom < ZOOM_MIN_VALUE)
        zoom = ZOOM_MIN_VALUE;
      scale *= zoom;
      QPointF after(center);
      window2viewport(after);
      offset -= before - after;
      QWidget::update();
      return true;
    }
  }
  return QWidget::event(event);
}

void ImageWidget::wheelEvent(QWheelEvent* event)
{
  QWidget::wheelEvent(event);
#ifndef MACOS
  QPointF beforeZoom = mousePos;
  window2viewport(beforeZoom);

  zoom += zoom * 0.1 * event->delta() / 120;
  if(zoom > ZOOM_MAX_VALUE)
    zoom = ZOOM_MAX_VALUE;
  else if(zoom < ZOOM_MIN_VALUE)
    zoom = ZOOM_MIN_VALUE;

  QPointF afterZoom = mousePos;
  window2viewport(afterZoom);
  offset += afterZoom - beforeZoom;

  QWidget::update();
#else
  float step = event->delta() / (scale * 2.f);
  offset += event->orientation() == Qt::Horizontal ? QPointF(step, 0) : QPointF(0, step);
  if(step != 0.f)
    QWidget::update();
#endif
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  zoom = 1.f;
  offset.setX(0);
  offset.setY(0);
  QWidget::update();
}

QMenu* ImageWidget::createUserMenu() const
{
  QMenu* menu = new QMenu(tr("&Image"));

  menu->addSeparator();

  QAction* saveImgAct = new QAction(tr("&Save Image"), menu);
  connect(saveImgAct, SIGNAL(triggered()), this, SLOT(saveImg()));
  menu->addAction(saveImgAct);

  return menu;
}

void ImageWidget::forwardLastImage()
{
  DebugImage* image = nullptr;
  RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
  RobotConsole::Images::const_iterator j = currentImages.find(imageView.background);
  if(j != currentImages.end())
    image = j->second.image;
}

void ImageWidget::saveImg()
{
  QSettings& settings = RoboCupCtrl::application->getSettings();
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as PNG"), settings.value("ExportDirectory", "").toString(), tr("(*.png)"));
  if(fileName.isEmpty())
    return;
  if(!QFileInfo(fileName).fileName().contains("."))
  {
    // Enforce .png ending
    fileName += ".png";
  }
  settings.setValue("ExportDirectory", QFileInfo(fileName).dir().path());

  SYNC_WITH(imageView.console);

  const DebugImage* image = nullptr;
  RobotConsole::Images& currentImages = imageView.upperCam ? imageView.console.upperCamImages : imageView.console.lowerCamImages;
  RobotConsole::Images::const_iterator i = currentImages.find(imageView.background);

  if(i != currentImages.end())
  {
    image = i->second.image;
    imageWidth = image->type == PixelTypes::YUYV ? image->width * 2 : image->width;
    imageHeight = image->height;
  }
  if(image)
  {
    QPixmap pixmap(image->type == PixelTypes::YUYV ? image->width * 2 : image->width, image->height);
    QPainter painter(&pixmap);
    paintImage(painter, *image);
    paintDrawings(painter);
    pixmap.save(fileName, "PNG");
  }
}

void ImageWidget::colorAct(int color)
{
  ColorCalibrationView* colorView = imageView.console.colorCalibrationView;
  if(colorView && colorView->widget)
  {
    colorView->widget->updateWidgets((FieldColors::Color) color);
    colorView->widget->currentCalibrationChanged();
  }
}

void ImageWidget::setUndoRedo(const bool enableUndo, const bool enableRedo)
{
  if(undoAction && redoAction)
  {
    ColorCalibrationView* colorView = imageView.console.colorCalibrationView;
    disconnect(undoAction, nullptr, nullptr, nullptr);
    disconnect(redoAction, nullptr, nullptr, nullptr);
    connect(undoAction, SIGNAL(triggered()), colorView->widget, SLOT(undoColorCalibration()));
    connect(redoAction, SIGNAL(triggered()), colorView->widget, SLOT(redoColorCalibration()));
    undoAction->setEnabled(enableUndo);
    redoAction->setEnabled(enableRedo);
  }
}
