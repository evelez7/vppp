#ifndef VPPP_H
#define VPPP_H

#include "game.h"
#include "games.h"
#include "shuffle.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <memory>
#include <random>

QT_BEGIN_NAMESPACE
namespace Ui
{
class VPPP;
}
QT_END_NAMESPACE

class VPPP : public QMainWindow
{
  Q_OBJECT
  Ui::VPPP *ui;

  std::shared_ptr<std::mt19937> mt;

  std::shared_ptr<QThread> shuffler;

  std::shared_ptr<Shuffle> task;

  mutable std::shared_ptr<std::vector<std::vector<Card>>> decks;

  mutable std::vector<std::vector<Card>> discardPile;

  float balance;

  // the current game being played, i.e. JoB, bonus
  Games gameType;

  Game game;

public:
  VPPP(QWidget *parent = nullptr);
  ~VPPP();

  // Pull a single card from the first deck
  Card pullCard() const;

  // Pull n cards from the first deck
  std::vector<Card> pullCards(unsigned char n) const;

  // Pull n cards from m decks after the first deck
  std::vector<std::vector<Card>> pullCards(unsigned char n,
                                           unsigned char m) const;

  // Indicates the game is ready to draw some cards, so stop the shuffler.
  void interruptShuffling() const;

  // Indicates that the game is ready to allow shuffling, so start the shuffler.
  void startShuffling() const;

public slots:
  void shuffleError(QString error);
  void finishedShuffling();
signals:
  void readyToPlay();
};
#endif // VIDEOPOKER_H
