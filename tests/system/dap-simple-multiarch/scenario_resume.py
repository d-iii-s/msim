from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(0))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(0))
# Make sure its actually stopped and not running yet
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(0))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(0))
adp.send(PauseRequest).expect_response(StatusOk)
# We don't expect a StoppedAtEvent here, because the CPU is already stopped and shouldn't generate a new event
adp.send(ReadPCRequest).expect_response(StatusOk, arg0=at(0))

# Should resume both CPUs
adp.send(ResumeRequest).expect_response(StatusOk)

adp.expect_event(TerminatedEvent)
adp.close()