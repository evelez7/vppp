#ifndef SHUFFLE_H
#define SHUFFLE_H

#include "card.h"
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

class Shuffle : public QObject
{
  Q_OBJECT
  std::vector<std::vector<Card>> decks;
  QMutex mut;

public:
  void run()
  {
    std::random_device r;
    std::mt19937 mt(r());
    bool end = false;
    while (!end)
    {
      if (QThread::currentThread()->isInterruptionRequested())
        end = true;

      for (int i = 0; i < decks.size(); ++i)
        std::shuffle(decks.at(i).begin(), decks.at(i).end(), mt);
    }
    emit finishedShuffling(std::move(decks));
  }

  Shuffle(std::vector<std::vector<Card>> decks) : decks(decks) {}
public slots:
  void deleteLater() {}
signals:
  void error(QString error);
  void finishedShuffling(std::vector<std::vector<Card>> decks);
};

#endif // SHUFFLE_H
