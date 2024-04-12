#include "card.h"
#include <type_traits>

Card::Card(Suit suit, std::optional<Face> face, unsigned char value)
    : suit(suit), face(face), value(value)
{
  if (face == std::nullopt)
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
}

Card::Card(Face face) : face(face), suit(Suit::EMPTY) {}

unsigned char Card::getValue()
{
  return static_cast<std::underlying_type<Face>::type>(getFace().value());
}

Suit Card::getSuit() { return suit; }

std::optional<Face> Card::getFace() { return face; }

bool Card::operator<(const Card &otherCard)
{
  if (face < otherCard.face)
    return true;
  return false;
}

bool Card::operator>(const Card &otherCard)
{
  if (face > otherCard.face)
    return true;
  return false;
}

bool Card::operator==(const Card &otherCard)
{
  if (face == otherCard.face)
    return true;
  return false;
}

unsigned char Card::operator-(Card &otherCard)
{
  return static_cast<typename std::underlying_type<Face>::type>(face.value()) -
         static_cast<typename std::underlying_type<Face>::type>(
             otherCard.face.value());
}

unsigned char Card::operator+(unsigned char operand)
{
  return static_cast<typename std::underlying_type<Face>::type>(face.value()) +
         operand;
}
