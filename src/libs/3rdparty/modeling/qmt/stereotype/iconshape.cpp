/***************************************************************************
**
** Copyright (C) 2015 Jochen Becher
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "iconshape.h"

#include "qmt/stereotype/shapevalue.h"
#include "qmt/stereotype/shape.h"
#include "qmt/stereotype/shapes.h"
#include "qmt/stereotype/shapevisitor.h"

#include <QList>
#include <QPainter>


template<class T>
QList<T *> CloneAll(const QList<T *> &rhs)
{
    QList<T *> result;
    foreach (T *t, rhs) {
        result.append(t != 0 ? t->Clone() : 0);
    }
    return result;
}

namespace qmt {

struct IconShape::IconShapePrivate {
    IconShapePrivate()
    {
    }

    IconShapePrivate(const IconShapePrivate &rhs)
        : _shapes(CloneAll(rhs._shapes))
    {
    }

    ~IconShapePrivate()
    {
        qDeleteAll(_shapes);
    }

    IconShapePrivate &operator=(const IconShapePrivate &rhs)
    {
        if (this != &rhs) {
            qDeleteAll(_shapes);
            _shapes = CloneAll(rhs._shapes);
        }
        return *this;
    }

    PathShape *getActivePath();

    QList<IShape *> _shapes;
};

PathShape *IconShape::IconShapePrivate::getActivePath()
{
    PathShape *path_shape = 0;
    if (_shapes.count() > 0) {
        path_shape = dynamic_cast<PathShape *>(_shapes.last());
    }
    if (path_shape == 0) {
        path_shape = new PathShape();
        _shapes.append(path_shape);
    }
    return path_shape;
}


IconShape::IconShape()
    : d(new IconShapePrivate)
{
}

IconShape::IconShape(const IconShape &rhs)
    : d(new IconShapePrivate(*rhs.d))
{
}

IconShape::~IconShape()
{
    delete d;
}

IconShape &IconShape::operator=(const IconShape &rhs)
{
    if (this != &rhs) {
        *d = *rhs.d;
    }
    return *this;
}

void IconShape::addLine(const ShapePointF &pos1, const ShapePointF &pos2)
{
    d->_shapes.append(new LineShape(pos1, pos2));
}

void IconShape::addRect(const ShapePointF &pos, const ShapeSizeF &size)
{
    d->_shapes.append(new RectShape(pos, size));
}

void IconShape::addRoundedRect(const ShapePointF &pos, const ShapeSizeF &size, const ShapeValueF &radius)
{
    d->_shapes.append(new RoundedRectShape(pos, size, radius));
}

void IconShape::addCircle(const ShapePointF &center, const ShapeValueF &radius)
{
    d->_shapes.append(new CircleShape(center, radius));
}

void IconShape::addEllipse(const ShapePointF &center, const ShapeSizeF &radius)
{
    d->_shapes.append(new EllipseShape(center, radius));
}

void IconShape::addArc(const ShapePointF &center, const ShapeSizeF &radius, qreal start_angle, qreal span_angle)
{
    d->_shapes.append(new ArcShape(center, radius, start_angle, span_angle));
}

void IconShape::moveTo(const ShapePointF &pos)
{
    d->getActivePath()->moveTo(pos);
}

void IconShape::lineTo(const ShapePointF &pos)
{
    d->getActivePath()->lineTo(pos);
}

void IconShape::arcMoveTo(const ShapePointF &center, const ShapeSizeF &radius, qreal angle)
{
    d->getActivePath()->arcMoveTo(center, radius, angle);
}

void IconShape::arcTo(const ShapePointF &center, const ShapeSizeF &radius, qreal start_angle, qreal sweep_length)
{
    d->getActivePath()->arcTo(center, radius, start_angle, sweep_length);
}

void IconShape::closePath()
{
    d->getActivePath()->close();
}

void IconShape::visitShapes(ShapeConstVisitor *visitor) const
{
    foreach (IShape *p, d->_shapes) {
        p->accept(visitor);
    }
}

} // namespace qmt
