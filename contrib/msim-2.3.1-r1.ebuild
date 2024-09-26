EAPI=8
DESCRIPTION="Light-weight MIPS R4000 and RISC-V system simulator"
HOMEPAGE="https://d3s.mff.cuni.cz/software/msim/"
SRC_URI="https://github.com/d-iii-s/${PN}/archive/refs/tags/v${PV}.tar.gz"
RESTRICT="mirror"

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
	dobin msim
	dodoc README.md AUTHORS CHANGELOG.md
}
