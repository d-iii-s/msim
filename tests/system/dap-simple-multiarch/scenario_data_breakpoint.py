from base import *

adp = Adapter(int(sys.argv[1]))

READ = 0x01
SIZE = INSTR_LEN

adp.send(SetDataBreakpointRequest, arg0=at(3), arg1=READ, arg2=SIZE).expect_response()
adp.send(ResumeRequest).expect_response()
# resumed both, as both fetch the same instruction, should get two events (two hits)
# both with default CPU, as data breakpoints can't distinguish which CPU caused the breakpoint
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(3), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(3), arg2=StoppedReasonBreakpoint)

adp.send(SetDataBreakpointRequest, arg0=at(5), arg1=READ, arg2=SIZE).expect_response()
adp.send(SetDataBreakpointRequest, arg0=at(6), arg1=READ, arg2=SIZE).expect_response()

adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(5), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(5), arg2=StoppedReasonBreakpoint)

adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(6), arg2=StoppedReasonBreakpoint)
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(6), arg2=StoppedReasonBreakpoint)

adp.send(ResumeRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
