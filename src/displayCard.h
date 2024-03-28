#ifndef DISPLAYCARD_H
#define DISPLAYCARD_H

#include "card.h"
#include <QMouseEvent>
#include <QSvgWidget>

class DisplayCard : public QSvgWidget
{
  Q_OBJECT
  Card card;
  bool initialized;
  bool selected;
    int row, col;

public:
  DisplayCard() = default;
  void setCard(Card card) { this->card = card; initialized = true; }
  Card getCard() { return card; }
  void setPos(int x, int y) {
      row = x;
      col = y;
    }
  bool isSelected()
{
      return selected;
    }
  void mousePressEvent(QMouseEvent *event) override
  {
    if (!initialized) return;
    if (selected)
    {
      selected = false;
      this->setStyleSheet("");
      emit clickedCardDisable(row, col);
    }
    else
    {
      selected = true;
      this->setStyleSheet("border: 1000px solid blue");
      emit clickedCardEnable(row, col);
    }
  }

  signals:
    void clickedCardEnable(int, int);
    void clickedCardDisable(int, int);
};
#endif // DISPLAYCARD_H
