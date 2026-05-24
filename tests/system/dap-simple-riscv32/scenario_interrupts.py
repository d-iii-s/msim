from base import *

adp = Adapter(int(sys.argv[1]))

MSTATUS = 0x300
MIE = 0x304
MTVEC = 0x305
MEPC = 0x341
MIP = 0x344

MSI = 3  # Machine Software Interrupt
MIE_MASK = (1 << MSI)

HANDLER = at(PROGRAM_LEN + 2)  # Past the halt, only reachable via interrupt dispatch

# Setup interrupt handler (MTVEC)
adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=MTVEC, arg2=HANDLER).expect_response()

# Enable interrupts (MSTATUS)
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=MSTATUS)
category, kind, mstatus_value, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
mstatus_modified = mstatus_value | MIE_MASK  # Set MIE (Machine Interrupt Enable) bit
adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=MSTATUS, arg2=mstatus_modified).expect_response()

# Enable software interrupts (MIE)
adp.send(WriteCsrRequest, arg0=DEFAULT_CPU, arg1=MIE, arg2=MIE_MASK).expect_response()

# Move a bit further
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=4).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(4), arg2=StoppedReasonStep)

# Set breakpoint at the handler to stop when we jump there
adp.send(SetCodeBreakpointRequest, arg0=HANDLER).expect_response()

# Raise software interrupt
adp.send(RaiseInterruptRequest, arg0=DEFAULT_CPU, arg1=MSI).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=HANDLER, arg2=StoppedReasonBreakpoint)

# Check that we jumped to the handler and that the interrupt is pending in MIP
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=HANDLER)
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=MIP)
category, kind, mip_value, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value & MIE_MASK != 0, "Interrupt should be pending in MIP"

# MEPC should point to the instruction we were executing when the interrupt was raised
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=MEPC).expect_response(StatusOk, arg0=at(5))

# Clear the interrupt, check not pending
adp.send(ClearInterruptRequest, arg0=DEFAULT_CPU, arg1=MSI).expect_response()
adp.send(ReadCsrRequest, arg0=DEFAULT_CPU, arg1=MIP)
category, kind, mip_value, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value & MIE_MASK == 0, "Interrupt should not be pending in MIP"

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
