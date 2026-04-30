from base import *

adp = Adapter(int(sys.argv[1]))

STATUS = 12
CAUSE = 13
EPC = 14

INT_LINE = 0  # IP0 / IM0 (software interrupt line 0)
IP_MASK = 1 << (8 + INT_LINE)  # bit 8 in Cause
IM_MASK = IP_MASK  # bit 8 in Status
IE_MASK = 1  # bit 0 in Status
EXL_MASK = 0x2  # bit 1 in Status
ERL_MASK = 0x4  # bit 2 in Status

# BEV=1 (reset default): EXCEPTION_BOOT_BASE_ADDRESS(0xBFC00200) + EXCEPTION_OFFSET(0x180)
HANDLER = 0xFFFFFFFFBFC00380

# Enable interrupts: set IE + IM0, clear EXL + ERL (BEV stays 1)
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=STATUS)
_, _, status_val, _, _ = adp.receive()
status_new = (status_val | IE_MASK | IM_MASK) & ~(EXL_MASK | ERL_MASK)
adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=STATUS, arg2=status_new).expect_response()

# Move a bit further
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=4).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(4), arg2=StoppedReasonStep)

# Set breakpoint at handler to stop when we jump there
adp.send(SetCodeBreakpointRequest, arg0=HANDLER).expect_response()

# Raise software interrupt line 0
adp.send(RaiseInterruptRequest, arg0=DEFAULT_CPU, arg1=INT_LINE).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=HANDLER, arg2=StoppedReasonBreakpoint)

# Check that we jumped to the handler
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=HANDLER)

# IP0 should be pending in Cause
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=CAUSE)
_, _, cause_val, _, _ = adp.receive()
assert cause_val & IP_MASK != 0, f"IP0 should be pending in Cause: {hex(cause_val)}"

# EPC points to the instruction that was about to execute when interrupt was taken
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=EPC).expect_response(StatusOk, arg0=at(5))

# Clear the interrupt, check not pending
adp.send(ClearInterruptRequest, arg0=DEFAULT_CPU, arg1=INT_LINE).expect_response()
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=CAUSE)
_, _, cause_val, _, _ = adp.receive()
assert cause_val & IP_MASK == 0, f"IP0 should not be pending: {hex(cause_val)}"

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
