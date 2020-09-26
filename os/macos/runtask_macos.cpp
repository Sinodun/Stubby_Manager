/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <assert.h>

#include <Security/AuthorizationDB.h>

#include <QDebug>
#include <QRegularExpression>

#include "runtask_macos.h"


RunTaskMacos::RunTaskMacos(const QString command, QObject *parent) :
    QProcess(parent),
    m_command(command)
{
    setProgram(m_command);
}

RunTaskMacos::~RunTaskMacos()
{
    if (state() == Running) {
        terminate();
        waitForFinished();
    }
}

void RunTaskMacos::start()
{
    QProcess::start(m_command, ReadWrite);
}

//static const char STUBBY_UI_HELPER[] = "/usr/local/sbin/stubby-ui-helper";

// TODO: MUST BE UPDATED ON INSTALL
static const char STUBBY_UI_HELPER[] = "/Applications/StubbyManager.app/Contents/MacOS/stubby-ui-helper";


const char *RunHelperTaskMacos::RIGHT_DAEMON_RUN = "net.getdnsapi.stubby.daemon.run";
const char *RunHelperTaskMacos::RIGHT_DNS_LOCAL = "net.getdnsapi.stubby.dns.local";

RunHelperTaskMacos::RunHelperTaskMacos(const QString command, QString need_right, const QString config, QObject *parent, MainWindow *mainwindow) :
    QProcess(parent),
    m_command(command),
    m_need_right(need_right),
    m_config(config),
    m_mainwindow(mainwindow)
{
    setProgram(STUBBY_UI_HELPER);

    OSStatus oss = AuthorizationCreate(NULL, NULL, 0, &m_auth_ref);
    assert(oss == errAuthorizationSuccess);
    //qDebug() << __FILE__ << ":" << __FUNCTION__ << "Auth create " << (oss == errAuthorizationSuccess);

    // Ensure rights have been created.
    oss = AuthorizationRightGet(RIGHT_DAEMON_RUN, NULL);
    assert(oss == errAuthorizationSuccess);
    //qDebug() << __FILE__ << ":" << __FUNCTION__ << "Auth daemon " << (oss == errAuthorizationSuccess);
    oss = AuthorizationRightGet(RIGHT_DNS_LOCAL, NULL);
    assert(oss == errAuthorizationSuccess);
    //qDebug() << __FILE__ << ":" << __FUNCTION__ << "Auth dns " << (oss == errAuthorizationSuccess);
}

RunHelperTaskMacos::~RunHelperTaskMacos()
{
    AuthorizationFree(m_auth_ref, kAuthorizationFlagDefaults);

    //qDebug() << __FILE__ << ":" << __FUNCTION__;
    if (state() == Running) {
        terminate();
        waitForFinished();
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "waited for finish";
    }
}

int RunHelperTaskMacos::execute()
{
    QString cmd = makeCommandLine();
    if (cmd.isNull()) {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "auth failed";
        return 1;
    }
    else {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "executing " << cmd;
        int result = QProcess::execute(cmd);
        qInfo("Result of cmd execution is %d", result);
        if (result == -2)
            qDebug() << __FILE__ << ":" << __FUNCTION__ << "Command could not be executed, process not found (-2)" << cmd;
        if (result == -1)
            qDebug() << __FILE__ << ":" << __FUNCTION__ << "Command returned -1 " << cmd;
        return result;
    }
}

void RunHelperTaskMacos::start()
{
    QString cmd = makeCommandLine();
    if (cmd.isNull())
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "auth failed";
    else {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "starting " << cmd;
        QProcess::start(cmd, ReadWrite);
    }
}

QString RunHelperTaskMacos::makeCommandLine()
{
    QString cmd(STUBBY_UI_HELPER);

    if ( !m_need_right.isEmpty() ) {
        AuthorizationItem right_detail[] = {
            { NULL, 0, NULL, 0 },
        };
        AuthorizationRights the_right = { 1, &right_detail[0] };
        OSStatus oss;
        QByteArray needed_right = m_need_right.toUtf8();
        right_detail[0].name = needed_right.constData();

        oss = AuthorizationCopyRights(
                m_auth_ref,
                &the_right,
                kAuthorizationEmptyEnvironment,
                kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize,
                NULL);
        if (oss != errAuthorizationSuccess)
            return NULL;

        QString ext_auth = makeExternalAuth();
        if (ext_auth.isEmpty())
            return NULL;

        cmd += " -auth ";
        cmd += ext_auth;
    }

    if ( !m_config.isEmpty() ) {
        cmd += " -config ";
        cmd += m_config;
    }

    cmd += " " + m_command;
    return cmd;
}

QString RunHelperTaskMacos::makeExternalAuth()
{
    QString res;
    AuthorizationExternalForm auth_ext_form;

    OSStatus oss = AuthorizationMakeExternalForm(m_auth_ref, &auth_ext_form);
    if ( oss == errAuthorizationSuccess)
        for (size_t i = 0; i < kAuthorizationExternalFormLength; ++i) {
            // Turn into a printable string of hex digits.
            char b = auth_ext_form.bytes[i];
            char c;

            c = (b >> 4) & 0xf;
            res.append(c >= 10 ? 'a' + c - 10 : '0' + c);
            c = b & 0xf;
            res.append(c >= 10 ? 'a' + c - 10 : '0' + c);
        }
    return res;
}
