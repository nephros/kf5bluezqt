Name:       kf5bluezqt
Summary:    KF5BluezQt - Qt wrapper for BlueZ 5 DBus API
Version:    5.24.0
Release:    1
Group:      System/Libraries
License:    LGPLv2.1
URL:        git://anongit.kde.org/bluez-qt.git
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires: bluez >= 5.0
Requires: bluez-obexd >= 5.0

%description
This package contains the KF5BluezQt library.

%package declarative
Summary:    Declarative plugin for kf5bluezqt
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description declarative
This package contains declarative plugin for kf5bluezqt

%package devel
Summary:    Development files for kf5bluezqt
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description devel
This package contains the development header files for kf5bluezqt

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libKF5BluezQt.so.*

%files declarative
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/org/kde/bluezqt

%files devel
%defattr(-,root,root,-)
%{_libdir}/libKF5BluezQt.so
%{_libdir}/pkgconfig/KF5BluezQt.pc
%{_includedir}/KF5/BluezQt/bluezqt/*.h

