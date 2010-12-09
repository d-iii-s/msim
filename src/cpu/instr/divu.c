if (urrt.lo == 0) {
			cpu->loreg.val = 0;
			cpu->hireg.val = 0;
		} else {
			cpu->loreg.val = sign_extend_32_64(urrs.lo / urrt.lo);
			cpu->hireg.val = sign_extend_32_64(urrs.lo % urrt.lo);
		}