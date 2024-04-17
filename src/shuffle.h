#ifndef SHUFFLE_H
#define SHUFFLE_H

#include "card.h"
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <algorithm>
#include <random>
#include <vector>

class Shuffle : public QObject
{
  Q_OBJECT
  // keep a local copy of decks to not worry about reinserting drawn cards
  std::shared_ptr<std::vector<std::vector<Card>>> decks;
  QMutex mutex;
  std::shared_ptr<std::mt19937> mt;

public:
  void run()
  {
    QMutexLocker locker(&mutex);
    bool end = false;
    qDebug() << "Beginning a new shuffle run\n";
    while (!end)
    {
      if (QThread::currentThread()->isInterruptionRequested())
        end = true;

      for (int i = 0; i < decks->size(); ++i)
      {
        // if (decks->at(i).size() != 52)
        //   throw std::length_error("deck size is not 52");
        std::shuffle(decks->at(i).begin(), decks->at(i).end(), *mt);
      }
    }
    emit finishedShuffling();
  }

  Shuffle(std::shared_ptr<std::vector<std::vector<Card>>> decks,
          std::shared_ptr<std::mt19937> mt)
      : decks(decks), mt(mt)
  {
  }
public slots:
  void deleteLater() {}
signals:
  void error(QString error);
  void finishedShuffling();
};

#endif // SHUFFLE_H
