if (CPU_64BIT_INSTRUCTION(cpu)) {
			if (urrt.val == 0) {
				cpu->loreg.val = 0;
				cpu->hireg.val = 0;
			} else {
				cpu->loreg.val = urrs.val / urrt.val;
				cpu->hireg.val = urrs.val % urrt.val;
			}
		} else
			res = excRI;