from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(GetCpuInfoRequest, arg0=CPU0).expect_response(StatusOk, arg0=0x02)  # 0x02 is RISCV32 identifier
adp.send(GetCpuInfoRequest, arg0=CPU1).expect_response(StatusOk, arg0=0x03)  # 0x03 is RISCV64 identifier
adp.send(GetConfigRequest).expect_response(StatusOk, arg0=2)  # Expect 2 CPUs in the system

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()