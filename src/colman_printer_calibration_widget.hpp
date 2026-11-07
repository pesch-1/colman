// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of PrinterCalibration and PrinterCalibrationWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PRINTER_CALIBRATION_WIDGET_HPP
#  define COLMAN_PRINTER_CALIBRATION_WIDGET_HPP

#  ifndef COLMAN_PAGE_LAYOUT_HPP
#    include "colman_page_layout.hpp"
#  endif
#  ifndef COLMAN_PRINTER_CALIBRATION_HPP
#    include "colman_printer_calibration.hpp"
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

class QComboBox;
class QDoubleSpinBox;
class QGraphicsScene;
class QGraphicsView;

namespace colman
{

  /// Widget for calibrating print position and scale for a specific printer.
  /**
     Prints a calibration sheet with three reference crosses at fixed,
     measurable positions on plain A4 paper:

     - **A** at (50 mm | 50 mm) — anchor / reference point
     - **B** at (150 mm | 50 mm) — 100 mm to the right of A
     - **C** at (50 mm | 150 mm) — 100 mm below A

     The user measures all three cross centres with a ruler and enters
     the values.  Clicking <b>Versatz berechnen</b> computes both the
     translation offset and the per-axis scale correction, which are
     then saved to QSettings under
     @c "printer_calibration/<printer_name>/".

     @par Calibration procedure
     -# Select the printer in the combo box.
     -# Click <b>Kalibrierblatt drucken</b> (plain paper).
     -# Measure the X and Y distances from the paper edge to cross A,
        the X distance to cross B, and the Y distance to cross C.
     -# Enter the four values in step 2 and click
        <b>Versatz berechnen</b>.
     -# Verify the computed values in step 3 and click
        <b>Speichern</b>.
  */
  class PrinterCalibrationWidget : public QWidget
  {
    Q_OBJECT

  public:
    /// Constructs the calibration widget.
    /**
       @param layout  Label sheet geometry used for the background grid
                      in the preview.  The caller must ensure the object
                      outlives this widget.
       @param parent  Optional parent widget.
    */
    explicit PrinterCalibrationWidget(colman::PageLayout const& layout,
                                      QWidget* parent = nullptr);

  Q_SIGNALS:
    /// Emitted after the user saves a new calibration.
    void calibration_saved();

  protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private Q_SLOTS:
    void on_printer_changed(int index);
    void on_print_calibration_sheet();
    void on_compute_offset();
    void on_save();
    void on_reset();
    void update_preview();

  private:
    void load_from_settings();

    colman::PageLayout const& layout_;  ///< label layout for preview

    QComboBox* printer_combo_ = nullptr;  ///< available printers
    // step 2 — measured positions (mm from paper edge):
    QDoubleSpinBox* meas_a_x_spin_ = nullptr;  ///< measured X of cross A
    QDoubleSpinBox* meas_a_y_spin_ = nullptr;  ///< measured Y of cross A
    QDoubleSpinBox* meas_b_x_spin_ = nullptr;  ///< measured X of cross B
    QDoubleSpinBox* meas_c_y_spin_ = nullptr;  ///< measured Y of cross C
    // step 3 — computed / stored calibration:
    QDoubleSpinBox* offset_x_spin_ = nullptr;  ///< x offset in mm
    QDoubleSpinBox* offset_y_spin_ = nullptr;  ///< y offset in mm
    QDoubleSpinBox* scale_x_spin_ = nullptr;   ///< x scale correction
    QDoubleSpinBox* scale_y_spin_ = nullptr;   ///< y scale correction
    // preview:
    QGraphicsView* preview_view_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_PRINTER_CALIBRATION_WIDGET_HPP */

/* EOF */
