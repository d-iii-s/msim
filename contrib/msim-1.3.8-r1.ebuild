DESCRIPTION="A virtual machine simulator based on a MIPS R4000 processor"
HOMEPAGE="http://d3s.mff.cuni.cz/~holub/sw/msim/"
SRC_URI="http://d3s.mff.cuni.cz/~holub/sw/${PN}/${P}.tar.bz2"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="amd64 x86 ppc sparc"

IUSE=""
DEPEND="sys-libs/readline x11-misc/makedepend sys-apps/diffutils"
RDEPEND="sys-libs/readline"

src_compile() {
	econf || die "Configuration failed"
	emake || die "Compilation failed"
}

src_install() {
	dobin bin/msim
	dodoc doc/reference.html doc/default.css
}
