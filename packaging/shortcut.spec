#sbs-git:slp/pkgs/s/shortcut shortcut 0.0.5 ce4e4e05c8dd47d74ba65fe004b54288f337da73
Name:    shortcut
Summary: Shortcut (Add to home) service supporting library
Version: 0.0.5
Release: 1
Group:   devel
License: Apache
Source0: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)

BuildRequires: cmake
BuildRequires: gettext-tools

%description
Shortcut (Add to home) service supporting library

%package devel
Summary:    Shortcut (Add to home) service supporting library
Group:      devel
Requires:   %{name} = %{version}-%{release}

%description devel
Shortcut (Add to home) service supporting library

%prep
%setup -q

%build

cmake . -DCMAKE_INSTALL_PREFIX=/usr
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post

%postun


%files
%defattr(-,root,root,-)
%{_libdir}/libshortcut.so.0
%{_libdir}/libshortcut.so.0.0.1

%files devel 
%defattr(-,root,root,-)
/usr/include/shortcut/*.h
%{_libdir}/libshortcut.so
%{_libdir}/pkgconfig/shortcut.pc
