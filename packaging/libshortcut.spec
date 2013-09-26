Name: libshortcut
Summary: Shortcut add feature supporting library
Version: 0.6.8
Release: 0
Group: HomeTF/Framework
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
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
%endif
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/opt/dbspace
touch %{buildroot}/opt/dbspace/.shortcut_service.db
touch %{buildroot}/opt/dbspace/.shortcut_service.db-journal

%post

%postun

%files -n libshortcut
%manifest libshortcut.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_prefix}/etc/package-manager/parserlib/*
%{_datarootdir}/license/*
%attr(640,root,app) /opt/dbspace/.shortcut_service.db
%attr(640,root,app) /opt/dbspace/.shortcut_service.db-journal

%files devel
%defattr(-,root,root,-)
%{_includedir}/shortcut/shortcut_PG.h
%{_includedir}/shortcut/shortcut.h
%{_libdir}/pkgconfig/shortcut.pc

# End of a file
