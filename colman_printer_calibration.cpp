// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of printer calibration persistence.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_printer_calibration.hpp"

#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif

namespace
{

/// Returns the QSettings group that stores @p printer_name's calibration.
  QString settings_group(QString const& printer_name)
  {
    return "printer_calibration/" + printer_name;
  }

} // anonymous namespace


colman::PrinterCalibration
colman::load_printer_calibration(QString const& printer_name)
{
  QSettings settings;
  settings.beginGroup(settings_group(printer_name));
  PrinterCalibration cal;
  cal.offset_x = settings.value("offset_x", cal.offset_x).toDouble();
  cal.offset_y = settings.value("offset_y", cal.offset_y).toDouble();
  cal.scale_x = settings.value("scale_x", cal.scale_x).toDouble();
  cal.scale_y = settings.value("scale_y", cal.scale_y).toDouble();
  settings.endGroup();
  return cal;
}


void colman::save_printer_calibration(QString const& printer_name,
                                      PrinterCalibration const& cal)
{
  QSettings settings;
  settings.beginGroup(settings_group(printer_name));
  settings.setValue("offset_x", cal.offset_x);
  settings.setValue("offset_y", cal.offset_y);
  settings.setValue("scale_x", cal.scale_x);
  settings.setValue("scale_y", cal.scale_y);
  settings.endGroup();
}

/* EOF */
