#ifndef CLICK_STATEMACHINE_HH
#define CLICK_STATEMACHINE_HH
#include <click/element.hh>
#include <vector>
CLICK_DECLS

enum State {
	NEW,
	ESTABLISHED,
	RELATED,
	INVALID
};

enum Protocol {
	ICMP = 1,
	TCP = 6,
	UDP = 17,
};

struct StateEntry {
	Protocol protocol;
	in_addr srcIP;
	in_addr dstIP;
	uint8_t srcPort;	//TCP-UDP
	uint8_t dstPort;	//TCP-UDP
	uint8_t type;	//ICMP
	uint8_t code;	//ICMP
	bool gotReply;
};

class StateMachine : public Element { public:

	/**
	 * Constructor
	 */
	StateMachine();

	/**
	 * Destructor
	 */
    ~StateMachine();

    /**
     * Return the element's class name.
     * @return the class name
     */
    const char *class_name() const		{ return "StateMachine"; }

    /**
     * Returns the element's port count specifier.
     * Click extracts port count specifiers from the source for use by tools.
     * For Click to find a port count specifier, the function definition must appear inline,
     * on a single line, inside the element class's declaration, and must return a C string constant.
     * It should also have public accessibility.
     * @return C string constant as an indication for the number of input and output ports of this element
     */
    const char *port_count() const		{ return "1/1"; }

    /**
     * Return the element's processing specifier.
     * @return C string constant as an indication for the processing mode.
     */
    const char *processing() const		{ return PUSH; }

    /**
     * Parse the element's configuration arguments.
     * @param conf The configuration arguments
     * @param errh The error handler
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);

    /**
     * Push packet p from an certain input port to an push input port.
     */
    void push(int, Packet *);

private:
	uint8_t _anno;
	std::vector<StateEntry> _stateEntryList;

};

CLICK_ENDDECLS
#endif // CLICK_STATEMACHINE_HH
