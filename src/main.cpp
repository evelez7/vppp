#include "videopoker.h"
#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <qglobal.h>

int main(int argc, char *argv[])
{
  qDebug() << "Starting QApp";
  QApplication app(argc, argv);
  qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
  VideoPoker vp;
  vp.show();
  return app.exec();
}
