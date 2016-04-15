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

#ifndef QMT_MELEMENT_H
#define QMT_MELEMENT_H

#include "qmt/infrastructure/uid.h"

#include <QList>
#include <QString>


namespace qmt {

class MElement;
class MObject;
class MVisitor;
class MConstVisitor;


class MExpansion {
public:

    virtual ~MExpansion() { }

    virtual MExpansion *clone(const MElement &rhs) const = 0;

    virtual void assign(MElement *lhs, const MElement &rhs);

    virtual void destroy(MElement *element);
};


class QMT_EXPORT MElement
{
    friend class MExpansion;

public:

    enum Flag {
        REVERSE_ENGINEERED = 0x01
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:

    MElement();

    MElement(const MElement &rhs);

    virtual ~MElement();

public:

    MElement &operator=(const MElement &rhs);

public:

    Uid getUid() const { return _uid; }

    void setUid(const Uid &uid);

    void renewUid();

    MObject *getOwner() const { return _owner; }

    void setOwner(MObject *owner);

    MExpansion *getExpansion() const { return _expansion; }

    void setExpansion(MExpansion *expansion);

    QList<QString> getStereotypes() const { return _stereotypes; }

    void setStereotypes(const QList<QString> &stereotypes);

    Flags getFlags() const { return _flags; }

    void setFlags(const Flags &flags);

public:

    virtual void accept(MVisitor *visitor);

    virtual void accept(MConstVisitor *visitor) const;

private:

    Uid _uid;

    MObject *_owner;

    MExpansion *_expansion;

    QList<QString> _stereotypes;

    Flags _flags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MElement::Flags)

}

#endif // QMT_MELEMENT_H
