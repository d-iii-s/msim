from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(0xff).expect_response(StatusUnsupportedRequestError, arg0=0xff)
adp.send(0xf0).expect_response(StatusUnsupportedRequestError, arg0=0xf0)
adp.send(0xff).expect_response(StatusUnsupportedRequestError, arg0=0xff)

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
