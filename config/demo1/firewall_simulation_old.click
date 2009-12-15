//Please don't alter the content of this generated file by FireSim
//authors: Nico Van Looy & Jens De Wit - 2009

//Translation of iptables to click syntax
Idle -> MANGLE_FORWARD1 :: IPClassifier(-);
Idle -> MANGLE_INPUT1 :: IPClassifier(-);
Idle -> MANGLE_OUTPUT1 :: IPClassifier(-);
Idle -> MANGLE_POSTROUTING1 :: IPClassifier(-);
Idle -> MANGLE_PREROUTING1 :: IPClassifier(-);
Idle -> NAT_OUTPUT1 :: IPClassifier(-);
Idle -> NAT_POSTROUTING1 :: IPClassifier(-);
Idle -> NAT_PREROUTING1 :: IPClassifier(-);
Idle -> FILTER_FORWARD1 :: IPClassifier(-);
Idle -> FILTER_INPUT1 :: IPClassifier(ip proto 1 , -);
Idle -> FILTER_INPUT1B :: Classifier(34/0303, -);
Idle -> FILTER_INPUT2 :: IPClassifier(ip proto 6 src host 1.2.3.4, -);
Idle -> FILTER_INPUT3 :: IPClassifier(ip proto 17 dst host 10.0.0.1 dst port 80, -);
Idle -> FILTER_INPUT4 :: IPClassifier(-);
Idle -> FILTER_OUTPUT1 :: IPClassifier(-);
Idle -> FILTER_net67_in1 :: IPClassifier(dst host 5.6.7.8, -);
Idle -> FILTER_net67_in2 :: IPClassifier(dst host 10.0.0.1, -);
Idle -> FILTER_net67_in3 :: IPClassifier(-);
Idle -> FILTER_net70_in1 :: IPClassifier(dst port 54, -);
Idle -> FILTER_net70_in2 :: IPClassifier(-);

//Painters for statistics
Idle -> ACCEPT_CHECK :: CheckPaint(COLOR 0);
Idle -> REJECT :: CheckPaint(COLOR 1);
Idle -> DROP :: CheckPaint(COLOR 2);

//Link tables
ACCEPT :: TableLinker(ACCEPT ACCEPT_CHECK, MANGLE_PREROUTING MANGLE_PREROUTING1, MANGLE_INPUT MANGLE_INPUT1, MANGLE_FORWARD MANGLE_FORWARD1, MANGLE_OUTPUT MANGLE_OUTPUT1, MANGLE_POSTROUTING MANGLE_POSTROUTING1, NAT_PREROUTING NAT_PREROUTING1, NAT_POSTROUTING NAT_POSTROUTING1, NAT_OUTPUT NAT_OUTPUT1, FILTER_INPUT FILTER_INPUT1, FILTER_FORWARD FILTER_FORWARD1, FILTER_OUTPUT FILTER_OUTPUT1);

//Intern - input - forward - output iptables switch
IPTABLES :: IPClassifier(
	(dst 143.129.67.80/32 or 5.6.7.8/32 ) and (src 143.129.67.80/32 or 5.6.7.8/32), 
	src 143.129.67.80/32 or 5.6.7.8/32, 
	dst 143.129.67.80/32 or 5.6.7.8/32, 
	-);
IPTABLES[0] -> Script(TYPE PACKET, write ACCEPT.chain "INTERN") -> MANGLE_OUTPUT1; //Internal traffic
IPTABLES[1] -> Script(TYPE PACKET, write ACCEPT.chain "OUTPUT") -> MANGLE_OUTPUT1; //Output Chain
IPTABLES[2] -> Script(TYPE PACKET, write ACCEPT.chain "INPUT") -> MANGLE_PREROUTING1; //Input Chain
IPTABLES[3] -> Script(TYPE PACKET, write ACCEPT.chain "FORWARD") -> MANGLE_PREROUTING1; //Forward Chain

//Counters for statistics
inputCounter :: Counter -> IPTABLES;
ACCEPT_true :: Counter;
ACCEPT_false :: Counter;
REJECT_true :: Counter;
REJECT_false :: Counter;
DROP_true :: Counter;
DROP_false :: Counter;

//Traffic copy
copy :: Tee;

//Traffic input queue
queue :: SimpleQueue
	-> unqueue :: PokeUnqueue
	-> copy;

//Faulty accept output queue
copy[0] -> faulty_accept_queue :: SimpleQueue
	-> faulty_accept :: PokeUnqueue
	-> ToDump("output/faulty_accept.dump");

//Faulty reject output queue
copy[1] -> faulty_reject_queue :: SimpleQueue
	-> faulty_reject :: PokeUnqueue
	-> ToDump("output/faulty_reject.dump");

//Faulty drop output queue
copy[2] -> faulty_drop_queue :: SimpleQueue
	-> faulty_drop :: PokeUnqueue
	-> ToDump("output/faulty_drop.dump");

//Do simulation with input traffic
copy[3] -> Script(TYPE PACKET, write bt.reset)
	-> MarkIPHeader(14)
	-> inputCounter;

//Clear the output queues and get the next input packet going
	next :: Script(TYPE PACKET, write faulty_accept_queue.reset)
	-> Script(TYPE PACKET, write faulty_reject_queue.reset)
	-> Script(TYPE PACKET, write faulty_drop_queue.reset)
	-> Script(TYPE PACKET, write unqueue.poke) -> Discard;

//Feedback
ACCEPT_CHECK[0] -> ACCEPT_true -> next
ACCEPT_CHECK[1] -> ACCEPT_false -> Script(TYPE PACKET, write faulty_accept.poke) -> next
REJECT[0] -> REJECT_true -> next
REJECT[1] -> REJECT_false -> Script(TYPE PACKET, write faulty_reject.poke) -> next
DROP[0] -> DROP_true -> next
DROP[1] -> DROP_false -> Script(TYPE PACKET, write faulty_drop.poke) -> next

Idle -> MANGLE_INPUT1;
Idle -> MANGLE_FORWARD1;
Idle -> MANGLE_POSTROUTING1;
Idle -> NAT_OUTPUT1;
Idle -> NAT_PREROUTING1;
Idle -> NAT_POSTROUTING1;
Idle -> FILTER_FORWARD1;
Idle -> FILTER_INPUT1;
Idle -> FILTER_OUTPUT1;
Idle -> ACCEPT_CHECK;

//Backtracker declaration
Idle -> bt :: Backtracker;

//Masquerade handler
Idle -> masq :: Classifier(0/000e0ca989ab, 0/0050ba2b9a50);
masq[0] -> IPAddrPairRewriter(pattern 5.6.7.8 - 0 0) -> bt;
masq[1] -> IPAddrPairRewriter(pattern 143.129.67.80 - 0 0) -> bt;

//Traffic Block
Script(write tcp1.send 1.2.3.4 30 5.6.7.8 80 0 0 2);
tcp1 :: TCPIPSend
	-> EtherEncap(0x0800, 00:50:ba:2b:9a:50, 00:0e:0c:a9:89:ab)
	-> Paint(COLOR 0)
	-> queue;

//Poke unqueue to start automatically
Script(write unqueue.poke);

//Simulation of the iptables
MANGLE_FORWARD1[0] ->  Script(TYPE PACKET, write ACCEPT.from MANGLE_FORWARD1) -> ACCEPT; //Default policy
MANGLE_INPUT1[0] ->  Script(TYPE PACKET, write ACCEPT.from MANGLE_INPUT1) -> ACCEPT; //Default policy
MANGLE_OUTPUT1[0] ->  Script(TYPE PACKET, write ACCEPT.from MANGLE_OUTPUT1) -> ACCEPT; //Default policy
MANGLE_POSTROUTING1[0] ->  Script(TYPE PACKET, write ACCEPT.from MANGLE_POSTROUTING1) -> ACCEPT; //Default policy
MANGLE_PREROUTING1[0] ->  Script(TYPE PACKET, write ACCEPT.from MANGLE_PREROUTING1) -> ACCEPT; //Default policy
NAT_OUTPUT1[0] ->  Script(TYPE PACKET, write ACCEPT.from NAT_OUTPUT1) -> ACCEPT; //Default policy
NAT_POSTROUTING1[0] ->  Script(TYPE PACKET, write ACCEPT.from NAT_POSTROUTING1) -> ACCEPT; //Default policy
NAT_PREROUTING1[0] ->  Script(TYPE PACKET, write ACCEPT.from NAT_PREROUTING1) -> ACCEPT; //Default policy
FILTER_FORWARD1[0] ->  Script(TYPE PACKET, write ACCEPT.from FILTER_FORWARD1) -> DROP; //Default policy
FILTER_INPUT1[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT1) -> FILTER_INPUT1B;
FILTER_INPUT1[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT1) -> FILTER_INPUT2;
FILTER_INPUT1B[0] -> Script(TYPE PACKET, write bt.trace FILTER_INPUT2) -> FILTER_net70_in1;
FILTER_INPUT1B[1] -> FILTER_INPUT2;
FILTER_INPUT2[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT2) -> Script(TYPE PACKET, write bt.trace FILTER_INPUT3) -> FILTER_net67_in1;
FILTER_INPUT2[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT2) -> FILTER_INPUT3;
FILTER_INPUT3[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT3) -> ACCEPT;
FILTER_INPUT3[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT3) -> FILTER_INPUT4;
FILTER_INPUT4[0]  ->  Script(TYPE PACKET, write ACCEPT.from FILTER_INPUT4) -> DROP; //Default policy
FILTER_OUTPUT1[0] ->  Script(TYPE PACKET, write ACCEPT.from FILTER_OUTPUT1) -> ACCEPT; //Default policy
FILTER_net67_in1[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net67_in1) -> Script(TYPE PACKET, write bt.trace FILTER_net67_in2) -> FILTER_net70_in1;
FILTER_net67_in1[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net67_in1) -> FILTER_net67_in2;
FILTER_net67_in2[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net67_in2) -> ACCEPT;
FILTER_net67_in2[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net67_in2) -> FILTER_net67_in3;
FILTER_net67_in3[0]  ->  Script(TYPE PACKET, write ACCEPT.from FILTER_net67_in3) -> bt;
FILTER_net70_in1[0] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net70_in1) -> REJECT;
FILTER_net70_in1[1] -> Script(TYPE PACKET, write ACCEPT.from FILTER_net70_in1) -> FILTER_net70_in2;
FILTER_net70_in2[0]  ->  Script(TYPE PACKET, write ACCEPT.from FILTER_net70_in2) -> bt;
