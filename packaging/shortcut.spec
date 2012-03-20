Name:	    shortcut
Summary:    Shortcut add feature supporting library
Version:    0.0.5
Release:    0
Group:      main/devel
License:    SAMSUNG
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake, gettext-tools
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)

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
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post

%postun

%files
%defattr(-,root,root,-)
${_libdir}/*.so*

%files devel
%defattr(-,root,root,-)
/usr/include/shortcut/SLP_shortcut_PG.h
/usr/include/shortcut/shortcut.h
${_libdir}/pkgconfig/shortcut.pc
