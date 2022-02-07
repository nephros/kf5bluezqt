Name:       kf5bluezqt-bluez5
Summary:    KF5BluezQt - Qt wrapper for BlueZ 5 DBus API
Version:    5.34.0
Release:    1
License:    LGPLv2
URL:        https://github.com/sailfishos/kf5bluezqt
Source0:    %{name}-%{version}.tar.bz2
Provides:   kf5bluezqt
Conflicts:  kf5bluezqt-bluez4

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires: bluez5
Requires: bluez5-obexd

Patch1: 0001-Add-.pro-files.patch
Patch2: 0002-Add-MediaTransport-org.bluez.MediaTransport1-wrapper.patch
Patch3: 0003-Add-Manager-pairWithDevice-QString-to-pair-with-unkn.patch
Patch4: 0004-Check-for-object-validity-in-macros.patch
Patch5: 0005-Don-t-connect-to-signals-with-QVariantMapMap-paramet.patch
Patch6: 0006-Expose-adapter.connected-property.-Contributes-to-JB.patch
Patch7: 0007-Fix-crash-after-unloading-the-obex-manager.-Fixes-JB.patch
Patch8: 0008-Add-Manager-monitorObjectManagerInterfaces.-Contribu.patch
Patch9: 0009-Add-filtering-options-to-DeclarativeDevicesModel.-Co.patch

%description
This package contains the KF5BluezQt library.

%package declarative
Summary:    Declarative plugin for kf5bluezqt
Provides:   kf5bluezqt-declarative
Conflicts:  kf5bluezqt-bluez4-declarative
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description declarative
This package contains declarative plugin for kf5bluezqt

%package devel
Summary:    Development files for kf5bluezqt
Provides:   kf5bluezqt-devel
Conflicts:  kf5bluezqt-bluez4-devel
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description devel
This package contains the development header files for kf5bluezqt

%prep
%autosetup -p1 -n %{name}-%{version}/upstream

%build
%qmake5 KF5BLUEZQT_BLUEZ_VERSION=5
%make_build

%install
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license COPYING.LIB
%{_libdir}/libKF5BluezQt.so.*

%files declarative
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/org/kde/bluezqt

%files devel
%defattr(-,root,root,-)
%{_libdir}/libKF5BluezQt.so
%{_libdir}/pkgconfig/KF5BluezQt.pc
%{_includedir}/KF5/BluezQt/bluezqt/*.h

