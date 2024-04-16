#ifndef SHUFFLE_H
#define SHUFFLE_H

#include "card.h"
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

class Shuffle : public QObject
{
  Q_OBJECT
  // keep a local copy of decks to not worry about reinserting drawn cards
  std::vector<std::vector<Card>> decks;
  std::random_device r;
  std::mt19937 mt;
  QMutex mutex;

public:
  void run()
  {
    QMutexLocker locker(&mutex);
    bool end = false;
    while (!end)
    {
      if (QThread::currentThread()->isInterruptionRequested())
        end = true;

      for (int i = 0; i < decks.size(); ++i)
        std::shuffle(decks.at(i).begin(), decks.at(i).end(), mt);
    }
    emit finishedShuffling(decks);
  }

  Shuffle(std::vector<std::vector<Card>> decks)
      : decks(decks),
        mt(r() ^
           std::chrono::high_resolution_clock::now().time_since_epoch().count())
  {
  }
public slots:
  void deleteLater() {}
signals:
  void error(QString error);
  void finishedShuffling(std::vector<std::vector<Card>> decks);
};

#endif // SHUFFLE_H
