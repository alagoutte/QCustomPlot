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
**             Date: 25.04.15                                             **
**          Version: 1.3.1                                                **
****************************************************************************/

#include "plottable-bars.h"

#include "../painter.h"
#include "../core.h"
#include "../axis/axis.h"
#include "../layoutelements/layoutelement-axisrect.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPBarsGroup
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPBarsGroup
  \brief Groups multiple QCPBars together so they appear side by side
  
  \image html QCPBarsGroup.png
  
  When showing multiple QCPBars in one plot which have bars at identical keys, it may be desirable
  to have them appearing next to each other at each key. This is what adding the respective QCPBars
  plottables to a QCPBarsGroup achieves. (An alternative approach is to stack them on top of each
  other, see \ref QCPBars::moveAbove.)
  
  \section qcpbarsgroup-usage Usage
  
  To add a QCPBars plottable to the group, create a new group and then add the respective bars
  intances:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpbarsgroup-creation
  Alternatively to appending to the group like shown above, you can also set the group on the
  QCPBars plottable via \ref QCPBars::setBarsGroup.
  
  The spacing between the bars can be configured via \ref setSpacingType and \ref setSpacing. The
  bars in this group appear in the plot in the order they were appended. To insert a bars plottable
  at a certain index position, or to reposition a bars plottable which is already in the group, use
  \ref insert.
  
  To remove specific bars from the group, use either \ref remove or call \ref
  QCPBars::setBarsGroup "QCPBars::setBarsGroup(0)" on the respective bars plottable.
  
  To clear the entire group, call \ref clear, or simply delete the group.
  
  \section qcpbarsgroup-example Example
  
  The image above is generated with the following code:
  \snippet documentation/doc-image-generator/mainwindow.cpp qcpbarsgroup-example
*/

/* start of documentation of inline functions */

/*! \fn QList<QCPBars*> QCPBarsGroup::bars() const
  
  Returns all bars currently in this group.
  
  \see bars(int index)
*/

/*! \fn int QCPBarsGroup::size() const
  
  Returns the number of QCPBars plottables that are part of this group.
  
*/

/*! \fn bool QCPBarsGroup::isEmpty() const
  
  Returns whether this bars group is empty.
  
  \see size
*/

/*! \fn bool QCPBarsGroup::contains(QCPBars *bars)
  
  Returns whether the specified \a bars plottable is part of this group.
  
*/

/* end of documentation of inline functions */

/*!
  Constructs a new bars group for the specified QCustomPlot instance.
*/
QCPBarsGroup::QCPBarsGroup(QCustomPlot *parentPlot) :
  QObject(parentPlot),
  mParentPlot(parentPlot),
  mSpacingType(stAbsolute),
  mSpacing(4)
{
}

QCPBarsGroup::~QCPBarsGroup()
{
  clear();
}

/*!
  Sets how the spacing between adjacent bars is interpreted. See \ref SpacingType.
  
  The actual spacing can then be specified with \ref setSpacing.

  \see setSpacing
*/
void QCPBarsGroup::setSpacingType(SpacingType spacingType)
{
  mSpacingType = spacingType;
}

/*!
  Sets the spacing between adjacent bars. What the number passed as \a spacing actually means, is
  defined by the current \ref SpacingType, which can be set with \ref setSpacingType.

  \see setSpacingType
*/
void QCPBarsGroup::setSpacing(double spacing)
{
  mSpacing = spacing;
}

/*!
  Returns the QCPBars instance with the specified \a index in this group. If no such QCPBars
  exists, returns 0.

  \see bars(), size
*/
QCPBars *QCPBarsGroup::bars(int index) const
{
  if (index >= 0 && index < mBars.size())
  {
    return mBars.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return 0;
  }
}

/*!
  Removes all QCPBars plottables from this group.

  \see isEmpty
*/
void QCPBarsGroup::clear()
{
  foreach (QCPBars *bars, mBars) // since foreach takes a copy, removing bars in the loop is okay
    bars->setBarsGroup(0); // removes itself via removeBars
}

/*!
  Adds the specified \a bars plottable to this group. Alternatively, you can also use \ref
  QCPBars::setBarsGroup on the \a bars instance.

  \see insert, remove
*/
void QCPBarsGroup::append(QCPBars *bars)
{
  if (!bars)
  {
    qDebug() << Q_FUNC_INFO << "bars is 0";
    return;
  }
    
  if (!mBars.contains(bars))
    bars->setBarsGroup(this);
  else
    qDebug() << Q_FUNC_INFO << "bars plottable is already in this bars group:" << reinterpret_cast<quintptr>(bars);
}

/*!
  Inserts the specified \a bars plottable into this group at the specified index position \a i.
  This gives you full control over the ordering of the bars.
  
  \a bars may already be part of this group. In that case, \a bars is just moved to the new index
  position.

  \see append, remove
*/
void QCPBarsGroup::insert(int i, QCPBars *bars)
{
  if (!bars)
  {
    qDebug() << Q_FUNC_INFO << "bars is 0";
    return;
  }
  
  // first append to bars list normally:
  if (!mBars.contains(bars))
    bars->setBarsGroup(this);
  // then move to according position:
  mBars.move(mBars.indexOf(bars), qBound(0, i, mBars.size()-1));
}

/*!
  Removes the specified \a bars plottable from this group.
  
  \see contains, clear
*/
void QCPBarsGroup::remove(QCPBars *bars)
{
  if (!bars)
  {
    qDebug() << Q_FUNC_INFO << "bars is 0";
    return;
  }
  
  if (mBars.contains(bars))
    bars->setBarsGroup(0);
  else
    qDebug() << Q_FUNC_INFO << "bars plottable is not in this bars group:" << reinterpret_cast<quintptr>(bars);
}

/*! \internal
  
  Adds the specified \a bars to the internal mBars list of bars. This method does not change the
  barsGroup property on \a bars.
  
  \see unregisterBars
*/
void QCPBarsGroup::registerBars(QCPBars *bars)
{
  if (!mBars.contains(bars))
    mBars.append(bars);
}

/*! \internal
  
  Removes the specified \a bars from the internal mBars list of bars. This method does not change
  the barsGroup property on \a bars.
  
  \see registerBars
*/
void QCPBarsGroup::unregisterBars(QCPBars *bars)
{
  mBars.removeOne(bars);
}

/*! \internal
  
  Returns the pixel offset in the key dimension the specified \a bars plottable should have at the
  given key coordinate \a keyCoord. The offset is relative to the pixel position of the key
  coordinate \a keyCoord.
*/
double QCPBarsGroup::keyPixelOffset(const QCPBars *bars, double keyCoord)
{
  // find list of all base bars in case some mBars are stacked:
  QList<const QCPBars*> baseBars;
  foreach (const QCPBars *b, mBars)
  {
    while (b->barBelow())
      b = b->barBelow();
    if (!baseBars.contains(b))
      baseBars.append(b);
  }
  // find base bar this "bars" is stacked on:
  const QCPBars *thisBase = bars;
  while (thisBase->barBelow())
    thisBase = thisBase->barBelow();
  
  // determine key pixel offset of this base bars considering all other base bars in this barsgroup:
  double result = 0;
  int index = baseBars.indexOf(thisBase);
  if (index >= 0)
  {
    int startIndex;
    double lowerPixelWidth = 0, upperPixelWidth = 0;
    if (baseBars.size() % 2 == 1 && index == (baseBars.size()-1)/2) // is center bar (int division on purpose)
    {
      return result;
    } else if (index < (baseBars.size()-1)/2.0) // bar is to the left of center
    {
      if (baseBars.size() % 2 == 0) // even number of bars
      {
        startIndex = baseBars.size()/2-1;
        result -= getPixelSpacing(baseBars.at(startIndex), keyCoord)*0.5; // half of middle spacing
      } else // uneven number of bars
      {
        startIndex = (baseBars.size()-1)/2-1;
        baseBars.at((baseBars.size()-1)/2)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
        result -= qAbs(upperPixelWidth-lowerPixelWidth)*0.5; // half of center bar
        result -= getPixelSpacing(baseBars.at((baseBars.size()-1)/2), keyCoord); // center bar spacing
      }
      for (int i=startIndex; i>index; --i) // add widths and spacings of bars in between center and our bars
      {
        baseBars.at(i)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
        result -= qAbs(upperPixelWidth-lowerPixelWidth);
        result -= getPixelSpacing(baseBars.at(i), keyCoord);
      }
      // finally half of our bars width:
      baseBars.at(index)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
      result -= qAbs(upperPixelWidth-lowerPixelWidth)*0.5;
    } else // bar is to the right of center
    {
      if (baseBars.size() % 2 == 0) // even number of bars
      {
        startIndex = baseBars.size()/2;
        result += getPixelSpacing(baseBars.at(startIndex), keyCoord)*0.5; // half of middle spacing
      } else // uneven number of bars
      {
        startIndex = (baseBars.size()-1)/2+1;
        baseBars.at((baseBars.size()-1)/2)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
        result += qAbs(upperPixelWidth-lowerPixelWidth)*0.5; // half of center bar
        result += getPixelSpacing(baseBars.at((baseBars.size()-1)/2), keyCoord); // center bar spacing
      }
      for (int i=startIndex; i<index; ++i) // add widths and spacings of bars in between center and our bars
      {
        baseBars.at(i)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
        result += qAbs(upperPixelWidth-lowerPixelWidth);
        result += getPixelSpacing(baseBars.at(i), keyCoord);
      }
      // finally half of our bars width:
      baseBars.at(index)->getPixelWidth(keyCoord, lowerPixelWidth, upperPixelWidth);
      result += qAbs(upperPixelWidth-lowerPixelWidth)*0.5;
    }
  }
  return result;
}

/*! \internal
  
  Returns the spacing in pixels which is between this \a bars and the following one, both at the
  key coordinate \a keyCoord.
  
  \note Typically the returned value doesn't depend on \a bars or \a keyCoord. \a bars is only
  needed to get acces to the key axis transformation and axis rect for the modes \ref
  stAxisRectRatio and \ref stPlotCoords. The \a keyCoord is only relevant for spacings given in
  \ref stPlotCoords on a logarithmic axis.
*/
double QCPBarsGroup::getPixelSpacing(const QCPBars *bars, double keyCoord)
{
  switch (mSpacingType)
  {
    case stAbsolute:
    {
      return mSpacing;
    }
    case stAxisRectRatio:
    {
      if (bars->keyAxis()->orientation() == Qt::Horizontal)
        return bars->keyAxis()->axisRect()->width()*mSpacing;
      else
        return bars->keyAxis()->axisRect()->height()*mSpacing;
    }
    case stPlotCoords:
    {
      double keyPixel = bars->keyAxis()->coordToPixel(keyCoord);
      return bars->keyAxis()->coordToPixel(keyCoord+mSpacing)-keyPixel;
    }
  }
  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPBarsData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPBarsData
  \brief Holds the data of one single data point (one bar) for QCPBars.
  
  The stored data is:
  \li \a key: coordinate on the key axis of this bar (this is the \a mainKey and the \a sortKey)
  \li \a value: height coordinate on the value axis of this bar (this is the \a mainValue)
  
  The container for storing multiple data points is \ref QCPBarsDataContainer. It is a typedef for
  \ref QCPDataContainer with \ref QCPBarsData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.
  
  \see QCPBarsDataContainer
*/

/* start documentation of inline functions */

/*! \fn double QCPBarsData::sortKey() const
  
  Returns the \a key member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static QCPBarsData QCPBarsData::fromSortKey(double sortKey)
  
  Returns a data point with the specified \a sortKey. All other members are set to zero.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static static bool QCPBarsData::sortKeyIsMainKey()
  
  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPBarsData::mainKey() const
  
  Returns the \a key member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPBarsData::mainValue() const
  
  Returns the \a value member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn QCPRange QCPBarsData::valueRange() const
  
  Returns a QCPRange with both lower and upper boundary set to \a value of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a bar data point with key and value set to zero.
*/
QCPBarsData::QCPBarsData() :
  key(0),
  value(0)
{
}

/*!
  Constructs a bar data point with the specified \a key and \a value.
*/
QCPBarsData::QCPBarsData(double key, double value) :
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPBars
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPBars
  \brief A plottable representing a bar chart in a plot.

  \image html QCPBars.png
  
  To plot data, assign it with the \ref setData or \ref addData functions.
  
  \section appearance Changing the appearance
  
  The appearance of the bars is determined by the pen and the brush (\ref setPen, \ref setBrush).
  The width of the individual bars can be controlled with \ref setWidthType and \ref setWidth.
  
  Bar charts are stackable. This means, two QCPBars plottables can be placed on top of each other
  (see \ref QCPBars::moveAbove). So when two bars are at the same key position, they will appear
  stacked.
  
  If you would like to group multiple QCPBars plottables together so they appear side by side as
  shown below, use QCPBarsGroup.
  
  \image html QCPBarsGroup.png
  
  \section usage Usage
  
  Like all data representing objects in QCustomPlot, the QCPBars is a plottable
  (QCPAbstractPlottable). So the plottable-interface of QCustomPlot applies
  (QCustomPlot::plottable, QCustomPlot::removePlottable, etc.)
  
  Usually, you first create an instance:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpbars-creation-1
  which registers it with the QCustomPlot instance of the passed axes. Note that this QCustomPlot instance takes
  ownership of the plottable, so do not delete it manually but use QCustomPlot::removePlottable() instead.
  The newly created plottable can be modified, e.g.:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpbars-creation-2
*/

/* start of documentation of inline functions */

/*! \fn QCPBars *QCPBars::barBelow() const
  Returns the bars plottable that is directly below this bars plottable.
  If there is no such plottable, returns 0.
  
  \see barAbove, moveBelow, moveAbove
*/

/*! \fn QCPBars *QCPBars::barAbove() const
  Returns the bars plottable that is directly above this bars plottable.
  If there is no such plottable, returns 0.
  
  \see barBelow, moveBelow, moveAbove
*/

/* end of documentation of inline functions */

/*!
  Constructs a bar chart which uses \a keyAxis as its key axis ("x") and \a valueAxis as its value
  axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance and not have
  the same orientation. If either of these restrictions is violated, a corresponding message is
  printed to the debug output (qDebug), the construction is not aborted, though.
  
  The created QCPBars is automatically registered with the QCustomPlot instance inferred from \a
  keyAxis. This QCustomPlot instance takes ownership of the QCPBars, so do not delete it manually
  but use QCustomPlot::removePlottable() instead.
*/
QCPBars::QCPBars(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable(keyAxis, valueAxis),
  mDataContainer(new QCPBarsDataContainer),
  mWidth(0.75),
  mWidthType(wtPlotCoords),
  mBarsGroup(0),
  mBaseValue(0)
{
  // modify inherited properties from abstract plottable:
  mPen.setColor(Qt::blue);
  mPen.setStyle(Qt::SolidLine);
  mBrush.setColor(QColor(40, 50, 255, 30));
  mBrush.setStyle(Qt::SolidPattern);
  mSelectedPen = mPen;
  mSelectedPen.setWidthF(2.5);
  mSelectedPen.setColor(QColor(80, 80, 255)); // lighter than Qt::blue of mPen
  mSelectedBrush = mBrush;
}

QCPBars::~QCPBars()
{
  setBarsGroup(0);
  if (mBarBelow || mBarAbove)
    connectBars(mBarBelow.data(), mBarAbove.data()); // take this bar out of any stacking
}

/*!
  Sets the width of the bars.

  How the number passed as \a width is interpreted (e.g. screen pixels, plot coordinates,...),
  depends on the currently set width type, see \ref setWidthType and \ref WidthType.
*/
void QCPBars::setWidth(double width)
{
  mWidth = width;
}

/*!
  Sets how the width of the bars is defined. See the documentation of \ref WidthType for an
  explanation of the possible values for \a widthType.
  
  The default value is \ref wtPlotCoords.
  
  \see setWidth
*/
void QCPBars::setWidthType(QCPBars::WidthType widthType)
{
  mWidthType = widthType;
}

/*!
  Sets to which QCPBarsGroup this QCPBars instance belongs to. Alternatively, you can also use \ref
  QCPBarsGroup::append.
  
  To remove this QCPBars from any group, set \a barsGroup to 0.
*/
void QCPBars::setBarsGroup(QCPBarsGroup *barsGroup)
{
  // deregister at old group:
  if (mBarsGroup)
    mBarsGroup->unregisterBars(this);
  mBarsGroup = barsGroup;
  // register at new group:
  if (mBarsGroup)
    mBarsGroup->registerBars(this);
}

/*!
  Sets the base value of this bars plottable.

  The base value defines where on the value coordinate the bars start. How far the bars extend from
  the base value is given by their individual value data. For example, if the base value is set to
  1, a bar with data value 2 will have its lowest point at value coordinate 1 and highest point at
  3.
  
  For stacked bars, only the base value of the bottom-most QCPBars has meaning.
  
  The default base value is 0.
*/
void QCPBars::setBaseValue(double baseValue)
{
  mBaseValue = baseValue;
}

void QCPBars::setData(QSharedPointer<QCPBarsDataContainer> data)
{
  mDataContainer = data;
}

/*! \overload
  
  Replaces the current data with the provided points in \a keys and \a values tuples. The provided
  vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCPBars::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  addData(keys, values, alreadySorted);
}

/*!
  Moves this bars plottable below \a bars. In other words, the bars of this plottable will appear
  below the bars of \a bars. The move target \a bars must use the same key and value axis as this
  plottable.
  
  Inserting into and removing from existing bar stacking is handled gracefully. If \a bars already
  has a bars object below itself, this bars object is inserted between the two. If this bars object
  is already between two other bars, the two other bars will be stacked on top of each other after
  the operation.
  
  To remove this bars plottable from any stacking, set \a bars to 0.
  
  \see moveBelow, barAbove, barBelow
*/
void QCPBars::moveBelow(QCPBars *bars)
{
  if (bars == this) return;
  if (bars && (bars->keyAxis() != mKeyAxis.data() || bars->valueAxis() != mValueAxis.data()))
  {
    qDebug() << Q_FUNC_INFO << "passed QCPBars* doesn't have same key and value axis as this QCPBars";
    return;
  }
  // remove from stacking:
  connectBars(mBarBelow.data(), mBarAbove.data()); // Note: also works if one (or both) of them is 0
  // if new bar given, insert this bar below it:
  if (bars)
  {
    if (bars->mBarBelow)
      connectBars(bars->mBarBelow.data(), this);
    connectBars(this, bars);
  }
}

/*!
  Moves this bars plottable above \a bars. In other words, the bars of this plottable will appear
  above the bars of \a bars. The move target \a bars must use the same key and value axis as this
  plottable.
  
  Inserting into and removing from existing bar stacking is handled gracefully. If \a bars already
  has a bars object above itself, this bars object is inserted between the two. If this bars object
  is already between two other bars, the two other bars will be stacked on top of each other after
  the operation.
  
  To remove this bars plottable from any stacking, set \a bars to 0.
  
  \see moveBelow, barBelow, barAbove
*/
void QCPBars::moveAbove(QCPBars *bars)
{
  if (bars == this) return;
  if (bars && (bars->keyAxis() != mKeyAxis.data() || bars->valueAxis() != mValueAxis.data()))
  {
    qDebug() << Q_FUNC_INFO << "passed QCPBars* doesn't have same key and value axis as this QCPBars";
    return;
  }
  // remove from stacking:
  connectBars(mBarBelow.data(), mBarAbove.data()); // Note: also works if one (or both) of them is 0
  // if new bar given, insert this bar above it:
  if (bars)
  {
    if (bars->mBarAbove)
      connectBars(this, bars->mBarAbove.data());
    connectBars(bars, this);
  }
}

/*! \overload
  Adds the provided data points as \a key and \a value pairs to the current data.
  
  Alternatively, you can also access and modify the graph's data via the \ref data method, which
  returns a pointer to the internal \ref QCPGraphDataContainer.
*/
void QCPBars::addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  if (keys.size() != values.size())
    qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
  const int n = qMin(keys.size(), values.size());
  QVector<QCPBarsData> tempData(n);
  QVector<QCPBarsData>::iterator it = tempData.begin();
  const QVector<QCPBarsData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

/*! \overload
  Adds the provided data point as \a key and \a value to the current data.
  
  Alternatively, you can also access and modify the graph's data via the \ref data method, which
  returns a pointer to the internal \ref QCPGraphDataContainer.
*/
void QCPBars::addData(double key, double value)
{
  mDataContainer->add(QCPBarsData(key, value));
}

/* inherits documentation from base class */
double QCPBars::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return -1; }
  
  if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()))
  {
    // get visible data range:
    QCPBarsDataContainer::const_iterator lower, upperEnd;
    getVisibleDataBounds(lower, upperEnd);
    if (lower == upperEnd)
      return -1;
    for (QCPBarsDataContainer::const_iterator it = lower; it != upperEnd; ++it)
    {
      if (getBarPolygon(it->key, it->value).boundingRect().contains(pos))
        return mParentPlot->selectionTolerance()*0.99;
    }
  }
  return -1;
}

/* inherits documentation from base class */
void QCPBars::draw(QCPPainter *painter)
{
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mDataContainer->isEmpty()) return;
  
  QCPBarsDataContainer::const_iterator it, lower, upperEnd;
  getVisibleDataBounds(lower, upperEnd);
  for (it = lower; it != upperEnd; ++it)
  {
    // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
    if (QCP::isInvalidData(it->key, it->value))
      qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "of drawn range invalid." << "Plottable name:" << name();
#endif
    QPolygonF barPolygon = getBarPolygon(it->key, it->value);
    // draw bar fill:
    if (mainBrush().style() != Qt::NoBrush && mainBrush().color().alpha() != 0)
    {
      applyFillAntialiasingHint(painter);
      painter->setPen(Qt::NoPen);
      painter->setBrush(mainBrush());
      painter->drawPolygon(barPolygon);
    }
    // draw bar line:
    if (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0)
    {
      applyDefaultAntialiasingHint(painter);
      painter->setPen(mainPen());
      painter->setBrush(Qt::NoBrush);
      painter->drawPolyline(barPolygon);
    }
  }
}

/* inherits documentation from base class */
void QCPBars::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
  // draw filled rect:
  applyDefaultAntialiasingHint(painter);
  painter->setBrush(mBrush);
  painter->setPen(mPen);
  QRectF r = QRectF(0, 0, rect.width()*0.67, rect.height()*0.67);
  r.moveCenter(rect.center());
  painter->drawRect(r);
}

/*!  \internal
  
  called by \ref draw to determine which data (key) range is visible at the current key axis range
  setting, so only that needs to be processed. It also takes into account the bar width.
  
  \a begin returns an iterator to the lowest data point that needs to be taken into account when
  plotting. Note that in order to get a clean plot all the way to the edge of the axis rect, \a
  lower may still be just outside the visible range.
  
  \a end returns an iterator one higher than the highest visible data point. Same as before, \a end
  may also lie just outside of the visible range.
  
  if the plottable contains no data, both \a begin and \a end point to constEnd.
*/
void QCPBars::getVisibleDataBounds(QCPBarsDataContainer::const_iterator &begin, QCPBarsDataContainer::const_iterator &end) const
{
  if (!mKeyAxis) { qDebug() << Q_FUNC_INFO << "invalid key axis"; return; }
  if (mDataContainer->isEmpty())
  {
    begin = mDataContainer->constEnd();
    end = mDataContainer->constEnd();
    return;
  }
  
  // get visible data range as QMap iterators
  begin = mDataContainer->findBeginBelowKey(mKeyAxis.data()->range().lower);
  end = mDataContainer->findEndAboveKey(mKeyAxis.data()->range().upper);
  double lowerPixelBound = mKeyAxis.data()->coordToPixel(mKeyAxis.data()->range().lower);
  double upperPixelBound = mKeyAxis.data()->coordToPixel(mKeyAxis.data()->range().upper);
  bool isVisible = false;
  // walk left from begin to find lower bar that actually is completely outside visible pixel range:
  QCPBarsDataContainer::const_iterator it = begin;
  while (it != mDataContainer->constBegin())
  {
    --it;
    QRectF barBounds = getBarPolygon(it->key, it->value).boundingRect();
    if (mKeyAxis.data()->orientation() == Qt::Horizontal)
      isVisible = ((!mKeyAxis.data()->rangeReversed() && barBounds.right() >= lowerPixelBound) || (mKeyAxis.data()->rangeReversed() && barBounds.left() <= lowerPixelBound));
    else // keyaxis is vertical
      isVisible = ((!mKeyAxis.data()->rangeReversed() && barBounds.top() <= lowerPixelBound) || (mKeyAxis.data()->rangeReversed() && barBounds.bottom() >= lowerPixelBound));
    if (isVisible)
      begin = it;
    else
      break;
  }
  // walk right from ubound to find upper bar that actually is completely outside visible pixel range:
  it = end;
  while (it != mDataContainer->constEnd())
  {
    QRectF barBounds = getBarPolygon(it->key, it->value).boundingRect();
    if (mKeyAxis.data()->orientation() == Qt::Horizontal)
      isVisible = ((!mKeyAxis.data()->rangeReversed() && barBounds.left() <= upperPixelBound) || (mKeyAxis.data()->rangeReversed() && barBounds.right() >= upperPixelBound));
    else // keyaxis is vertical
      isVisible = ((!mKeyAxis.data()->rangeReversed() && barBounds.bottom() >= upperPixelBound) || (mKeyAxis.data()->rangeReversed() && barBounds.top() <= upperPixelBound));
    if (isVisible)
      end = it+1;
    else
      break;
    ++it;
  }
}

/*! \internal
  
  Returns the polygon of a single bar with \a key and \a value. The Polygon is open at the bottom
  and shifted according to the bar stacking (see \ref moveAbove) and base value (see \ref
  setBaseValue).
*/
QPolygonF QCPBars::getBarPolygon(double key, double value) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return QPolygonF(); }
  
  QPolygonF result;
  double lowerPixelWidth = 0, upperPixelWidth = 0;
  getPixelWidth(key, lowerPixelWidth, upperPixelWidth);
  double base = getStackedBaseValue(key, value >= 0);
  double basePixel = valueAxis->coordToPixel(base);
  double valuePixel = valueAxis->coordToPixel(base+value);
  double keyPixel = keyAxis->coordToPixel(key);
  if (mBarsGroup)
    keyPixel += mBarsGroup->keyPixelOffset(this, key);
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    result << QPointF(keyPixel+lowerPixelWidth, basePixel);
    result << QPointF(keyPixel+lowerPixelWidth, valuePixel);
    result << QPointF(keyPixel+upperPixelWidth, valuePixel);
    result << QPointF(keyPixel+upperPixelWidth, basePixel);
  } else
  {
    result << QPointF(basePixel, keyPixel+lowerPixelWidth);
    result << QPointF(valuePixel, keyPixel+lowerPixelWidth);
    result << QPointF(valuePixel, keyPixel+upperPixelWidth);
    result << QPointF(basePixel, keyPixel+upperPixelWidth);
  }
  return result;
}

/*! \internal
  
  This function is used to determine the width of the bar at coordinate \a key, according to the
  specified width (\ref setWidth) and width type (\ref setWidthType).
  
  The output parameters \a lower and \a upper return the number of pixels the bar extends to lower
  and higher keys, relative to the \a key coordinate (so with a non-reversed horizontal axis, \a
  lower is negative and \a upper positive).
*/
void QCPBars::getPixelWidth(double key, double &lower, double &upper) const
{
  switch (mWidthType)
  {
    case wtAbsolute:
    {
      upper = mWidth*0.5;
      lower = -upper;
      if (mKeyAxis && (mKeyAxis.data()->rangeReversed() ^ (mKeyAxis.data()->orientation() == Qt::Vertical)))
        qSwap(lower, upper);
      break;
    }
    case wtAxisRectRatio:
    {
      if (mKeyAxis && mKeyAxis.data()->axisRect())
      {
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
          upper = mKeyAxis.data()->axisRect()->width()*mWidth*0.5;
        else
          upper = mKeyAxis.data()->axisRect()->height()*mWidth*0.5;
        lower = -upper;
        if (mKeyAxis && (mKeyAxis.data()->rangeReversed() ^ (mKeyAxis.data()->orientation() == Qt::Vertical)))
          qSwap(lower, upper);
      } else
        qDebug() << Q_FUNC_INFO << "No key axis or axis rect defined";
      break;
    }
    case wtPlotCoords:
    {
      if (mKeyAxis)
      {
        double keyPixel = mKeyAxis.data()->coordToPixel(key);
        upper = mKeyAxis.data()->coordToPixel(key+mWidth*0.5)-keyPixel;
        lower = mKeyAxis.data()->coordToPixel(key-mWidth*0.5)-keyPixel;
        // no need to qSwap(lower, higher) when range reversed, because higher/lower are gained by
        // coordinate transform which includes range direction
      } else
        qDebug() << Q_FUNC_INFO << "No key axis defined";
      break;
    }
  }
}

/*! \internal
  
  This function is called to find at which value to start drawing the base of a bar at \a key, when
  it is stacked on top of another QCPBars (e.g. with \ref moveAbove).
  
  positive and negative bars are separated per stack (positive are stacked above baseValue upwards,
  negative are stacked below baseValue downwards). This can be indicated with \a positive. So if the
  bar for which we need the base value is negative, set \a positive to false.
*/
double QCPBars::getStackedBaseValue(double key, bool positive) const
{
  if (mBarBelow)
  {
    double max = 0; // don't initialize with mBaseValue here because only base value of bottom-most bar has meaning in a bar stack
    // find bars of mBarBelow that are approximately at key and find largest one:
    double epsilon = qAbs(key)*1e-6; // should be safe even when changed to use float at some point
    if (key == 0)
      epsilon = 1e-6;
    QCPBarsDataContainer::const_iterator it = mBarBelow.data()->mDataContainer->findBeginBelowKey(key-epsilon);
    QCPBarsDataContainer::const_iterator itEnd = mBarBelow.data()->mDataContainer->findEndAboveKey(key+epsilon);
    while (it != itEnd)
    {
      if (it->key > key-epsilon && it->key < key+epsilon)
      {
        if ((positive && it->value > max) ||
            (!positive && it->value < max))
          max = it->value;
      }
      ++it;
    }
    // recurse down the bar-stack to find the total height:
    return max + mBarBelow.data()->getStackedBaseValue(key, positive);
  } else
    return mBaseValue;
}

/*! \internal

  Connects \a below and \a above to each other via their mBarAbove/mBarBelow properties. The bar(s)
  currently above lower and below upper will become disconnected to lower/upper.
  
  If lower is zero, upper will be disconnected at the bottom.
  If upper is zero, lower will be disconnected at the top.
*/
void QCPBars::connectBars(QCPBars *lower, QCPBars *upper)
{
  if (!lower && !upper) return;
  
  if (!lower) // disconnect upper at bottom
  {
    // disconnect old bar below upper:
    if (upper->mBarBelow && upper->mBarBelow.data()->mBarAbove.data() == upper)
      upper->mBarBelow.data()->mBarAbove = 0;
    upper->mBarBelow = 0;
  } else if (!upper) // disconnect lower at top
  {
    // disconnect old bar above lower:
    if (lower->mBarAbove && lower->mBarAbove.data()->mBarBelow.data() == lower)
      lower->mBarAbove.data()->mBarBelow = 0;
    lower->mBarAbove = 0;
  } else // connect lower and upper
  {
    // disconnect old bar above lower:
    if (lower->mBarAbove && lower->mBarAbove.data()->mBarBelow.data() == lower)
      lower->mBarAbove.data()->mBarBelow = 0;
    // disconnect old bar below upper:
    if (upper->mBarBelow && upper->mBarBelow.data()->mBarAbove.data() == upper)
      upper->mBarBelow.data()->mBarAbove = 0;
    lower->mBarAbove = upper;
    upper->mBarBelow = lower;
  }
}

/* inherits documentation from base class */
QCPRange QCPBars::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  QCPRange range;
  range = mDataContainer->keyRange(foundRange, inSignDomain);
  
  // determine exact range of bars by including bar width and barsgroup offset:
  if (foundRange && mKeyAxis)
  {
    double lowerPixelWidth = 0 , upperPixelWidth = 0, keyPixel;
    // lower range bound:
    getPixelWidth(range.lower, lowerPixelWidth, upperPixelWidth);
    keyPixel = mKeyAxis.data()->coordToPixel(range.lower) + lowerPixelWidth;
    if (mBarsGroup)
      keyPixel += mBarsGroup->keyPixelOffset(this, range.lower);
    range.lower = mKeyAxis.data()->pixelToCoord(keyPixel);
    // upper range bound:
    getPixelWidth(range.upper, lowerPixelWidth, upperPixelWidth);
    keyPixel = mKeyAxis.data()->coordToPixel(range.upper) + upperPixelWidth;
    if (mBarsGroup)
      keyPixel += mBarsGroup->keyPixelOffset(this, range.upper);
    range.upper = mKeyAxis.data()->pixelToCoord(keyPixel);
  }
  return range;
}

/* inherits documentation from base class */
QCPRange QCPBars::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  // Note: can't simply use mDataContainer->valueRange here because we need to
  // take into account bar base value and possible stacking of multiple bars
  QCPRange range;
  range.lower = mBaseValue;
  range.upper = mBaseValue;
  bool haveLower = true; // set to true, because baseValue should always be visible in bar charts
  bool haveUpper = true; // set to true, because baseValue should always be visible in bar charts
  double current;
  
  QCPBarsDataContainer::const_iterator it = mDataContainer->constBegin();
  while (it != mDataContainer->constEnd())
  {
    current = it->value + getStackedBaseValue(it->key, it->value >= 0);
    if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
    {
      if (current < range.lower || !haveLower)
      {
        range.lower = current;
        haveLower = true;
      }
      if (current > range.upper || !haveUpper)
      {
        range.upper = current;
        haveUpper = true;
      }
    }
    ++it;
  }
  
  foundRange = true; // return true because bar charts always have the 0-line visible
  return range;
}

