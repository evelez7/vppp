#include "card.h"

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

unsigned char Card::getValue() { return value; }

Suit Card::getSuit() { return suit; }

std::optional<Face> Card::getFace() { return face; }
