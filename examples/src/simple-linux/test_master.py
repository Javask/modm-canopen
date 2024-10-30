import canopen

network = canopen.Network()

node = canopen.RemoteNode(5, "test.eds")
network.add_node(node)
network.connect(bustype="socketcan", channel="vcan0")
# Send periodic sync
# network.sync.start(0.01)

# Wait for bootup
# print("Waiting for bootup...")
# try:
#    node.nmt.wait_for_bootup()
# except:
#    print("Timed out waiting for boot!")
#    exit()


network.nmt.state = "PRE-OPERATIONAL"

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

# Send a heartbeat
# heartbeat = node.sdo[0x1017]
# heartbeat.raw = 1000

network.nmt.state = "OPERATIONAL"

# Wait for heartbeat and verify state
# node.nmt.wait_for_heartbeat()
# assert node.nmt.state == 'OPERATIONAL'

node.nmt.start_node_guarding(0.25)

while True:
    if node.tpdo[1].wait_for_reception() is not None:
        print("TPDO1 received")
        print("\tTest value 1:", node.tpdo[1]["Test 1"].raw)
        print("\tTest value 2:", node.tpdo[1]["Test 2"].raw)
    else:
        print("Timed out")
