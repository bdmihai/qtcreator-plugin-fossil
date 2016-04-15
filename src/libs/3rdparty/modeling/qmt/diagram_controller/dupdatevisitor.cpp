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

#include "dupdatevisitor.h"

#include "qmt/diagram/delement.h"
#include "qmt/diagram/dobject.h"
#include "qmt/diagram/dclass.h"
#include "qmt/diagram/ditem.h"
#include "qmt/diagram/drelation.h"
#include "qmt/diagram/ddependency.h"
#include "qmt/diagram/dassociation.h"

#include "qmt/model/melement.h"
#include "qmt/model/mobject.h"
#include "qmt/model/mclass.h"
#include "qmt/model/mcomponent.h"
#include "qmt/model/mpackage.h"
#include "qmt/model/mdiagram.h"
#include "qmt/model/mcanvasdiagram.h"
#include "qmt/model/mitem.h"
#include "qmt/model/mrelation.h"
#include "qmt/model/massociation.h"
#include "qmt/model/mdependency.h"
#include "qmt/model/minheritance.h"


namespace qmt {

DUpdateVisitor::DUpdateVisitor(DElement *target, const MDiagram *diagram, bool check_needs_update)
    : _target(target),
      _diagram(diagram),
      _check_needs_update(check_needs_update),
      _update_needed(!check_needs_update)
{
}

void DUpdateVisitor::setCheckNeedsUpdate(bool check_needs_update)
{
    _check_needs_update = check_needs_update;
    _update_needed = !check_needs_update;
}

void DUpdateVisitor::visitMElement(const MElement *element)
{
    Q_UNUSED(element);

    QMT_CHECK(_target);
}

void DUpdateVisitor::visitMObject(const MObject *object)
{
    DObject *dobject = dynamic_cast<DObject *>(_target);
    QMT_CHECK(dobject);
    if (isUpdating(object->getStereotypes() != dobject->getStereotypes())) {
        dobject->setStereotypes(object->getStereotypes());
    }
    const MObject *object_owner = object->getOwner();
    const MObject *diagram_owner = _diagram->getOwner();
    if (object_owner && diagram_owner && object_owner->getUid() != diagram_owner->getUid()) {
        if (isUpdating(object_owner->getName() != dobject->getContext())) {
            dobject->setContext(object_owner->getName());
        }
    } else {
        if (isUpdating(!dobject->getContext().isEmpty())) {
            dobject->setContext(QString());
        }
    }
    if (isUpdating(object->getName() != dobject->getName())) {
        dobject->setName(object->getName());
    }
    // TODO unlikely that this is called for all objects if hierarchy is modified
    // PERFORM remove loop
    int depth = 1;
    const MObject *owner = object->getOwner();
    while (owner) {
        owner = owner->getOwner();
        depth += 1;
    }
    if (isUpdating(depth != dobject->getDepth())) {
        dobject->setDepth(depth);
    }
    visitMElement(object);
}

void DUpdateVisitor::visitMPackage(const MPackage *package)
{
    visitMObject(package);
}

void DUpdateVisitor::visitMClass(const MClass *klass)
{
    DClass *dclass = dynamic_cast<DClass *>(_target);
    QMT_CHECK(dclass);
    if (isUpdating(klass->getNamespace() != dclass->getNamespace())) {
        dclass->setNamespace(klass->getNamespace());
    }
    if (isUpdating(klass->getTemplateParameters() != dclass->getTemplateParameters())) {
        dclass->setTemplateParameters(klass->getTemplateParameters());
    }
    if (isUpdating(klass->getMembers() != dclass->getMembers())) {
        dclass->setMembers(klass->getMembers());
    }
    visitMObject(klass);
}

void DUpdateVisitor::visitMComponent(const MComponent *component)
{
    visitMObject(component);
}

void DUpdateVisitor::visitMDiagram(const MDiagram *diagram)
{
    visitMObject(diagram);
}

void DUpdateVisitor::visitMCanvasDiagram(const MCanvasDiagram *diagram)
{
    visitMDiagram(diagram);
}

void DUpdateVisitor::visitMItem(const MItem *item)
{
    DItem *ditem = dynamic_cast<DItem *>(_target);
    QMT_CHECK(ditem);
    if (isUpdating(item->isShapeEditable() != ditem->isShapeEditable())) {
        ditem->setShapeEditable(item->isShapeEditable());
    }
    if (isUpdating(item->getVariety() != ditem->getVariety())) {
        ditem->setVariety(item->getVariety());
    }
    visitMObject(item);
}

void DUpdateVisitor::visitMRelation(const MRelation *relation)
{
    DRelation *drelation = dynamic_cast<DRelation *>(_target);
    QMT_CHECK(drelation);
    if (isUpdating(relation->getStereotypes() != drelation->getStereotypes())) {
        drelation->setStereotypes(relation->getStereotypes());
    }
    if (isUpdating(relation->getName() != drelation->getName())) {
        drelation->setName(relation->getName());
    }
    visitMElement(relation);
}

void DUpdateVisitor::visitMDependency(const MDependency *dependency)
{
    DDependency *ddependency = dynamic_cast<DDependency *>(_target);
    QMT_CHECK(ddependency);
    if (isUpdating(dependency->getDirection() != ddependency->getDirection())) {
        ddependency->setDirection(dependency->getDirection());
    }
    visitMRelation(dependency);
}

void DUpdateVisitor::visitMInheritance(const MInheritance *inheritance)
{
    visitMRelation(inheritance);
}

void DUpdateVisitor::visitMAssociation(const MAssociation *association)
{
    DAssociation *dassociation = dynamic_cast<DAssociation *>(_target);
    QMT_CHECK(dassociation);
    DAssociationEnd end_a;
    end_a.setName(association->getA().getName());
    end_a.setCardinatlity(association->getA().getCardinality());
    end_a.setNavigable(association->getA().isNavigable());
    end_a.setKind(association->getA().getKind());
    if (isUpdating(end_a != dassociation->getA())) {
        dassociation->setA(end_a);
    }
    DAssociationEnd end_b;
    end_b.setName(association->getB().getName());
    end_b.setCardinatlity(association->getB().getCardinality());
    end_b.setNavigable(association->getB().isNavigable());
    end_b.setKind(association->getB().getKind());
    if (isUpdating(end_b != dassociation->getB())) {
        dassociation->setB(end_b);
    }
    visitMRelation(association);
}

bool DUpdateVisitor::isUpdating(bool value_changed)
{
    if (_check_needs_update) {
        if (value_changed) {
            _update_needed = true;
        }
        return false;
    }
    return value_changed;
}

}
