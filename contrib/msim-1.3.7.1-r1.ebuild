DESCRIPTION="A virtual machine simulator based on a MIPS R4000 processor"
HOMEPAGE="http://dsrg.mff.cuni.cz/~holub/sw/msim/"
SRC_URI="http://dsrg.mff.cuni.cz/~holub/sw/${PN}/${P}.tar.bz2"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="amd64 x86 ppc sparc"

IUSE=""
DEPEND="sys-libs/readline"
RDEPEND="${DEPEND}"

src_compile() {
	econf || die "Configuration failed"
	emake || die "Compilation failed"
}

src_install() {
	dobin src/msim
	dodoc doc/reference.html doc/default.css
}
