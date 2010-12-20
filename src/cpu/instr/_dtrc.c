if (!totrace) {
			reg_view(cpu);
			printf("\n");
		}
		cpu_update_debug(cpu);
		totrace = true;