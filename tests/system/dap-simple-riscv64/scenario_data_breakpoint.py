from base import *

adp = Adapter(int(sys.argv[1]))

READ = 0x01
SIZE = INSTR_LEN

# Set breakpoints at the end of the program, we will remove later
adp.send(SetDataBreakpointRequest, arg0=at(7), arg1=READ, arg2=SIZE).expect_response()
adp.send(SetDataBreakpointRequest, arg0=at(8), arg1=READ, arg2=SIZE).expect_response()

# Remove a non-existent breakpoint
adp.send(RemoveDataBreakpointRequest, arg0=at(6), arg1=READ, arg2=SIZE).expect_response()

# Set a breakpoint, run the program, and check that we stop at the right place
adp.send(SetDataBreakpointRequest, arg0=at(3), arg1=READ, arg2=SIZE).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(3), arg2=StoppedReasonBreakpoint)
# the instruction fetched is still executed before we stop
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=at(4))

# Set a breakpoint behind current PC, should never get hit
adp.send(SetDataBreakpointRequest, arg0=at(0), arg1=READ, arg2=SIZE).expect_response()

# Set two breakpoints in front of PC
adp.send(SetDataBreakpointRequest, arg0=at(5), arg1=READ, arg2=SIZE).expect_response()
adp.send(SetDataBreakpointRequest, arg0=at(6), arg1=READ, arg2=SIZE).expect_response()

# Remove the first breakpoint, should not get hit
adp.send(RemoveDataBreakpointRequest, arg0=at(5)).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(6), arg2=StoppedReasonBreakpoint)

# Remove the breakpoints we set at the start, should not get hit
adp.send(RemoveDataBreakpointRequest, arg0=at(7)).expect_response()
adp.send(RemoveDataBreakpointRequest, arg0=at(8)).expect_response()

adp.send(ResumeRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
