from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(SetCodeBreakpointRequest, arg0=at(3)).expect_response()
adp.send(ResumeRequest).expect_response()
# resumed both, as both are at the same PC, should get two events (two hits)
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(3), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(3), arg2=StoppedReasonBreakpoint)

adp.send(SetCodeBreakpointRequest, arg0=at(5)).expect_response()
adp.send(SetCodeBreakpointRequest, arg0=at(6)).expect_response()

adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(5), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(5), arg2=StoppedReasonBreakpoint)

adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(6), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(6), arg2=StoppedReasonBreakpoint)

adp.send(ResumeRequest).expect_response().expect_event(TerminatedEvent)
adp.close()