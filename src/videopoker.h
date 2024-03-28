#ifndef VIDEOPOKER_H
#define VIDEOPOKER_H

#include "displayCard.h"
#include "games.h"
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <qboxlayout.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class VideoPoker;
}
QT_END_NAMESPACE

class VideoPoker : public QMainWindow
{
  Q_OBJECT
  Ui::VideoPoker *ui;
  QGridLayout *handBox;
  std::array<DisplayCard *, 5> hand;
  // multiple decks to account for ultimate x
  std::vector<std::vector<Card>> decks;
  QThread *shuffler;
  Games gameType;

  void startShuffling();

public:
  VideoPoker(QWidget *parent = nullptr);
  ~VideoPoker();

public slots:
  void dealButtonClicked();
  void shufflerFinished(std::vector<std::vector<Card>> decks);
  void shuffleError(QString error);
  void deal();
  void cardSelectedEnable(int row, int col);
  void cardSelectedDisable(int row, int col);

signals:
  void readyToPlay();
  void shufflingInterrupted(bool);
};
#endif // VIDEOPOKER_H
