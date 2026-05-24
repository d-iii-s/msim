from base import *

adp = Adapter(int(sys.argv[1]))

SSCRATCH = 0x140
MSCRATCH = 0x340
MTVEC = 0x305
MEPC = 0x341

for csr in (SSCRATCH, MSCRATCH, MTVEC, MEPC):
    adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=csr).expect_response(StatusOk, arg0=0x00)  # CSRs are 0 at reset
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

for csr in (SSCRATCH, MSCRATCH, MTVEC, MEPC):
    adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=csr).expect_response(StatusOk,
                                                                         arg0=0x12345678)  # CSRs should be unchanged

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
