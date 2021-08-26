/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dfmio_global.h"
#include "local/dlocaloperator.h"
#include "local/dlocaloperator_p.h"
#include "local/dlocalhelper.h"

#include "core/doperator_p.h"

#include <gio/gio.h>

#include <QDebug>

USING_IO_NAMESPACE

DLocalOperatorPrivate::DLocalOperatorPrivate(DLocalOperator *q)
    : q_ptr(q)
{
}

DLocalOperatorPrivate::~DLocalOperatorPrivate()
{
}

bool DLocalOperatorPrivate::renameFile(const QString &new_name)
{
    Q_Q(DLocalOperator);

    const QUrl &uri = q->uri();

    GError *error = nullptr;

    const char *name = new_name.toStdString().c_str();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    GFile *gfile_ret = g_file_set_display_name(gfile, name, nullptr, &error);
    g_object_unref(gfile);

    if (!gfile_ret) {
        setErrorInfo(error);
        g_error_free(error);
        return false;
    }

    if (error)
        g_error_free(error);
    g_object_unref(gfile_ret);
    return true;
}

bool DLocalOperatorPrivate::copyFile(const QUrl &to, DOperator::CopyFlag flag)
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    const QUrl &from = uri;
    GFile *gfile_from = makeGFile(from);
    if (!gfile_from)
        return false;

    GFile *gfile_to = makeGFile(to);
    if (!gfile_to)
        return false;

    bool ret = g_file_copy(gfile_from, gfile_to, GFileCopyFlags(flag), nullptr, nullptr, nullptr, &error);
    if (!ret)
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(gfile_from);
    g_object_unref(gfile_to);
    return ret;
}

bool DLocalOperatorPrivate::moveFile(const QUrl &to, DOperator::CopyFlag flag)
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    const QUrl &from = uri;
    GFile *gfile_from = makeGFile(from);
    if (!gfile_from)
        return false;

    GFile *gfile_to = makeGFile(to);
    if (!gfile_to)
        return false;

    bool ret = g_file_move(gfile_from, gfile_to, GFileCopyFlags(flag), nullptr, nullptr, nullptr, &error);
    if (!ret)
        setErrorInfo(error);
    if (error)
        g_error_free(error);
    g_object_unref(gfile_from);
    g_object_unref(gfile_to);
    return ret;
}

bool DLocalOperatorPrivate::trashFile()
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    bool ret = g_file_trash(gfile, nullptr, &error);
    if (!ret)
        setErrorInfo(error);
    if (error)
        g_error_free(error);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::deleteFile()
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    bool ret = g_file_delete(gfile, nullptr, &error);
    if (!ret)
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::restoreFile()
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *file = makeGFile(uri);
    if (!file)
        return false;

    GFileInfo *gfileinfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NONE, nullptr, &error);
    g_object_unref(file);

    if (!gfileinfo) {
        setErrorInfo(error);
        g_error_free(error);
        return false;
    }
    if (error)
        g_error_free(error);

    const std::string &src_path = g_file_info_get_attribute_byte_string(gfileinfo, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);
    if (src_path.empty()) {
        g_object_unref(gfileinfo);
        return false;
    }

    QUrl url_dest;
    url_dest.setPath(QString::fromStdString(src_path.c_str()));
    url_dest.setScheme(QString("file"));

    bool ret = moveFile(url_dest, DOperator::CopyFlag::None);

    g_object_unref(gfileinfo);
    return ret;
}

bool DLocalOperatorPrivate::touchFile()
{
    // GFileCreateFlags
    // value:
    // G_FILE_CREATE_NONE
    // G_FILE_CREATE_PRIVATE
    // G_FILE_CREATE_REPLACE_DESTINATION

    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    // if file exist, return failed
    GFileOutputStream *stream = g_file_create(gfile, GFileCreateFlags::G_FILE_CREATE_REPLACE_DESTINATION, nullptr, &error);
    if (stream)
        g_object_unref(stream);
    else
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(gfile);
    return stream != nullptr;
}

bool DLocalOperatorPrivate::makeDirectory()
{
    Q_Q(DLocalOperator);

    // only create direct path
    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    bool ret = g_file_make_directory(gfile, nullptr, &error);
    if (!ret)
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::createLink(const QUrl &link)
{
    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    std::string str = link.toString().toStdString();
    const char *symlink_value = str.c_str();
    bool ret = g_file_make_symbolic_link(gfile, symlink_value, nullptr, &error);
    if (!ret)
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::setFileInfo(const DFileInfo &fileInfo)
{
    // GFileQueryInfoFlags value: G_FILE_QUERY_INFO_NONE/G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS

    Q_Q(DLocalOperator);

    GError *error = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);
    if (!gfile)
        return false;

    GFileInfo *file_info = DLocalHelper::getFileInfoFromDFileInfo(fileInfo);
    bool ret = g_file_set_attributes_from_info(gfile, file_info, GFileQueryInfoFlags::G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &error);
    if (!ret)
        setErrorInfo(error);

    if (error)
        g_error_free(error);
    g_object_unref(file_info);
    g_object_unref(gfile);
    return ret;
}

GFile *DLocalOperatorPrivate::makeGFile(const QUrl &url)
{
    return g_file_new_for_uri(url.toString().toStdString().c_str());
}

void DLocalOperatorPrivate::setErrorInfo(GError *gerror)
{
    //error.setCode(DFMIOErrorCode(gerror->code));

    qWarning() << QString::fromStdString(gerror->message);
}

DLocalOperator::DLocalOperator(const QUrl &uri) : DOperator(uri)
  , d_ptr(new DLocalOperatorPrivate(this))
{
    registerRenameFile(std::bind(&DLocalOperator::renameFile, this, std::placeholders::_1));
    registerCopyFile(std::bind(&DLocalOperator::copyFile, this, std::placeholders::_1, std::placeholders::_2));
    registerMoveFile(std::bind(&DLocalOperator::moveFile, this, std::placeholders::_1, std::placeholders::_2));

    registerTouchFile(std::bind(&DLocalOperator::touchFile, this));
    registerDeleteFile(std::bind(&DLocalOperator::deleteFile, this));
    registerRestoreFile(std::bind(&DLocalOperator::restoreFile, this));

    registerTouchFile(std::bind(&DLocalOperator::touchFile, this));
    registerMakeDirectory(std::bind(&DLocalOperator::makeDirectory, this));
    registerCreateLink(std::bind(&DLocalOperator::createLink, this, std::placeholders::_1));
    registerSetFileInfo(std::bind(&DLocalOperator::setFileInfo, this, std::placeholders::_1));
}

DLocalOperator::~DLocalOperator()
{
}

bool DLocalOperator::renameFile(const QString &newName)
{
    Q_D(DLocalOperator);

    return d->renameFile(newName);
}

bool DLocalOperator::copyFile(const QUrl &destUri, DOperator::CopyFlag flag)
{
    Q_D(DLocalOperator);

    return d->copyFile(destUri, flag);
}

bool DLocalOperator::moveFile(const QUrl &destUri, DOperator::CopyFlag flag)
{
    Q_D(DLocalOperator);

    return d->moveFile(destUri, flag);
}

bool DLocalOperator::trashFile()
{
    Q_D(DLocalOperator);

    return d->trashFile();
}

bool DLocalOperator::deleteFile()
{
    Q_D(DLocalOperator);

    return d->deleteFile();
}

bool DLocalOperator::restoreFile()
{
    Q_D(DLocalOperator);

    return d->restoreFile();
}

bool DLocalOperator::touchFile()
{
    Q_D(DLocalOperator);

    return d->touchFile();
}

bool DLocalOperator::makeDirectory()
{
    Q_D(DLocalOperator);

    return d->makeDirectory();
}

bool DLocalOperator::createLink(const QUrl &link)
{
    Q_D(DLocalOperator);

    return d->createLink(link);
}

bool DLocalOperator::setFileInfo(const DFileInfo &fileInfo)
{
    Q_D(DLocalOperator);

    return d->setFileInfo(fileInfo);
}