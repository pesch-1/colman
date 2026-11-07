// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PreviewFileDialog.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_preview_file_dialog.hpp"
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif

/**
   Constructs a PreviewFileDialog with an image preview panel.
   Adds a preview label to the right side of the standard file dialog layout
   and connects the currentChanged signal to show_image().
   @param parent    parent widget.
   @param caption   dialog title.
   @param directory initial directory.
   @param filter    file name filter string.
*/
colman::PreviewFileDialog::
    PreviewFileDialog(QWidget* parent,
                      QString const& caption,
                      QString const& directory,
                      QString const& filter)
    : QFileDialog(parent, caption, directory, filter)
{
  setObjectName("PreviewFileDialog");
  QVBoxLayout* box = new QVBoxLayout;
  preview_ = new QLabel(tr("Preview"), this);
  preview_->setAlignment(Qt::AlignCenter);
  preview_->setObjectName("labelPreview");
  preview_->setMinimumWidth(128);
  preview_->setMinimumHeight(128);
  box->addWidget(preview_);
  box->addStretch();

  QGridLayout* layout = qobject_cast<QGridLayout*>(this->layout());
  if((layout)) {
    layout->addLayout(box, 1, 3, 3, 1);
  }
  connect(this, &QFileDialog::currentChanged,
          this, &PreviewFileDialog::show_image);
}


/**
   Loads and displays the image at @p path in the preview label.
   Shows "not an image" text if the file cannot be loaded as a pixmap.
   @param path absolute file path selected in the dialog.
*/
void colman::PreviewFileDialog::show_image(QString const& path)
{
  if((preview_)) {
    QPixmap pixmap = QPixmap(path);
    if(pixmap.isNull()) {
      preview_->setText(tr("No preview available"));
    }
    else {
      preview_->setPixmap(pixmap.scaled(preview_->width(), preview_->height(),
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
    }
  }
}
/* EOF */
