#include "vppp.h"
#include "./ui_vppp.h"
#include "card.h"
#include "hand.h"
#include "shuffle.h"
#include "videopoker.h"
#include <QBoxLayout>
#include <QDebug>
#include <QObject>
#include <QSvgWidget>
#include <algorithm>
#include <array>
#include <qnamespace.h>
#include <utility>

namespace
{
// No jokers
std::vector<Card> createStandardDeck()
{
  std::vector<Card> deck;
  deck.reserve(52);
  std::array<Suit, 4> suits = {Suit::Clubs, Suit::Diamonds, Suit::Hearts,
                               Suit::Spades};
  std::array<Face, 3> faces = {Face::Jack, Face::Queen, Face::King};

  for (const auto suit : suits)
  {                                        // all suits
    for (unsigned char j = 2; j < 11; ++j) // numbered cards
      deck.emplace_back(suit, std::nullopt, j);
    for (unsigned char j = 0; j < faces.size(); ++j)
      deck.emplace_back(suit, faces.at(j), 10);
    deck.emplace_back(suit, Face::Ace, 11);
  }

  return deck;
}

} // namespace

VPPP::VPPP(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::VPPP), gameType(Games::JACKS_OR_BETTER),
      shuffler(new QThread()),
      mt(std::make_shared<std::mt19937>(std::random_device{}())), balance(100)
{
  //  allow for passing back from the shuffling thread
  qRegisterMetaType<std::vector<std::vector<Card>>>(
      "std::vector<std::vector<Card>>");

  if (gameType == Games::JACKS_OR_BETTER)
  {
    game = static_cast<Game>(VideoPoker(this));
    decks->push_back(createStandardDeck());
  }

  task = std::make_shared<Shuffle>(decks, mt);
  QObject::connect(task.get(), &Shuffle::finishedShuffling, this,
                   &VPPP::finishedShuffling);
  QObject::connect(task.get(), &Shuffle::error, this, &VPPP::shuffleError);
  QObject::connect(shuffler.get(), &QThread::started, task.get(),
                   &Shuffle::run);
  // QObject::connect(task, &Shuffle::finishedShuffling, shuffler,
  // &QThread::quit);
  task->moveToThread(shuffler.get());
  shuffler->start();
  ui->setupUi(this);

  Q_INIT_RESOURCE(images);
}

Card VPPP::pullCard() const
{
  Card toReturn = decks->at(0).back();
  discardPile.at(0).push_back(toReturn);
  decks->at(0).pop_back();
  return toReturn;
}

std::vector<Card> VPPP::pullCards(unsigned char n) const
{
  std::vector<Card> toReturn;
  toReturn.reserve(n);
  for (unsigned char i = 0; i < n; ++i)
  {
    Card currentCard = pullCard();
    toReturn.push_back(currentCard);
    discardPile.at(0).push_back(currentCard);
    decks->at(0).pop_back();
  }
  return toReturn;
}

std::vector<std::vector<Card>> VPPP::pullCards(unsigned char n,
                                               unsigned char m) const
{
  std::vector<std::vector<Card>> toReturn;
  toReturn.reserve(n);
  for (unsigned char i = 1; i < m; ++i)
  {
    for (unsigned char j = 0; j < n; ++j)
    {
      Card currentCard = decks->at(i).back();
      decks->at(i).pop_back();
      toReturn.at(i).push_back(currentCard);
      discardPile.at(i).push_back(currentCard);
    }
  }
  return toReturn;
}

void VPPP::startShuffling() const
{
  for (unsigned char i = 0; i < decks->size(); ++i)
  {
    decks->at(i).push_back(discardPile.at(i).back());
    discardPile.at(i).pop_back();
  }
  shuffler->start();
}

void VPPP::interruptShuffling() const { shuffler->requestInterruption(); }

VPPP::~VPPP()
{
  delete ui;
  // delete shuffler;
}

void VPPP::shuffleError(QString error) { qDebug() << error; }

void VPPP::finishedShuffling() { game.play(); }
