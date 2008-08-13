# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit qt4

DESCRIPTION="A tool for handling the Akai EWI4000s Wind Synthesizer"
HOMEPAGE="http://code.google.com/p/ewitool/"
SRC_URI="http://ewitool.googlecode.com/files/EWItool-${PV}.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~x86 ~amd64"

DEPEND=">=x11-libs/qt-4
		media-libs/alsa-lib"

pkg_setup() {
	if ! built_with_use media-libs/alsa-lib midi; then
		eerror "media-libs/alsa-lib needs to be built with midi use flag"
		eerror "in order to use ${PN}"
		die "Please rebuild media-libs/alsa-lib with midi use flag"
	fi
}

src_compile() {
	cd EWItool
	eqmake4 || die "qmake failed"
	emake || die "make failed"
}

src_install() {
	dobin EWItool/bin/ewitool
}
