#include "card.h"

Card::Card(Suit suit, std::optional<Face> face, unsigned char value)
    : suit(suit), face(face), value(value)
{
}

unsigned char Card::getValue() { return value; }

Suit Card::getSuit() { return suit; }

std::optional<Face> Card::getFace() { return face; }
