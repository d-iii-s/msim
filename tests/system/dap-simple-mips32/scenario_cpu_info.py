from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(GetCpuInfoRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=0x01)  # 0x01 is MIPS R4000 identifier
adp.send(GetConfigRequest).expect_response(StatusOk, arg0=1)  # Expect 1 CPU in the system

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()