from base import *

adp = Adapter(int(sys.argv[1]))

# Set breakpoints at the end, we will remove later
adp.send(SetCodeBreakpointRequest, arg0=at(7)).expect_response()
adp.send(SetCodeBreakpointRequest, arg0=at(8)).expect_response()

# Remove a non-existent breakpoint
adp.send(RemoveCodeBreakpointRequest, arg0=at(6)).expect_response()

adp.send(SetCodeBreakpointRequest, arg0=at(3)).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(3), arg2=StoppedReasonBreakpoint)

adp.send(SetCodeBreakpointRequest, arg0=at(0)).expect_response()  # BP should not be hit

adp.send(SetCodeBreakpointRequest, arg0=at(5)).expect_response()
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(5), arg2=StoppedReasonBreakpoint)

# Remove the breakpoints we set at the start, should not get hit
adp.send(RemoveCodeBreakpointRequest, arg0=at(7)).expect_response()
adp.send(RemoveCodeBreakpointRequest, arg0=at(8)).expect_response()

adp.send(ResumeRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
