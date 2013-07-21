/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013, Artur Shepilko.
**
**  Based on Bazaar VCS plugin for Qt Creator by Hugues Delorme.
**
**  Permission is hereby granted, free of charge, to any person obtaining a copy
**  of this software and associated documentation files (the "Software"), to deal
**  in the Software without restriction, including without limitation the rights
**  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
**  copies of the Software, and to permit persons to whom the Software is
**  furnished to do so, subject to the following conditions:
**
**  The above copyright notice and this permission notice shall be included in
**  all copies or substantial portions of the Software.
**
**  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
**  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
**  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
**  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
**  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
**  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
**  THE SOFTWARE.
**************************************************************************/

#ifndef FOSSILCOMMITWIDGET_H
#define FOSSILCOMMITWIDGET_H

#include "ui_fossilcommitpanel.h"

#include <utils/submiteditorwidget.h>

QT_BEGIN_NAMESPACE
class QValidator;
QT_END_NAMESPACE


namespace Fossil {
namespace Internal {

class BranchInfo;

/*submit editor widget based on git SubmitEditor
  Some extra fields have been added to the standard SubmitEditorWidget,
  to help to conform to the commit style that is used by both git and Fossil*/

class FossilCommitWidget : public Utils::SubmitEditorWidget
{
    Q_OBJECT

public:
    explicit FossilCommitWidget(QWidget *parent = 0);

    void setFields(const QString &repoPath,
                   const BranchInfo &newBranch, const QStringList &tags, const QString &userName);

    QString newBranch() const;
    QStringList tags() const;
    QString committer() const;
    bool isPrivateOptionEnabled() const;

protected:
    bool canSubmit() const;

private slots:
    void branchChanged();

private:
    bool isValidBranch() const;

    const QChar m_spaceChar;
    const QChar m_commaChar;
    const QString m_tagsSeparator;

    QWidget *m_commitPanel;
    Ui::FossilCommitPanel m_commitPanelUi;
    QValidator *m_branchValidator;
};

} // namespace Internal
} // namespace Fossil

#endif // FOSSILCOMMITWIDGET_H