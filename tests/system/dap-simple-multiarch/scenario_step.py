from base import *

adp = Adapter(int(sys.argv[1]))

# Step with count 0 should clear the stepping state, but not resume execution
adp.send(StepRequest, arg0=CPU0, arg1=0).expect_response()
adp.send(StepRequest, arg0=CPU1, arg1=0).expect_response()

# Step CPU0 by 1 instruction and CPU1 by 2 instructions, then resume both
for i in range(1, PROGRAM_LEN + 1, 2):
    adp.send(StepRequest, arg0=CPU0, arg1=1).expect_response()
    adp.send(StepRequest, arg0=CPU1, arg1=2).expect_response()
    adp.send(ResumeRequest).expect_response(StatusOk)
    adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(i), arg2=StoppedReasonStep)
    adp.send(ResumeRequest).expect_response(StatusOk)
    adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(i + 1), arg2=StoppedReasonStep)

adp.send(StepRequest, arg0=CPU0, arg1=0).expect_response()
adp.send(StepRequest, arg0=CPU1, arg1=0).expect_response()

# Resume to consume the halt instruction and terminate
adp.send(ResumeRequest).expect_response(StatusOk)
adp.expect_event(TerminatedEvent)
adp.close()