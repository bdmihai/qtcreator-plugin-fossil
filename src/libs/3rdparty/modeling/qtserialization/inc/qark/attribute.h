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

#ifndef QARK_ATTRIBUTE_H
#define QARK_ATTRIBUTE_H

#include "parameters.h"

#include <QString>

namespace qark {

template<typename T>
class Attr {
public:
    Attr(const QString &qualified_name, T *value)
        : _qualified_name(qualified_name),
          _value(value)
    {
    }

    Attr(const QString &qualified_name, T *value, const Parameters &parameters)
        : _qualified_name(qualified_name),
          _value(value),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    T *getValue() const { return _value; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    T *_value;
    Parameters _parameters;
};

template<typename T>
Attr<T * const> attr(const QString &qualified_name, T * const &value)
{
    return Attr<T * const>(qualified_name, &value);
}

template<typename T>
Attr<T * const> attr(const QString &qualified_name, T * const &value, const Parameters &parameters)
{
    return Attr<T * const>(qualified_name, &value, parameters);
}

template<typename T>
Attr<T> attr(const QString &qualified_name, T &value)
{
    return Attr<T>(qualified_name, &value);
}

template<typename T>
Attr<T> attr(const QString &qualified_name, T &value, const Parameters &parameters)
{
    return Attr<T>(qualified_name, &value, parameters);
}


template<class U, typename T>
class GetterAttr {
public:
    GetterAttr(const QString &qualified_name, const U &u, T (U::*getter)() const)
        : _qualified_name(qualified_name),
          _u(u),
          _getter(getter)
    {
    }

    GetterAttr(const QString &qualified_name, const U &u, T (U::*getter)() const, const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _getter(getter),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    const U &getObject() const { return _u; }

    T (U::*getGetter() const)() const { return _getter; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    const U &_u;
    T (U::*_getter)() const;
    Parameters _parameters;
};

template<class U, typename T>
GetterAttr<U, T> attr(const QString &qualified_name, const U &u, T (U::*getter)() const)
{
    return GetterAttr<U, T>(qualified_name, u, getter);
}

template<class U, typename T>
GetterAttr<U, T> attr(const QString &qualified_name, const U &u, T (U::*getter)() const, const Parameters &parameters)
{
    return GetterAttr<U, T>(qualified_name, u, getter, parameters);
}


template<class U, typename T>
class SetterAttr {
public:
    SetterAttr(const QString &qualified_name, U &u, void (U::*setter)(T))
        : _qualified_name(qualified_name),
          _u(u),
          _setter(setter)
    {
    }

    SetterAttr(const QString &qualified_name, U &u, void (U::*setter)(T), const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _setter(setter),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    U &getObject() const { return _u; }

    void (U::*getSetter() const)(T) { return _setter; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    U &_u;
    void (U::*_setter)(T);
    Parameters _parameters;
};

template<class U, typename T>
SetterAttr<U, T> attr(const QString &qualified_name, U &u, void (U::*setter)(T))
{
    return SetterAttr<U, T>(qualified_name, u, setter);
}

template<class U, typename T>
SetterAttr<U, T> attr(const QString &qualified_name, U &u, void (U::*setter)(T), const Parameters &parameters)
{
    return SetterAttr<U, T>(qualified_name, u, setter, parameters);
}


template<class U, typename T, typename V>
class GetterSetterAttr {
public:
    GetterSetterAttr(const QString &qualified_name, U &u, T (U::*getter)() const, void (U::*setter)(V))
        : _qualified_name(qualified_name),
          _u(u),
          _getter(getter),
          _setter(setter)
    {
    }

    GetterSetterAttr(const QString &qualified_name, U &u, T (U::*getter)() const, void (U::*setter)(V), const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _getter(getter),
          _setter(setter),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    U &getObject() const { return _u; }

    T (U::*getGetter() const)() const { return _getter; }

    void (U::*getSetter() const)(V) { return _setter; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    U &_u;
    T (U::*_getter)() const;
    void (U::*_setter)(V);
    Parameters _parameters;
};

template<class U, typename T, typename V>
GetterSetterAttr<U, T, V> attr(const QString &qualified_name, U &u, T (U::*getter)() const, void (U::*setter)(V))
{
    return GetterSetterAttr<U, T, V>(qualified_name, u, getter, setter);
}

template<class U, typename T, typename V>
GetterSetterAttr<U, T, V> attr(const QString &qualified_name, U &u, T (U::*getter)() const, void (U::*setter)(V), const Parameters &parameters)
{
    return GetterSetterAttr<U, T, V>(qualified_name, u, getter, setter, parameters);
}


template<class U, typename T>
class GetFuncAttr {
public:
    GetFuncAttr(const QString &qualified_name, U &u, T (*get_func)(const U &))
        : _qualified_name(qualified_name),
          _u(u),
          _get_func(get_func)
    {
    }

    GetFuncAttr(const QString &qualified_name, U &u, T (*get_func)(const U &), const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _get_func(get_func),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    U &getObject() const { return _u; }

    T (*getGetFunc() const)(const U &) { return _get_func; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    U &_u;
    T (*_get_func)(const U &);
    Parameters _parameters;
};

template<class U, typename T>
GetFuncAttr<U, T> attr(const QString &qualified_name, const U &u, T (*get_func)(const U &))
{
    return GetFuncAttr<U, T>(qualified_name, u, get_func);
}

template<class U, typename T>
GetFuncAttr<U, T> attr(const QString &qualified_name, const U &u, T (*get_func)(const U &), const Parameters &parameters)
{
    return GetFuncAttr<U, T>(qualified_name, u, get_func, parameters);
}


template<class U, typename T>
class SetFuncAttr {
public:
    SetFuncAttr(const QString &qualified_name, U &u, void (*set_func)(U &, T))
        : _qualified_name(qualified_name),
          _u(u),
          _set_func(set_func)
    {
    }

    SetFuncAttr(const QString &qualified_name, U &u, void (*set_func)(U &, T), const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _set_func(set_func),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    U &getObject() const { return _u; }

    void (*getSetFunc() const)(U &, T) { return _set_func; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    U &_u;
    void (*_set_func)(U &, T);
    Parameters _parameters;
};

template<class U, typename T>
SetFuncAttr<U, T> attr(const QString &qualified_name, U &u, void (*set_func)(U &, T))
{
    return SetFuncAttr<U, T>(qualified_name, u, set_func);
}

template<class U, typename T>
SetFuncAttr<U, T> attr(const QString &qualified_name, U &u, void (*set_func)(U &, T), const Parameters &parameters)
{
    return SetFuncAttr<U, T>(qualified_name, u, set_func, parameters);
}


template<class U, typename T, typename V>
class GetSetFuncAttr {
public:
    GetSetFuncAttr(const QString &qualified_name, U &u, T (*get_func)(const U &), void (*set_func)(U &, V))
        : _qualified_name(qualified_name),
          _u(u),
          _get_func(get_func),
          _set_func(set_func)
    {
    }

    GetSetFuncAttr(const QString &qualified_name, U &u, T (*get_func)(const U &), void (*set_func)(U &, V), const Parameters &parameters)
        : _qualified_name(qualified_name),
          _u(u),
          _get_func(get_func),
          _set_func(set_func),
          _parameters(parameters)
    {
    }

    const QString &getQualifiedName() const { return _qualified_name; }

    U &getObject() const { return _u; }

    T (*getGetFunc() const)(const U &) { return _get_func; }

    void (*getSetFunc() const)(U &, V) { return _set_func; }

    Parameters getParameters() const { return _parameters; }

private:
    QString _qualified_name;
    U &_u;
    T (*_get_func)(const U &);
    void (*_set_func)(U &, V);
    Parameters _parameters;
};

template<class U, typename T, typename V>
GetSetFuncAttr<U, T, V> attr(const QString &qualified_name, U &u, T (*get_func)(const U &), void (*set_func)(U &, V))
{
    return GetSetFuncAttr<U, T, V>(qualified_name, u, get_func, set_func);
}

template<class U, typename T, typename V>
GetSetFuncAttr<U, T, V> attr(const QString &qualified_name, U &u, T (*get_func)(const U &), void (*set_func)(U &, V), const Parameters &parameters)
{
    return GetSetFuncAttr<U, T, V>(qualified_name, u, get_func, set_func, parameters);
}

}

#endif // QARK_ATTRIBUTE_H
