#include "Platform/File.h"
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPixmap>
#include <QSplitter>
#include <QSettings>
#include <QApplication>
#include <qstyleoption.h>
#include "Utils/bush/ui/Console.h"
#include "Utils/bush/ui/MainWindow.h"
#include "Utils/bush/ui/RobotPool.h"
#include "Utils/bush/ui/ShortcutBar.h"
#include "Utils/bush/ui/TeamSelector.h"
#include "Utils/bush/ui/SizeManager.h"

MainWindow::MainWindow()
{
  setWindowTitle("bush");
  SizeManager sizeManager;
  int widgetWidth = sizeManager.widgetWidth - frameGeometry().width();
  QSplitter* splitter(new QSplitter(Qt::Vertical));
  TeamSelector* teamSelector(new TeamSelector());
  splitter->addWidget(teamSelector);
  splitter->setStretchFactor(0, 0);

  QSplitter* hSplitter(new QSplitter(Qt::Horizontal));
  hSplitter->addWidget(splitter);
  RobotPool* robotPool = new RobotPool(teamSelector);
  connect(teamSelector, SIGNAL(currentChanged(int)), robotPool, SLOT(update()));
  QFrame* rightSide = new QFrame(this);
  QGridLayout* rsLayout = new QGridLayout(rightSide);
  rsLayout->addWidget(new QLabel("<b>Robot Pool:</b>"), 0, 0);
  rsLayout->addWidget(robotPool, 1, 0);
  rightSide->setLayout(rsLayout);
  rightSide->setMinimumWidth(static_cast<int>(widgetWidth / 6));
  hSplitter->addWidget(rightSide);
  setCentralWidget(hSplitter);

  Console* console = new Console(teamSelector);
  this->setWindowIcon(QPixmap(":icons/bush.png"));
  console->setFocus(Qt::OtherFocusReason);
  console->resize(60, 500);

  ShortcutBar* shortcutBar = new ShortcutBar(console);
#ifdef MACOS
  addToolBar(Qt::TopToolBarArea, shortcutBar);
  setUnifiedTitleAndToolBarOnMac(true);
  shortcutBar->setMovable(false);
#else
  addToolBar(Qt::BottomToolBarArea, shortcutBar);
#endif
  shortcutBar->addShortcut("help", "help");
  QAction* deplayAction = shortcutBar->addShortcut("deploy", "deploy");
  deplayAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
  deplayAction->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
  shortcutBar->addShortcut("download logs", "downloadLogs");
  shortcutBar->addShortcut("delete logs", "deleteLogs");
  shortcutBar->addShortcut("simulator", "sim");
  shortcutBar->addShortcut("shutdown", "shutdown -s");
  splitter->addWidget(console);

  int widgetWidthResize = 1128;
  int widgetHeightResize = 695;
  teamSelector->loadTeams();
  splitter->setMinimumWidth(100);
  QWidget::resize(widgetWidthResize, widgetHeightResize);
  QDesktopWidget desktop;
  QPoint position((desktop.width() - frameGeometry().width()) / 2, (desktop.height() - frameGeometry().height()) / 2);
  QWidget::move(position);
  readSettings();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings("B-Human", "bush");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
  QSettings settings("B-Human", "bush");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}
