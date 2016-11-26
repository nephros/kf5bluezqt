/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Bea Lam <bea.lam@jolla.com>
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

#ifndef BLUEZQT_MACROS_BLUEZ4_H
#define BLUEZQT_MACROS_BLUEZ4_H

// Calls propertiesChanged() for a single property value change
#define INVOKE_PROPERTIES_CHANGED(interface, obj, property, value) {\
    QVariantMap changed; \
    QStringList invalidated; \
    if (value.isValid()) { \
        changed[property] = value; \
    } else { \
        invalidated << property; \
    } \
    if (obj) { \
        obj->propertiesChanged(interface, changed, invalidated); \
    } \
}

#endif // BLUEZQT_MACROS_H
