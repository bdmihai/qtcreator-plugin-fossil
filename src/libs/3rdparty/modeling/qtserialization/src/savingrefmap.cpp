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

#include "qark/impl/savingrefmap.h"

namespace qark {

namespace impl {

int SavingRefMap::countDanglingReferences()
{
    int dangling = 0;
    for (map_type::const_iterator it = _references.begin(); it != _references.end(); ++it) {
        if (!it.value().second) {
            ++dangling;
        }
    }
    return dangling;
}

bool SavingRefMap::hasRef(const void *address, const char *type_name)
{
    return _references.find(key_type(address, type_name)) != _references.end();
}

bool SavingRefMap::hasDefinedRef(const void *address, const char *type_name)
{
    map_type::const_iterator it = _references.find(key_type(address, type_name));
    if (it == _references.end()) {
        return false;
    }
    return it.value().second;
}

ObjectId SavingRefMap::getRef(const void *address, const char *type_name, bool define)
{
    key_type k = key_type(address, type_name);
    map_type::iterator it = _references.find(k);
    if (it != _references.end()) {
        if (define) {
            it.value().second = true;
        }
        return it.value().first;
    }
    ObjectId id = _next_ref++;
    _references[k] = value_type(id, define);
    return id;
}

}

}
