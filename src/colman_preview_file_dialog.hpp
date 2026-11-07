// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PreviewFileDialog.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PREVIEW_FILE_DIALOG_HPP
#  define COLMAN_PREVIEW_FILE_DIALOG_HPP

#  ifndef SYSTEM_QFILEDIALOG
#    include <QFileDialog>
#    define SYSTEM_QFILEDIALOG
#  endif
#  ifndef SYSTEM_QLABEL
#    include <QLabel>
#    define SYSTEM_QLABEL
#  endif

namespace colman
{
  /// File open dialog with an image preview panel.
  /**
     Extends QFileDialog by adding a preview label on the right side
     that shows a thumbnail of the currently highlighted image file.
  */
  class PreviewFileDialog : public QFileDialog
  {
    Q_OBJECT
  public:
    /// Constructs the dialog.
    /**
       @param parent parent widget
       @param caption dialog window title
       @param directory initial directory shown in the dialog
       @param filter file name filter string
    */
    explicit PreviewFileDialog(QWidget* parent = nullptr,
                               QString const& caption = QString(),
                               QString const& directory = QString(),
                               QString const& filter = QString());
  private Q_SLOTS:
    void show_image(QString const& path);

  private:
    QLabel* preview_ = nullptr; ///< label used to display the image preview
  };

} // namespace colman

#endif /* #ifndef COLMAN_PREVIEW_FILE_DIALOG_HPP */

/* EOF */
