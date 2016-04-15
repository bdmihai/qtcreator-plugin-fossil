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

#ifndef QMT_MODELCONTROLLER_H
#define QMT_MODELCONTROLLER_H

#include "qmt/infrastructure/uid.h"

#include <QObject>
#include <QHash>
#include <QMultiHash>


namespace qmt {

class UndoController;

class MElement;

class MObject;
class MPackage;
class MDiagram;

class MRelation;

class MSelection;
class MContainer;
class MReferences;


class QMT_EXPORT ModelController :
        public QObject
{
    Q_OBJECT

    enum ElementType { TYPE_UNKNOWN, TYPE_OBJECT, TYPE_RELATION };

    struct Clone;

    class UpdateObjectCommand;
    class UpdateRelationCommand;
    class AddElementsCommand;
    class RemoveElementsCommand;
    class MoveObjectCommand;
    class MoveRelationCommand;

public:

    explicit ModelController(QObject *parent = 0);

    ~ModelController();

signals:

    void beginResetModel();

    void endResetModel();

    void beginUpdateObject(int row, const MObject *owner);

    void endUpdateObject(int row, const MObject *owner);

    void beginInsertObject(int row, const MObject *owner);

    void endInsertObject(int row, const MObject *owner);

    void beginRemoveObject(int row, const MObject *owner);

    void endRemoveObject(int row, const MObject *owner);

    void beginMoveObject(int former_row, const MObject *former_owner);

    void endMoveObject(int new_row, const MObject *new_owner);

    void beginUpdateRelation(int row, const MObject *owner);

    void endUpdateRelation(int row, const MObject *owner);

    void beginInsertRelation(int row, const MObject *owner);

    void endInsertRelation(int row, const MObject *owner);

    void beginRemoveRelation(int row, const MObject *owner);

    void endRemoveRelation(int row, const MObject *owner);

    void beginMoveRelation(int former_row, const MObject *former_owner);

    void endMoveRelation(int new_row, const MObject *new_owner);

    void packageNameChanged(MPackage *package, const QString &old_package_name);

    void relationEndChanged(MRelation *relation, MObject *end_object);

    void modified();

public:

    MPackage *getRootPackage() const { return _root_package; }

    void setRootPackage(MPackage *root_package);

    UndoController *getUndoController() const { return _undo_controller; }

    void setUndoController(UndoController *undo_controller);

public:

    Uid getOwnerKey(const MElement *element) const;

    MElement *findElement(const Uid &key);

    template<class T>
    T *findElement(const Uid &key) { return dynamic_cast<T *>(findElement(key)); }

public:

    void startResetModel();

    void finishResetModel(bool modified);

public:

    MObject *getObject(int row, const MObject *owner) const;

    MObject *findObject(const Uid &key) const;

    template<class T>
    T *findObject(const Uid &key) const { return dynamic_cast<T *>(findObject(key)); }

    void addObject(MPackage *parent_package, MObject *object);

    void removeObject(MObject *object);

    void startUpdateObject(MObject *object);

    void finishUpdateObject(MObject *object, bool cancelled);

    void moveObject(MPackage *new_owner, MObject *object);

public:

    MRelation *findRelation(const Uid &key) const;

    template<class T>
    T *findRelation(const Uid &key) const { return dynamic_cast<T *>(findRelation(key)); }

    void addRelation(MObject *owner, MRelation *relation);

    void removeRelation(MRelation *relation);

    void startUpdateRelation(MRelation *relation);

    void finishUpdateRelation(MRelation *relation, bool cancelled);

    void moveRelation(MObject *new_owner, MRelation *relation);

public:

    QList<MRelation *> findRelationsOfObject(const MObject *object) const;

public:

    MContainer cutElements(const MSelection &model_selection);

    MContainer copyElements(const MSelection &model_selection);

    void pasteElements(MObject *owner, const MContainer &model_container);

    void deleteElements(const MSelection &model_selection);

private:

    void deleteElements(const MSelection &model_selection, const QString &command_label);

    void removeRelatedRelations(MObject *object);

public:

    void unloadPackage(MPackage *package);

    void loadPackage(MPackage *package);

private:

    void renewElementKey(MElement *element, QHash<Uid, Uid> *renewed_keys);

    void updateRelationKeys(MElement *element, const QHash<Uid, Uid> &renewed_keys);

    void updateRelationEndKeys(MRelation *relation, const QHash<Uid, Uid> &renewed_keys);

    void mapObject(MObject *object);

    void unmapObject(MObject *object);

    void mapRelation(MRelation *relation);

    void unmapRelation(MRelation *relation);

    MReferences simplify(const MSelection &model_selection);

    void verifyModelIntegrity() const;

    void verifyModelIntegrity(const MObject *object, QHash<Uid, const MObject *> *objects_map, QHash<Uid,
                              const MRelation *> *relations_map,
                              QMultiHash<Uid, MRelation *> *object_relations_map) const;

private:

    MPackage *_root_package;

    UndoController *_undo_controller;

    QHash<Uid, MObject *> _objects_map;

    QHash<Uid, MRelation *> _relations_map;

    QMultiHash<Uid, MRelation *> _object_relations_map;

    bool _resetting_model;

    QString _old_package_name;

};

}

#endif // QMT_MODELCONTROLLER_H
