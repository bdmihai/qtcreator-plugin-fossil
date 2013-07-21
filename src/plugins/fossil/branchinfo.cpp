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

#include "branchinfo.h"

#include <QtCore/QDebug>

namespace Fossil {
namespace Internal {

BranchInfo::BranchInfo(const QString &name, bool isCurrent, BranchFlags flags) :
    m_name(name),
    m_isCurrent(isCurrent),
    m_flags(flags)
{
}

const QString &BranchInfo::name() const
{
    return m_name;
}

bool BranchInfo::isCurrent() const
{
    return m_isCurrent;
}

bool BranchInfo::isClosed() const
{
    return m_flags.testFlag(Closed);
}

bool BranchInfo::isPrivate() const
{
    return m_flags.testFlag(Private);
}

QDebug BranchInfo::printDebug(QDebug dbg) const
{
    dbg.nospace() << "("
        << m_name << ", "
        << m_isCurrent << ", "
        << m_flags
        << ")";

    return dbg.nospace();
}

} // namespace Internal
} // namespace Fossil