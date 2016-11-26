/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Bea Lam <bea.lam@jolla.com>
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This file is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this file; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
