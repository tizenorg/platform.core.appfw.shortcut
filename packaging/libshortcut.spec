Name: libshortcut
Summary: Shortcut add feature supporting library
Version: 0.6.14
Release: 0
Group: HomeTF/Framework
License: Apache
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest

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
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(aul)

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
cp %{SOURCE1001} .

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
%endif
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
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
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_prefix}/etc/package-manager/parserlib/*
%{_datarootdir}/license/*
%attr(640,root,app) /opt/dbspace/.shortcut_service.db
%attr(640,root,app) /opt/dbspace/.shortcut_service.db-journal

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/shortcut/shortcut.h
%{_includedir}/shortcut/shortcut_private.h
%{_includedir}/shortcut/shortcut_manager.h
%{_libdir}/pkgconfig/shortcut.pc

# End of a file
