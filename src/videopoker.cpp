#include "videopoker.h"
#include <QDebug>
#include <QObject>

namespace
{
std::map<Hand, unsigned> createPaytable()
{
  std::map<Hand, unsigned> paytable;
  paytable.insert({Hand::Pair, 1});
  paytable.insert({Hand::TwoPair, 2});
  paytable.insert({Hand::ThreeOfAKind, 3});
  paytable.insert({Hand::Straight, 4});
  paytable.insert({Hand::Flush, 6});
  paytable.insert({Hand::FullHouse, 9});
  paytable.insert({Hand::FourOfAKind, 25});
  paytable.insert({Hand::StraightFlush, 90});
  paytable.insert({Hand::RoyalFlush, 800});
  return paytable;
}
} // namespace

VideoPoker::VideoPoker(const VPPP *vppp) : vppp(vppp)
{
  mainLayout = new QVBoxLayout(vppp->centralWidget());
  mainLayout->addWidget(handLabel);
  handBox = new QGridLayout();
  mainLayout->addLayout(handBox);
  for (int i = 0; i < 5; ++i)
  {
    auto *card = new DisplayCard();
    QObject::connect(card, &DisplayCard::cardSelectedEnable, this,
                     &VideoPoker::cardSelectedEnable);
    QObject::connect(card, &DisplayCard::cardSelectedDisable, this,
                     &VideoPoker::cardSelectedDisable);
    card->setMaximumSize(QSize(400, 400));
    card->load(QString(":/assets/back.svg"));
    handBox->addWidget(card, 1, i);
    hand.at(i) = card;
    card->setPos(1, i);
  }
  playButton = new QPushButton(QString("Deal"));
  QObject::connect(playButton, &QAbstractButton::clicked, this,
                   &VideoPoker::dealButtonClicked);
  mainLayout->addWidget(playButton);
}

void VideoPoker::dealButtonClicked()
{
  playButton->setDisabled(true);
  if (playButton->text() == QString("Deal"))
  {
    // TODO: add GUI elements for the paytable, bet, adjust balance
    // TODO: abstract the game so that it can also play joker poker
    // deuces wild is just an adjustment to the paytable/check hand
    toggleHand(false);
    clearKeepLabels();
    vppp->interruptShuffling();
    qDebug() << "Requested shuffle interruption after deal";
  }
  else if (playButton->text() == QString("Draw"))
  {
    toggleHand(true);
    vppp->interruptShuffling();
    qDebug() << "Requested shuffle interruption after draw";
  }
}

void VideoPoker::play()
{
  if (playButton->text() == QString("Deal"))
  {
    qDebug() << "Dealing cards";
    auto pulled = vppp->pullCards(5);
    // for (unsigned char i = 0; i < 5; ++i)
    // TODO: move these card string functions to card.h as static?
    // TODO: Set card should load these cards?
    // hand.at(i)->load(QString(":/assets/" +
    //                          getSuitString(pulled.at(i).getSuit()) +
    //                          getFaceString(pulled.at(i).getFace()) +
    //                          ".svg"));

    vppp->startShuffling();
    playButton->setText(QString("Draw"));
    playButton->setEnabled(true);
  }
  else if (playButton->text() == QString("Draw"))
  {
    qDebug() << "Drawing cards";
    for (auto *card : hand)
    {
      if (!card->isSelected())
        continue;
      card->setCard(vppp->pullCard());
    }
    vppp->startShuffling();
    playButton->setText(QString("Deal"));
    playButton->setEnabled(true);
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

void VideoPoker::toggleHand(bool disable)
{
  for (unsigned char i = 0; i < hand.size(); ++i)
    hand.at(i)->setDisabled(disable);
}

void VideoPoker::addHandLabel(Hand hand)
{
  // TODO: Abstract this to get strings from the enum somewhere else
  QString handString;
  switch (hand)
  {
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

Hand VideoPoker::checkHand()
{
  // TODO: Abstract this and put in VPPP to check hands for hold em etc
  unsigned char flush = 0;
  Suit currentSuit = Suit::EMPTY;
  std::map<std::string, unsigned char> count;
  Card lowCard(Face::Ace), highCard(Face::Two);
  std::vector<Card> handCards;
  handCards.reserve(5);
  // keep track of the face that makes a pair, could be the winning pair or not
  Face pairFace;
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

    std::string qualifiedCard(Card::getFaceString(currentCard.getFace()));
    try
    {
      auto &existingCard = count.at(qualifiedCard);
      existingCard++;
      if (existingCard == 2)
        pairFace = currentCard.getFace();
    }
    catch (std::out_of_range &e)
    {
      count.insert({qualifiedCard, 1});
    }
  }

  bool straight = true;
  if (highCard - lowCard == 4)
  {
    std::sort(handCards.begin(), handCards.end(),
              [](Card a, Card b) { return a < b; });
    for (unsigned char i = 1; i < handCards.size(); ++i)
      if (handCards.at(i - 1).getValue() + 1 != handCards.at(i).getValue())
      {
        straight = false;
        break;
      }
  }
  else
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
  for (const auto &it : count)
  {
    if (it.second == static_cast<unsigned char>(2))
    {
      if (pair)
        twoPair = true;
      else
        pair = true;
    }
    else if (it.second == static_cast<unsigned char>(3))
      threeOfAKind = true;
    else if (it.second == static_cast<unsigned char>(4))
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
  {
    // if the pair is not the winning pair, make it a high card
    if (std::to_underlying(pairFace) < std::to_underlying(Face::Jack))
      return Hand::HighCard;
    else
      return Hand::Pair;
  }
  return Hand::HighCard;
}