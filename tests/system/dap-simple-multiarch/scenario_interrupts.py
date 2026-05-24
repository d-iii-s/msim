from base import *

adp = Adapter(int(sys.argv[1]))

MSTATUS = 0x300
MIE = 0x304
MTVEC = 0x305
MEPC = 0x341
MIP = 0x344

MSI = 3  # Machine Software Interrupt
MIE_MASK = (1 << MSI)

HANDLER0 = at(PROGRAM_LEN + 2)
HANDLER1 = at(PROGRAM_LEN + 3)

# Setup both CPUs with unique handlers
for cpu, handler in ((CPU0, HANDLER0), (CPU1, HANDLER1)):
    # Setup interrupt handler (MTVEC)
    adp.send(WriteCsrRequest, arg0=cpu, arg1=MTVEC, arg2=handler).expect_response()

    # Enable interrupts (MSTATUS)
    adp.send(ReadCsrRequest, arg0=cpu, arg1=MSTATUS)
    category, kind, mstatus_value, _, _ = adp.receive()
    assert category == ResponseCategory and kind == StatusOk
    mstatus_modified = mstatus_value | MIE_MASK  # Set MIE (Machine Interrupt Enable) bit
    adp.send(WriteCsrRequest, arg0=cpu, arg1=MSTATUS, arg2=mstatus_modified).expect_response()

    # Enable software interrupts (MIE)
    adp.send(WriteCsrRequest, arg0=cpu, arg1=MIE, arg2=MIE_MASK).expect_response()

    # Set breakpoint at the handler to stop when we jump there
    adp.send(SetCodeBreakpointRequest, arg0=handler).expect_response()

# Move a bit further
adp.send(StepRequest, arg0=CPU0, arg1=4).expect_response()
adp.send(StepRequest, arg0=CPU1, arg1=4).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(4), arg2=StoppedReasonStep)
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(4), arg2=StoppedReasonStep)

# Raise software interrupt on CPU0
adp.send(RaiseInterruptRequest, arg0=CPU0, arg1=MSI).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=HANDLER0, arg2=StoppedReasonBreakpoint)

# Check that we jumped to the handler and that the interrupt is pending in MIP CPU0
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=HANDLER0)
adp.send(ReadCsrRequest, arg0=CPU0, arg1=MIP)
category, kind, mip_value0, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value0 & MIE_MASK != 0, "Interrupt should be pending in MIP CPU0"

# Check that CPU1 is unaffected
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(5))
adp.send(ReadCsrRequest, arg0=CPU1, arg1=MIP)
category, kind, mip_value1, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value1 & MIE_MASK == 0, "Interrupt should not be pending in MIP CPU1"

# MEPC should point to the instruction we were executing when the interrupt was raised
adp.send(ReadCsrRequest, arg0=CPU0, arg1=MEPC).expect_response(StatusOk, arg0=at(5))

# Raise software interrupt on CPU1
adp.send(RaiseInterruptRequest, arg0=CPU1, arg1=MSI).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=HANDLER0, arg2=StoppedReasonBreakpoint) # CPU0 standing
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=HANDLER1, arg2=StoppedReasonBreakpoint)

# Check that we jumped to the handler and that the interrupt is pending in MIP CPU1
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=HANDLER1)
adp.send(ReadCsrRequest, arg0=CPU1, arg1=MIP)
category, kind, mip_value1, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value1 & MIE_MASK != 0, "Interrupt should be pending in MIP CPU1"

# Check that CPU0 is unaffected (its interrupt still pending)
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=HANDLER0)
adp.send(ReadCsrRequest, arg0=CPU0, arg1=MIP)
category, kind, mip_value0, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value0 & MIE_MASK != 0, "Interrupt should still be pending in MIP CPU0"

# MEPC should point to the instruction we were executing when the interrupt was raised
adp.send(ReadCsrRequest, arg0=CPU1, arg1=MEPC).expect_response(StatusOk, arg0=at(6))

# Clear the interrupt CPU0, check not pending
adp.send(ClearInterruptRequest, arg0=CPU0, arg1=MSI).expect_response()
adp.send(ReadCsrRequest, arg0=CPU0, arg1=MIP)
category, kind, mip_value, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value & MIE_MASK == 0, "Interrupt should not be pending in MIP"

# Check that CPU1 is unaffected
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=HANDLER1)
adp.send(ReadCsrRequest, arg0=CPU1, arg1=MIP)
category, kind, mip_value1, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value1 & MIE_MASK != 0, "Interrupt should still be pending in MIP CPU1"

# Clear the interrupt CPU1, check not pending
adp.send(ClearInterruptRequest, arg0=CPU1, arg1=MSI).expect_response()
adp.send(ReadCsrRequest, arg0=CPU1, arg1=MIP)
category, kind, mip_value, _, _ = adp.receive()
assert category == ResponseCategory and kind == StatusOk
assert mip_value & MIE_MASK == 0, "Interrupt should not be pending in MIP"

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
