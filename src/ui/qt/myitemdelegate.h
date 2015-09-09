#ifndef MYITEMDELEGATE_H
#define MYITEMDELEGATE_H

#include <QItemDelegate>


class MyItemDelegate : public QItemDelegate
{
  Q_OBJECT
  
public:
  MyItemDelegate();
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const qModelIndex &index) const;
};

#endif // MYITEMDELEGATE_H
