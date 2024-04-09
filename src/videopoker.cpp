#include "videopoker.h"
#include "./ui_videopoker.h"
#include "card.h"
#include "displayCard.h"
#include "shuffle.h"
#include <QDebug>
#include <QLabel>
#include <QSvgWidget>
#include <map>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qsvgwidget.h>

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
    deck.emplace_back(suit, std::nullopt, 11);
  }

  return deck;
}

QString getSuitString(Card card)
{
  switch (card.getSuit())
  {
  case Suit::Clubs:
    return "clubs";
  case Suit::Diamonds:
    return "diamonds";
  case Suit::Hearts:
    return "hearts";
  case Suit::Spades:
    return "spades";
  case Suit::EMPTY:
    // this is really bad?
    return NULL;
  }
}

QString getFaceString(Card card)
{
  if (card.getFace())
  {
    if (card.getValue() == 11)
      return "ace";
    else
      return QString::number(card.getValue());
  }
  else
  {
    switch (card.getFace().value())
    {
    case Face::Jack:
      return "jack";
    case Face::Queen:
      return "queen";
      break;
    case Face::King:
      return "king";
    }
  }
}
} // namespace

VideoPoker::VideoPoker(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::VideoPoker),
      gameType(Games::JACKS_OR_BETTER), shuffler(new QThread()) 
{
  // allow for passing back from the shuffling thread
  qRegisterMetaType<std::vector<std::vector<Card>>>(
      "std::vector<std::vector<Card>>");

  decks.push_back(createStandardDeck());
  task = new Shuffle(decks);
  QObject::connect(task, &Shuffle::finishedShuffling, this,
                   &VideoPoker::finishedShuffling);
  QObject::connect(task, &Shuffle::error, this,
                   &VideoPoker::shuffleError);
  QObject::connect(shuffler, &QThread::started, task, &Shuffle::run);
  QObject::connect(task, &Shuffle::finishedShuffling, shuffler,
                   &QThread::quit);
  // QObject::connect(shuffleTask, &Shuffle::finishedShuffling, shuffleTask,
  //                  &Shuffle::deleteLater);
  // QObject::connect(shuffler, &QThread::finished, shuffler,
  //                  &QThread::deleteLater);
  task->moveToThread(shuffler);
  shuffler->start();
  ui->setupUi(this);

  Q_INIT_RESOURCE(images);
  QVBoxLayout *vbox = new QVBoxLayout(this->centralWidget());
  handBox = new QGridLayout();
  vbox->addLayout(handBox);
  for (int i = 0; i < 5; ++i)
  {
    auto *card = new DisplayCard();
    connect(card, &DisplayCard::cardSelectedEnable, this,
            &VideoPoker::cardSelectedEnable);
    connect(card, &DisplayCard::cardSelectedDisable, this,
            &VideoPoker::cardSelectedDisable);
    card->setMaximumSize(QSize(400, 400));
    card->load(QString(":/assets/back.svg"));
    handBox->addWidget(card, 1, i);
    hand.at(i) = card;
    card->setPos(1, i);
  }

  playButton = new QPushButton(QString("Deal"));
  connect(playButton, &QAbstractButton::clicked, this,
          &VideoPoker::dealButtonClicked);
  vbox->addWidget(playButton);
}

VideoPoker::~VideoPoker()
{
  delete ui;
  delete shuffler;
}

void VideoPoker::dealButtonClicked()
{
  playButton->setDisabled(true);
  if (playButton->text() == QString("Deal"))
  {
    toggleHand(false);
    clearKeepLabels();
    qDebug() << "Requested shuffle interruption";
    shuffler->requestInterruption();
  }
  else if (playButton->text() == QString("Draw"))
  {
    toggleHand(true);
    draw();
    checkHand();

    for (unsigned char i = 0; i < discardedCards.size(); ++i)
      decks.at(0).push_back(std::move(discardedCards.at(i)));
    discardedCards.clear();
    for (unsigned char i = 0; i < hand.size(); ++i)
    {
      decks.at(0).push_back(std::move(hand.at(i)->getCard()));
      if (hand.at(i)->isSelected())
        hand.at(i)->select();
    }

    shuffler->start();
    playButton->setText(QString("Deal"));
    playButton->setEnabled(true);
  }
}

void VideoPoker::finishedShuffling(std::vector<std::vector<Card>> decks)
{
  this->decks = decks;
  qDebug() << "Received decks";
  deal();
  playButton->setText(QString("Draw"));
  playButton->setEnabled(true);
}

void VideoPoker::deal()
{
  for (unsigned char i = 0; i < hand.size(); ++i)
    hand.at(i)->load(QString(":/assets/back.svg"));
  pullCards();
}

void VideoPoker::draw() { pullCards(); }

void VideoPoker::clearKeepLabels()
{
  for (unsigned char i = 0; i < 5; ++i)
  {
    auto *layoutItem = handBox->itemAtPosition(0, i);
    if (layoutItem == nullptr)
      continue;
    delete layoutItem->widget();
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

void VideoPoker::pullCards()
{
  for (unsigned char i = 0; i < 5; ++i)
  {
    // keep selected cards
    if (hand.at(i)->isSelected())
      continue;
    // this card has been dealt and is being discarded for the draw action
    if (hand.at(i)->isInitialized())
      discardedCards.push_back(std::move(hand.at(i)->getCard()));

    hand.at(i)->setCard(decks.at(0).back());
    decks.at(0).pop_back();

    QString card = getFaceString(hand.at(i)->getCard());
    QString suit = getSuitString(hand.at(i)->getCard());
    QString path(":/assets/" + suit + "_" + card + ".svg");
    static_cast<DisplayCard *>(handBox->itemAt(i)->widget())->load(path);
  }
}

void VideoPoker::toggleHand(bool disable)
{
  for (unsigned char i = 0; i < hand.size(); ++i)
    hand.at(i)->setDisabled(disable);
}

void VideoPoker::checkHand()
{
  unsigned char flush;
  Suit currentSuit = Suit::EMPTY;
  std::map<QString, unsigned char> count;
  Card lowCard(Face::Ace), highCard(Face::Two);
  for (auto *displayCard : hand)
  {
    auto currentCard = displayCard->getCard();
    if (currentCard.getFace() > highCard)
      highCard = *currentCard.getFace();

    if (currentCard.getFace() < lowCard)
      lowCard = *currentCard.getFace();

    if (currentCard.getSuit() == currentSuit)
      flush++;
    if (currentSuit == Suit::EMPTY)
      currentSuit = currentCard.getSuit();
    QString qualifiedCard(getFaceString(currentCard) +
                          getSuitString(currentCard));

    if (auto it = count.find(qualifiedCard); it != count.end())
      count.insert({qualifiedCard, static_cast<unsigned char>(1)});
    else
      *it++;
  }

  bool hasFlush = false;
  if (flush == 5)
    hasFlush = true;

  bool pair = false, threeOfAKind = false, fourOfAKind = false;
  for (auto it = count.begin(); it != count.end(); ++it)
  {
    if (it->second == 2)
      pair = true;
    else if (it->second == 3)
      threeOfAKind = true;
    else if (it->second == 4)
      fourOfAKind = true;
  }

  bool fullHouse = false;
  if (pair && threeOfAKind)
    fullHouse = true;
}
