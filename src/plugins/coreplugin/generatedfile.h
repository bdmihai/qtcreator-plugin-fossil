/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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


#ifndef CORE_GENERATEDFILE_H
#define CORE_GENERATEDFILE_H

#include "coreplugin/id.h"

#include <QSharedDataPointer>
#include <QList>

namespace Core {
class GeneratedFilePrivate;

class CORE_EXPORT GeneratedFile
{
public:
    enum Attribute { // Open this file in editor
                     OpenEditorAttribute = 0x01,
                     // Open project
                     OpenProjectAttribute = 0x02,
                     /* File is generated by external scripts, do not write out,
                      * see BaseFileWizard::writeFiles() */
                     CustomGeneratorAttribute = 0x4,
                     /* File exists and the user indicated that he wants to keep it */
                     KeepExistingFileAttribute = 0x8,
                     /* Force overwriting of a file without asking the user to keep it */
                     ForceOverwrite = 0x10
                   };
    Q_DECLARE_FLAGS(Attributes, Attribute)

    GeneratedFile();
    explicit GeneratedFile(const QString &path);
    GeneratedFile(const GeneratedFile &);
    GeneratedFile &operator=(const GeneratedFile &);
    ~GeneratedFile();

    // Full path of the file should be created, or the suggested file name
    QString path() const;
    void setPath(const QString &p);

    // Contents of the file (UTF8)
    QString contents() const;
    void setContents(const QString &c);

    QByteArray binaryContents() const;
    void setBinaryContents(const QByteArray &c);

    // Defaults to false (Text file).
    bool isBinary() const;
    void setBinary(bool b);

    // Id of editor to open the file with
    Id editorId() const;
    void setEditorId(Id id);

    bool write(QString *errorMessage) const;

    Attributes attributes() const;
    void setAttributes(Attributes a);

private:
    QSharedDataPointer<GeneratedFilePrivate> m_d;
};

typedef QList<GeneratedFile> GeneratedFiles;

} // namespace Core

#endif // CORE_GENERATEDFILE_H
