#ifndef ALBUMMANAGERDIALOG_H
#define ALBUMMANAGERDIALOG_H

#include "abstractdialog.h"
#include "ui_albummanagerdialog.h"

namespace Ms {

class AlbumManagerDialog : public QDialog, public Ui::AlbumManagerDialog
      {
      Q_OBJECT

   private:
      AlbumManagerDialog *ui;
      void apply();
      void update();

   private slots:
      void buttonBoxClicked(QAbstractButton*);

   public:
      AlbumManagerDialog(QWidget *parent = nullptr);
      ~AlbumManagerDialog();
      void start();
      };

}

#endif // ALBUMMANAGERDIALOG_H
