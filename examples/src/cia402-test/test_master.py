import canopen
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
node.tpdo.read()
# Re-map TxPDO1
node.tpdo[1].clear()
node.tpdo[1].add_variable("Test 1")
node.tpdo[1].add_variable("Test 2")
node.tpdo[1].trans_type = 0xFF
node.tpdo[1].event_timer = 500
node.tpdo[1].enabled = True

# Save new PDO configuration to node
node.tpdo.save()
node.emcy.add_callback(on_emcy)


# Send a heartbeat
heartbeat = node.sdo[0x1017]
heartbeat.raw = 1000

network.nmt.state = "OPERATIONAL"

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

while True:
    print(node.op_mode)
    if node.tpdo[1].wait_for_reception() is not None:
        print("TPDO1 received")
        print("\tTest value 1:", node.tpdo[1]["Test 1"].raw)
        print("\tTest value 2:", node.tpdo[1]["Test 2"].raw)
    else:
        print("Timed out")
