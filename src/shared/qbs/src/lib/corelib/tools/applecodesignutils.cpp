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

#include "applecodesignutils.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QSslCertificate>
#include <QSslCertificateExtension>

#include <QtCore/private/qcore_mac_p.h>
#include <Security/Security.h>

#include <QDebug>

namespace qbs {
namespace Internal {

QByteArray smimeMessageContent(const QByteArray &data)
{
    QCFType<CMSDecoderRef> decoder = NULL;
    if (CMSDecoderCreate(&decoder) != noErr)
        return QByteArray();

    if (CMSDecoderUpdateMessage(decoder, data.constData(), data.size()) != noErr)
        return QByteArray();

    if (CMSDecoderFinalizeMessage(decoder) != noErr)
        return QByteArray();

    QCFType<CFDataRef> content = NULL;
    if (CMSDecoderCopyContent(decoder, &content) != noErr)
        return QByteArray();

    return QByteArray::fromCFData(content);
}

QVariantMap certificateInfo(const QByteArray &data)
{
    const QSslCertificate cert(data, QSsl::Der);

    // Also potentially useful, but these are for signing pkgs which aren't used here
    // 1.2.840.113635.100.4.9 - 3rd Party Mac Developer Installer: <name>
    // 1.2.840.113635.100.4.13 - Developer ID Installer: <name>
    for (const auto &extension : cert.extensions()) {
        if (extension.name() == QStringLiteral("extendedKeyUsage")) {
             if (!extension.value().toStringList().contains(QStringLiteral("Code Signing")))
                 return QVariantMap();
        }
    }

    const auto subjectInfo = [](const QSslCertificate &cert) {
        QVariantMap map;
        for (const auto &attr : cert.subjectInfoAttributes())
            map.insert(QString::fromLatin1(attr), cert.subjectInfo(attr).first());
        return map;
    };

    return {
        {QStringLiteral("SHA1"), cert.digest(QCryptographicHash::Sha1).toHex().toUpper()},
        {QStringLiteral("subjectInfo"), subjectInfo(cert)},
        {QStringLiteral("validBefore"), cert.effectiveDate()},
        {QStringLiteral("validAfter"), cert.expiryDate()}
    };
}

QVariantMap identitiesProperties()
{
    // Apple documentation states that the Sec* family of functions are not thread-safe on OS X
    // https://developer.apple.com/library/mac/documentation/Security/Reference/certifkeytrustservices/
    static QMutex securityMutex;
    QMutexLocker locker(&securityMutex);
    Q_UNUSED(locker);

    const void *keys[] = {kSecClass, kSecMatchLimit, kSecAttrCanSign};
    const void *values[] = {kSecClassIdentity, kSecMatchLimitAll, kCFBooleanTrue};
    QCFType<CFDictionaryRef> query = CFDictionaryCreate(kCFAllocatorDefault,
                                                        keys,
                                                        values,
                                                        sizeof(keys) / sizeof(keys[0]),
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
    QCFType<CFTypeRef> result = NULL;
    if (SecItemCopyMatching(query, &result) != errSecSuccess)
        return QVariantMap();

    QVariantMap items;
    const auto tryAppend = [&](SecIdentityRef identity) {
        if (!identity)
            return;

        QCFType<SecCertificateRef> certificate = NULL;
        if (SecIdentityCopyCertificate(identity, &certificate) != errSecSuccess)
            return;

        QCFType<CFDataRef> certificateData = SecCertificateCopyData(certificate);
        if (!certificateData)
            return;

        auto props = certificateInfo(QByteArray::fromRawCFData(certificateData));
        if (!props.isEmpty())
            items.insert(props[QStringLiteral("SHA1")].toString(), props);
    };

    if (CFGetTypeID(result) == SecIdentityGetTypeID()) {
        tryAppend((SecIdentityRef)result.operator const void *());
    } else if (CFGetTypeID(result) == CFArrayGetTypeID()) {
        for (CFIndex i = 0; i < CFArrayGetCount((CFArrayRef)result.operator const void *()); ++i)
            tryAppend((SecIdentityRef)CFArrayGetValueAtIndex(result.as<CFArrayRef>(), i));
    }

    return items;
}

} // namespace Internal
} // namespace qbs
