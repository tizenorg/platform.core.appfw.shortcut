Name: libshortcut
Summary:    Shortcut add feature supporting library
Version:    0.3.3
Release:    0
Group:      main/devel
License:    SAMSUNG
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake, gettext-tools
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(libxml-2.0)

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

%files
%defattr(-,root,root,-)
/usr/lib/*.so*
/usr/etc/package-manager/parserlib/*

%files devel
%defattr(-,root,root,-)
/usr/include/shortcut/shortcut_PG.h
/usr/include/shortcut/shortcut.h
/usr/lib/pkgconfig/shortcut.pc
