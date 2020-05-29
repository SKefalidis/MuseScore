#include "albummanagerdialog.h"
#include "albummanager.h"
#include "ui_albummanagerdialog.h"

namespace Ms {

AlbumManagerDialog::AlbumManagerDialog(QWidget *parent)
   : QDialog(parent)
      {
      setObjectName("AlbumManagerDialog");
      setupUi(this);
      connect(buttonBox, &QDialogButtonBox::clicked, this, &AlbumManagerDialog::buttonBoxClicked);
      }

AlbumManagerDialog::~AlbumManagerDialog()
      {
      delete ui;
      }

void AlbumManagerDialog::start()
      {
      update();
      show();
      }

void AlbumManagerDialog::apply()
      {
      AlbumManager* albumManager = static_cast<AlbumManager*>(parent());
      albumManager->playbackDelay = playbackDelayBox->value();
      }

void AlbumManagerDialog::update()
      {
      AlbumManager* albumManager = static_cast<AlbumManager*>(parent());
      playbackDelayBox->setValue(albumManager->playbackDelay);
      }

void AlbumManagerDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch(buttonBox->standardButton(button)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
                  // intentional ??
                  // fall through
            case QDialogButtonBox::Cancel:
            default:
                  hide();
                  break;
            }
      }

}
