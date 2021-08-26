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

#include "factory/dafciofactory.h"
#include "factory/dafciofactory_p.h"
#include "local/dlocalhelper.h"

#include "local/dlocalenumerator.h"
#include "local/dlocalwatcher.h"
#include "local/dlocaloperator.h"
#include "local/dlocalfile.h"

#include <QDebug>

USING_IO_NAMESPACE

DAfcIOFactoryPrivate::DAfcIOFactoryPrivate(DAfcIOFactory *q) : DIOFactoryPrivate(q)
{
}

DAfcIOFactoryPrivate::~DAfcIOFactoryPrivate()
{
}

QSharedPointer<DFileInfo> DAfcIOFactoryPrivate::doCreateFileInfo() const
{
    const QString &url = uri.url();

    return DLocalHelper::getFileInfo(url);
}

QSharedPointer<DFile> DAfcIOFactoryPrivate::doCreateFile() const
{
    return QSharedPointer<DLocalFile>(new DLocalFile(uri));
}

QSharedPointer<DEnumerator> DAfcIOFactoryPrivate::doCreateEnumerator() const
{
    return QSharedPointer<DLocalEnumerator>(new DLocalEnumerator(uri));
}

QSharedPointer<DWatcher> DAfcIOFactoryPrivate::doCreateWatcher() const
{
    return QSharedPointer<DLocalWatcher>(new DLocalWatcher(uri));
}

QSharedPointer<DOperator> DAfcIOFactoryPrivate::doCreateOperator() const
{
    return QSharedPointer<DLocalOperator>(new DLocalOperator(uri));
}

DAfcIOFactory::DAfcIOFactory(const QUrl &uri)
    : DIOFactory()
{
    d_ptr.reset(new DAfcIOFactoryPrivate(this));
    d_ptr->uri = uri;
}

DAfcIOFactory::~DAfcIOFactory()
{
}