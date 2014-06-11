Name:       libshortcut
Summary:    Shortcut add feature supporting library
Version:    0.6.11
Release:    0
Group:      Graphics & UI Framework/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: %{name}.manifest

Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:         tizen-platform-config-tools

BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(libtzplatform-config)

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
sed -i %{SOURCE1001} -e "s|TZ_SYS_DB|%TZ_SYS_DB|g"
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
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}%{TZ_SYS_DB}
touch %{buildroot}%{TZ_SYS_DB}/.shortcut_service.db
touch %{buildroot}%{TZ_SYS_DB}/.shortcut_service.db-journal

%post -n libshortcut -p /sbin/ldconfig

%postun -n libshortcut -p /sbin/ldconfig

%files -n libshortcut
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_prefix}/etc/package-manager/parserlib/*
%{_datarootdir}/license/*
%attr(640,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_DB}/.shortcut_service.db
%attr(640,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_DB}/.shortcut_service.db-journal

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/shortcut/shortcut_PG.h
%{_includedir}/shortcut/shortcut.h
%{_libdir}/pkgconfig/shortcut.pc

# End of a file
