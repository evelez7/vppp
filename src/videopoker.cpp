#include "videopoker.h"
#include "./ui_videopoker.h"
#include "card.h"
#include "displayCard.h"
#include "shuffle.h"
#include <QDebug>
#include <QLabel>
#include <QSvgWidget>
#include <algorithm>
#include <array>
#include <map>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qsvgwidget.h>
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

QString getSuitString(Suit suit)
{
  switch (suit)
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

QString getFaceString(Face face)
{
  switch (face)
  {
  case Face::Ace:
    return QString("ace");
  case Face::King:
    return QString("king");
  case Face::Queen:
    return QString("queen");
  case Face::Jack:
    return QString("jack");
  case Face::Two:
  case Face::Three:
  case Face::Four:
  case Face::Five:
  case Face::Six:
  case Face::Seven:
  case Face::Eight:
  case Face::Nine:
  case Face::Ten:
    return QString(QString::number(std::to_underlying(face)));
  }
}
} // namespace

VideoPoker::VideoPoker(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::VideoPoker),
      gameType(Games::JACKS_OR_BETTER), shuffler(new QThread()),
      handLabel(new QLabel()) {
  // allow for passing back from the shuffling thread
  qRegisterMetaType<std::vector<std::vector<Card>>>(
      "std::vector<std::vector<Card>>");

  decks.push_back(createStandardDeck());
  task = new Shuffle(decks);
  QObject::connect(task, &Shuffle::finishedShuffling, this,
                   &VideoPoker::finishedShuffling);
  QObject::connect(task, &Shuffle::error, this, &VideoPoker::shuffleError);
  QObject::connect(shuffler, &QThread::started, task, &Shuffle::run);
  QObject::connect(task, &Shuffle::finishedShuffling, shuffler, &QThread::quit);
  // QObject::connect(shuffleTask, &Shuffle::finishedShuffling, shuffleTask,
  //                  &Shuffle::deleteLater);
  // QObject::connect(shuffler, &QThread::finished, shuffler,
  //                  &QThread::deleteLater);
  task->moveToThread(shuffler);
  shuffler->start();
  ui->setupUi(this);

  Q_INIT_RESOURCE(images);
  mainLayout = new QVBoxLayout(this->centralWidget());
  mainLayout->addWidget(handLabel);
  handBox = new QGridLayout();
  mainLayout->addLayout(handBox);
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
  mainLayout->addWidget(playButton);
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
  addHandLabel(checkHand());
}

void VideoPoker::draw()
{
  pullCards();
  addHandLabel(checkHand());
}

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

    QString card = getFaceString(hand.at(i)->getCard().getFace());
    QString suit = getSuitString(hand.at(i)->getCard().getSuit());
    QString path(":/assets/" + suit + "_" + card + ".svg");
    static_cast<DisplayCard *>(handBox->itemAt(i)->widget())->load(path);
  }
}

void VideoPoker::toggleHand(bool disable)
{
  for (unsigned char i = 0; i < hand.size(); ++i)
    hand.at(i)->setDisabled(disable);
}

Hand VideoPoker::checkHand()
{
  unsigned char flush;
  Suit currentSuit = Suit::EMPTY;
  std::map<QString, unsigned char> count;
  Card lowCard(Face::Ace), highCard(Face::Two);
  std::vector<Card> handCards;
  handCards.reserve(5);
  for (DisplayCard *displayCard : hand)
  {
    auto currentCard = displayCard->getCard();
    handCards.push_back(currentCard);
    if (currentCard > highCard)
      highCard = currentCard.getFace();

    if (currentCard < lowCard)
      lowCard = currentCard.getFace();

    if (currentSuit == Suit::EMPTY)
      currentSuit = currentCard.getSuit();
    else if (currentCard.getSuit() == currentSuit)
      flush++;
    QString qualifiedCard(getFaceString(currentCard.getFace()) +
                          getSuitString(currentCard.getSuit()));

    auto it = count.find(qualifiedCard);
    if (it == count.end())
      count.insert({qualifiedCard, static_cast<unsigned char>(1)});
    else
      count.at(qualifiedCard)++;
  }

  bool straight = true;
  if (highCard - lowCard == 4)
  {
    std::sort(handCards.begin(), handCards.end(),
              [](Card a, Card b) { return a < b; });
    for (unsigned char i = 1; i < handCards.size(); ++i)
      if (handCards.at(i - 1) + 1 != handCards.at(i).getValue())
      {
        straight = false;
        break;
      }
  } else
    straight = false;

  bool hasFlush = false;
  if (flush == 5)
    hasFlush = true;

  if (straight && hasFlush)
  {
    if (lowCard == Card(Face::Ten) && highCard == Card(Face::Ace))
      return Hand::RoyalFlush;
    return Hand::StraightFlush;
  }
  else if (straight)
    return Hand::Straight;
  else if (hasFlush)
    return Hand::Flush;

  bool pair = false, twoPair = false, threeOfAKind = false, fourOfAKind = false;
  for (auto it = count.begin(); it != count.end(); ++it)
  {
    if (it->second == static_cast<unsigned char>(2)) {
      if (pair)
        twoPair = true;
      else
        pair = true;
    } else if (it->second == static_cast<unsigned char>(3))
      threeOfAKind = true;
    else if (it->second == static_cast<unsigned char>(4))
      return Hand::FourOfAKind;
  }

  bool fullHouse = false;
  if (pair && threeOfAKind)
    return Hand::FullHouse;
  else if (threeOfAKind)
    return Hand::ThreeOfAKind;
  else if (twoPair)
    return Hand::TwoPair;
  else if (pair)
    return Hand::Pair;
  return Hand::HighCard;
}

void VideoPoker::addHandLabel(Hand hand) {
  QString handString;
  switch (hand) {
  case Hand::Pair:
    handString = "Pair";
    break;
  case Hand::TwoPair:
    handString = "Two Pair";
    break;
  case Hand::Flush:
    handString = "Flush";
    break;
  case Hand::FullHouse:
    handString = "Full House";
    break;
  case Hand::FourOfAKind:
    handString = "Four of a Kind";
    break;
  case Hand::HighCard:
    handString = "High Card";
    break;
  case Hand::Straight:
    handString = "Straight";
    break;
  case Hand::StraightFlush:
    handString = "Straight Flush";
    break;
  case Hand::ThreeOfAKind:
    handString = "Three of a Kind";
    break;
  case Hand::RoyalFlush:
    handString = "Royal Flush";
    break;
  }
  handLabel->setText(handString);
}
