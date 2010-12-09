if (CPU_64BIT_INSTRUCTION(cpu)) {
			if (urrt.val == 0) {
				cpu->loreg.val = 0;
				cpu->hireg.val = 0;
			} else {
				cpu->loreg.val = (uint64_t)
				    (((int64_t) urrs.val) / ((int64_t) urrt.val));
				cpu->hireg.val = (uint64_t)
				    (((int64_t) urrs.val) % ((int64_t) urrt.val));
			}
		} else
			res = excRI;