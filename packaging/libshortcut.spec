Name: libshortcut
Summary: Shortcut add feature supporting library
Version: 0.3.16
Release: 0
Group: main/devel
License: Apache License
Source0: %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(vconf)

%description
[Shortcut] AddToHome feature supporting library for menu/home screen developers.

%package devel
Summary:    AddToHome feature supporting library development files
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
[Shortcut] AddToHome feature supporting library for menu/home screen developers
(dev).

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post

%postun

%files -n libshortcut
%manifest libshortcut.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_prefix}/etc/package-manager/parserlib/*
%{_datarootdir}/license/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/shortcut/shortcut_PG.h
%{_includedir}/shortcut/shortcut.h
%{_libdir}/pkgconfig/shortcut.pc

# End of a file
