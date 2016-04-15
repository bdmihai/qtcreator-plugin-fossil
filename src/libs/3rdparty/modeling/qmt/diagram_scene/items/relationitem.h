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

#ifndef QMT_GRAPHICSRELATIONITEM_H
#define QMT_GRAPHICSRELATIONITEM_H

#include <QGraphicsItem>
#include "qmt/diagram_scene/capabilities/moveable.h"
#include "qmt/diagram_scene/capabilities/selectable.h"
#include "qmt/diagram_scene/capabilities/windable.h"

QT_BEGIN_NAMESPACE
class QGraphicsSimpleTextItem;
QT_END_NAMESPACE


namespace qmt {

class Uid;
class DRelation;
class DiagramSceneModel;
class ArrowItem;
class StereotypesItem;
class PathSelectionItem;
class Style;


class RelationItem :
        public QGraphicsItem,
        public IMoveable,
        public ISelectable,
        public IWindable
{
    class ArrowConfigurator;

public:
    RelationItem(DRelation *relation, DiagramSceneModel *diagram_scene_model, QGraphicsItem *parent = 0);

    ~RelationItem();

public:

    DRelation *getRelation() const { return _relation; }

public:

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QPainterPath shape() const;

public:

    void moveDelta(const QPointF &delta);

    void alignItemPositionToRaster(double raster_width, double raster_height);

public:

    bool isSecondarySelected() const;

    void setSecondarySelected(bool secondary_selected);

    bool isFocusSelected() const;

    void setFocusSelected(bool focus_selected);

public:

    QPointF getHandlePos(int index);

    void insertHandle(int before_index, const QPointF &pos);

    void deleteHandle(int index);

    void setHandlePos(int index, const QPointF &pos);

    void alignHandleToRaster(int index, double raster_width, double raster_height);

public:

    virtual void update();

protected:

    virtual void update(const Style *style);

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:

    const Style *getAdaptedStyle();

    QPointF calcEndPoint(const Uid &end, const Uid &other_end, int nearest_intermediate_point_index);

    QPointF calcEndPoint(const Uid &end, const QPointF &other_end_pos, int nearest_intermediate_point_index);

private:

    DRelation *_relation;

protected:

    DiagramSceneModel *_diagram_scene_model;

    bool _secondary_selected;

    bool _focus_selected;

    ArrowItem *_arrow;

    QGraphicsSimpleTextItem *_name;

    StereotypesItem *_stereotypes;

    PathSelectionItem *_selection_handles;
};

}

#endif // QMT_GRAPHICSRELATIONITEM_H
