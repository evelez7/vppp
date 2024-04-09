#ifndef EV_CARD_HPP
#define EV_CARD_HPP
#include <QSvgWidget>
#include <optional>

enum class Face : unsigned char
{
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Jack = 11,
  Queen = 12,
  King = 13,
  Ace = 14
};

enum class Suit
{
  EMPTY,
  Clubs = 1,
  Diamonds = 2,
  Hearts = 3,
  Spades = 4
};

class Card
{
  Suit suit;
  std::optional<Face> face;
  unsigned char value;

public:
  Card() = default;
  Card(Suit suit, std::optional<Face> face, unsigned char value);
  Card(Face face);
  unsigned char getValue();
  Suit getSuit();
  std::optional<Face> getFace();
};
#endif // !EV_CARD_HPP
