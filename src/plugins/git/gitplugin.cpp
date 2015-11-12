/****************************************************************************
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
** This file is part of Qt Creator.
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
****************************************************************************/
#include "logchangedialog.h"
#include "mergetool.h"
#include "gitutils.h"

#include "gerrit/gerritplugin.h"
#include <coreplugin/documentmanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/infobar.h>
#include <coreplugin/locator/commandlocator.h>
#include <coreplugin/vcsmanager.h>
#include <coreplugin/messagebox.h>
#include <utils/mimetypes/mimedatabase.h>
#include <vcsbase/submitfilemodel.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/vcsoutputwindow.h>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QtPlugin>

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QScopedPointer>

#ifdef WITH_TESTS
#include <QTest>
#endif

Q_DECLARE_METATYPE(Git::Internal::FileStates)
using namespace Core;
using namespace Utils;
using namespace VcsBase;
namespace Git {
namespace Internal {

const unsigned minimumRequiredVersion = 0x010702;
const char RC_GIT_MIME_XML[] = ":/git/Git.mimetypes.xml";

const VcsBaseEditorParameters editorParameters[] = {
    OtherContent,
    "text/vnd.qtcreator.git.commandlog"},
{   LogOutput,
    "text/vnd.qtcreator.git.log"},
{   AnnotateOutput,
    "text/vnd.qtcreator.git.annotation"},
{   OtherContent,
    Git::Constants::GIT_COMMIT_TEXT_EDITOR_ID,
    Git::Constants::GIT_COMMIT_TEXT_EDITOR_DISPLAY_NAME,
    "text/vnd.qtcreator.git.commit"},
{   OtherContent,
    Git::Constants::GIT_REBASE_EDITOR_ID,
    Git::Constants::GIT_REBASE_EDITOR_DISPLAY_NAME,
    "text/vnd.qtcreator.git.rebase"},
static GitPlugin *m_instance = 0;
    m_repositoryActions.reserve(50);
const VcsBaseSubmitEditorParameters submitParameters = {
    VcsBaseSubmitEditorParameters::DiffRows
ParameterAction *GitPlugin::createParameterAction(ActionContainer *ac,
                                                  const QString &defaultText, const QString &parameterText,
                                                  Id id, const Context &context,
                                                  bool addToLocator, const QKeySequence &keys)
{
    auto action = new ParameterAction(defaultText, parameterText, ParameterAction::EnabledWithParameter, this);
    Command *command = ActionManager::registerAction(action, id, context);
    if (!keys.isEmpty())
        command->setDefaultKeySequence(keys);
    command->setAttribute(Command::CA_UpdateText);
    return action;
QAction *GitPlugin::createFileAction(ActionContainer *ac,
                                     const QString &defaultText, const QString &parameterText,
                                     Id id, const Context &context, bool addToLocator,
                                     const char *pluginSlot, const QKeySequence &keys)
    ParameterAction *action = createParameterAction(ac, defaultText, parameterText, id, context, addToLocator, keys);
    m_fileActions.push_back(action);
    connect(action, SIGNAL(triggered()), this, pluginSlot);
    return action;
QAction *GitPlugin::createProjectAction(ActionContainer *ac,
                                        const QString &defaultText, const QString &parameterText,
                                        Id id, const Context &context, bool addToLocator,
                                        const char *pluginSlot, const QKeySequence &keys)
    ParameterAction *action = createParameterAction(ac, defaultText, parameterText, id, context, addToLocator, keys);
    m_projectActions.push_back(action);
    connect(action, SIGNAL(triggered()), this, pluginSlot);
    return action;
QAction *GitPlugin::createRepositoryAction(ActionContainer *ac,
                                           const QString &text, Id id,
                                           const Context &context, bool addToLocator,
                                           const QKeySequence &keys)
    auto action = new QAction(text, this);
    Command *command = ActionManager::registerAction(action, id, context);
    if (!keys.isEmpty())
        command->setDefaultKeySequence(keys);
    if (ac)
        ac->addAction(command);
    return action;
QAction *GitPlugin::createRepositoryAction(ActionContainer *ac,
                                           const QString &text, Id id,
                                           const Context &context, bool addToLocator,
                                           const std::function<void()> &callback, const QKeySequence &keys)
    QAction *action = createRepositoryAction(ac, text, id, context, addToLocator, keys);
    connect(action, &QAction::triggered, this, callback);
    return action;
}

QAction *GitPlugin::createChangeRelatedRepositoryAction(ActionContainer *ac,
                                                        const QString &text, Id id,
                                                        const Context &context, bool addToLocator,
                                                        const std::function<void(Id)> &callback,
                                                        const QKeySequence &keys)
{
    QAction *action = createRepositoryAction(ac, text, id, context, addToLocator, keys);
    connect(action, &QAction::triggered, this, [callback, id] { callback(id); });
    return action;
// taking the directory.
QAction *GitPlugin::createRepositoryAction(ActionContainer *ac,
                                           const QString &text, Id id,
                                           const Context &context, bool addToLocator,
                                           GitClientMemberFunc func, const QKeySequence &keys)
    QAction *action = createRepositoryAction(ac, text, id, context, addToLocator, keys);
    connect(action, &QAction::triggered, [this, func]() -> void {
        QTC_ASSERT(currentState().hasTopLevel(), return);
        (m_gitClient->*func)(currentState().topLevel());
    });
    return action;
    Context context(Constants::GIT_CONTEXT);
    m_gitClient = new GitClient;
    initializeVcs(new GitVersionControl(m_gitClient), context);
    addAutoReleasedObject(new SettingsPage(versionControl()));
    const int editorCount = sizeof(editorParameters) / sizeof(editorParameters[0]);
    const auto widgetCreator = []() { return new GitEditorWidget; };
        addAutoReleasedObject(new VcsEditorFactory(editorParameters + i, widgetCreator, m_gitClient, describeSlot));
    addAutoReleasedObject(new VcsSubmitEditorFactory(&submitParameters,
        []() { return new GitSubmitEditor(&submitParameters); }));
    m_commandLocator = new CommandLocator("Git", prefix, prefix);
    ActionContainer *toolsContainer = ActionManager::actionContainer(Core::Constants::M_TOOLS);
    ActionContainer *gitContainer = ActionManager::createMenu("Git");
    /*  "Current File" menu */
    ActionContainer *currentFileMenu = ActionManager::createMenu("Git.CurrentFileMenu");
    currentFileMenu->menu()->setTitle(tr("Current &File"));
    gitContainer->addMenu(currentFileMenu);
    createFileAction(currentFileMenu, tr("Diff Current File"), tr("Diff of \"%1\""),
                     "Git.Diff", context, true, SLOT(diffCurrentFile()),
                      QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+D") : tr("Alt+G,Alt+D")));
    createFileAction(currentFileMenu, tr("Log Current File"), tr("Log of \"%1\""),
                     "Git.Log", context, true, SLOT(logFile()),
                     QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+L") : tr("Alt+G,Alt+L")));
    createFileAction(currentFileMenu, tr("Blame Current File"), tr("Blame for \"%1\""),
                     "Git.Blame", context, true, SLOT(blameFile()),
                     QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+B") : tr("Alt+G,Alt+B")));
    currentFileMenu->addSeparator(context);
    createFileAction(currentFileMenu, tr("Stage File for Commit"), tr("Stage \"%1\" for Commit"),
                     "Git.Stage", context, true, SLOT(stageFile()),
                     QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+A") : tr("Alt+G,Alt+A")));
    createFileAction(currentFileMenu, tr("Unstage File from Commit"), tr("Unstage \"%1\" from Commit"),
                     "Git.Unstage", context, true, SLOT(unstageFile()));
    createFileAction(currentFileMenu, tr("Undo Unstaged Changes"), tr("Undo Unstaged Changes for \"%1\""),
                     "Git.UndoUnstaged", context,
                     true, SLOT(undoUnstagedFileChanges()));

    createFileAction(currentFileMenu, tr("Undo Uncommitted Changes"), tr("Undo Uncommitted Changes for \"%1\""),
                     "Git.Undo", context,
                     true, SLOT(undoFileChanges()),
                     QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+U") : tr("Alt+G,Alt+U")));


    /*  "Current Project" menu */
    ActionContainer *currentProjectMenu = ActionManager::createMenu("Git.CurrentProjectMenu");
    currentProjectMenu->menu()->setTitle(tr("Current &Project"));
    gitContainer->addMenu(currentProjectMenu);

    createProjectAction(currentProjectMenu, tr("Diff Current Project"), tr("Diff Project \"%1\""),
                        "Git.DiffProject", context, true, SLOT(diffCurrentProject()),
                        QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+Shift+D") : tr("Alt+G,Alt+Shift+D")));

    createProjectAction(currentProjectMenu, tr("Log Project"), tr("Log Project \"%1\""),
                        "Git.LogProject", context, true, SLOT(logProject()),
                        QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+K") : tr("Alt+G,Alt+K")));

    createProjectAction(currentProjectMenu, tr("Clean Project..."), tr("Clean Project \"%1\"..."),
                        "Git.CleanProject", context, true, SLOT(cleanProject()));

    /*  "Local Repository" menu */
    ActionContainer *localRepositoryMenu = ActionManager::createMenu("Git.LocalRepositoryMenu");
    localRepositoryMenu->menu()->setTitle(tr("&Local Repository"));
    gitContainer->addMenu(localRepositoryMenu);

    createRepositoryAction(localRepositoryMenu, tr("Diff"), "Git.DiffRepository",
                           context, true, [this] { diffRepository(); });

    createRepositoryAction(localRepositoryMenu, tr("Log"), "Git.LogRepository",
                           context, true, [this] { logRepository(); });

    createRepositoryAction(localRepositoryMenu, tr("Reflog"), "Git.ReflogRepository",
                           context, true, [this] { reflogRepository(); });

    createRepositoryAction(localRepositoryMenu, tr("Clean..."), "Git.CleanRepository",
                           context, true, [this] { cleanRepository(); });

    createRepositoryAction(localRepositoryMenu, tr("Status"), "Git.StatusRepository",
                           context, true, &GitClient::status);
    localRepositoryMenu->addSeparator(context);

    createRepositoryAction(localRepositoryMenu, tr("Commit..."), "Git.Commit",
                           context, true, [this] { startCommit(); },
                           QKeySequence(UseMacShortcuts ? tr("Meta+G,Meta+C") : tr("Alt+G,Alt+C")));
    createRepositoryAction(localRepositoryMenu,
                           tr("Amend Last Commit..."), "Git.AmendCommit",
                           context, true, [this] { startAmendCommit(); });
    m_fixupCommitAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Fixup Previous Commit..."), "Git.FixupCommit",
                                   context, true, [this] { startFixupCommit(); });
    // --------------
    localRepositoryMenu->addSeparator(context);

    createRepositoryAction(localRepositoryMenu,
                           tr("Reset..."), "Git.Reset",
                           context, true, [this] { resetRepository(); });

    m_interactiveRebaseAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Interactive Rebase..."), "Git.InteractiveRebase",
                                   context, true, [this] { startRebase(); });

    m_submoduleUpdateAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Update Submodules"), "Git.SubmoduleUpdate",
                                   context, true, [this] { updateSubmodules(); });
    m_abortMergeAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Abort Merge"), "Git.MergeAbort",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_abortRebaseAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Abort Rebase"), "Git.RebaseAbort",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_abortCherryPickAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Abort Cherry Pick"), "Git.CherryPickAbort",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_abortRevertAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Abort Revert"), "Git.RevertAbort",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_continueRebaseAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Continue Rebase"), "Git.RebaseContinue",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_continueCherryPickAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Continue Cherry Pick"), "Git.CherryPickContinue",
                                   context, true, [this] { continueOrAbortCommand(); });

    m_continueRevertAction =
            createRepositoryAction(localRepositoryMenu,
                                   tr("Continue Revert"), "Git.RevertContinue",
                                   context, true, [this] { continueOrAbortCommand(); });
    // --------------
    localRepositoryMenu->addSeparator(context);
    createRepositoryAction(localRepositoryMenu,
                           tr("Branches..."), "Git.BranchList",
                           context, true, [this] { branchList(); });
    localRepositoryMenu->addSeparator(context);
    // "Patch" menu
    ActionContainer *patchMenu = ActionManager::createMenu("Git.PatchMenu");
    patchMenu->menu()->setTitle(tr("&Patch"));
    localRepositoryMenu->addMenu(patchMenu);
    m_applyCurrentFilePatchAction =
            createParameterAction(patchMenu,
                                  "Git.ApplyCurrentFilePatch",
                                  context, true);

    connect(m_applyCurrentFilePatchAction, &QAction::triggered,
            this, &GitPlugin::applyCurrentFilePatch);

    createRepositoryAction(patchMenu,
                           tr("Apply from File..."), "Git.ApplyPatch",
                           context, true, [this] { promptApplyPatch(); });

    // "Stash" menu
    ActionContainer *stashMenu = ActionManager::createMenu("Git.StashMenu");
    stashMenu->menu()->setTitle(tr("&Stash"));
    localRepositoryMenu->addMenu(stashMenu);

    createRepositoryAction(stashMenu,
                           tr("Stashes..."), "Git.StashList",
                           context, false, [this] { stashList(); });

    stashMenu->addSeparator(context);

    QAction *action = createRepositoryAction(stashMenu, tr("Stash"), "Git.Stash",
                                             context, true, [this] { stash(); });
    action->setToolTip(tr("Saves the current state of your work and resets the repository."));

    action = createRepositoryAction(stashMenu, tr("Stash Unstaged Files"), "Git.StashUnstaged",
                                    context, true, [this] { stashUnstaged(); });
    action->setToolTip(tr("Saves the current state of your unstaged files and resets the repository "
                          "to its staged state."));

    action = createRepositoryAction(stashMenu, tr("Take Snapshot..."), "Git.StashSnapshot",
                                    context, true, [this] { stashSnapshot(); });
    action->setToolTip(tr("Saves the current state of your work."));

    stashMenu->addSeparator(context);

    action = createRepositoryAction(stashMenu, tr("Stash Pop"), "Git.StashPop",
                                    context, true, [this] { stashPop(); });
    action->setToolTip(tr("Restores changes saved to the stash list using \"Stash\"."));


    /* \"Local Repository" menu */

    // --------------

    /*  "Remote Repository" menu */
    ActionContainer *remoteRepositoryMenu = ActionManager::createMenu("Git.RemoteRepositoryMenu");
    remoteRepositoryMenu->menu()->setTitle(tr("&Remote Repository"));
    gitContainer->addMenu(remoteRepositoryMenu);

    createRepositoryAction(remoteRepositoryMenu, tr("Fetch"), "Git.Fetch",
                           context, true, [this] { fetch(); });

    createRepositoryAction(remoteRepositoryMenu, tr("Pull"), "Git.Pull",
                           context, true, [this] { pull(); });

    createRepositoryAction(remoteRepositoryMenu, tr("Push"), "Git.Push",
                           context, true, [this] { push(); });

    // --------------
    remoteRepositoryMenu->addSeparator(context);

    // "Subversion" menu
    ActionContainer *subversionMenu = ActionManager::createMenu("Git.Subversion");
    subversionMenu->menu()->setTitle(tr("&Subversion"));
    remoteRepositoryMenu->addMenu(subversionMenu);

    createRepositoryAction(subversionMenu,
                           tr("Log"), "Git.Subversion.Log",
                           context, false, &GitClient::subversionLog);

    createRepositoryAction(subversionMenu,
                           tr("Fetch"), "Git.Subversion.Fetch",
                           context, false, &GitClient::synchronousSubversionFetch);

    // --------------
    remoteRepositoryMenu->addSeparator(context);

    createRepositoryAction(remoteRepositoryMenu,
                           tr("Manage Remotes..."), "Git.RemoteList",
                           context, false, [this] { remoteList(); });

    /* \"Remote Repository" menu */

    // --------------

    /*  Actions only in locator */
    const auto startChangeRelated = [this](Id id) { startChangeRelatedAction(id); };
    createChangeRelatedRepositoryAction(0, tr("Show..."), "Git.Show",
                                        context, true, startChangeRelated);

    createChangeRelatedRepositoryAction(0, tr("Revert..."), "Git.Revert",
                                        context, true, startChangeRelated);

    createChangeRelatedRepositoryAction(0, tr("Cherry Pick..."), "Git.CherryPick",
                                        context, true, startChangeRelated);

    createChangeRelatedRepositoryAction(0, tr("Checkout..."), "Git.Checkout",
                                        context, true, startChangeRelated);

    createRepositoryAction(0, tr("Rebase..."), "Git.Rebase",
                           context, true, [this] { branchList(); });

    createRepositoryAction(0, tr("Merge..."), "Git.Merge",
                           context, true, [this] { branchList(); });

    /*  \Actions only in locator */

    // --------------

    /*  "Git Tools" menu */
    ActionContainer *gitToolsMenu = ActionManager::createMenu("Git.GitToolsMenu");
    gitToolsMenu->menu()->setTitle(tr("Git &Tools"));
    gitContainer->addMenu(gitToolsMenu);

    createRepositoryAction(gitToolsMenu,
                           tr("Gitk"), "Git.LaunchGitK",
                           context, true, &GitClient::launchGitK);

    createFileAction(gitToolsMenu, tr("Gitk Current File"), tr("Gitk of \"%1\""),
                     "Git.GitkFile", context, true, SLOT(gitkForCurrentFile()));

    createFileAction(gitToolsMenu, tr("Gitk for folder of Current File"), tr("Gitk for folder of \"%1\""),
                     "Git.GitkFolder", context, true, SLOT(gitkForCurrentFolder()));

    // --------------
    gitToolsMenu->addSeparator(context);

    createRepositoryAction(gitToolsMenu, tr("Git Gui"), "Git.GitGui",
                           context, true, [this] { gitGui(); });

    // --------------
    gitToolsMenu->addSeparator(context);

    m_repositoryBrowserAction =
            createRepositoryAction(gitToolsMenu,
                                   tr("Repository Browser"), "Git.LaunchRepositoryBrowser",
                                   context, true, &GitClient::launchRepositoryBrowser);

    m_mergeToolAction =
            createRepositoryAction(gitToolsMenu,
                                   tr("Merge Tool"), "Git.MergeTool",
                                   context, true, [this] { startMergeTool(); });

    /* \"Git Tools" menu */

    // --------------
    gitContainer->addSeparator(context);

    createRepositoryAction(gitContainer, tr("Actions on Commits..."), "Git.ChangeActions",
                           context, false, [this] { startChangeRelatedAction("Git.ChangeActions"); });

    m_createRepositryAction = new QAction(tr("Create Repository..."), this);
    Command *createRepositoryCommand = ActionManager::registerAction(
                m_createRepositryAction, "Git.CreateRepository");
    connect(m_createRepositryAction, &QAction::triggered, this, &GitPlugin::createRepository);
    gitContainer->addAction(createRepositoryCommand);
    Context submitContext(Constants::GITSUBMITEDITOR_ID);
    m_submitCurrentAction = new QAction(VcsBaseSubmitEditor::submitIcon(), tr("Commit"), this);
    Command *command = ActionManager::registerAction(m_submitCurrentAction, Constants::SUBMIT_CURRENT, submitContext);
    command->setAttribute(Command::CA_UpdateText);
    connect(m_submitCurrentAction, &QAction::triggered, this, &GitPlugin::submitCurrentLog);
    m_diffSelectedFilesAction = new QAction(VcsBaseSubmitEditor::diffIcon(), tr("Diff &Selected Files"), this);
    ActionManager::registerAction(m_diffSelectedFilesAction, Constants::DIFF_SELECTED, submitContext);
    ActionManager::registerAction(m_undoAction, Core::Constants::UNDO, submitContext);
    ActionManager::registerAction(m_redoAction, Core::Constants::REDO, submitContext);
    connect(VcsManager::instance(), &VcsManager::repositoryChanged,
            this, &GitPlugin::updateContinueAndAbortCommands);
    connect(VcsManager::instance(), &VcsManager::repositoryChanged,
            this, &GitPlugin::updateBranches, Qt::QueuedConnection);

    Utils::MimeDatabase::addMimeTypes(QLatin1String(RC_GIT_MIME_XML));

    /* "Gerrit" */
    m_gerritPlugin = new Gerrit::Internal::GerritPlugin(this);
    const bool ok = m_gerritPlugin->initialize(remoteRepositoryMenu);
    m_gerritPlugin->updateActions(currentState().hasTopLevel());
    m_gerritPlugin->addToLocator(m_commandLocator);

    return ok;
    m_gitClient->diffFiles(m_submitRepository, unstaged, staged);
}

void GitPlugin::submitEditorMerge(const QStringList &unmerged)
{
    m_gitClient->merge(m_submitRepository, unmerged);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_gitClient->diffFile(state.currentFileTopLevel(), state.relativeCurrentFile());
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasProject(), return);
    const QString relativeProject = state.relativeCurrentProject();
    if (relativeProject.isEmpty())
        m_gitClient->diffRepository(state.currentProjectTopLevel());
    else
        m_gitClient->diffProject(state.currentProjectTopLevel(), relativeProject);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->diffRepository(state.topLevel());
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_gitClient->log(state.currentFileTopLevel(), state.relativeCurrentFile(), true);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    const int lineNumber = VcsBaseEditor::lineNumberOfCurrentEditor(state.currentFile());
    m_gitClient->annotate(state.currentFileTopLevel(), state.relativeCurrentFile(), QString(), lineNumber);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasProject(), return);
void GitPlugin::logRepository()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->log(state.topLevel());
}

void GitPlugin::reflogRepository()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->reflog(state.topLevel());
}

    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    FileChangeBlocker fcb(state.currentFile());
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
class ResetItemDelegate : public LogItemDelegate
{
public:
    ResetItemDelegate(LogChangeWidget *widget) : LogItemDelegate(widget) {}
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        if (index.row() < currentRow())
            option->font.setStrikeOut(true);
        LogItemDelegate::initStyleOption(option, index);
    }
};

class RebaseItemDelegate : public IconItemDelegate
{
public:
    RebaseItemDelegate(LogChangeWidget *widget)
        : IconItemDelegate(widget, QLatin1String(Core::Constants::ICON_UNDO))
    {
    }

protected:
    bool hasIcon(int row) const override
    {
        return row <= currentRow();
    }
};

void GitPlugin::resetRepository()
{
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    QString topLevel = state.topLevel();

    LogChangeDialog dialog(true, ICore::mainWindow());
    ResetItemDelegate delegate(dialog.widget());
    dialog.setWindowTitle(tr("Undo Changes to %1").arg(QDir::toNativeSeparators(topLevel)));
    if (dialog.runDialog(topLevel, QString(), LogChangeWidget::IncludeRemotes))
        m_gitClient->reset(topLevel, dialog.resetFlag(), dialog.commit());
}

void GitPlugin::startRebase()
    if (!DocumentManager::saveAllModifiedDocuments())
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    const QString topLevel = state.topLevel();
    if (topLevel.isEmpty() || !m_gitClient->canRebase(topLevel))
        return;
    LogChangeDialog dialog(false, ICore::mainWindow());
    RebaseItemDelegate delegate(dialog.widget());
    dialog.setWindowTitle(tr("Interactive Rebase"));
    if (!dialog.runDialog(topLevel))
        return;
    if (m_gitClient->beginStashScope(topLevel, QLatin1String("Rebase-i")))
        m_gitClient->interactiveRebase(topLevel, dialog.commit(), false);
}

void GitPlugin::startChangeRelatedAction(const Id &id)
{
    const VcsBasePluginState state = currentState();
    if (!state.hasTopLevel())
        return;

    ChangeSelectionDialog dialog(state.topLevel(), id, ICore::mainWindow());

    int result = dialog.exec();

    if (result == QDialog::Rejected)
        return;

    const QString workingDirectory = dialog.workingDirectory();
    const QString change = dialog.change();

    if (workingDirectory.isEmpty() || change.isEmpty())
        return;

    if (dialog.command() == Show) {
        m_gitClient->show(workingDirectory, change);
        return;
    }

    if (!DocumentManager::saveAllModifiedDocuments())
        return;

    switch (dialog.command()) {
    case CherryPick:
        m_gitClient->synchronousCherryPick(workingDirectory, change);
        break;
    case Revert:
        m_gitClient->synchronousRevert(workingDirectory, change);
        break;
    case Checkout:
        m_gitClient->stashAndCheckout(workingDirectory, change);
        break;
    default:
        return;
    }
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
void GitPlugin::gitkForCurrentFile()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_gitClient->launchGitK(state.currentFileTopLevel(), state.relativeCurrentFile());
}

void GitPlugin::gitkForCurrentFolder()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);

    /*
     *  entire lower part of the code can be easily replaced with one line:
     *
     *  m_gitClient->launchGitK(dir.currentFileDirectory(), QLatin1String("."));
     *
     *  However, there is a bug in gitk in version 1.7.9.5, and if you run above
     *  command, there will be no documents listed in lower right section.
     *
     *  This is why I use lower combination in order to avoid this problems in gitk.
     *
     *  Git version 1.7.10.4 does not have this issue, and it can easily use
     *  one line command mentioned above.
     *
     */
    QDir dir(state.currentFileDirectory());
    if (QFileInfo(dir,QLatin1String(".git")).exists() || dir.cd(QLatin1String(".git"))) {
        m_gitClient->launchGitK(state.currentFileDirectory());
    } else {
        QString folderName = dir.absolutePath();
        dir.cdUp();
        folderName = folderName.remove(0, dir.absolutePath().length() + 1);
        m_gitClient->launchGitK(dir.absolutePath(), folderName);
    }
}

void GitPlugin::gitGui()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->launchGitGui(state.topLevel());
}

    startCommit(AmendCommit);
void GitPlugin::startFixupCommit()
    startCommit(FixupCommit);
void GitPlugin::startCommit()
    startCommit(SimpleCommit);
}
void GitPlugin::startCommit(CommitType commitType)
{
    if (raiseSubmitEditor())
        VcsOutputWindow::appendWarning(tr("Another submit is currently being executed."));
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    CommitData data(commitType);
    if (!m_gitClient->getCommitData(state.topLevel(), &commitTemplate, data, &errorMessage)) {
        VcsOutputWindow::appendError(errorMessage);
    TempFileSaver saver;
        VcsOutputWindow::appendError(saver.errorString());
    openSubmitEditor(m_commitMessageFileName, data);
}

void GitPlugin::updateVersionWarning()
{
    unsigned version = m_gitClient->gitVersion();
    if (!version || version >= minimumRequiredVersion)
        return;
    IDocument *curDocument = EditorManager::currentDocument();
    if (!curDocument)
        return;
    InfoBar *infoBar = curDocument->infoBar();
    Id gitVersionWarning("GitVersionWarning");
    if (!infoBar->canInfoBeAdded(gitVersionWarning))
        return;
    infoBar->addInfo(InfoBarEntry(gitVersionWarning,
                        tr("Unsupported version of Git found. Git %1 or later required.")
                        .arg(versionString(minimumRequiredVersion)),
                        InfoBarEntry::GlobalSuppressionEnabled));
IEditor *GitPlugin::openSubmitEditor(const QString &fileName, const CommitData &cd)
    IEditor *editor = EditorManager::openEditor(fileName, Constants::GITSUBMITEDITOR_ID);
    setSubmitEditor(submitEditor);
    QString title;
    switch (cd.commitType) {
    case AmendCommit:
        title = tr("Amend %1").arg(cd.amendSHA1);
        break;
    case FixupCommit:
        title = tr("Git Fixup Commit");
        break;
    default:
        title = tr("Git Commit");
    }
    IDocument *document = submitEditor->document();
    document->setPreferredDisplayName(title);
    VcsBasePlugin::setSource(document, m_submitRepository);
    connect(submitEditor, SIGNAL(merge(QStringList)), this, SLOT(submitEditorMerge(QStringList)));
    connect(submitEditor, SIGNAL(show(QString,QString)), m_gitClient, SLOT(show(QString,QString)));
    QTC_ASSERT(submitEditor(), return);
    EditorManager::closeDocument(submitEditor()->document());
bool GitPlugin::submitEditorAboutToClose()
    GitSubmitEditor *editor = qobject_cast<GitSubmitEditor *>(submitEditor());
    QTC_ASSERT(editor, return true);
    IDocument *editorDocument = editor->document();
    QTC_ASSERT(editorDocument, return true);
    const QFileInfo editorFile = editorDocument->filePath().toFileInfo();
    bool promptData = false;
    const VcsBaseSubmitEditor::PromptSubmitResult answer
            = editor->promptSubmit(tr("Closing Git Editor"),
                 tr("Do you want to commit the change?"),
                 tr("Git will not accept this commit. Do you want to continue to edit it?"),
                 &promptData, !m_submitActionTriggered, false);
    case VcsBaseSubmitEditor::SubmitCanceled:
    case VcsBaseSubmitEditor::SubmitDiscarded:


    SubmitFileModel *model = qobject_cast<SubmitFileModel *>(editor->fileModel());
    CommitType commitType = editor->commitType();
    QString amendSHA1 = editor->amendSHA1();
    if (model->hasCheckedFiles() || !amendSHA1.isEmpty()) {
        if (!DocumentManager::saveDocument(editorDocument))
        closeEditor = m_gitClient->addAndCommit(m_submitRepository, editor->panelData(),
                                                commitType, amendSHA1,
                                                m_commitMessageFileName, model);
    if (!closeEditor)
        return false;
    cleanCommitMessageFile();
    if (commitType == FixupCommit) {
        if (!m_gitClient->beginStashScope(m_submitRepository, QLatin1String("Rebase-fixup"),
                                          NoPrompt, editor->panelData().pushAction)) {
            return false;
        }
        m_gitClient->interactiveRebase(m_submitRepository, amendSHA1, true);
    } else {
        m_gitClient->continueCommandIfNeeded(m_submitRepository);
        if (editor->panelData().pushAction == NormalPush)
            m_gitClient->push(m_submitRepository);
        else if (editor->panelData().pushAction == PushToGerrit)
            connect(editor, SIGNAL(destroyed()), this, SLOT(delayedPushToGerrit()));
    }

    return true;
    m_gitClient->fetch(currentState().topLevel(), QString());
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    QString topLevel = state.topLevel();
    bool rebase = client()->settings().boolValue(GitSettings::pullRebaseKey);

    if (!rebase) {
        QString currentBranch = m_gitClient->synchronousCurrentLocalBranch(topLevel);
        if (!currentBranch.isEmpty()) {
            currentBranch.prepend(QLatin1String("branch."));
            currentBranch.append(QLatin1String(".rebase"));
            rebase = (m_gitClient->readConfigValue(topLevel, currentBranch) == QLatin1String("true"));
        }

    if (!m_gitClient->beginStashScope(topLevel, QLatin1String("Pull"), rebase ? Default : AllowUnstashed))
        return;
    m_gitClient->synchronousPull(topLevel, rebase);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->push(state.topLevel());
void GitPlugin::startMergeTool()
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->merge(state.topLevel());
void GitPlugin::continueOrAbortCommand()
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    QObject *action = QObject::sender();

    if (action == m_abortMergeAction)
        m_gitClient->synchronousMerge(state.topLevel(), QLatin1String("--abort"));
    else if (action == m_abortRebaseAction)
        m_gitClient->rebase(state.topLevel(), QLatin1String("--abort"));
    else if (action == m_abortCherryPickAction)
        m_gitClient->synchronousCherryPick(state.topLevel(), QLatin1String("--abort"));
    else if (action == m_abortRevertAction)
        m_gitClient->synchronousRevert(state.topLevel(), QLatin1String("--abort"));
    else if (action == m_continueRebaseAction)
        m_gitClient->rebase(state.topLevel(), QLatin1String("--continue"));
    else if (action == m_continueCherryPickAction)
        m_gitClient->cherryPick(state.topLevel(), QLatin1String("--continue"));
    else if (action == m_continueRevertAction)
        m_gitClient->revert(state.topLevel(), QLatin1String("--continue"));

    updateContinueAndAbortCommands();
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasProject(), return);
    const VcsBasePluginState state = currentState();
    QStringList ignoredFiles;
    const bool gotFiles = m_gitClient->synchronousCleanList(directory, &files, &ignoredFiles, &errorMessage);
        Core::AsynchronousMessageBox::warning(tr("Unable to retrieve file list"), errorMessage);
    if (files.isEmpty() && ignoredFiles.isEmpty()) {
        Core::AsynchronousMessageBox::information(tr("Repository Clean"),
                                                   tr("The repository is clean."));
    CleanDialog dialog(ICore::dialogParent());
    dialog.setFileList(directory, files, ignoredFiles);
void GitPlugin::updateSubmodules()
{
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_gitClient->updateSubmodulesIfNeeded(state.topLevel(), false);
}

    return DocumentManager::saveModifiedDocument(DocumentModel::documentForFilePath(fileName));
    const VcsBasePluginState state = currentState();
    const VcsBasePluginState state = currentState();
    if (!m_gitClient->beginStashScope(workingDirectory, QLatin1String("Apply-Patch"), AllowUnstashed))
        file = QFileDialog::getOpenFileName(ICore::mainWindow(), tr("Choose Patch"), QString(), filter);
        if (file.isEmpty()) {
            m_gitClient->endStashScope(workingDirectory);
        }
        if (errorMessage.isEmpty())
            VcsOutputWindow::appendMessage(tr("Patch %1 successfully applied to %2").arg(file, workingDirectory));
        else
            VcsOutputWindow::appendError(errorMessage);
        VcsOutputWindow::appendError(errorMessage);
    m_gitClient->endStashScope(workingDirectory);
void GitPlugin::stash(bool unstagedOnly)
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    const QString topLevel = state.topLevel();
    m_gitClient->executeSynchronousStash(topLevel, QString(), unstagedOnly);
    if (m_stashDialog)
        m_stashDialog->refresh(topLevel, true);
}

void GitPlugin::stashUnstaged()
{
    stash(true);
    const VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    const QString id = m_gitClient->synchronousStash(state.topLevel(), QString(),
                GitClient::StashImmediateRestore | GitClient::StashPromptDescription);
void GitPlugin::stashPop()
{
    if (!DocumentManager::saveAllModifiedDocuments())
        return;
    const QString repository = currentState().topLevel();
    m_gitClient->stashPop(repository);
    if (m_stashDialog)
        m_stashDialog->refresh(repository, true);
}

// Create a non-modal dialog with refresh function or raise if it exists
        dialog = new NonModalDialog(ICore::mainWindow());
    showNonModalDialog(currentState().topLevel(), m_branchDialog);
void GitPlugin::updateActions(VcsBasePlugin::ActionState as)
    m_createRepositryAction->setEnabled(true);
    if (repositoryEnabled)
        updateVersionWarning();
    foreach (ParameterAction *fileAction, m_fileActions)
    foreach (ParameterAction *projectAction, m_projectActions)
    m_submoduleUpdateAction->setVisible(repositoryEnabled
            && !m_gitClient->submoduleList(currentState().topLevel()).isEmpty());

    updateContinueAndAbortCommands();
    updateRepositoryBrowserAction();

    m_gerritPlugin->updateActions(repositoryEnabled);
void GitPlugin::updateContinueAndAbortCommands()
    if (currentState().hasTopLevel()) {
        GitClient::CommandInProgress gitCommandInProgress =
                m_gitClient->checkCommandInProgress(currentState().topLevel());

        m_mergeToolAction->setVisible(gitCommandInProgress != GitClient::NoCommand);
        m_abortMergeAction->setVisible(gitCommandInProgress == GitClient::Merge);
        m_abortCherryPickAction->setVisible(gitCommandInProgress == GitClient::CherryPick);
        m_abortRevertAction->setVisible(gitCommandInProgress == GitClient::Revert);
        m_abortRebaseAction->setVisible(gitCommandInProgress == GitClient::Rebase
                                        || gitCommandInProgress == GitClient::RebaseMerge);
        m_continueCherryPickAction->setVisible(gitCommandInProgress == GitClient::CherryPick);
        m_continueRevertAction->setVisible(gitCommandInProgress == GitClient::Revert);
        m_continueRebaseAction->setVisible(gitCommandInProgress == GitClient::Rebase
                                           || gitCommandInProgress == GitClient::RebaseMerge);
        m_fixupCommitAction->setEnabled(gitCommandInProgress == GitClient::NoCommand);
        m_interactiveRebaseAction->setEnabled(gitCommandInProgress == GitClient::NoCommand);
    } else {
        m_mergeToolAction->setVisible(false);
        m_abortMergeAction->setVisible(false);
        m_abortCherryPickAction->setVisible(false);
        m_abortRevertAction->setVisible(false);
        m_abortRebaseAction->setVisible(false);
        m_continueCherryPickAction->setVisible(false);
        m_continueRevertAction->setVisible(false);
        m_continueRebaseAction->setVisible(false);
    }
}
void GitPlugin::delayedPushToGerrit()
{
    m_gerritPlugin->push(m_submitRepository);
}
void GitPlugin::updateBranches(const QString &repository)
{
    if (m_branchDialog && m_branchDialog->isVisible())
        m_branchDialog->refreshIfSame(repository);
}
void GitPlugin::updateRepositoryBrowserAction()
{
    const bool repositoryEnabled = currentState().hasTopLevel();
    const bool hasRepositoryBrowserCmd
            = !client()->settings().stringValue(GitSettings::repositoryBrowserCmd).isEmpty();
    m_repositoryBrowserAction->setEnabled(repositoryEnabled && hasRepositoryBrowserCmd);
}
GitClient *GitPlugin::client() const
{
    return m_gitClient;
Gerrit::Internal::GerritPlugin *GitPlugin::gerritPlugin() const
    return m_gerritPlugin;
#ifdef WITH_TESTS

void GitPlugin::testStatusParsing_data()
    QTest::addColumn<FileStates>("first");
    QTest::addColumn<FileStates>("second");

    QTest::newRow(" M") << FileStates(ModifiedFile) << FileStates(UnknownFileState);
    QTest::newRow(" D") << FileStates(DeletedFile) << FileStates(UnknownFileState);
    QTest::newRow("M ") << (ModifiedFile | StagedFile) << FileStates(UnknownFileState);
    QTest::newRow("MM") << (ModifiedFile | StagedFile) << FileStates(ModifiedFile);
    QTest::newRow("MD") << (ModifiedFile | StagedFile) << FileStates(DeletedFile);
    QTest::newRow("A ") << (AddedFile | StagedFile) << FileStates(UnknownFileState);
    QTest::newRow("AM") << (AddedFile | StagedFile) << FileStates(ModifiedFile);
    QTest::newRow("AD") << (AddedFile | StagedFile) << FileStates(DeletedFile);
    QTest::newRow("D ") << (DeletedFile | StagedFile) << FileStates(UnknownFileState);
    QTest::newRow("DM") << (DeletedFile | StagedFile) << FileStates(ModifiedFile);
    QTest::newRow("R ") << (RenamedFile | StagedFile) << FileStates(UnknownFileState);
    QTest::newRow("RM") << (RenamedFile | StagedFile) << FileStates(ModifiedFile);
    QTest::newRow("RD") << (RenamedFile | StagedFile) << FileStates(DeletedFile);
    QTest::newRow("C ") << (CopiedFile | StagedFile) << FileStates(UnknownFileState);
    QTest::newRow("CM") << (CopiedFile | StagedFile) << FileStates(ModifiedFile);
    QTest::newRow("CD") << (CopiedFile | StagedFile) << FileStates(DeletedFile);
    QTest::newRow("??") << FileStates(UntrackedFile) << FileStates(UnknownFileState);

    // Merges
    QTest::newRow("DD") << (DeletedFile | UnmergedFile | UnmergedUs | UnmergedThem) << FileStates(UnknownFileState);
    QTest::newRow("AA") << (AddedFile | UnmergedFile | UnmergedUs | UnmergedThem) << FileStates(UnknownFileState);
    QTest::newRow("UU") << (ModifiedFile | UnmergedFile | UnmergedUs | UnmergedThem) << FileStates(UnknownFileState);
    QTest::newRow("AU") << (AddedFile | UnmergedFile | UnmergedUs) << FileStates(UnknownFileState);
    QTest::newRow("UD") << (DeletedFile | UnmergedFile | UnmergedThem) << FileStates(UnknownFileState);
    QTest::newRow("UA") << (AddedFile | UnmergedFile | UnmergedThem) << FileStates(UnknownFileState);
    QTest::newRow("DU") << (DeletedFile | UnmergedFile | UnmergedUs) << FileStates(UnknownFileState);
}

void GitPlugin::testStatusParsing()
{
    CommitData data;
    QFETCH(FileStates, first);
    QFETCH(FileStates, second);
    QString output = QLatin1String("## master...origin/master [ahead 1]\n");
    output += QString::fromLatin1(QTest::currentDataTag()) + QLatin1String(" main.cpp\n");
    data.parseFilesFromStatus(output);
    QCOMPARE(data.files.at(0).first, first);
    if (second == UnknownFileState)
        QCOMPARE(data.files.size(), 1);
    else
        QCOMPARE(data.files.at(1).first, second);
}

void GitPlugin::testDiffFileResolving_data()
{
    QTest::addColumn<QByteArray>("header");
    QTest::addColumn<QByteArray>("fileName");

    QTest::newRow("New") << QByteArray(
            "diff --git a/src/plugins/git/giteditor.cpp b/src/plugins/git/giteditor.cpp\n"
            "new file mode 100644\n"
            "index 0000000..40997ff\n"
            "--- /dev/null\n"
            "+++ b/src/plugins/git/giteditor.cpp\n"
            "@@ -0,0 +1,281 @@\n\n")
        << QByteArray("src/plugins/git/giteditor.cpp");
    QTest::newRow("Deleted") << QByteArray(
            "diff --git a/src/plugins/git/giteditor.cpp b/src/plugins/git/giteditor.cpp\n"
            "deleted file mode 100644\n"
            "index 40997ff..0000000\n"
            "--- a/src/plugins/git/giteditor.cpp\n"
            "+++ /dev/null\n"
            "@@ -1,281 +0,0 @@\n\n")
        << QByteArray("src/plugins/git/giteditor.cpp");
    QTest::newRow("Normal") << QByteArray(
            "diff --git a/src/plugins/git/giteditor.cpp b/src/plugins/git/giteditor.cpp\n"
            "index 69e0b52..8fc974d 100644\n"
            "--- a/src/plugins/git/giteditor.cpp\n"
            "+++ b/src/plugins/git/giteditor.cpp\n"
            "@@ -49,6 +49,8 @@\n\n")
        << QByteArray("src/plugins/git/giteditor.cpp");
}
void GitPlugin::testDiffFileResolving()
{
    VcsBaseEditorWidget::testDiffFileResolving(editorParameters[3].id);
void GitPlugin::testLogResolving()
    QByteArray data(
                "commit 50a6b54c03219ad74b9f3f839e0321be18daeaf6 (HEAD, origin/master)\n"
                "Merge: 3587b51 bc93ceb\n"
                "Author: Junio C Hamano <gitster@pobox.com>\n"
                "Date:   Fri Jan 25 12:53:31 2013 -0800\n"
                "\n"
                "   Merge branch 'for-junio' of git://bogomips.org/git-svn\n"
                "    \n"
                "    * 'for-junio' of git://bogomips.org/git-svn:\n"
                "      git-svn: Simplify calculation of GIT_DIR\n"
                "      git-svn: cleanup sprintf usage for uppercasing hex\n"
                "\n"
                "commit 3587b513bafd7a83d8c816ac1deed72b5e3a27e9\n"
                "Author: Junio C Hamano <gitster@pobox.com>\n"
                "Date:   Fri Jan 25 12:52:55 2013 -0800\n"
                "\n"
                "    Update draft release notes to 1.8.2\n"
                "    \n"
                "    Signed-off-by: Junio C Hamano <gitster@pobox.com>\n"
                );

    VcsBaseEditorWidget::testLogResolving(editorParameters[1].id, data,
                            "50a6b54c - Merge branch 'for-junio' of git://bogomips.org/git-svn",
                            "3587b513 - Update draft release notes to 1.8.2");
#endif
} // namespace Internal
} // namespace Git