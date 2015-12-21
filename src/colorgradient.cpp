/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2015 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 22.12.15                                             **
**          Version: 1.3.2                                                **
****************************************************************************/

#include "colorgradient.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPColorGradient
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPColorGradient
  \brief Defines a color gradient for use with e.g. \ref QCPColorMap
  
  This class describes a color gradient which can be used to encode data with color. For example,
  QCPColorMap and QCPColorScale have \ref QCPColorMap::setGradient "setGradient" methods which
  take an instance of this class. Colors are set with \ref setColorStopAt(double position, const QColor &color)
  with a \a position from 0 to 1. In between these defined color positions, the
  color will be interpolated linearly either in RGB or HSV space, see \ref setColorInterpolation.

  Alternatively, load one of the preset color gradients shown in the image below, with \ref
  loadPreset, or by directly specifying the preset in the constructor.
  
  \image html QCPColorGradient.png
  
  The fact that the \ref QCPColorGradient(GradientPreset preset) constructor allows directly
  converting a \ref GradientPreset to a QCPColorGradient, you can also directly pass \ref
  GradientPreset to all the \a setGradient methods, e.g.:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolorgradient-setgradient
  
  The total number of levels used in the gradient can be set with \ref setLevelCount. Whether the
  color gradient shall be applied periodically (wrapping around) to data values that lie outside
  the data range specified on the plottable instance can be controlled with \ref setPeriodic.
*/

/*!
  Constructs a new QCPColorGradient initialized with the colors and color interpolation according
  to \a preset.
  
  The color level count is initialized to 350.
*/
QCPColorGradient::QCPColorGradient(GradientPreset preset) :
  mLevelCount(350),
  mColorInterpolation(ciRGB),
  mPeriodic(false),
  mColorBufferInvalidated(true)
{
  mColorBuffer.fill(qRgb(0, 0, 0), mLevelCount);
  loadPreset(preset);
}

/* undocumented operator */
bool QCPColorGradient::operator==(const QCPColorGradient &other) const
{
  return ((other.mLevelCount == this->mLevelCount) &&
          (other.mColorInterpolation == this->mColorInterpolation) &&
          (other.mPeriodic == this->mPeriodic) &&
          (other.mColorStops == this->mColorStops));
}

/*!
  Sets the number of discretization levels of the color gradient to \a n. The default is 350 which
  is typically enough to create a smooth appearance.
  
  \image html QCPColorGradient-levelcount.png
*/
void QCPColorGradient::setLevelCount(int n)
{
  if (n < 2)
  {
    qDebug() << Q_FUNC_INFO << "n must be greater or equal 2 but was" << n;
    n = 2;
  }
  if (n != mLevelCount)
  {
    mLevelCount = n;
    mColorBufferInvalidated = true;
  }
}

/*!
  Sets at which positions from 0 to 1 which color shall occur. The positions are the keys, the
  colors are the values of the passed QMap \a colorStops. In between these color stops, the color
  is interpolated according to \ref setColorInterpolation.
  
  A more convenient way to create a custom gradient may be to clear all color stops with \ref
  clearColorStops and then adding them one by one with \ref setColorStopAt.
  
  \see clearColorStops
*/
void QCPColorGradient::setColorStops(const QMap<double, QColor> &colorStops)
{
  mColorStops = colorStops;
  mColorBufferInvalidated = true;
}

/*!
  Sets the \a color the gradient will have at the specified \a position (from 0 to 1). In between
  these color stops, the color is interpolated according to \ref setColorInterpolation.
  
  \see setColorStops, clearColorStops
*/
void QCPColorGradient::setColorStopAt(double position, const QColor &color)
{
  mColorStops.insert(position, color);
  mColorBufferInvalidated = true;
}

/*!
  Sets whether the colors in between the configured color stops (see \ref setColorStopAt) shall be
  interpolated linearly in RGB or in HSV color space.
  
  For example, a sweep in RGB space from red to green will have a muddy brown intermediate color,
  whereas in HSV space the intermediate color is yellow.
*/
void QCPColorGradient::setColorInterpolation(QCPColorGradient::ColorInterpolation interpolation)
{
  if (interpolation != mColorInterpolation)
  {
    mColorInterpolation = interpolation;
    mColorBufferInvalidated = true;
  }
}

/*!
  Sets whether data points that are outside the configured data range (e.g. \ref
  QCPColorMap::setDataRange) are colored by periodically repeating the color gradient or whether
  they all have the same color, corresponding to the respective gradient boundary color.
  
  \image html QCPColorGradient-periodic.png
  
  As shown in the image above, gradients that have the same start and end color are especially
  suitable for a periodic gradient mapping, since they produce smooth color transitions throughout
  the color map. A preset that has this property is \ref gpHues.
  
  In practice, using periodic color gradients makes sense when the data corresponds to a periodic
  dimension, such as an angle or a phase. If this is not the case, the color encoding might become
  ambiguous, because multiple different data values are shown as the same color.
*/
void QCPColorGradient::setPeriodic(bool enabled)
{
  mPeriodic = enabled;
}

/*!
  This method is used to quickly convert a \a data array to colors. The colors will be output in
  the array \a scanLine. Both \a data and \a scanLine must have the length \a n when passed to this
  function. The data range that shall be used for mapping the data value to the gradient is passed
  in \a range. \a logarithmic indicates whether the data values shall be mapped to colors
  logarithmically.
  
  if \a data actually contains 2D-data linearized via <tt>[row*columnCount + column]</tt>, you can
  set \a dataIndexFactor to <tt>columnCount</tt> to convert a column instead of a row of the data
  array, in \a scanLine. \a scanLine will remain a regular (1D) array. This works because \a data
  is addressed <tt>data[i*dataIndexFactor]</tt>.
*/
void QCPColorGradient::colorize(const double *data, const QCPRange &range, QRgb *scanLine, int n, int dataIndexFactor, bool logarithmic)
{
  // If you change something here, make sure to also adapt ::color()
  if (!data)
  {
    qDebug() << Q_FUNC_INFO << "null pointer given as data";
    return;
  }
  if (!scanLine)
  {
    qDebug() << Q_FUNC_INFO << "null pointer given as scanLine";
    return;
  }
  if (mColorBufferInvalidated)
    updateColorBuffer();
  
  if (!logarithmic)
  {
    const double posToIndexFactor = (mLevelCount-1)/range.size();
    if (mPeriodic)
    {
      for (int i=0; i<n; ++i)
      {
        int index = (int)((data[dataIndexFactor*i]-range.lower)*posToIndexFactor) % mLevelCount;
        if (index < 0)
          index += mLevelCount;
        scanLine[i] = mColorBuffer.at(index);
      }
    } else
    {
      for (int i=0; i<n; ++i)
      {
        int index = (data[dataIndexFactor*i]-range.lower)*posToIndexFactor;
        if (index < 0)
          index = 0;
        else if (index >= mLevelCount)
          index = mLevelCount-1;
        scanLine[i] = mColorBuffer.at(index);
      }
    }
  } else // logarithmic == true
  {
    if (mPeriodic)
    {
      for (int i=0; i<n; ++i)
      {
        int index = (int)(qLn(data[dataIndexFactor*i]/range.lower)/qLn(range.upper/range.lower)*(mLevelCount-1)) % mLevelCount;
        if (index < 0)
          index += mLevelCount;
        scanLine[i] = mColorBuffer.at(index);
      }
    } else
    {
      for (int i=0; i<n; ++i)
      {
        int index = qLn(data[dataIndexFactor*i]/range.lower)/qLn(range.upper/range.lower)*(mLevelCount-1);
        if (index < 0)
          index = 0;
        else if (index >= mLevelCount)
          index = mLevelCount-1;
        scanLine[i] = mColorBuffer.at(index);
      }
    }
  }
}

/*! \internal
  
  This method is used to colorize a single data value given in \a position, to colors. The data
  range that shall be used for mapping the data value to the gradient is passed in \a range. \a
  logarithmic indicates whether the data value shall be mapped to a color logarithmically.
  
  If an entire array of data values shall be converted, rather use \ref colorize, for better
  performance.
*/
QRgb QCPColorGradient::color(double position, const QCPRange &range, bool logarithmic)
{
  // If you change something here, make sure to also adapt ::colorize()
  if (mColorBufferInvalidated)
    updateColorBuffer();
  int index = 0;
  if (!logarithmic)
    index = (position-range.lower)*(mLevelCount-1)/range.size();
  else
    index = qLn(position/range.lower)/qLn(range.upper/range.lower)*(mLevelCount-1);
  if (mPeriodic)
  {
    index = index % mLevelCount;
    if (index < 0)
      index += mLevelCount;
  } else
  {
    if (index < 0)
      index = 0;
    else if (index >= mLevelCount)
      index = mLevelCount-1;
  }
  return mColorBuffer.at(index);
}

/*!
  Clears the current color stops and loads the specified \a preset. A preset consists of predefined
  color stops and the corresponding color interpolation method.
  
  The available presets are:
  \image html QCPColorGradient.png
*/
void QCPColorGradient::loadPreset(GradientPreset preset)
{
  clearColorStops();
  switch (preset)
  {
    case gpGrayscale:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, Qt::black);
      setColorStopAt(1, Qt::white);
      break;
    case gpHot:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(50, 0, 0));
      setColorStopAt(0.2, QColor(180, 10, 0));
      setColorStopAt(0.4, QColor(245, 50, 0));
      setColorStopAt(0.6, QColor(255, 150, 10));
      setColorStopAt(0.8, QColor(255, 255, 50));
      setColorStopAt(1, QColor(255, 255, 255));
      break;
    case gpCold:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(0, 0, 50));
      setColorStopAt(0.2, QColor(0, 10, 180));
      setColorStopAt(0.4, QColor(0, 50, 245));
      setColorStopAt(0.6, QColor(10, 150, 255));
      setColorStopAt(0.8, QColor(50, 255, 255));
      setColorStopAt(1, QColor(255, 255, 255));
      break;
    case gpNight:
      setColorInterpolation(ciHSV);
      setColorStopAt(0, QColor(10, 20, 30));
      setColorStopAt(1, QColor(250, 255, 250));
      break;
    case gpCandy:
      setColorInterpolation(ciHSV);
      setColorStopAt(0, QColor(0, 0, 255));
      setColorStopAt(1, QColor(255, 250, 250));
      break;
    case gpGeography:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(70, 170, 210));
      setColorStopAt(0.20, QColor(90, 160, 180));
      setColorStopAt(0.25, QColor(45, 130, 175));
      setColorStopAt(0.30, QColor(100, 140, 125));
      setColorStopAt(0.5, QColor(100, 140, 100));
      setColorStopAt(0.6, QColor(130, 145, 120));
      setColorStopAt(0.7, QColor(140, 130, 120));
      setColorStopAt(0.9, QColor(180, 190, 190));
      setColorStopAt(1, QColor(210, 210, 230));
      break;
    case gpIon:
      setColorInterpolation(ciHSV);
      setColorStopAt(0, QColor(50, 10, 10));
      setColorStopAt(0.45, QColor(0, 0, 255));
      setColorStopAt(0.8, QColor(0, 255, 255));
      setColorStopAt(1, QColor(0, 255, 0));
      break;
    case gpThermal:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(0, 0, 50));
      setColorStopAt(0.15, QColor(20, 0, 120));
      setColorStopAt(0.33, QColor(200, 30, 140));
      setColorStopAt(0.6, QColor(255, 100, 0));
      setColorStopAt(0.85, QColor(255, 255, 40));
      setColorStopAt(1, QColor(255, 255, 255));
      break;
    case gpPolar:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(50, 255, 255));
      setColorStopAt(0.18, QColor(10, 70, 255));
      setColorStopAt(0.28, QColor(10, 10, 190));
      setColorStopAt(0.5, QColor(0, 0, 0));
      setColorStopAt(0.72, QColor(190, 10, 10));
      setColorStopAt(0.82, QColor(255, 70, 10));
      setColorStopAt(1, QColor(255, 255, 50));
      break;
    case gpSpectrum:
      setColorInterpolation(ciHSV);
      setColorStopAt(0, QColor(50, 0, 50));
      setColorStopAt(0.15, QColor(0, 0, 255));
      setColorStopAt(0.35, QColor(0, 255, 255));
      setColorStopAt(0.6, QColor(255, 255, 0));
      setColorStopAt(0.75, QColor(255, 30, 0));
      setColorStopAt(1, QColor(50, 0, 0));
      break;
    case gpJet:
      setColorInterpolation(ciRGB);
      setColorStopAt(0, QColor(0, 0, 100));
      setColorStopAt(0.15, QColor(0, 50, 255));
      setColorStopAt(0.35, QColor(0, 255, 255));
      setColorStopAt(0.65, QColor(255, 255, 0));
      setColorStopAt(0.85, QColor(255, 30, 0));
      setColorStopAt(1, QColor(100, 0, 0));
      break;
    case gpHues:
      setColorInterpolation(ciHSV);
      setColorStopAt(0, QColor(255, 0, 0));
      setColorStopAt(1.0/3.0, QColor(0, 0, 255));
      setColorStopAt(2.0/3.0, QColor(0, 255, 0));
      setColorStopAt(1, QColor(255, 0, 0));
      break;
  }
}

/*!
  Clears all color stops.
  
  \see setColorStops, setColorStopAt
*/
void QCPColorGradient::clearColorStops()
{
  mColorStops.clear();
  mColorBufferInvalidated = true;
}

/*!
  Returns an inverted gradient. The inverted gradient has all properties as this \ref
  QCPColorGradient, but the order of the color stops is inverted.
  
  \see setColorStops, setColorStopAt
*/
QCPColorGradient QCPColorGradient::inverted() const
{
  QCPColorGradient result(*this);
  result.clearColorStops();
  for (QMap<double, QColor>::const_iterator it=mColorStops.constBegin(); it!=mColorStops.constEnd(); ++it)
    result.setColorStopAt(1.0-it.key(), it.value());
  return result;
}

/*! \internal
  
  Updates the internal color buffer which will be used by \ref colorize and \ref color, to quickly
  convert positions to colors. This is where the interpolation between color stops is calculated.
*/
void QCPColorGradient::updateColorBuffer()
{
  if (mColorBuffer.size() != mLevelCount)
    mColorBuffer.resize(mLevelCount);
  if (mColorStops.size() > 1)
  {
    double indexToPosFactor = 1.0/(double)(mLevelCount-1);
    for (int i=0; i<mLevelCount; ++i)
    {
      double position = i*indexToPosFactor;
      QMap<double, QColor>::const_iterator it = mColorStops.lowerBound(position);
      if (it == mColorStops.constEnd()) // position is on or after last stop, use color of last stop
      {
        mColorBuffer[i] = (it-1).value().rgb();
      } else if (it == mColorStops.constBegin()) // position is on or before first stop, use color of first stop
      {
        mColorBuffer[i] = it.value().rgb();
      } else // position is in between stops (or on an intermediate stop), interpolate color
      {
        QMap<double, QColor>::const_iterator high = it;
        QMap<double, QColor>::const_iterator low = it-1;
        double t = (position-low.key())/(high.key()-low.key()); // interpolation factor 0..1
        switch (mColorInterpolation)
        {
          case ciRGB:
          {
            mColorBuffer[i] = qRgb((1-t)*low.value().red() + t*high.value().red(),
                                   (1-t)*low.value().green() + t*high.value().green(),
                                   (1-t)*low.value().blue() + t*high.value().blue());
            break;
          }
          case ciHSV:
          {
            QColor lowHsv = low.value().toHsv();
            QColor highHsv = high.value().toHsv();
            double hue = 0;
            double hueDiff = highHsv.hueF()-lowHsv.hueF();
            if (hueDiff > 0.5)
              hue = lowHsv.hueF() - t*(1.0-hueDiff);
            else if (hueDiff < -0.5)
              hue = lowHsv.hueF() + t*(1.0+hueDiff);
            else
              hue = lowHsv.hueF() + t*hueDiff;
            if (hue < 0) hue += 1.0;
            else if (hue >= 1.0) hue -= 1.0;
            mColorBuffer[i] = QColor::fromHsvF(hue, (1-t)*lowHsv.saturationF() + t*highHsv.saturationF(), (1-t)*lowHsv.valueF() + t*highHsv.valueF()).rgb();
            break;
          }
        }
      }
    }
  } else if (mColorStops.size() == 1)
  {
    mColorBuffer.fill(mColorStops.constBegin().value().rgb());
  } else // mColorStops is empty, fill color buffer with black
  {
    mColorBuffer.fill(qRgb(0, 0, 0));
  }
  mColorBufferInvalidated = false;
}
