Summary: A virtual machine simulator based on a MIPS R4000 and RISC-V processor
Name: {{{ git_name name="msim-git" }}}
Version: {{{ git_version }}}
Release: 1%{?dist}
License: GPLv2+
Group: Development/Tools
URL: https://d3s.mff.cuni.cz/software/msim/
Source: {{{ git_pack }}}
Requires: readline

BuildRequires: readline-devel, makedepend, diffutils

%description
MSIM is a light-weight computer simulator based on MIPS R4000 and RISC-V.
It is used for education and research purposes, mainly to teach the construction
and implementation of operating systems. MSIM is distributed with source code under
the GNU GPL license to make possible modifications for users and works on most
POSIX-compliance environments (mainly GNU/Linux, Mac OS X, but can be also
compiled in Cygwin or MinGW in Windows). The user interface is simple
terminal-style.

%global debug_package %{nil}
%define pkgdocdir %{_datadir}/doc/%{name}-%{version}

%prep
%setup -q

%build
./configure --prefix=%{_prefix}
make

%install
rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{pkgdocdir}/
install -m 0644 -t $RPM_BUILD_ROOT%{pkgdocdir}/ doc/reference.html doc/default.css

%files
%defattr(-,root,root)
%doc %{pkgdocdir}
%{_bindir}/msim

%changelog
