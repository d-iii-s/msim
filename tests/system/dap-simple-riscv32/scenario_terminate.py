from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
