from base import *

adp = Adapter(int(sys.argv[1]))

# Step with count 0 should clear the stepping state, but not resume execution
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=0).expect_response()
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=0).expect_response()

# Step through the program one instruction at a time and check that we stop at the right place
for i in range(1, PROGRAM_LEN + 1):
    adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=1).expect_response()
    adp.send(ResumeRequest).expect_response()
    adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(i), arg2=StoppedReasonStep)

adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=0).expect_response()

# Resume to consume the halt instruction and terminate
adp.send(ResumeRequest).expect_response(StatusOk)
adp.expect_event(TerminatedEvent)
adp.close()