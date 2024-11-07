import canopen
import time
from canopen.profiles.p402 import BaseNode402

node = BaseNode402(5, 'test.eds')
network = canopen.Network()
network.add_node(node)

network.connect(bustype="socketcan", channel="vcan0")
# Send periodic sync
network.sync.start(10)

# Wait for bootup
# print("Waiting for bootup...")
# try:
#    node.nmt.wait_for_bootup()
# except:
#    print("Timed out waiting for boot!")
#    exit()


network.nmt.state = "PRE-OPERATIONAL"

def on_emcy(err):
    print("EMCY Received: {}".format(err))
node.emcy.add_callback(on_emcy)



# Send a heartbeat
heartbeat = node.sdo[0x1017]
heartbeat.raw = 1000

network.nmt.state = "OPERATIONAL"

node.tpdo.read()
node.tpdo[1].clear()
node.tpdo[1].add_variable("Status Word")
node.tpdo[1].add_variable("Mode of Operation Display")
node.tpdo[1].trans_type = 0xFF
node.tpdo[1].event_timer = 100
node.tpdo[1].enabled = True
node.tpdo.save()


node.rpdo.read()
node.rpdo[1].clear()
node.rpdo[1].add_variable("Control Word")
node.rpdo[1].add_variable("Mode of Operation")
node.rpdo[1].trans_type = 0xFF
node.rpdo[1].enabled = True
node.rpdo.save()

# Wait for heartbeat and verify state
node.nmt.wait_for_heartbeat()
node.nmt.wait_for_heartbeat()
assert node.nmt.state == 'OPERATIONAL'


node.setup_402_state_machine()
print("---------------------")
print(node.is_op_mode_supported("CYCLIC SYNCHRONOUS POSITION"))

node.state = "OPERATION ENABLED"
#node.nmt.start_node_guarding(0.25)
node.op_mode = "PROFILED POSITION"

time.sleep(1)

control = node.sdo[0x6040]
control.raw |= 0x0100

time.sleep(5)

node.state = "SWITCHED ON"

time.sleep(1)

node.state = "OPERATION ENABLED"
