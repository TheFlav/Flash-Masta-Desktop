#ifndef NGP_LINKMASTA_DETAIL_WIDGET_H
#define NGP_LINKMASTA_DETAIL_WIDGET_H

#include <QWidget>

namespace Ui {
class NgpLinkmastaDetailWidget;
}

class NgpLinkmastaDetailWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpLinkmastaDetailWidget(QWidget *parent = 0);
  ~NgpLinkmastaDetailWidget();
  
private:
  Ui::NgpLinkmastaDetailWidget *ui;
};

#endif // NGP_LINKMASTA_DETAIL_WIDGET_H
