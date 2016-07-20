/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: bea.lam@jollamobile.com
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#ifndef QBLUEZ_EXPORT_H
#define QBLUEZ_EXPORT_H

#include <QtCore>

#if defined(BLUEZQT_LIBRARY)
#  define BLUEZQT_EXPORT Q_DECL_EXPORT
#else
#  define BLUEZQT_EXPORT Q_DECL_IMPORT
#endif

#endif // QBLUEZ_EXPORT_H
