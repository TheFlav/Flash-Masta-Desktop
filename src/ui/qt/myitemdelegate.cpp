#include "myitemdelegate.h"

MyItemDelegate::MyItemDelegate(MainWindow* mainWindow) : QItemDelegate(mainWindow)
{
  // Nothing else to do
}

void MyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const qModelIndex &index) const
{
  if (index.column() == 0)
  {
    QStyleOptionViewItem myOption = option;
    myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    myOption.rect.setHeight(64);
    
    QString text = index.model()->data(index, Qt::DisplayRole);
    
    drawDisplay(painter, myOption, myOption.rect, text);
    drawFocus(painter, myOption, myOption.rect);
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}


