import socket, struct

REQUEST_FRAME_SIZE = 25
INBOUND_FRAME_SIZE = 26  # response / event frame size

# Request types
ResumeRequest = 0x01
PauseRequest = 0x02
TerminateRequest = 0x03
StepRequest = 0x04
SetCodeBreakpointRequest = 0x05
RemoveCodeBreakpointRequest = 0x06
SetDataBreakpointRequest = 0x07
RemoveDataBreakpointRequest = 0x08
ReadGeneralRegisterRequest = 0x09
WriteGeneralRegisterRequest = 0x0A
ReadCsrRequest = 0x0B
WriteCsrRequest = 0x0C
ReadPCRequest = 0x0D
WritePCRequest = 0x0E
ReadPhysMemoryRequest = 0x0F
WritePhysMemoryRequest = 0x10
ReadVirtMemoryRequest = 0x11
WriteVirtMemoryRequest = 0x12
TranslateAddressRequest = 0x13
RaiseInterruptRequest = 0x14
ClearInterruptRequest = 0x15
GetConfigRequest = 0x16
GetCpuInfoRequest = 0x17

# Response Categories
ResponseCategory = 0x01
EventCategory = 0x02

# Response statuses
StatusOk = 0x01
StatusUnspecifiedError = 0x02
StatusUnsupportedRequestError = 0x03
StatusCpuNotFoundError = 0x04
StatusRegisterNotFoundError = 0x05
StatusBadAddressError = 0x06

# Event types
TerminatedEvent = 0x01
StoppedAtEvent = 0x02

# Stopped reasons
StoppedReasonPaused = 0x01
StoppedReasonBreakpoint = 0x02
StoppedReasonStep = 0x03
StoppedReasonInterrupt = 0x04

class Adapter:
    def __init__(self, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(.5)
        self.sock.connect(("127.0.0.1", port))

    def send(self, req_type, arg0=0x00, arg1=0x00, arg2=0x00):
        packed = struct.pack('>B3Q', req_type, arg0, arg1, arg2)
        self.sock.sendall(packed)
        return self

    def receive(self):
        data = self._recv_exact(INBOUND_FRAME_SIZE)
        return struct.unpack('>BB3Q', data)

    def _recv_exact(self, num_bytes):
        data = b''
        while len(data) < num_bytes:
            chunk = self.sock.recv(num_bytes - len(data))
            if not chunk:
                raise EOFError("Connection to MSIM closed")
            data += chunk
        return data

    def close(self):
        self.sock.close()

    def expect_response(self, status=StatusOk, arg0=0x00, arg1=0x00, arg2=0x00):
        category, kind, a0, a1, a2 = self.receive()
        assert category == ResponseCategory, f"Got category {hex(category)}, but expected {hex(ResponseCategory)}"
        assert kind == status, f"Got response status {hex(kind)}, but expected {hex(status)}"
        assert a0 == arg0, f"Got response arg0 {hex(a0)}, but expected {hex(arg0)}"
        assert a1 == arg1, f"Got response arg1 {hex(a1)}, but expected {hex(arg1)}"
        assert a2 == arg2, f"Got response arg2 {hex(a2)}, but expected {hex(arg2)}"
        return self

    def expect_event(self, event_type, arg0=0x00, arg1=0x00, arg2=0x00):
        category, kind, a0, a1, a2 = self.receive()
        assert category == EventCategory, f"Got category {hex(category)}, but expected {hex(EventCategory)}"
        assert kind == event_type, f"Got event type {hex(kind)}, but expected {hex(event_type)}"
        assert a0 == arg0, f"Got event arg0 {hex(a0)}, but expected {hex(arg0)}"
        assert a1 == arg1, f"Got event arg1 {hex(a1)}, but expected {hex(arg1)}"
        assert a2 == arg2, f"Got event arg2 {hex(a2)}, but expected {hex(arg2)}"
        return self

def make_at(reset_vector, instr_len):
    """ Compute the address of the nth instruction in the test program """
    def at(instruction_number):
        return reset_vector + instruction_number * instr_len
    return at
