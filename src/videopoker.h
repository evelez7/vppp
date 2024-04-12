#ifndef VIDEOPOKER_H
#define VIDEOPOKER_H

#include "displayCard.h"
#include "games.h"
#include "shuffle.h"
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <qboxlayout.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class VideoPoker;
}
QT_END_NAMESPACE

enum Hand : unsigned char
{
  HighCard = 0,
  Pair = 1,
  TwoPair = 2,
  ThreeOfAKind = 3,
  Straight = 4,
  Flush = 5,
  FullHouse = 6,
  FourOfAKind = 7,
  StraightFlush = 8,
  RoyalFlush = 9
};

class VideoPoker : public QMainWindow
{
  Q_OBJECT
  Ui::VideoPoker *ui;

  // the box that holds the hand and "Keep" text
  QGridLayout *handBox;
  std::array<DisplayCard *, 5> hand;

  // put cards back in deck after draw
  std::vector<Card> discardedCards;

  // multiple decks to account for ultimate x
  std::vector<std::vector<Card>> decks;
  
  QThread *shuffler;
  
  Shuffle *task;

  // the current game being played, i.e. JoB, bonus
  Games gameType;
  QPushButton *playButton;

  void startShuffling();
  void pullCards();
  void clearKeepLabels();
  
  // enable or disable displayed cards in hand
  void toggleHand(bool disable);

  // check the hand for a win
  Hand checkHand();

public:
  VideoPoker(QWidget *parent = nullptr);
  ~VideoPoker();

public slots:
  void dealButtonClicked();
  void finishedShuffling(std::vector<std::vector<Card>> decks);
  void shuffleError(QString error);
  void deal();
  void draw();
  void cardSelectedEnable(int row, int col);
  void cardSelectedDisable(int row, int col);

signals:
  void readyToPlay();
  void shufflingInterrupted(bool);
};
#endif // VIDEOPOKER_H
