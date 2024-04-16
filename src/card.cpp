#include "card.h"
#include <utility>

Card::Card(Suit suit, std::optional<Face> optFace, unsigned char value)
    : suit(suit), value(value)
{
  if (optFace == std::nullopt)
    switch (value)
    {
    case 2:
      face = Face::Two;
      break;
    case 3:
      face = Face::Three;
      break;
    case 4:
      face = Face::Four;
      break;
    case 5:
      face = Face::Five;
      break;
    case 6:
      face = Face::Six;
      break;
    case 7:
      face = Face::Seven;
      break;
    case 8:
      face = Face::Eight;
      break;
    case 9:
      face = Face::Nine;
      break;
    case 10:
      face = Face::Ten;
      break;
    }
  else
    face = std::move(optFace.value());
}

Card::Card(Face face) : face(face), suit(Suit::EMPTY) {}

unsigned char Card::getValue() { return value; }

Suit Card::getSuit() { return suit; }

Face Card::getFace() { return face; }

bool Card::operator<(const Card &otherCard)
{
  if (std::to_underlying(face) < std::to_underlying(otherCard.face))
    return true;
  return false;
}

bool Card::operator>(const Card &otherCard)
{
  if (std::to_underlying(face) > std::to_underlying(otherCard.face))
    return true;
  return false;
}

bool Card::operator==(const Card &otherCard) const
{
  if (std::to_underlying(face) == std::to_underlying(otherCard.face))
    return true;
  return false;
}

unsigned char Card::operator-(Card &otherCard)
{
  return std::to_underlying(face) - std::to_underlying(otherCard.face);
}

unsigned char Card::operator+(unsigned char operand)
{
  return std::to_underlying(face) + operand;
}
