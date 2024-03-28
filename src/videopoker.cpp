#include "videopoker.h"
#include "./ui_videopoker.h"
#include "displayCard.h"
#include "shuffle.h"
#include <QDebug>
#include <QLabel>
#include <QSvgWidget>
#include <iostream>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qsvgwidget.h>
#include <random>

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
    deck.emplace_back(suit, std::nullopt, 11);
  }

  return deck;
}

VideoPoker::VideoPoker(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::VideoPoker),
      gameType(Games::JACKS_OR_BETTER), shuffler(new QThread())
{
  // allow for passing back from the shuffling thread
  qRegisterMetaType<std::vector<std::vector<Card>>>(
      "std::vector<std::vector<Card>>");
  ui->setupUi(this);

  Q_INIT_RESOURCE(images);
  QVBoxLayout *vbox = new QVBoxLayout(this->centralWidget());
  handBox = new QGridLayout();
  vbox->addLayout(handBox);
  for (int i = 0; i < 5; ++i)
  {
    auto *card = new DisplayCard();
    connect(card, &DisplayCard::clickedCardEnable, this,
            &VideoPoker::cardSelectedEnable);
    connect(card, &DisplayCard::clickedCardDisable, this,
            &VideoPoker::cardSelectedDisable);
    card->setMaximumSize(QSize(400, 400));
    card->load(QString(":/assets/back.svg"));
    handBox->addWidget(card, 1, i);
    hand.at(i) = card;
    card->setPos(1, i);
  }

  QPushButton *dealButton = new QPushButton(QString("Deal"));
  connect(dealButton, &QAbstractButton::clicked, this,
          &VideoPoker::dealButtonClicked);
  connect(this, &VideoPoker::shufflingInterrupted, dealButton,
          &QAbstractButton::setDisabled);
  connect(this, &VideoPoker::readyToPlay, dealButton, &QAbstractButton::hide);
  vbox->addWidget(dealButton);

  decks.push_back(createStandardDeck());
  startShuffling();
}

VideoPoker::~VideoPoker()
{
  delete ui;
  delete shuffler;
}

void VideoPoker::dealButtonClicked()
{
  emit shufflingInterrupted(true);
  qDebug() << "Requested shuffle interruption";
  shuffler->requestInterruption();
}

void VideoPoker::shufflerFinished(std::vector<std::vector<Card>> decks)
{
  this->decks = decks;
  qDebug() << "Received decks";
  emit readyToPlay();
  deal();
}

void VideoPoker::deal()
{
  for (unsigned char i = 0; i < 5; ++i)
  {
    hand.at(i)->setCard(decks.at(0).back());
    decks.at(0).pop_back();

    QString card;
    if (!hand.at(i)->getCard().getFace())
    {
      if (hand.at(i)->getCard().getValue() == 11)
        card = "ace";
      else
        card = QString::number(hand.at(i)->getCard().getValue());
    }
    else
    {
      switch (hand.at(i)->getCard().getFace().value())
      {
      case Face::Jack:
        card = "jack";
        break;
      case Face::Queen:
        card = "queen";
        break;
      case Face::King:
        card = "king";
        break;
      }
    }

    QString suit;
    switch (hand.at(i)->getCard().getSuit())
    {
    case Suit::Clubs:
      suit = "clubs";
      break;
    case Suit::Diamonds:
      suit = "diamonds";
      break;
    case Suit::Hearts:
      suit = "hearts";
      break;
    case Suit::Spades:
      suit = "spades";
      break;
    }
    QString path(":/assets/" + suit + "_" + card + ".svg");
    static_cast<DisplayCard *>(handBox->itemAt(i)->widget())->load(path);
  }
}

void VideoPoker::draw()
{
  for (unsigned char i = 0; i < hand.size(); ++i)
  {
    if (!hand.at(i)->isSelected())
    {
    }
  }
}

void VideoPoker::cardSelectedEnable(int row, int col)
{
  handBox->addWidget(new QLabel("Keep"), row - 1, col);
}

void VideoPoker::cardSelectedDisable(int row, int col)
{
  delete handBox->itemAtPosition(row - 1, col)->widget();
}

void VideoPoker::shuffleError(QString error) { qDebug() << error; }

void VideoPoker::startShuffling()
{
  Shuffle *shuffleTask = new Shuffle(decks);
  shuffleTask->moveToThread(shuffler);
  QObject::connect(shuffleTask, &Shuffle::signalFinishedShuffling, this,
                   &VideoPoker::shufflerFinished);
  QObject::connect(shuffleTask, &Shuffle::error, this,
                   &VideoPoker::shuffleError);
  QObject::connect(shuffler, &QThread::started, shuffleTask, &Shuffle::run);
  QObject::connect(shuffleTask, &Shuffle::signalFinishedShuffling, shuffler,
                   &QThread::quit);
  QObject::connect(shuffleTask, &Shuffle::signalFinishedShuffling, shuffleTask,
                   &Shuffle::deleteLater);
  QObject::connect(shuffler, &QThread::finished, shuffler,
                   &QThread::deleteLater);
  shuffler->start();
}
