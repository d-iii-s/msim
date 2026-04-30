from base import *

adp = Adapter(int(sys.argv[1]))

BADVADDR = 0x08
ENTRYHI = 0x0a
CAUSE = 0x0d
EPC = 0x0e
CONFIG = 0x10
LLADDR = 0x11

for csr in (BADVADDR, ENTRYHI, CAUSE, EPC, CONFIG, LLADDR):
    adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=csr, arg2=0x00).expect_response(StatusOk)
    adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=csr).expect_response(StatusOk, arg0=0x00)
    adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=csr, arg2=0x12345678).expect_response(StatusOk)
    adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=csr, arg2=0x12345678).expect_response(StatusOk)  # Write twice
    adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=csr).expect_response(StatusOk, arg0=0x12345678)

# Step a bit and check that the PC is updated, but registers are unchanged
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(2), arg2=StoppedReasonStep)
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(4), arg2=StoppedReasonStep)

for csr in (BADVADDR, ENTRYHI, CAUSE, EPC, CONFIG, LLADDR):
    adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=csr).expect_response(StatusOk,
                                                                         arg0=0x12345678)  # CSRs should be unchanged

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()