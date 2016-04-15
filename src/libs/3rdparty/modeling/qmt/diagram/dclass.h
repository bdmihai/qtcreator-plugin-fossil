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

#ifndef QMT_DCLASS_H
#define QMT_DCLASS_H

#include "dobject.h"
#include "qmt/model/mclassmember.h"

#include <QSet>


namespace qmt {

class QMT_EXPORT DClass :
        public DObject
{
public:

    enum TemplateDisplay {
        TEMPLATE_SMART,
        TEMPLATE_BOX,
        TEMPLATE_NAME
    };

public:
    DClass();

public:

    QString getNamespace() const { return _namespace; }

    void setNamespace(const QString &name_space);

    QList<QString> getTemplateParameters() const { return _template_parameters; }

    void setTemplateParameters(const QList<QString> &template_parameters);

    QList<MClassMember> getMembers() const { return _members; }

    void setMembers(const QList<MClassMember> &members);

    QSet<Uid> getVisibleMembers() const { return _visible_members; }

    void setVisibleMembers(const QSet<Uid> &visible_members);

    TemplateDisplay getTemplateDisplay() const { return _template_display; }

    void setTemplateDisplay(TemplateDisplay template_display);

    bool getShowAllMembers() const { return _show_all_members; }

    void setShowAllMembers(bool show_all_members);

public:

    virtual void accept(DVisitor *visitor);

    virtual void accept(DConstVisitor *visitor) const;

private:

    QString _namespace;

    QList<QString> _template_parameters;

    QList<MClassMember> _members;

    QSet<Uid> _visible_members;

    TemplateDisplay _template_display;

    bool _show_all_members;
};

}

#endif // QMT_DCLASS_H
