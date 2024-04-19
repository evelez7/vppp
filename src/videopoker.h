#ifndef VIDEOPOKER_H
#define VIDEOPOKER_H

#include "displayCard.h"
#include "game.h"
#include "hand.h"
#include "vppp.h"
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <map>

class VideoPoker : public Game
{
  Q_OBJECT
  const VPPP *vppp;
  // the vertical layout that keeps everything
  QVBoxLayout *mainLayout;

  // the grid that holds the hand and "Keep" text
  QGridLayout *handBox;

  // the button that controls deal and draw phasses
  QPushButton *playButton;

  // The SVG entities that also hold card info
  std::array<DisplayCard *, 5> hand;

  std::map<Hand, unsigned> paytable;

  // The label for the best hand present
  QLabel *handLabel;

  // Check for the existing winning hand
  Hand checkHand();

  // Add the label of the best hand present
  void addHandLabel(Hand hand);

  // Toggle the hand to be interactive or not
  // Important to not allow selection while waiting for the draw
  void toggleHand(bool disable);

  void dealButtonClicked();

  void clearKeepLabels();

  virtual void play();

public:
  VideoPoker(const VPPP *vppp);
  ~VideoPoker();

public slots:
  void cardSelectedEnable(int row, int col);
  void cardSelectedDisable(int row, int col);
};

#endif
