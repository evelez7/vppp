#ifndef EV_CARD_HPP
#define EV_CARD_HPP
#include <QSvgWidget>
#include <optional>
#include <string>
enum Face
{
  Jack,
  Queen,
  King
};

enum class Suit
{
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
  unsigned char getValue();
  Suit getSuit();
  std::optional<Face> getFace();
};
#endif // !EV_CARD_HPP
