/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
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

#include "environmentextension.h"

#include <language/scriptengine.h>
#include <logging/translator.h>
#include <tools/error.h>
#include <tools/fileinfo.h>
#include <tools/qbsassert.h>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScriptable>
#include <QScriptEngine>

namespace qbs {
namespace Internal {

class EnvironmentExtension : public QObject, QScriptable
{
    Q_OBJECT
public:
    void initializeJsExtensionEnvironment(QScriptValue extensionObject);
    static QScriptValue js_ctor(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue js_getEnv(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue js_putEnv(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue js_unsetEnv(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue js_currentEnv(QScriptContext *context, QScriptEngine *engine);
};

void initializeJsExtensionEnvironment(QScriptValue extensionObject)
{
    QScriptEngine *engine = extensionObject.engine();
    QScriptValue environmentObj = engine->newQMetaObject(&EnvironmentExtension::staticMetaObject,
                                             engine->newFunction(&EnvironmentExtension::js_ctor));
    environmentObj.setProperty(QStringLiteral("getEnv"),
                               engine->newFunction(EnvironmentExtension::js_getEnv, 1));
    environmentObj.setProperty(QStringLiteral("putEnv"),
                               engine->newFunction(EnvironmentExtension::js_putEnv, 2));
    environmentObj.setProperty(QStringLiteral("unsetEnv"),
                               engine->newFunction(EnvironmentExtension::js_unsetEnv, 1));
    environmentObj.setProperty(QStringLiteral("currentEnv"),
                               engine->newFunction(EnvironmentExtension::js_currentEnv, 0));
    extensionObject.setProperty(QStringLiteral("Environment"), environmentObj);
}

QScriptValue EnvironmentExtension::js_ctor(QScriptContext *context, QScriptEngine *engine)
{
    // Add instance variables here etc., if needed.
    Q_UNUSED(context);
    Q_UNUSED(engine);
    return true;
}

static QProcessEnvironment *getProcessEnvironment(QScriptContext *context, QScriptEngine *engine,
                                                  const QString &func, bool doThrow = true)
{
    QVariant v = engine->property("_qbs_procenv");
    QProcessEnvironment *procenv = reinterpret_cast<QProcessEnvironment *>(v.value<void *>());
    if (!procenv && doThrow)
        throw context->throwError(QScriptContext::UnknownError,
                                  QStringLiteral("%1 can only be called from ").arg(func) +
                                  QStringLiteral("Module.setupBuildEnvironment and ") +
                                  QStringLiteral("Module.setupRunEnvironment"));
    return procenv;
}

QScriptValue EnvironmentExtension::js_getEnv(QScriptContext *context, QScriptEngine *engine)
{
    if (Q_UNLIKELY(context->argumentCount() != 1))
        return context->throwError(QScriptContext::SyntaxError,
                                   QStringLiteral("getEnv expects 1 argument"));
    const QProcessEnvironment env = static_cast<ScriptEngine *>(engine)->environment();
    const QProcessEnvironment *procenv = getProcessEnvironment(context, engine,
                                                               QStringLiteral("getEnv"), false);
    if (!procenv)
        procenv = &env;

    const QString name = context->argument(0).toString();
    const QString value = procenv->value(name);

    // Don't track environment variable access inside environment setup scripts
    // since change tracking is irrelevant for them
    if (procenv == &env)
        static_cast<ScriptEngine *>(engine)->addEnvironmentVariable(name, value);

    return value.isNull() ? engine->undefinedValue() : value;
}

QScriptValue EnvironmentExtension::js_putEnv(QScriptContext *context, QScriptEngine *engine)
{
    if (Q_UNLIKELY(context->argumentCount() != 2))
        return context->throwError(QScriptContext::SyntaxError,
                                   QStringLiteral("putEnv expects 2 arguments"));
    getProcessEnvironment(context, engine, QStringLiteral("putEnv"))->insert(
                context->argument(0).toString(),
                context->argument(1).toString());
    return engine->undefinedValue();
}

QScriptValue EnvironmentExtension::js_unsetEnv(QScriptContext *context, QScriptEngine *engine)
{
    if (Q_UNLIKELY(context->argumentCount() != 1))
        return context->throwError(QScriptContext::SyntaxError,
                                   QStringLiteral("unsetEnv expects 1 argument"));
    getProcessEnvironment(context, engine, QStringLiteral("unsetEnv"))->remove(
                context->argument(0).toString());
    return engine->undefinedValue();
}

QScriptValue EnvironmentExtension::js_currentEnv(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    const QProcessEnvironment env = static_cast<ScriptEngine *>(engine)->environment();
    const QProcessEnvironment *procenv = getProcessEnvironment(context, engine,
                                                               QStringLiteral("currentEnv"), false);
    if (!procenv)
        procenv = &env;
    QScriptValue envObject = engine->newObject();
    foreach (const QString &key, procenv->keys())
        envObject.setProperty(key, QScriptValue(procenv->value(key)));
    return envObject;
}

static void printDeprecationWarning(const QString &message, const QScriptContext *context,
                                    QScriptEngine *engine)
{
    ErrorInfo fullError(message, context->backtrace());
    ErrorInfo error;
    if (fullError.items().count() == 1) {
        error = fullError;
    } else {
        QBS_CHECK(fullError.items().count() >= 2);
        error.append(fullError.items().first().description(),
                     fullError.items().at(1).codeLocation());
    }
    static_cast<ScriptEngine *>(engine)->logger().printWarning(error);
}

QScriptValue js_getEnvDeprecated(QScriptContext *context, QScriptEngine *qtengine)
{
    const QString message = Tr::tr("qbs.getEnv is deprecated and will be removed in Qbs 1.6. "
                                   "Use Environment.getEnv instead.");
    printDeprecationWarning(message, context, qtengine);
    return EnvironmentExtension::js_getEnv(context, qtengine);
}

QScriptValue js_currentEnvDeprecated(QScriptContext *context, QScriptEngine *qtengine)
{
    const QString message = Tr::tr("qbs.currentEnv is deprecated and will be removed in Qbs 1.6. "
                                   "Use Environment.currentEnv instead.");
    printDeprecationWarning(message, context, qtengine);
    return EnvironmentExtension::js_currentEnv(context, qtengine);
}

} // namespace Internal
} // namespace qbs

Q_DECLARE_METATYPE(qbs::Internal::EnvironmentExtension *)

#include "environmentextension.moc"
