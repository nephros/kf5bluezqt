Name:       kf5bluezqt-bluez4
Summary:    KF5BluezQt - Qt wrapper for BlueZ 5 DBus API (for BlueZ 4 backend)
Version:    5.24.0+git16
Release:    1
License:    LGPLv2
URL:        https://github.com/sailfishos/kf5bluezqt
Source0:    %{name}-%{version}.tar.bz2
Provides:   kf5bluezqt
Conflicts:  kf5bluezqt-bluez5

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires: bluez
Requires: obexd

Patch1:  0001-packaging-Add-rpm-spec-and-.pro-files.patch
Patch2:  0002-bluez-qt-Add-MediaTransport-org.bluez.MediaTransport.patch
Patch3:  0003-bluez-qt-Add-limited-support-for-a-BlueZ-4-backend.-.patch
Patch4:  0004-bluez-qt-Add-Manager-pairWithDevice-QString-to-pair-.patch
Patch5:  0005-bluez-qt-Provide-binary-compatibility-between-BlueZ-.patch
Patch6:  0006-bluez-qt-Implement-org.bluez.Agent.ConfirmModeChange.patch
Patch7:  0007-bluez-qt-Check-for-object-validity-in-macros.patch
Patch8:  0008-bluez-qt-Don-t-connect-to-signals-with-QVariantMapMa.patch
Patch9:  0009-bluez-qt-Expose-adapter.connected-property.-Contribu.patch
Patch10: 0010-bluez-qt-Fix-crash-after-unloading-the-obex-manager..patch
Patch11: 0011-bluez-qt-Add-Manager-monitorObjectManagerInterfaces..patch
Patch12: 0012-bluez-qt-Add-filtering-options-to-DeclarativeDevices.patch

%description
This package contains the KF5BluezQt library.

%package declarative
Summary:    Declarative plugin for kf5bluezqt (for BlueZ 4 backend)
Provides:   kf5bluezqt-declarative
Conflicts:  kf5bluezqt-bluez5-declarative
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description declarative
This package contains declarative plugin for kf5bluezqt

%package devel
Summary:    Development files for kf5bluezqt (for BlueZ 4 backend)
Provides:   kf5bluezqt-devel
Conflicts:  kf5bluezqt-bluez5-devel
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description devel
This package contains the development header files for kf5bluezqt

%prep
%autosetup -p1 -n %{name}-%{version}/upstream

%build
%qmake5 KF5BLUEZQT_BLUEZ_VERSION=4
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

