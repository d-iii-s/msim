static r4k_exc_t instr__warning(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (!machine_undefined) {
        alert("Undefined instruction (silent exception)");
        machine_undefined = true;
    }

    return r4k_excNone;
}
